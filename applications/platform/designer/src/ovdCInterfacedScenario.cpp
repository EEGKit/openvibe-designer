#include <boost/filesystem.hpp>
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"
#include "ovdCBoxProxy.h"
#include "ovdCCommentProxy.h"
#include "ovdCLinkProxy.h"
#include "ovdCConnectorEditor.h"
#include "ovdCInterfacedObject.h"
#include "ovdTAttributeHandler.h"
#include "ovdCDesignerVisualization.h"
#include "ovdCPlayerVisualization.h"
#include "ovdCRenameDialog.h"
#include "ovdCAboutPluginDialog.h"
#include "ovdCAboutScenarioDialog.h"
#include "ovdCSettingEditorDialog.h"
#include "ovdCCommentEditorDialog.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <algorithm>

#include <gdk/gdkkeysyms.h>
#include <fs/Files.h>

#include <cstdlib>

#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
#include <strings.h>
#define _strcmpi strcasecmp
#endif

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace OpenViBEVisualizationToolkit;
using namespace std;

extern map<size_t, GdkColor> gColors;

static GtkTargetEntry targets[] = { { static_cast<gchar*>("STRING"), 0, 0 }, { static_cast<gchar*>("text/plain"), 0, 0 } };

static GdkColor colorFromIdentifier(const CIdentifier& id, const bool isDeprecated = false)
{
	GdkColor color;
	uint32_t value1 = 0;
	uint32_t value2 = 0;
	uint64_t res    = 0;

	sscanf(id.toString(), "(0x%08X, 0x%08X)", &value1, &value2);
	res += value1;
	res <<= 32;
	res += value2;

	color.pixel = guint16(0);
	color.red   = guint16((res & 0xffff) | 0x8000);
	color.green = guint16(((res >> 16) & 0xffff) | 0x8000);
	color.blue  = guint16(((res >> 32) & 0xffff) | 0x8000);

	if (isDeprecated)
	{
		color.blue  = 2 * color.blue / 3;
		color.red   = 2 * color.red / 3;
		color.green = 2 * color.green / 3;
	}

	return color;
}

static std::string getBoxAlgorithmURL(const std::string& in, const bool removeSlash = false)
{
	std::string tmp(in);
	std::string out;
	bool lastWasSeparator = true;

	for (char c : tmp)
	{
		if (std::isalnum(c) || (!removeSlash && c == '/'))
		{
			if (c == '/') { out += "_"; }
			else
			{
				if (lastWasSeparator) { out += std::to_string(std::toupper(c)); }
				else { out += c; }
			}
			lastWasSeparator = false;
		}
		else
		{
			// if(!lastWasSeparator) { out += "_"; }
			lastWasSeparator = true;
		}
	}
	return out;
}

static void count_widget_cb(GtkWidget* /*widget*/, gpointer data)
{
	int* i = reinterpret_cast<int*>(data);
	if (i) { (*i)++; }
}

static int gtk_container_get_children_count(GtkContainer* container)
{
	int res = 0;
	gtk_container_foreach(container, count_widget_cb, &res);
	return res;
}

static gboolean scenario_scrolledwindow_scroll_event_cb(GtkWidget* /*widget*/, GdkEventScroll* event)
{
	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	/* Shift+Wheel scrolls the in the perpendicular direction */
	if (state & GDK_SHIFT_MASK)
	{
		if (event->direction == GDK_SCROLL_UP) { event->direction = GDK_SCROLL_LEFT; }
		else if (event->direction == GDK_SCROLL_LEFT) { event->direction = GDK_SCROLL_UP; }
		else if (event->direction == GDK_SCROLL_DOWN) { event->direction = GDK_SCROLL_RIGHT; }
		else if (event->direction == GDK_SCROLL_RIGHT) { event->direction = GDK_SCROLL_DOWN; }

		event->state &= ~GDK_SHIFT_MASK;
		state &= ~GDK_SHIFT_MASK;
	}

	return FALSE;
}

static void scenario_drawing_area_expose_cb(GtkWidget* /*widget*/, GdkEventExpose* event, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaExposeCB(event);
}

static void scenario_drawing_area_drag_data_received_cb(GtkWidget* /*widget*/, GdkDragContext* dc, const gint x, const gint y, GtkSelectionData* selectionData,
														const guint info, const guint t, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaDragDataReceivedCB(dc, x, y, selectionData, info, t);
}

static gboolean scenario_drawing_area_motion_notify_cb(GtkWidget* widget, GdkEventMotion* event, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaMotionNotifyCB(widget, event);
	return FALSE;
}

static void scenario_drawing_area_button_pressed_cb(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaButtonPressedCB(widget, event);
}

static void scenario_drawing_area_button_released_cb(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaButtonReleasedCB(widget, event);
}

static void scenario_drawing_area_key_press_event_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaKeyPressEventCB(widget, event);
}

static void scenario_drawing_area_key_release_event_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaKeyReleaseEventCB(widget, event);
}

static void context_menu_cb(GtkMenuItem* /*item*/, CInterfacedScenario::box_ctx_menu_cb_t* cb)
{
	//CInterfacedScenario::box_ctx_menu_cb_t* pContextMenuCB=static_cast < CInterfacedScenario::box_ctx_menu_cb_t* >(data);
	switch (cb->command)
	{
		case ContextMenu_SelectionCopy: cb->scenario->copySelection();
			break;
		case ContextMenu_SelectionCut: cb->scenario->cutSelection();
			break;
		case ContextMenu_SelectionPaste: cb->scenario->pasteSelection();
			break;
		case ContextMenu_SelectionDelete: cb->scenario->deleteSelection();
			break;

		case ContextMenu_BoxRename: cb->scenario->contextMenuBoxRenameCB(*cb->box);
			break;
		case ContextMenu_BoxUpdate:
		{
			cb->scenario->snapshotCB();
			cb->scenario->contextMenuBoxUpdateCB(*cb->box);
			cb->scenario->redraw();
			break;
		}
		case ContextMenu_BoxRemoveDeprecatedInterfacors:
		{
			cb->scenario->contextMenuBoxRemoveDeprecatedInterfacorsCB(*cb->box);
			cb->scenario->redraw();
			break;
		}
			//case ContextMenu_BoxRename: cb->pInterfacedScenario->contextMenuBoxRenameAllCB(); break;
		case ContextMenu_BoxDelete:
		{
			// If selection is empty delete the box under cursor
			if (cb->scenario->m_SelectedObjects.empty())
			{
				cb->scenario->deleteBox(cb->box->getIdentifier());
				cb->scenario->redraw();
				cb->scenario->snapshotCB();
			}
			else { cb->scenario->deleteSelection(); }
			break;
		}
		case ContextMenu_BoxAddInput: cb->scenario->contextMenuBoxAddInputCB(*cb->box);
			break;
		case ContextMenu_BoxEditInput: cb->scenario->contextMenuBoxEditInputCB(*cb->box, cb->index);
			break;
		case ContextMenu_BoxRemoveInput: cb->scenario->contextMenuBoxRemoveInputCB(*cb->box, cb->index);
			break;
		case ContextMenu_BoxAddOutput: cb->scenario->contextMenuBoxAddOutputCB(*cb->box);
			break;
		case ContextMenu_BoxEditOutput: cb->scenario->contextMenuBoxEditOutputCB(*cb->box, cb->index);
			break;
		case ContextMenu_BoxRemoveOutput: cb->scenario->contextMenuBoxRemoveOutputCB(*cb->box, cb->index);
			break;

		case ContextMenu_BoxConnectScenarioInput: cb->scenario->contextMenuBoxConnectScenarioInputCB(
				*cb->box, cb->index, cb->secondaryIndex);
			break;
		case ContextMenu_BoxConnectScenarioOutput: cb->scenario->contextMenuBoxConnectScenarioOutputCB(
				*cb->box, cb->index, cb->secondaryIndex);
			break;

		case ContextMenu_BoxDisconnectScenarioInput: cb->scenario->contextMenuBoxDisconnectScenarioInputCB(
				*cb->box, cb->index, cb->secondaryIndex);
			break;
		case ContextMenu_BoxDisconnectScenarioOutput: cb->scenario->contextMenuBoxDisconnectScenarioOutputCB(
				*cb->box, cb->index, cb->secondaryIndex);
			break;

		case ContextMenu_BoxAddSetting: cb->scenario->contextMenuBoxAddSettingCB(*cb->box);
			break;
		case ContextMenu_BoxEditSetting: cb->scenario->contextMenuBoxEditSettingCB(*cb->box, cb->index);
			break;
		case ContextMenu_BoxRemoveSetting: cb->scenario->contextMenuBoxRemoveSettingCB(*cb->box, cb->index);
			break;
		case ContextMenu_BoxConfigure: cb->scenario->contextMenuBoxConfigureCB(*cb->box);
			break;
		case ContextMenu_BoxAbout: cb->scenario->contextMenuBoxAboutCB(*cb->box);
			break;
		case ContextMenu_BoxEnable:
		{
			if (cb->scenario->m_SelectedObjects.empty()) { cb->scenario->contextMenuBoxEnableCB(*cb->box); }
			else { cb->scenario->contextMenuBoxEnableAllCB(); }
			break;
		}
		case ContextMenu_BoxDisable:
		{
			if (cb->scenario->m_SelectedObjects.empty())
			{
				cb->scenario->contextMenuBoxDisableCB(*cb->box);
				break;
			}
			cb->scenario->contextMenuBoxDisableAllCB();
			break;
		}
		case ContextMenu_BoxDocumentation: cb->scenario->contextMenuBoxDocumentationCB(*cb->box);
			break;

		case ContextMenu_BoxEditMetabox: cb->scenario->contextMenuBoxEditMetaboxCB(*cb->box);
			break;

		case ContextMenu_ScenarioAbout: cb->scenario->contextMenuScenarioAboutCB();
			break;
		case ContextMenu_ScenarioAddComment: cb->scenario->contextMenuScenarioAddCommentCB();
			break;
		default: break;
	}
	// Redraw in any case, as some of the actual callbacks can forget to redraw. As this callback is only called after the user has accessed
	// the right-click menu, so its not a large overhead to do it in general. @TODO might remove the individual redraws.
	cb->scenario->redraw();
}

static void gdk_draw_rounded_rectangle(GdkDrawable* drawable, GdkGC* drawGC, const gboolean fill, const gint x, const gint y, const gint width,
									   const gint height, const gint radius = 8)
{
	if (fill != 0)
	{
#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		gdk_draw_rectangle(drawable, drawGC, TRUE, x + radius, y, width - 2 * radius, height);
		gdk_draw_rectangle(drawable, drawGC, TRUE, x, y + radius, width, height - 2 * radius);
#elif defined TARGET_OS_Windows
		gdk_draw_rectangle(drawable, drawGC, TRUE, x + radius, y, width - 2 * radius + 1, height + 1);
		gdk_draw_rectangle(drawable, drawGC, TRUE, x, y + radius, width + 1, height - 2 * radius + 1);
#else
#pragma error("you should give a version of this function for your OS")
#endif
	}
	else
	{
		gdk_draw_line(drawable, drawGC, x + radius, y, x + width - radius, y);
		gdk_draw_line(drawable, drawGC, x + radius, y + height, x + width - radius, y + height);
		gdk_draw_line(drawable, drawGC, x, y + radius, x, y + height - radius);
		gdk_draw_line(drawable, drawGC, x + width, y + radius, x + width, y + height - radius);
	}
#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
	gdk_draw_arc(drawable, drawGC, fill, x + width - radius * 2, y, radius * 2, radius * 2, 0 * 64, 90 * 64);
	gdk_draw_arc(drawable, drawGC, fill, x, y, radius * 2, radius * 2, 90 * 64, 90 * 64);
	gdk_draw_arc(drawable, drawGC, fill, x, y + height - radius * 2, radius * 2, radius * 2, 180 * 64, 90 * 64);
	gdk_draw_arc(drawable, drawGC, fill, x + width - radius * 2, y + height - radius * 2, radius * 2, radius * 2, 270 * 64, 90 * 64);
#elif defined TARGET_OS_Windows
	gdk_draw_arc(drawable, drawGC, fill, x + width - radius * 2, y, radius * 2 + (fill != 0 ? 2 : 1), radius * 2 + (fill != 0 ? 2 : 1), 0 * 64, 90 * 64);
	gdk_draw_arc(drawable, drawGC, fill, x, y, radius * 2 + (fill != 0 ? 2 : 1), radius * 2 + (fill != 0 ? 2 : 1), 90 * 64, 90 * 64);
	gdk_draw_arc(drawable, drawGC, fill, x, y + height - radius * 2, radius * 2 + (fill != 0 ? 2 : 1), radius * 2 + (fill != 0 ? 2 : 1), 180 * 64, 90 * 64);
	gdk_draw_arc(drawable, drawGC, fill, x + width - radius * 2, y + height - radius * 2, radius * 2 + (fill != 0 ? 2 : 1), radius * 2 + (fill != 0 ? 2 : 1),
				 270 * 64, 90 * 64);
#else
#pragma error("you should give a version of this function for your OS")
#endif
}

static void scenario_title_button_close_cb(GtkButton* /*button*/, gpointer data)
{
	static_cast<CInterfacedScenario*>(data)->m_Application.closeScenarioCB(static_cast<CInterfacedScenario*>(data));
}

static gboolean editable_widget_focus_in_cb(GtkWidget* /*widget*/, GdkEvent* /*event*/, CApplication* app)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(app->m_Builder, "openvibe-menu_edit")), 0);
	return 0;
}

static gboolean editable_widget_focus_out_cb(GtkWidget* /*widget*/, GdkEvent* /*event*/, CApplication* app)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(app->m_Builder, "openvibe-menu_edit")), 1);

	return 0;
}

//static void scenario_configuration_add_setting_cb(GtkWidget*, CInterfacedScenario* pInterfacedScenario) { pInterfacedScenario->addScenarioSettingCB(); }

static void modify_scenario_setting_value_cb(GtkWidget* /*widget*/, CInterfacedScenario::setting_cb_data_t* data)
{
	CIdentifier typeID = OV_UndefinedIdentifier;
	data->scenario->m_Scenario.getSettingType(data->index, typeID);
	data->scenario->m_Scenario.setSettingValue(data->index, data->scenario->m_SettingHelper->getValue(typeID, data->widgetValue));
	data->scenario->m_HasBeenModified = true;
	data->scenario->updateScenarioLabel();
}

static void modify_scenario_setting_default_value_cb(GtkWidget* /*widget*/, CInterfacedScenario::setting_cb_data_t* data)
{
	CIdentifier typeID = OV_UndefinedIdentifier;
	data->scenario->m_Scenario.getSettingType(data->index, typeID);
	data->scenario->m_Scenario.setSettingDefaultValue(data->index, data->scenario->m_SettingHelper->getValue(typeID, data->widgetValue));

	// We also se the 'actual' value to this
	data->scenario->m_Scenario.setSettingValue(data->index, data->scenario->m_SettingHelper->getValue(typeID, data->widgetValue));
	data->scenario->m_HasBeenModified = true;
	data->scenario->updateScenarioLabel();
}

static void modify_scenario_setting_move_up_cb(GtkWidget* /*widget*/, CInterfacedScenario::setting_cb_data_t* data)
{
	if (data->index == 0) { return; }
	data->scenario->swapScenarioSettings(data->index - 1, data->index);
}

static void modify_scenario_setting_move_down_cb(GtkWidget* /*widget*/, CInterfacedScenario::setting_cb_data_t* data)
{
	if (data->index >= data->scenario->m_Scenario.getSettingCount() - 1) { return; }
	data->scenario->swapScenarioSettings(data->index, data->index + 1);
}

static void modify_scenario_setting_revert_to_default_cb(GtkWidget* /*widget*/, CInterfacedScenario::setting_cb_data_t* data)
{
	CString value;
	data->scenario->m_Scenario.getSettingDefaultValue(data->index, value);
	data->scenario->m_Scenario.setSettingValue(data->index, value);
	data->scenario->redrawScenarioSettings();
}

static void copy_scenario_setting_token_cb(GtkWidget* /*widget*/, CInterfacedScenario::setting_cb_data_t* data)
{
	CString name;
	data->scenario->m_Scenario.getSettingName(data->index, name);
	name = CString("$var{") + name + CString("}");

	GtkClipboard* defaultClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(defaultClipboard, name.toASCIIString(), -1);

	// On X11 there is another clipboard that it is useful to set as well
	GtkClipboard* x11Clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
	gtk_clipboard_set_text(x11Clipboard, name.toASCIIString(), -1);
}

static void modify_scenario_setting_type_cb(GtkWidget* combobox, CInterfacedScenario::setting_cb_data_t* data)
{
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_string(builder, data->scenario->m_SerializedSettingGUIXML.c_str(), data->scenario->m_SerializedSettingGUIXML.length(), nullptr);

	gtk_widget_destroy(data->widgetValue);

	const CIdentifier typeID = data->scenario->m_SettingTypes[gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox))];
	data->scenario->m_Scenario.setSettingType(data->index, typeID);

	const CString name = data->scenario->m_SettingHelper->getSettingWidgetName(typeID);

	GtkWidget* value = GTK_WIDGET(gtk_builder_get_object(builder, name.toASCIIString()));

	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(value)), value);
	gtk_table_attach_defaults(GTK_TABLE(data->container), value, 1, 5, 1, 2);

	// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
	CString str;
	data->scenario->m_Scenario.getSettingDefaultValue(data->index, str);
	data->scenario->m_SettingHelper->setValue(typeID, value, str);

	// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
	const CString entryName = data->scenario->m_SettingHelper->getSettingEntryWidgetName(typeID);
	GtkWidget* entryValue   = GTK_WIDGET(gtk_builder_get_object(builder, entryName.toASCIIString()));

	data->widgetValue      = value;
	data->widgetEntryValue = entryValue;

	g_signal_connect(entryValue, "changed", G_CALLBACK(modify_scenario_setting_default_value_cb), data);

	g_object_unref(builder);
}

static void delete_scenario_setting_cb(GtkWidget* /*button*/, CInterfacedScenario::setting_cb_data_t* data)
{
	data->scenario->m_Scenario.removeSetting(data->index);
	data->scenario->redrawConfigureScenarioSettingsDialog();
}

static void modify_scenario_setting_name_cb(GtkWidget* entry, CInterfacedScenario::setting_cb_data_t* data)
{
	data->scenario->m_Scenario.setSettingName(data->index, gtk_entry_get_text(GTK_ENTRY(entry)));
}

static void reset_scenario_setting_identifier_cb(GtkWidget* /*button*/, CInterfacedScenario::setting_cb_data_t* data)
{
	const CIdentifier id = data->scenario->m_Scenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier);
	if (id != OV_UndefinedIdentifier)
	{
		data->scenario->m_Scenario.updateInterfacorIdentifier(EBoxInterfacorType::Setting, data->index, id);
		data->scenario->redrawConfigureScenarioSettingsDialog();
	}
}

static void modify_scenario_setting_identifier_cb(GtkWidget* entry, CInterfacedScenario::setting_cb_data_t* data)
{
	CIdentifier id;
	if (id.fromString(gtk_entry_get_text(GTK_ENTRY(entry))))
	{
		data->scenario->m_Scenario.updateInterfacorIdentifier(EBoxInterfacorType::Setting, data->index, id);
	}
}

static void edit_scenario_link_cb(GtkWidget* /*widget*/, CInterfacedScenario::link_cb_data_t* data)
{
	if (data->input) { data->scenario->editScenarioInputCB(data->index); }
	else { data->scenario->editScenarioOutputCB(data->index); }
	data->scenario->redraw();
}

static void modify_scenario_link_move_up_cb(GtkWidget* /*widget*/, CInterfacedScenario::link_cb_data_t* data)
{
	if (data->index == 0) { return; }
	if (data->input) { data->scenario->swapScenarioInputs(data->index - 1, data->index); }
	else { data->scenario->swapScenarioOutputs(data->index - 1, data->index); }

	data->scenario->snapshotCB();
}

static void modify_scenario_link_move_down_cb(GtkWidget* /*widget*/, CInterfacedScenario::link_cb_data_t* data)
{
	const auto interfacorType = data->input ? Input : Output;
	if (data->scenario->m_Scenario.getInterfacorCount(interfacorType) < 2
		|| data->index >= data->scenario->m_Scenario.getInterfacorCount(interfacorType) - 1) { return; }

	if (data->input) { data->scenario->swapScenarioInputs(data->index, data->index + 1); }
	else { data->scenario->swapScenarioOutputs(data->index, data->index + 1); }
	data->scenario->snapshotCB();
}

static void delete_scenario_link_cb(GtkButton* /*button*/, CInterfacedScenario::link_cb_data_t* data)
{
	if (data->input)
	{
		data->scenario->m_Scenario.removeScenarioInput(data->index);
		data->scenario->redrawScenarioInputSettings();
	}
	else
	{
		data->scenario->m_Scenario.removeScenarioOutput(data->index);
		data->scenario->redrawScenarioOutputSettings();
	}

	data->scenario->snapshotCB();
	data->scenario->redraw();
}

/*
static void modify_scenario_link_name_cb(GtkWidget* entry, CInterfacedScenario::link_cb_data_t* data)
{
	if (data->m_isInput) { data->m_interfacedScenario->m_scenario.setInputName(data->m_uiLinkIdx, gtk_entry_get_text(GTK_ENTRY(entry))); }
	else { data->m_interfacedScenario->m_scenario.setOutputName(data->m_uiLinkIdx, gtk_entry_get_text(GTK_ENTRY(entry))); }
}

static void modify_scenario_link_type_cb(GtkWidget* comboBox, CInterfacedScenario::link_cb_data_t* data)
{
	const CIdentifier typeID = data->m_interfacedScenario->m_mStreamType[gtk_combo_box_get_active_text(GTK_COMBO_BOX(comboBox))];
	if (data->m_isInput) { data->m_interfacedScenario->m_scenario.setInputType(data->m_uiLinkIdx, typeID); }
	else { data->m_interfacedScenario->m_scenario.setOutputType(data->m_uiLinkIdx, typeID); }
	data->m_interfacedScenario->redraw();
}
//*/

// Redraw Static Helper
static std::array<GdkPoint, 4> get4PointsInterfacorRedraw(const int size, const int shiftX, const int shiftY)
{
	std::array<GdkPoint, 4> points;
	points[0].x = size >> 1;
	points[0].y = size;
	points[1].x = 0;
	points[1].y = 0;
	points[2].x = size - 1;
	points[2].y = 0;
	for (int j = 0; j < 3; ++j)
	{
		points[j].x += shiftX;
		points[j].y += shiftY;
	}
	return points;
}

static void drawScenarioTextIOIndex(GtkWidget* widget, GdkGC* gcline, const size_t index, const gint xText, const gint yText, const gint xL1, const gint yL1,
									const gint xL2, const gint yL2)
{
	PangoContext* pangoCtx   = gtk_widget_get_pango_context(widget);
	PangoLayout* pangoLayout = pango_layout_new(pangoCtx);
	pango_layout_set_alignment(pangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(pangoLayout, std::to_string(index + 1).c_str(), -1);
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], xText, yText, pangoLayout);
	g_object_unref(pangoLayout);
	gdk_draw_line(widget->window, gcline, xL1, yL1, xL2, yL2);
}

static void drawBorderInterfacor(GtkWidget* widget, GdkGC* gc, GdkColor& color, std::array<GdkPoint, 4> points, const int border, const bool isDeprecated)
{
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_polygon(widget->window, gc, TRUE, points.data(), 3);
	if (isDeprecated) { gdk_gc_set_rgb_fg_color(gc, &gColors[Color_LinkInvalid]); }
	else { gdk_gc_set_rgb_fg_color(gc, &gColors[border]); }
	gdk_draw_polygon(widget->window, gc, FALSE, points.data(), 3);
}

static void drawCircleWithBorder(GtkWidget* widget, GdkGC* gc, GdkColor& bgColor, GdkColor& fgColor, const gint x, const gint y, const gint radius)
{
	gdk_gc_set_rgb_fg_color(gc, &bgColor);
	gdk_draw_arc(widget->window, gc, TRUE, x, y, radius, radius, 0, 64 * 360);
	gdk_gc_set_rgb_fg_color(gc, &fgColor);
	gdk_draw_arc(widget->window, gc, FALSE, x, y, radius, radius, 0, 64 * 360);
}

static void linkHandler(ILink* link, const int x, const int y, const CIdentifier& attX, const CIdentifier& attY)
{
	if (link)
	{
		TAttributeHandler handler(*link);

		if (!handler.hasAttribute(attX)) { handler.addAttribute<int>(attX, x); }
		else { handler.setAttributeValue<int>(attX, x); }

		if (!handler.hasAttribute(attY)) { handler.addAttribute<int>(attY, y); }
		else { handler.setAttributeValue<int>(attY, y); }
	}
}

CInterfacedScenario::CInterfacedScenario(const IKernelContext& ctx, CApplication& application, IScenario& scenario, CIdentifier& scenarioID,
										 GtkNotebook& notebook, const char* guiFilename, const char* guiSettingsFilename)
	: m_PlayerStatus(PlayerStatus_Stop), m_ScenarioID(scenarioID), m_Application(application), m_Scenario(scenario), m_kernelCtx(ctx), m_notebook(notebook),
	  m_guiFilename(guiFilename), m_guiSettingsFilename(guiSettingsFilename)
{
	m_guiBuilder = gtk_builder_new();
	gtk_builder_add_from_file(m_guiBuilder, m_guiFilename.c_str(), nullptr);
	gtk_builder_connect_signals(m_guiBuilder, nullptr);

	std::ifstream settingGUIFilestream;
	FS::Files::openIFStream(settingGUIFilestream, m_guiSettingsFilename.c_str());
	m_SerializedSettingGUIXML = std::string((std::istreambuf_iterator<char>(settingGUIFilestream)), std::istreambuf_iterator<char>());

	m_SettingHelper = new CSettingCollectionHelper(m_kernelCtx, m_guiSettingsFilename.c_str());

	// We will need to access setting types by their name later
	CIdentifier typeID;
	while ((typeID = m_kernelCtx.getTypeManager().getNextTypeIdentifier(typeID)) != OV_UndefinedIdentifier)
	{
		if (!m_kernelCtx.getTypeManager().isStream(typeID)) { m_SettingTypes[m_kernelCtx.getTypeManager().getTypeName(typeID).toASCIIString()] = typeID; }
		else { m_streamTypes[m_kernelCtx.getTypeManager().getTypeName(typeID).toASCIIString()] = typeID; }
	}

	m_notebookPageTitle   = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "openvibe_scenario_notebook_title"));
	m_notebookPageContent = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "openvibe_scenario_notebook_scrolledwindow"));

	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_guiBuilder, "openvibe-scenario_notebook")), 0);
	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_guiBuilder, "openvibe-scenario_notebook")), 0);
	gtk_notebook_append_page(&m_notebook, m_notebookPageContent, m_notebookPageTitle);
	gtk_notebook_set_tab_reorderable(&m_notebook, m_notebookPageContent, 1);

	GtkWidget* closeWidget = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "openvibe-scenario_button_close"));
	g_signal_connect(G_OBJECT(closeWidget), "clicked", G_CALLBACK(scenario_title_button_close_cb), this);

	m_scenarioDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_guiBuilder, "openvibe-scenario_drawing_area"));
	m_scenarioViewport    = GTK_VIEWPORT(gtk_builder_get_object(m_guiBuilder, "openvibe-scenario_viewport"));
	gtk_drag_dest_set(GTK_WIDGET(m_scenarioDrawingArea), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "expose_event", G_CALLBACK(scenario_drawing_area_expose_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "drag_data_received", G_CALLBACK(scenario_drawing_area_drag_data_received_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "motion_notify_event", G_CALLBACK(scenario_drawing_area_motion_notify_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "button_press_event", G_CALLBACK(scenario_drawing_area_button_pressed_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "button_release_event", G_CALLBACK(scenario_drawing_area_button_released_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "key-press-event", G_CALLBACK(scenario_drawing_area_key_press_event_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioDrawingArea), "key-release-event", G_CALLBACK(scenario_drawing_area_key_release_event_cb), this);
	g_signal_connect(G_OBJECT(m_notebookPageContent), "scroll-event", G_CALLBACK(scenario_scrolledwindow_scroll_event_cb), this);

	m_mensiaLogoPixbuf = gdk_pixbuf_new_from_file(Directories::getDataDir() + "/applications/designer/mensia-decoration.png", nullptr);

#if defined TARGET_OS_Windows
	// add drag-n-drop capabilities onto the scenario notebook to open new scenario
	gtk_drag_dest_add_uri_targets(GTK_WIDGET(m_scenarioDrawingArea));
#endif

	//retrieve visualization tree

	m_Application.m_VisualizationMgr->createVisualizationTree(m_TreeID);
	m_Tree = &m_Application.m_VisualizationMgr->getVisualizationTree(m_TreeID);
	m_Tree->init(&m_Scenario);

	//create window manager
	m_DesignerVisualization = new CDesignerVisualization(m_kernelCtx, *m_Tree, *this);
	m_DesignerVisualization->init(string(guiFilename));

	m_configureSettingsDialog                 = GTK_WIDGET(gtk_builder_get_object(m_Application.m_Builder, "dialog_scenario_configuration"));
	m_settingsVBox                            = GTK_WIDGET(gtk_builder_get_object(m_Application.m_Builder, "dialog_scenario_configuration-vbox"));
	m_noHelpDialog                            = GTK_WIDGET(gtk_builder_get_object(m_Application.m_Builder, "dialog_no_help"));
	m_errorPendingDeprecatedInterfacorsDialog = GTK_WIDGET(gtk_builder_get_object(m_Application.m_Builder, "dialog_pending_deprecated_interfacors"));

	this->redrawScenarioSettings();
	this->redrawScenarioInputSettings();
	this->redrawScenarioOutputSettings();

	m_StateStack.reset(new CScenarioStateStack(ctx, *this, scenario));

	CInterfacedScenario::updateScenarioLabel();

	// Output a log message if any box of the scenario is in some special state
	CIdentifier boxID      = OV_UndefinedIdentifier;
	bool warningUpdate     = false;
	bool warningDeprecated = false;
	bool warningUnknown    = false;
	while ((boxID = m_Scenario.getNextBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
	{
		//const IBox *box = m_scenario.getBoxDetails(l_oBoxID);
		//const CBoxProxy proxy(m_kernelCtx, *box);
		const CBoxProxy proxy(m_kernelCtx, m_Scenario, boxID);

		if (!warningUpdate && !proxy.isUpToDate())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning <<
					"Scenario requires 'update' of some box(es). You need to replace these boxes or the scenario may not work correctly.\n";
			warningUpdate = true;
		}
		if (!warningDeprecated && proxy.isDeprecated())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Scenario constains deprecated box(es). Please consider using other boxes instead.\n";
			warningDeprecated = true;
		}
		//		if (!noteUnstable && proxy.isUnstable())
		//		{
		//			m_kernelCtx.getLogManager() << LogLevel_Debug << "Scenario contains unstable box(es).\n";
		//			noteUnstable = true;
		//		}
		if (!warningUnknown && !proxy.isBoxAlgorithmPluginPresent())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Scenario contains unknown box algorithm(s).\n";
			if (proxy.isMetabox())
			{
				CString mPath = m_kernelCtx.getConfigurationManager().expand("${Kernel_Metabox}");
				m_kernelCtx.getLogManager() << LogLevel_Warning << "Some Metaboxes could not be found in [" << mPath << "]\n";
			}
			warningUnknown = true;
		}
	}
}

CInterfacedScenario::~CInterfacedScenario()
{
	//delete window manager


	delete m_DesignerVisualization;


	if (m_stencilBuffer != nullptr) { g_object_unref(m_stencilBuffer); }

	g_object_unref(m_guiBuilder);
	/*
	g_object_unref(m_builder);
	g_object_unref(m_builder);
	*/

	gtk_notebook_remove_page(&m_notebook, gtk_notebook_page_num(&m_notebook, m_notebookPageContent));
}

void CInterfacedScenario::redraw()
{
	if (GDK_IS_WINDOW(GTK_WIDGET(m_scenarioDrawingArea)->window)) { gdk_window_invalidate_rect(GTK_WIDGET(m_scenarioDrawingArea)->window, nullptr, 1); }
}

// This function repaints the dialog which opens when configuring settings
void CInterfacedScenario::redrawConfigureScenarioSettingsDialog()
{
	if (m_HasFileName)
	{
		char filename[1024];
		FS::Files::getFilename(m_Filename.c_str(), filename);
		const std::string title = std::string("Settings for \"") + filename + "\"";
		gtk_window_set_title(GTK_WINDOW(m_configureSettingsDialog), title.c_str());
	}
	else { gtk_window_set_title(GTK_WINDOW(m_configureSettingsDialog), "Settings for an unnamed scenario"); }

	GList* widgets = gtk_container_get_children(GTK_CONTAINER(m_settingsVBox));
	for (GList* it = widgets; it != nullptr; it = g_list_next(it)) { gtk_widget_destroy(GTK_WIDGET(it->data)); }
	g_list_free(widgets);

	m_settingConfigCBDatas.clear();
	m_settingConfigCBDatas.resize(m_Scenario.getSettingCount());

	if (m_Scenario.getSettingCount() == 0)
	{
		GtkWidget* widget = gtk_label_new("This scenario has no settings");
		gtk_box_pack_start(GTK_BOX(m_settingsVBox), widget, TRUE, TRUE, 5);
	}
	else
	{
		for (size_t i = 0; i < m_Scenario.getSettingCount(); ++i)
		{
			GtkBuilder* builder = gtk_builder_new();
			gtk_builder_add_from_string(builder, m_SerializedSettingGUIXML.c_str(), m_SerializedSettingGUIXML.length(), nullptr);

			GtkWidget* container = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(::gtk_widget_get_parent(container)), container);
			gtk_box_pack_start(GTK_BOX(m_settingsVBox), container, FALSE, FALSE, 5);

			GtkWidget* entryName     = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-entry_name"));
			GtkWidget* comboBoxType  = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-combobox_type"));
			GtkWidget* buttonUp      = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-button_move_up"));
			GtkWidget* buttonDown    = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-button_move_down"));
			GtkWidget* buttonDelete  = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-button_delete"));
			GtkWidget* entryID       = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-entry_identifier"));
			GtkWidget* buttonResetID = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_configuration_setting-button_reset_identifier"));

			// fill the type dropdown
			CIdentifier typeID = OV_UndefinedIdentifier;
			m_Scenario.getSettingType(i, typeID);

			CIdentifier id;
			CString str;
			gint idx = 0;
			while ((id = m_kernelCtx.getTypeManager().getNextTypeIdentifier(id)) != OV_UndefinedIdentifier)
			{
				if (!m_kernelCtx.getTypeManager().isStream(id))
				{
					gtk_combo_box_append_text(GTK_COMBO_BOX(comboBoxType), m_kernelCtx.getTypeManager().getTypeName(id).toASCIIString());
					if (id == typeID) { gtk_combo_box_set_active(GTK_COMBO_BOX(comboBoxType), idx); }
					idx++;
				}
			}
			// Set name
			m_Scenario.getSettingName(i, str);
			gtk_entry_set_text(GTK_ENTRY(entryName), str.toASCIIString());

			// Set the identifer
			m_Scenario.getInterfacorIdentifier(EBoxInterfacorType::Setting, i, id);
			gtk_entry_set_text(GTK_ENTRY(entryID), id.str().c_str());

			// Add widget for the actual setting
			str                     = m_SettingHelper->getSettingWidgetName(typeID);
			GtkWidget* defaultValue = GTK_WIDGET(gtk_builder_get_object(builder, str.toASCIIString()));

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(defaultValue)), defaultValue);
			gtk_table_attach_defaults(GTK_TABLE(container), defaultValue, 1, 5, 1, 2);

			// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
			m_Scenario.getSettingDefaultValue(i, str);
			m_SettingHelper->setValue(typeID, defaultValue, str);

			// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
			str                          = m_SettingHelper->getSettingEntryWidgetName(typeID);
			GtkWidget* entryDefaultValue = GTK_WIDGET(gtk_builder_get_object(builder, str.toASCIIString()));

			// Set the callbacks
			setting_cb_data_t cbData;
			cbData.scenario         = this;
			cbData.index            = i;
			cbData.widgetValue      = defaultValue;
			cbData.widgetEntryValue = entryDefaultValue;
			cbData.container        = container;

			m_settingConfigCBDatas[i] = cbData;

			// Connect signals of the container
			g_signal_connect(G_OBJECT(comboBoxType), "changed", G_CALLBACK(modify_scenario_setting_type_cb), &m_settingConfigCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonDelete), "clicked", G_CALLBACK(delete_scenario_setting_cb), &m_settingConfigCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonUp), "clicked", G_CALLBACK(modify_scenario_setting_move_up_cb), &m_settingConfigCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonDown), "clicked", G_CALLBACK(modify_scenario_setting_move_down_cb), &m_settingConfigCBDatas[i]);
			g_signal_connect(G_OBJECT(entryName), "changed", G_CALLBACK(modify_scenario_setting_name_cb), &m_settingConfigCBDatas[i]);
			g_signal_connect(G_OBJECT(entryID), "activate", G_CALLBACK(modify_scenario_setting_identifier_cb), &m_settingConfigCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonResetID), "clicked", G_CALLBACK(reset_scenario_setting_identifier_cb), &m_settingConfigCBDatas[i]);

			// these callbacks assure that we can use copy/paste and undo within editable fields
			// as otherwise the keyboard shortucts are stolen by the designer
			g_signal_connect(G_OBJECT(entryName), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_Application);
			g_signal_connect(G_OBJECT(entryName), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_Application);
			g_signal_connect(G_OBJECT(entryDefaultValue), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_Application);
			g_signal_connect(G_OBJECT(entryDefaultValue), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_Application);

			// add callbacks for setting the settings
			g_signal_connect(entryDefaultValue, "changed", G_CALLBACK(modify_scenario_setting_default_value_cb), &m_settingConfigCBDatas[i]);

			g_object_unref(builder);
		}
	}
}

// This function, similar to the previous one, repaints the settings handling sidebar
void CInterfacedScenario::redrawScenarioSettings()
{
	GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_Application.m_Builder, "openvibe-scenario_configuration_vbox"));

	GList* widgets = gtk_container_get_children(GTK_CONTAINER(widget));
	for (GList* settingIterator = widgets; settingIterator != nullptr; settingIterator = g_list_next(settingIterator))
	{
		gtk_widget_destroy(GTK_WIDGET(settingIterator->data));
	}
	g_list_free(widgets);

	m_settingCBDatas.clear();
	m_settingCBDatas.resize(m_Scenario.getSettingCount());

	if (m_Scenario.getSettingCount() == 0)
	{
		GtkWidget* label = gtk_label_new("This scenario has no settings");
		gtk_box_pack_start(GTK_BOX(widget), label, TRUE, TRUE, 5);
	}
	else
	{
		for (size_t i = 0; i < m_Scenario.getSettingCount(); ++i)
		{
			GtkBuilder* builder = gtk_builder_new();
			gtk_builder_add_from_string(builder, m_SerializedSettingGUIXML.c_str(), m_SerializedSettingGUIXML.length(), nullptr);

			GtkWidget* container = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(container)), container);
			gtk_box_pack_start(GTK_BOX(widget), container, FALSE, FALSE, 5);

			GtkWidget* labelName     = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_setting-label"));
			GtkWidget* buttonDefault = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_setting-button_default"));
			GtkWidget* buttonCopy    = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_setting-button_copy"));

			// Set name
			CString str;
			m_Scenario.getSettingName(i, str);
			gtk_label_set_text(GTK_LABEL(labelName), str.toASCIIString());
			gtk_misc_set_alignment(GTK_MISC(labelName), 0.0, 0.5);

			// Add widget for the actual setting
			CIdentifier typeID = OV_UndefinedIdentifier;
			m_Scenario.getSettingType(i, typeID);
			str = m_SettingHelper->getSettingWidgetName(typeID);

			GtkWidget* value = GTK_WIDGET(gtk_builder_get_object(builder, str.toASCIIString()));

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(value)), value);
			gtk_table_attach_defaults(GTK_TABLE(container), value, 0, 1, 1, 2);

			// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
			m_Scenario.getSettingValue(i, str);
			m_SettingHelper->setValue(typeID, value, str);

			// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
			str                   = m_SettingHelper->getSettingEntryWidgetName(typeID);
			GtkWidget* entryValue = GTK_WIDGET(gtk_builder_get_object(builder, str.toASCIIString()));

			// Set the callbacks
			setting_cb_data_t cbData;
			cbData.scenario         = this;
			cbData.index            = i;
			cbData.widgetValue      = value;
			cbData.widgetEntryValue = entryValue;
			cbData.container        = container;

			m_settingCBDatas[i] = cbData;

			// these callbacks assure that we can use copy/paste and undo within editable fields
			// as otherwise the keyboard shortucts are stolen by the designer
			g_signal_connect(G_OBJECT(entryValue), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_Application);
			g_signal_connect(G_OBJECT(entryValue), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_Application);

			// add callbacks for setting the settings
			g_signal_connect(entryValue, "changed", G_CALLBACK(modify_scenario_setting_value_cb), &m_settingCBDatas[i]);
			g_signal_connect(buttonDefault, "clicked", G_CALLBACK(modify_scenario_setting_revert_to_default_cb), &m_settingCBDatas[i]);
			g_signal_connect(buttonCopy, "clicked", G_CALLBACK(copy_scenario_setting_token_cb), &m_settingCBDatas[i]);

			g_object_unref(builder);
		}
	}
	gtk_widget_show_all(widget);
}

void CInterfacedScenario::redrawScenarioInputSettings()
{
	size_t (IScenario::* getNLink)() const                      = &IScenario::getInputCount;
	bool (IScenario::* getLinkName)(size_t, CString&) const     = &IScenario::getInputName;
	bool (IScenario::* getLinkType)(size_t, CIdentifier&) const = &IScenario::getInputType;

	this->redrawScenarioLinkSettings(m_Application.m_Inputs, true, m_scenarioInputCBDatas, getNLink, getLinkName, getLinkType);
}

void CInterfacedScenario::redrawScenarioOutputSettings()
{
	size_t (IScenario::* getNLink)() const                      = &IScenario::getOutputCount;
	bool (IScenario::* getLinkName)(size_t, CString&) const     = &IScenario::getOutputName;
	bool (IScenario::* getLinkType)(size_t, CIdentifier&) const = &IScenario::getOutputType;

	this->redrawScenarioLinkSettings(m_Application.m_Outputs, false, m_scenarioOutputCBDatas, getNLink, getLinkName, getLinkType);
}

// Redraws the tab containing inputs or outputs of the scenario
// This method receives pointers to methods that manipulate either intpus or outputs so it can be generic
void CInterfacedScenario::redrawScenarioLinkSettings(GtkWidget* links, const bool isInput, std::vector<link_cb_data_t>& linkCBDatas,
													 size_t (IScenario::* getNLink)() const, bool (IScenario::* getLinkName)(size_t, CString&) const,
													 bool (IScenario::* getLinkType)(size_t, CIdentifier&) const)
{
	GList* widgets = gtk_container_get_children(GTK_CONTAINER(links));
	for (GList* it = widgets; it != nullptr; it = g_list_next(it)) { gtk_widget_destroy(GTK_WIDGET(it->data)); }
	g_list_free(widgets);

	const size_t nLink = (m_Scenario.*getNLink)();

	linkCBDatas.clear();
	linkCBDatas.resize(nLink);

	gtk_table_resize(GTK_TABLE(links), nLink == 0 ? 1 : nLink, 7);

	if (nLink == 0)
	{
		GtkWidget* settingPlaceholderLabel = gtk_label_new("This scenario has none");
		gtk_table_attach_defaults(GTK_TABLE(links), settingPlaceholderLabel, 0, 1, 0, 1);
	}
	else
	{
		for (size_t i = 0; i < nLink; ++i)
		{
			GtkBuilder* builder = gtk_builder_new();
			gtk_builder_add_from_string(builder, m_SerializedSettingGUIXML.c_str(), m_SerializedSettingGUIXML.length(), nullptr);

			GtkWidget* container = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(container)), container);

			GtkWidget* entryLinkName = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-label"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(entryLinkName)), entryLinkName);

			GtkWidget* comboBoxType = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-combobox_type"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(comboBoxType)), comboBoxType);

			// fill the type dropdown
			CIdentifier typeID = OV_UndefinedIdentifier;
			(m_Scenario.*getLinkType)(i, typeID);

			CIdentifier id;
			gint idx = 0;
			while ((id = m_kernelCtx.getTypeManager().getNextTypeIdentifier(id)) != OV_UndefinedIdentifier)
			{
				if (m_kernelCtx.getTypeManager().isStream(id))
				{
					gtk_combo_box_append_text(GTK_COMBO_BOX(comboBoxType), m_kernelCtx.getTypeManager().getTypeName(id).toASCIIString());
					if (id == typeID) { gtk_combo_box_set_active(GTK_COMBO_BOX(comboBoxType), idx); }

					idx++;
				}
			}
			gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(comboBoxType), GTK_SENSITIVITY_OFF);

			GtkWidget* buttonUp = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-button_move_up"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(buttonUp)), buttonUp);
			GtkWidget* buttonDown = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-button_move_down"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(buttonDown)), buttonDown);
			GtkWidget* buttonEdit = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-button_edit"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(buttonEdit)), buttonEdit);
			GtkWidget* buttonDelete = GTK_WIDGET(gtk_builder_get_object(builder, "scenario_io_setting-button_delete"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(buttonDelete)), buttonDelete);

			// Set name
			CString str;
			(m_Scenario.*getLinkName)(i, str);
			gtk_label_set_text(GTK_LABEL(entryLinkName), str.toASCIIString());
			gtk_misc_set_alignment(GTK_MISC(entryLinkName), 0.0, 0.5);
			gtk_widget_set_sensitive(GTK_WIDGET(entryLinkName), GTK_SENSITIVITY_OFF);

			gtk_table_attach(GTK_TABLE(links), entryLinkName, 0, 1, i, i + 1, GtkAttachOptions(GTK_EXPAND | GTK_FILL), GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(links), comboBoxType, 1, 2, i, i + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(links), buttonUp, 3, 4, i, i + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(links), buttonDown, 4, 5, i, i + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(links), buttonEdit, 5, 6, i, i + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(links), buttonDelete, 6, 7, i, i + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);

			// Set the callbacks
			link_cb_data_t cbData;
			cbData.scenario = this;
			cbData.index    = i;
			cbData.input    = isInput;

			linkCBDatas[i] = cbData;

			g_signal_connect(G_OBJECT(buttonDelete), "clicked", G_CALLBACK(delete_scenario_link_cb), &linkCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonEdit), "clicked", G_CALLBACK(edit_scenario_link_cb), &linkCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonUp), "clicked", G_CALLBACK(modify_scenario_link_move_up_cb), &linkCBDatas[i]);
			g_signal_connect(G_OBJECT(buttonDown), "clicked", G_CALLBACK(modify_scenario_link_move_down_cb), &linkCBDatas[i]);

			g_object_unref(builder);
		}
	}

	gtk_widget_show_all(links);
}

void CInterfacedScenario::updateScenarioLabel()
{
	GtkLabel* gtkLabel = GTK_LABEL(gtk_builder_get_object(m_guiBuilder, "openvibe-scenario_label"));
	string label;
	string filename       = m_Filename;
	string labelUntrimmed = "unsaved document";
	string::size_type pos;
	while ((pos = filename.find('\\')) != string::npos) { filename[pos] = '/'; }

	label += m_HasBeenModified ? "*" : "";
	label += " ";

	// trimming file name if the number of character is above ${Designer_ScenarioFileNameTrimmingLimit}
	// trim only unselected scenarios
	if (m_HasFileName)
	{
		labelUntrimmed   = filename;
		filename         = filename.substr(filename.rfind('/') + 1);
		size_t trimLimit = size_t(m_kernelCtx.getConfigurationManager().expandAsUInteger("${Designer_ScenarioFileNameTrimmingLimit}", 25));
		if (trimLimit > 3) { trimLimit -= 3; } // limit should include the '...'
		// default = we trim everything but the current scenario filename
		// if  {we are stacking horizontally the scenarios, we trim also } current filename to avoid losing too much of the edition panel.
		if (filename.size() > trimLimit)
		{
			if (m_Application.getCurrentInterfacedScenario() == this
				&& m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ScenarioTabsVerticalStack}", false))
			{
				filename = "..." + filename.substr(filename.size() - trimLimit, trimLimit);
			}
			if (m_Application.getCurrentInterfacedScenario() != this)
			{
				filename = filename.substr(0, trimLimit);
				filename += "...";
			}
		}
		label += filename;
	}
	else { label += "(untitled)"; }

	label += " ";
	label += m_HasBeenModified ? "*" : "";

	gtk_label_set_text(gtkLabel, label.c_str());

	label = labelUntrimmed;
	pos   = 0;
	while ((pos = label.find('&', pos)) != std::string::npos)
	{
		label.replace(pos, 1, "&amp;");
		pos += 5;
	}
	gtk_widget_set_tooltip_markup(GTK_WIDGET(gtkLabel), ("<i>" + label + (m_HasBeenModified ? " - unsaved" : "") + "</i>").c_str());
}

#define UPDATE_STENCIL_IDX(id,stencilgc) { (id)++; ::GdkColor sc={0, guint16(((id)&0xff0000)>>8), guint16((id)&0xff00), guint16(((id)&0xff)<<8) }; gdk_gc_set_rgb_fg_color(stencilgc, &sc); }

void CInterfacedScenario::redraw(IBox& box)
{
	GtkWidget* widget = GTK_WIDGET(m_scenarioDrawingArea);
	GdkGC* stencilGC  = gdk_gc_new(GDK_DRAWABLE(m_stencilBuffer));
	GdkGC* drawGC     = gdk_gc_new(widget->window);

	const int marginX     = int(round(5 * m_currentScale));
	const int marginY     = int(round(5 * m_currentScale));
	const int circleSize  = int(round(11 * m_currentScale));
	const int circleSpace = int(round(4 * m_currentScale));

	//CBoxProxy proxy(m_kernelCtx, box);
	CBoxProxy proxy(m_kernelCtx, m_Scenario, box.getIdentifier());

	if (box.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
		proxy.setBoxAlgorithmDescriptorOverride(static_cast<const IBoxAlgorithmDesc*>(m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId)));
	}

	int sizeX  = int(round(proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) * m_currentScale) + marginX * 2);
	int sizeY  = int(round(proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) * m_currentScale) + marginY * 2);
	int startX = int(round(proxy.getXCenter() * m_currentScale + m_viewOffsetX - (sizeX >> 1)));
	int startY = int(round(proxy.getYCenter() * m_currentScale + m_viewOffsetY - (sizeY >> 1)));

	UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, startX, startY, sizeX, sizeY, gint(round(8.0 * m_currentScale)));
	m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier());

	bool canCreate                    = proxy.isBoxAlgorithmPluginPresent();
	bool upToDate                     = canCreate ? proxy.isUpToDate() : true;
	bool pendingDeprecatedInterfacors = proxy.hasPendingDeprecatedInterfacors();
	bool deprecated                   = canCreate && proxy.isDeprecated();
	bool metabox                      = canCreate && proxy.isMetabox();
	bool disabled                     = proxy.isDisabled();


	// Check if this is a mensia box
	auto pod    = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(box.getAlgorithmClassIdentifier());
	bool mensia = (pod && pod->hasFunctionality(M_Functionality_IsMensia));

	// Add a thick dashed border around selected boxes
	if (m_SelectedObjects.count(box.getIdentifier()))
	{
		int offsetTL = 2;	// Offset Top Left
#if defined TARGET_OS_Windows
		int offsetBR = 4;	// Offset Bottom Right
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		int offsetBR = 5;	// Offset Bottom Right
#else
		int offsetBR = 4;	// Offset Bottom Right
#endif
		if (metabox)
		{
			offsetTL = 3;
			offsetBR = 6;
		}

		gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBorderSelected]);
		gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(widget->window, drawGC, TRUE, startX - offsetTL, startY - offsetTL, sizeX + offsetBR, sizeY + offsetBR);
	}

	if (!this->isLocked() || !m_DebugCPUUsage)
	{
		//if(m_currentObject[box.getIdentifier()]) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundSelected]); }
		//else
		if (!canCreate) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundMissing]); }
		else if (disabled) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundDisabled]); }
		else if (deprecated) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundDeprecated]); }
		else if (!upToDate || pendingDeprecatedInterfacors) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundOutdated]); }
		else if (mensia) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundMensia]); }
			//else if(metabox) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundMetabox]); }
		else { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackground]); }
	}
	else
	{
		CIdentifier timeID;
		timeID.fromString(box.getAttributeValue(OV_AttributeId_Box_ComputationTimeLastSecond));
		uint64_t time      = (timeID == OV_UndefinedIdentifier ? 0 : timeID.toUInteger());
		uint64_t reference = (1LL << 32) / (m_nBox == 0 ? 1 : m_nBox);

		GdkColor color;
		if (time < reference)
		{
			color.pixel = 0;
			color.red   = guint16((time << 16) / reference);
			color.green = 32768;
			color.blue  = 0;
		}
		else
		{
			if (time < reference * 4)
			{
				color.pixel = 0;
				color.red   = 65535;
				color.green = guint16(32768 - ((time << 15) / (reference * 4)));
				color.blue  = 0;
			}
			else
			{
				color.pixel = 0;
				color.red   = 65535;
				color.green = 0;
				color.blue  = 0;
			}
		}
		gdk_gc_set_rgb_fg_color(drawGC, &color);
	}

	gdk_draw_rounded_rectangle(widget->window, drawGC, TRUE, startX, startY, sizeX, sizeY, gint(round(8.0 * m_currentScale)));

	if (mensia) { gdk_draw_pixbuf(widget->window, drawGC, m_mensiaLogoPixbuf, 5, 5, startX, startY, 80, (sizeY < 50) ? sizeY : 50, GDK_RGB_DITHER_NONE, 0, 0); }

	int borderColor = Color_BoxBorder;
	if (mensia) { borderColor = Color_BoxBorderMensia; }
	gdk_gc_set_rgb_fg_color(drawGC, &gColors[borderColor]);
	gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
	gdk_draw_rounded_rectangle(widget->window, drawGC, FALSE, startX, startY, sizeX, sizeY, gint(round(8.0 * m_currentScale)));

	if (metabox)
	{
		gdk_gc_set_rgb_fg_color(drawGC, &gColors[borderColor]);
		gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(widget->window, drawGC, FALSE, startX - 3, startY - 3, sizeX + 6, sizeY + 6, gint(round(8.0 * m_currentScale)));
	}

	TAttributeHandler handler(box);

	int offset = sizeX / 2 - int(box.getInputCount()) * (circleSpace + circleSize) / 2 + circleSize / 4;
	for (size_t i = 0; i < box.getInterfacorCountIncludingDeprecated(Input); ++i)
	{
		CIdentifier id;
		bool isDeprecated;
		box.getInputType(i, id);
		box.getInterfacorDeprecatedStatus(Input, i, isDeprecated);

		GdkColor color    = colorFromIdentifier(id, isDeprecated);
		const auto points = get4PointsInterfacorRedraw(circleSize, startX + int(i) * (circleSpace + circleSize) + offset, startY - (circleSize >> 1));

		UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, points.data(), 3);
		m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Input, i);

		drawBorderInterfacor(widget, drawGC, color, points, Color_BoxInputBorder, isDeprecated);

		int x = startX + int(i) * (circleSpace + circleSize) + (circleSize >> 1) - m_viewOffsetX + offset;
		int y = startY - (circleSize >> 1) - m_viewOffsetY;
		id    = m_Scenario.getNextLinkIdentifierToBoxInput(OV_UndefinedIdentifier, box.getIdentifier(), i);
		while (id != OV_UndefinedIdentifier)
		{
			ILink* link = m_Scenario.getLinkDetails(id);
			linkHandler(link, x, y, OV_AttributeId_Link_XDst, OV_AttributeId_Link_YDst);
			id = m_Scenario.getNextLinkIdentifierToBoxInput(id, box.getIdentifier(), i);
		}

		// Display a circle above inputs that are linked to the box inputs
		for (size_t j = 0; j < m_Scenario.getInputCount(); j++)
		{
			size_t boxInputIdx;
			m_Scenario.getScenarioInputLink(j, id, boxInputIdx);

			if (id == box.getIdentifier() && boxInputIdx == i)
			{
				// Since the circle representing the input is quite large, we are going to offset each other one
				int offsetDisc = int(i % 2) * circleSize * 2;

				const int left = startX + int(i) * (circleSpace + circleSize) + offset - int(circleSize * 0.5);
				const int top  = startY - (circleSize >> 1) - circleSize * 3 - offsetDisc;

				this->m_Scenario.getInputType(j, id);
				color = colorFromIdentifier(id, false);

				UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, left, top, circleSize * 2, circleSize * 2, 0, 64 * 360);
				m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_ScenarioInput, i);

				drawCircleWithBorder(widget, drawGC, color, gColors[Color_BoxInputBorder], left, top, circleSize * 2);

				// Draw the text indicating the scenario input index
				drawScenarioTextIOIndex(widget, drawGC, j, left + marginX, top + marginY,
										startX + int(i) * (circleSpace + circleSize) + offset + (circleSize >> 1), top + circleSize * 2,
										startX + int(i) * (circleSpace + circleSize) + offset + (circleSize >> 1), startY - (circleSize >> 1));
			}
		}
	}

	gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);

	offset = sizeX / 2 - int(box.getOutputCount()) * (circleSpace + circleSize) / 2 + circleSize / 4;
	for (size_t i = 0; i < box.getInterfacorCountIncludingDeprecated(Output); ++i)
	{
		CIdentifier id;
		bool isDeprecated;
		box.getOutputType(i, id);
		box.getInterfacorDeprecatedStatus(Output, i, isDeprecated);
		GdkColor color = colorFromIdentifier(id, isDeprecated);

		const auto points = get4PointsInterfacorRedraw(circleSize, startX + int(i) * (circleSpace + circleSize) + offset, startY - (circleSize >> 1) + sizeY);
		UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, points.data(), 3);

		m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Output, i);

		drawBorderInterfacor(widget, drawGC, color, points, Color_BoxOutputBorder, isDeprecated);

		int x = startX + int(i) * (circleSpace + circleSize) + (circleSize >> 1) - m_viewOffsetX + offset;
		int y = startY + sizeY + (circleSize >> 1) + 1 - m_viewOffsetY;
		id    = m_Scenario.getNextLinkIdentifierFromBoxOutput(OV_UndefinedIdentifier, box.getIdentifier(), i);
		while (id != OV_UndefinedIdentifier)
		{
			ILink* link = m_Scenario.getLinkDetails(id);
			if (link)
			{
				TAttributeHandler attHandler(*link);
				linkHandler(link, x, y, OV_AttributeId_Link_XSrc, OV_AttributeId_Link_YSrc);
			}
			id = m_Scenario.getNextLinkIdentifierFromBoxOutput(id, box.getIdentifier(), i);
		}

		// Display a circle below outputs that are linked to the box outputs
		for (size_t j = 0; j < m_Scenario.getOutputCount(); j++)
		{
			size_t boxOutputIdx;
			m_Scenario.getScenarioOutputLink(j, id, boxOutputIdx);
			if (id == box.getIdentifier() && boxOutputIdx == i)
			{
				// Since the circle representing the Output is quite large, we are going to offset each other one
				int offsetDisc = (int(i) % 2) * circleSize * 2;

				const int left = startX + int(i) * (circleSpace + circleSize) + offset - int(circleSize * 0.5);
				const int top  = startY - (circleSize >> 1) + sizeY + offsetDisc + circleSize * 2;

				this->m_Scenario.getOutputType(j, id);
				color = colorFromIdentifier(id);

				UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, left, top, circleSize * 2, circleSize * 2, 0, 64 * 360);
				m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_ScenarioOutput, i);

				drawCircleWithBorder(widget, drawGC, color, gColors[Color_BoxOutputBorder], left, top, circleSize * 2);
				// Draw the text indicating the scenario output index
				// This is somewhat the bottom of the triangle indicating a box output
				drawScenarioTextIOIndex(widget, drawGC, j, left + marginX, top + marginY,
										startX + int(i) * (circleSpace + circleSize) + offset + (circleSize >> 1), top,
										startX + int(i) * (circleSpace + circleSize) + offset + (circleSize >> 1), startY + (circleSize >> 2) + sizeY + 2);
			}
		}
	}

	// Draw labels
	PangoContext* ctx   = gtk_widget_get_pango_context(widget);
	PangoLayout* layout = pango_layout_new(ctx);

	// Draw box label
	PangoRectangle labelRect;
	pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(layout, proxy.getLabel(), -1);
	pango_layout_get_pixel_extents(layout, nullptr, &labelRect);
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], startX + marginX, startY + marginY, layout);

	// Draw box status label
	PangoRectangle statusRect;
	pango_layout_set_markup(layout, proxy.getStatusLabel(), -1);
	pango_layout_get_pixel_extents(layout, nullptr, &statusRect);
	int shiftX = (max(labelRect.width, statusRect.width) - min(labelRect.width, statusRect.width)) / 2;

	UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
	gdk_draw_rectangle(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, startX + shiftX + marginX, startY + labelRect.height + marginY, statusRect.width,
					   statusRect.height);
	m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Update, 0);
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], startX + shiftX + marginX, startY + labelRect.height + marginY, layout);

	g_object_unref(layout);
	g_object_unref(drawGC);
	g_object_unref(stencilGC);
}

void CInterfacedScenario::redraw(IComment& comment)
{
	GtkWidget* widget = GTK_WIDGET(m_scenarioDrawingArea);
	GdkGC* stencilGC  = gdk_gc_new(GDK_DRAWABLE(m_stencilBuffer));
	GdkGC* drawGC     = gdk_gc_new(widget->window);

	// size_t i;
	const int marginX = static_cast<const int>(round(16 * m_currentScale));
	const int marginY = static_cast<const int>(round(16 * m_currentScale));

	const CCommentProxy proxy(m_kernelCtx, comment);
	const int sizeX  = proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) + marginX * 2;
	const int sizeY  = proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) + marginY * 2;
	const int startX = int(round(proxy.getXCenter() * m_currentScale + m_viewOffsetX - (sizeX >> 1)));
	const int startY = int(round(proxy.getYCenter() * m_currentScale + m_viewOffsetY - (sizeY >> 1)));

	UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, startX, startY, sizeX, sizeY, gint(round(16.0 * m_currentScale)));
	m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(comment.getIdentifier());

	gdk_gc_set_rgb_fg_color(drawGC, &gColors[m_SelectedObjects.count(comment.getIdentifier()) ? Color_CommentBackgroundSelected : Color_CommentBackground]);
	gdk_draw_rounded_rectangle(widget->window, drawGC, TRUE, startX, startY, sizeX, sizeY, gint(round(16.0 * m_currentScale)));
	gdk_gc_set_rgb_fg_color(drawGC, &gColors[m_SelectedObjects.count(comment.getIdentifier()) ? Color_CommentBorderSelected : Color_CommentBorder]);
	gdk_draw_rounded_rectangle(widget->window, drawGC, FALSE, startX, startY, sizeX, sizeY, gint(round(16.0 * m_currentScale)));

	PangoContext* ctx   = gtk_widget_get_pango_context(widget);
	PangoLayout* layout = pango_layout_new(ctx);
	pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
	if (pango_parse_markup(comment.getText().toASCIIString(), -1, 0, nullptr, nullptr, nullptr, nullptr))
	{
		pango_layout_set_markup(layout, comment.getText().toASCIIString(), -1);
	}
	else { pango_layout_set_text(layout, comment.getText().toASCIIString(), -1); }
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], startX + marginX, startY + marginY, layout);
	g_object_unref(layout);

	g_object_unref(drawGC);
	g_object_unref(stencilGC);
}

void CInterfacedScenario::redraw(ILink& link)
{
	GtkWidget* widget = GTK_WIDGET(m_scenarioDrawingArea);
	GdkGC* stencilGC  = gdk_gc_new(GDK_DRAWABLE(m_stencilBuffer));
	GdkGC* drawGC     = gdk_gc_new(widget->window);

	const CLinkProxy proxy(link);

	CIdentifier srcOutputTypeID;
	CIdentifier dstInputTypeID;

	m_Scenario.getBoxDetails(link.getSourceBoxIdentifier())->getOutputType(link.getSourceBoxOutputIndex(), srcOutputTypeID);
	m_Scenario.getBoxDetails(link.getTargetBoxIdentifier())->getInputType(link.getTargetBoxInputIndex(), dstInputTypeID);

	if (link.hasAttribute(OV_AttributeId_Link_Invalid)) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkInvalid]); }
	else if (m_SelectedObjects.count(link.getIdentifier())) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkSelected]); }
	else if (dstInputTypeID == srcOutputTypeID) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_Link]); }
	else
	{
		if (m_kernelCtx.getTypeManager().isDerivedFromStream(srcOutputTypeID, dstInputTypeID))
		{
			gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkDownCast]);
		}
		else if (m_kernelCtx.getTypeManager().isDerivedFromStream(dstInputTypeID, srcOutputTypeID))
		{
			gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkUpCast]);
		}
		else { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkInvalid]); }
	}

	UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
	gdk_draw_line(GDK_DRAWABLE(m_stencilBuffer), stencilGC, proxy.getXSource() + m_viewOffsetX, proxy.getYSource() + m_viewOffsetY,
				  proxy.getXTarget() + m_viewOffsetX, proxy.getYTarget() + m_viewOffsetY);
	gdk_draw_line(widget->window, drawGC, proxy.getXSource() + m_viewOffsetX, proxy.getYSource() + m_viewOffsetY, proxy.getXTarget() + m_viewOffsetX,
				  proxy.getYTarget() + m_viewOffsetY);
	m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(link.getIdentifier(), Box_Link, 0);

	g_object_unref(drawGC);
	g_object_unref(stencilGC);
}


#undef UPDATE_STENCIL_IDX

size_t CInterfacedScenario::pickInterfacedObject(const int x, const int y) const
{
	if (!GDK_DRAWABLE(m_stencilBuffer)) { return size_t(0xffffffff); }

	int maxX;
	int maxY;
	uint32_t res = 0xffffffff;
	gdk_drawable_get_size(GDK_DRAWABLE(m_stencilBuffer), &maxX, &maxY);
	if (x >= 0 && y >= 0 && x < maxX && y < maxY)
	{
		GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_stencilBuffer), nullptr, x, y, 0, 0, 1, 1);
		if (!pixbuf)
		{
			m_kernelCtx.getLogManager() << LogLevel_ImportantWarning <<
					"Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
			return size_t(0xffffffff);
		}

		guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
		if (!pixels)
		{
			m_kernelCtx.getLogManager() << LogLevel_ImportantWarning <<
					"Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		res = 0;
		res += (pixels[0] << 16);
		res += (pixels[1] << 8);
		res += (pixels[2]);
		g_object_unref(pixbuf);
	}
	return size_t(res);
}

bool CInterfacedScenario::pickInterfacedObject(const int x, const int y, int sizeX, int sizeY)
{
	if (!GDK_DRAWABLE(m_stencilBuffer))
	{
		// m_kernelCtx.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return false;
	}

	int maxX;
	int maxY;
	gdk_drawable_get_size(GDK_DRAWABLE(m_stencilBuffer), &maxX, &maxY);

	int startX = x;
	int startY = y;
	int endX   = x + sizeX;
	int endY   = y + sizeY;

	// crops according to drawing area boundings
	if (startX < 0) { startX = 0; }
	if (startY < 0) { startY = 0; }
	if (endX < 0) { endX = 0; }
	if (endY < 0) { endY = 0; }
	if (startX >= maxX - 1) { startX = maxX - 1; }
	if (startY >= maxY - 1) { startY = maxY - 1; }
	if (endX >= maxX - 1) { endX = maxX - 1; }
	if (endY >= maxY - 1) { endY = maxY - 1; }

	// recompute new size
	sizeX = endX - startX + 1;
	sizeY = endY - startY + 1;

	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_stencilBuffer), nullptr, startX, startY, 0, 0, sizeX, sizeY);
	if (!pixbuf)
	{
		m_kernelCtx.getLogManager() << LogLevel_ImportantWarning <<
				"Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
		return false;
	}

	guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
	if (!pixels)
	{
		m_kernelCtx.getLogManager() << LogLevel_ImportantWarning <<
				"Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
		return false;
	}

	const int nRowBytes = gdk_pixbuf_get_rowstride(pixbuf);
	const int nChannel  = gdk_pixbuf_get_n_channels(pixbuf);
	for (int j = 0; j < sizeY; ++j)
	{
		for (int i = 0; i < sizeX; ++i)
		{
			size_t idx = 0;
			idx += (pixels[j * nRowBytes + i * nChannel + 0] << 16);
			idx += (pixels[j * nRowBytes + i * nChannel + 1] << 8);
			idx += (pixels[j * nRowBytes + i * nChannel + 2]);
			if (m_interfacedObjects[idx].m_ID != OV_UndefinedIdentifier) { m_SelectedObjects.insert(m_interfacedObjects[idx].m_ID); }
		}
	}

	g_object_unref(pixbuf);
	return true;
}

#define OV_ClassId_Selected OpenViBE::CIdentifier(0xC67A01DC, 0x28CE06C1)

void CInterfacedScenario::undoCB(const bool manageModifiedStatusFlag)
{
	// When a box gets updated we generate a snapshot beforehand to enable undo in all cases
	// This will result in two indentical undo states, in order to avoid weird Redo, we drop the
	// reduntant state at this moment
	bool shouldDropLastState = false;
	if (m_Scenario.containsBoxWithDeprecatedInterfacors()) { shouldDropLastState = true; }

	if (m_StateStack->undo())
	{
		CIdentifier id;
		m_SelectedObjects.clear();
		while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getBoxDetails(id)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(id); }
		}
		while ((id = m_Scenario.getNextLinkIdentifier(id)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getLinkDetails(id)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(id); }
		}

		if (m_DesignerVisualization) { m_DesignerVisualization->load(); }
		if (manageModifiedStatusFlag) { m_HasBeenModified = true; }

		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();

		if (shouldDropLastState) { m_StateStack->dropLastState(); }

		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_redo")), m_StateStack->isRedoPossible());
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_undo")), m_StateStack->isUndoPossible());
	}
	else
	{
		m_kernelCtx.getLogManager() << LogLevel_Trace << "Can not undo\n";
		GtkWidget* undoButton = GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_undo"));
		gtk_widget_set_sensitive(undoButton, false);
	}
}

void CInterfacedScenario::redoCB(const bool manageModifiedStatusFlag)
{
	if (m_StateStack->redo())
	{
		CIdentifier id;
		m_SelectedObjects.clear();
		while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getBoxDetails(id)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(id); }
		}
		while ((id = m_Scenario.getNextLinkIdentifier(id)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getLinkDetails(id)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(id); }
		}

		if (m_DesignerVisualization) { m_DesignerVisualization->load(); }

		if (manageModifiedStatusFlag) { m_HasBeenModified = true; }
		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_redo")), m_StateStack->isRedoPossible());
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_undo")), m_StateStack->isUndoPossible());
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_redo")), false);
		m_kernelCtx.getLogManager() << LogLevel_Trace << "Can not redo\n";
	}
}

void CInterfacedScenario::snapshotCB(const bool manageModifiedStatusFlag)
{
	if (m_Scenario.containsBoxWithDeprecatedInterfacors())
	{
		OV_WARNING("Scenario containing boxes with deprecated I/O or Settings does not support undo", m_kernelCtx.getLogManager());
	}
	else
	{
		CIdentifier id;

		while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(id)) { m_Scenario.getBoxDetails(id)->addAttribute(OV_ClassId_Selected, ""); }
			else { m_Scenario.getBoxDetails(id)->removeAttribute(OV_ClassId_Selected); }
		}
		while ((id = m_Scenario.getNextLinkIdentifier(id)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(id)) { m_Scenario.getLinkDetails(id)->addAttribute(OV_ClassId_Selected, ""); }
			else { m_Scenario.getLinkDetails(id)->removeAttribute(OV_ClassId_Selected); }
		}

		if (manageModifiedStatusFlag) { m_HasBeenModified = true; }
		this->updateScenarioLabel();
		m_StateStack->snapshot();
	}
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_redo")), m_StateStack->isRedoPossible());
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_Builder, "openvibe-button_undo")), m_StateStack->isUndoPossible());
}

void CInterfacedScenario::addCommentCB(int x, int y)
{
	CIdentifier id;
	m_Scenario.addComment(id, OV_UndefinedIdentifier);
	if (x == -1 || y == -1)
	{
		GtkWidget* scrolledWindow  = gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(m_scenarioDrawingArea)));
		GtkAdjustment* adjustmentH = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolledWindow));
		GtkAdjustment* adjustmentV = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledWindow));

#if defined TARGET_OS_Linux && !defined TARGET_OS_MacOS
		x = int(gtk_adjustment_get_value(adjustmentH) + gtk_adjustment_get_page_size(adjustmentH) / 2);
		y = int(gtk_adjustment_get_value(adjustmentV) + gtk_adjustment_get_page_size(adjustmentV) / 2);
#elif defined TARGET_OS_Windows
		gint wx, wy;
		::gdk_window_get_size(gtk_widget_get_parent(GTK_WIDGET(m_scenarioDrawingArea))->window, &wx, &wy);
		x = int(gtk_adjustment_get_value(adjustmentH) + int(wx / 2));
		y = int(gtk_adjustment_get_value(adjustmentV) + int(wy / 2));
#else
		x = int(gtk_adjustment_get_value(adjustmentH) + 32);
		y = int(gtk_adjustment_get_value(adjustmentV) + 32);
#endif
	}

	CCommentProxy proxy(m_kernelCtx, m_Scenario, id);
	proxy.setCenter(x - m_viewOffsetX, y - m_viewOffsetY);

	// Aligns comemnts on grid
	proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0L), int((proxy.getYCenter() + 8) & 0xfffffff0L));

	// Applies modifications before snapshot
	proxy.apply();

	CCommentEditorDialog dialog(m_kernelCtx, *m_Scenario.getCommentDetails(id), m_guiFilename.c_str());
	if (!dialog.run()) { m_Scenario.removeComment(id); }
	else
	{
		m_SelectedObjects.clear();
		m_SelectedObjects.insert(id);

		this->snapshotCB();
	}

	this->redraw();
}

void CInterfacedScenario::configureScenarioSettingsCB()
{
	this->snapshotCB();

	// construct the dialog
	this->redrawConfigureScenarioSettingsDialog();

	gtk_widget_show_all(m_settingsVBox);

	const gint response = gtk_dialog_run(GTK_DIALOG(m_configureSettingsDialog));

	if (response == GTK_RESPONSE_CANCEL) { this->undoCB(false); }
	else { this->snapshotCB(); }

	gtk_widget_hide(m_configureSettingsDialog);
	this->redrawScenarioSettings();
}

void CInterfacedScenario::addScenarioSettingCB()
{
	const std::string name = "Setting " + std::to_string(m_Scenario.getSettingCount() + 1);
	m_Scenario.addSetting(name.c_str(), OVTK_TypeId_Integer, "0", size_t(-1), false,
						  m_Scenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));

	this->redrawConfigureScenarioSettingsDialog();
}

void CInterfacedScenario::addScenarioInputCB()
{
	const std::string name = "Input " + std::to_string(m_Scenario.getInputCount() + 1);
	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the inputs of the box.
	m_Scenario.addInput(name.c_str(), OVTK_TypeId_StreamedMatrix, m_Scenario.getUnusedInputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor editor(m_kernelCtx, m_Scenario, Box_Input, m_Scenario.getInputCount() - 1, "Add Input", m_guiFilename.c_str());
	if (editor.run()) { this->snapshotCB(); }
	else { m_Scenario.removeInput(m_Scenario.getInputCount() - 1); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::editScenarioInputCB(const size_t index)
{
	CConnectorEditor editor(m_kernelCtx, m_Scenario, Box_Input, index, "Edit Input", m_guiFilename.c_str());
	if (editor.run()) { this->snapshotCB(); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::addScenarioOutputCB()
{
	const std::string name = "Output " + std::to_string(m_Scenario.getOutputCount() + 1);
	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the outputs of the box.
	m_Scenario.addOutput(name.c_str(), OVTK_TypeId_StreamedMatrix, m_Scenario.getUnusedOutputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor editor(m_kernelCtx, m_Scenario, Box_Output, m_Scenario.getOutputCount() - 1, "Add Output", m_guiFilename.c_str());
	if (editor.run()) { this->snapshotCB(); }
	else { m_Scenario.removeOutput(m_Scenario.getOutputCount() - 1); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::editScenarioOutputCB(const size_t index)
{
	CConnectorEditor editor(m_kernelCtx, m_Scenario, Box_Output, index, "Edit Output", m_guiFilename.c_str());
	if (editor.run()) { this->snapshotCB(); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::swapScenarioSettings(const size_t indexA, const size_t indexB)
{
	m_Scenario.swapSettings(indexA, indexB);
	this->redrawConfigureScenarioSettingsDialog();
}

void CInterfacedScenario::swapScenarioInputs(const size_t indexA, const size_t indexB)
{
	CIdentifier idA, idB;
	size_t idxA, idxB;

	m_Scenario.getScenarioInputLink(indexA, idA, idxA);
	m_Scenario.getScenarioInputLink(indexB, idB, idxB);

	m_Scenario.swapInputs(indexA, indexB);

	m_Scenario.setScenarioInputLink(indexB, idA, idxA);
	m_Scenario.setScenarioInputLink(indexA, idB, idxB);

	this->redrawScenarioInputSettings();
	this->redraw();
}

void CInterfacedScenario::swapScenarioOutputs(const size_t indexA, const size_t indexB)
{
	CIdentifier idA, idB;
	size_t idxA, idxB;

	m_Scenario.getScenarioOutputLink(indexA, idA, idxA);
	m_Scenario.getScenarioOutputLink(indexB, idB, idxB);

	m_Scenario.swapOutputs(indexA, indexB);

	m_Scenario.setScenarioOutputLink(indexB, idA, idxA);
	m_Scenario.setScenarioOutputLink(indexA, idB, idxB);

	this->redrawScenarioOutputSettings();
	this->redraw();
}

void CInterfacedScenario::scenarioDrawingAreaExposeCB(GdkEventExpose* /*event*/)
{
	if (m_currentMode == Mode_None)
	{
		gint viewportX = -1;
		gint viewportY = -1;

		gint minX = 0x7fff;
		gint maxX = -0x7fff;
		gint minY = 0x7fff;
		gint maxY = -0x7fff;

		const gint marginX = gint(round(32.0 * m_currentScale));
		const gint marginY = gint(round(32.0 * m_currentScale));

		CIdentifier id;
		while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
		{
			//CBoxProxy proxy(m_kernelCtx, *m_scenario.getBoxDetails(l_oBoxID));
			CBoxProxy proxy(m_kernelCtx, m_Scenario, id);
			minX = std::min(minX, gint((proxy.getXCenter() - 1.0 * proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			maxX = std::max(maxX, gint((proxy.getXCenter() + 1.0 * proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			minY = std::min(minY, gint((proxy.getYCenter() - 1.0 * proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			maxY = std::max(maxY, gint((proxy.getYCenter() + 1.0 * proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
		}

		while ((id = m_Scenario.getNextCommentIdentifier(id)) != OV_UndefinedIdentifier)
		{
			CCommentProxy proxy(m_kernelCtx, *m_Scenario.getCommentDetails(id));
			minX = std::min(minX, gint((proxy.getXCenter() - 1.0 * proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			maxX = std::max(maxX, gint((proxy.getXCenter() + 1.0 * proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			minY = std::min(minY, gint((proxy.getYCenter() - 1.0 * proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			maxY = std::max(maxY, gint((proxy.getYCenter() + 1.0 * proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
		}

		const gint newSizeX = maxX - minX;
		const gint newSizeY = maxY - minY;
		gint oldSizeX       = -1;
		gint oldSizeY       = -1;

		gdk_window_get_size(GTK_WIDGET(m_scenarioViewport)->window, &viewportX, &viewportY);
		gtk_widget_get_size_request(GTK_WIDGET(m_scenarioDrawingArea), &oldSizeX, &oldSizeY);

		if (newSizeX >= 0 && newSizeY >= 0)
		{
			if (oldSizeX != newSizeX + 2 * marginX || oldSizeY != newSizeY + 2 * marginY)
			{
				gtk_widget_set_size_request(GTK_WIDGET(m_scenarioDrawingArea), newSizeX + 2 * marginX, newSizeY + 2 * marginY);
			}
			m_viewOffsetX = std::min(m_viewOffsetX, -maxX - marginX + std::max(viewportX, newSizeX + 2 * marginX));
			m_viewOffsetX = std::max(m_viewOffsetX, -minX + marginX);
			m_viewOffsetY = std::min(m_viewOffsetY, -maxY - marginY + std::max(viewportY, newSizeY + 2 * marginY));
			m_viewOffsetY = std::max(m_viewOffsetY, -minY + marginY);
		}
	}

	gint x, y;

	gdk_window_get_size(GTK_WIDGET(m_scenarioDrawingArea)->window, &x, &y);
	if (m_stencilBuffer) { g_object_unref(m_stencilBuffer); }
	m_stencilBuffer = gdk_pixmap_new(GTK_WIDGET(m_scenarioDrawingArea)->window, x, y, -1);

	GdkGC* stencilGC = gdk_gc_new(m_stencilBuffer);
	GdkColor color   = { 0, 0, 0, 0 };
	gdk_gc_set_rgb_fg_color(stencilGC, &color);
	gdk_draw_rectangle(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, 0, 0, x, y);
	g_object_unref(stencilGC);

	if (this->isLocked())
	{
		color.pixel = 0;
		color.red   = 0x0f00;
		color.green = 0x0f00;
		color.blue  = 0x0f00;

		GdkGC* drawGC = gdk_gc_new(GTK_WIDGET(m_scenarioDrawingArea)->window);
		gdk_gc_set_rgb_fg_color(drawGC, &color);
		gdk_gc_set_function(drawGC, GDK_XOR);
		gdk_draw_rectangle(GTK_WIDGET(m_scenarioDrawingArea)->window, drawGC, TRUE, 0, 0, x, y);
		g_object_unref(drawGC);
	}
	// TODO: optimize this as this will be called endlessly
	/*
	else if (false) //m_scenario.containsBoxWithDeprecatedInterfacors() 
	{
		color.pixel = 0;
		color.red = 0xffff;
		color.green = 0xefff;
		color.blue = 0xefff;

		GdkGC* drawGC = gdk_gc_new(GTK_WIDGET(m_scenarioDrawingArea)->window);
		gdk_gc_set_rgb_fg_color(drawGC, &color);
		gdk_gc_set_function(drawGC, GDK_AND);
		gdk_draw_rectangle(GTK_WIDGET(m_pScenarioDrawingArea)->window, drawGC, TRUE, 0, 0, x, y);
		g_object_unref(l_pDrawGC);
	}
	*/
	m_interfacedObjectId = 0;
	m_interfacedObjects.clear();

	size_t count = 0;
	CIdentifier id;
	while ((id = m_Scenario.getNextCommentIdentifier(id)) != OV_UndefinedIdentifier)
	{
		redraw(*m_Scenario.getCommentDetails(id));
		count++;
	}
	m_nComment = count;

	count = 0;
	while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
	{
		redraw(*m_Scenario.getBoxDetails(id));
		count++;
	}
	m_nBox = count;

	count = 0;
	while ((id = m_Scenario.getNextLinkIdentifier(id)) != OV_UndefinedIdentifier)
	{
		redraw(*m_Scenario.getLinkDetails(id));
		count++;
	}
	m_nLink = count;

	if (m_currentMode == Mode_Selection || m_currentMode == Mode_SelectionAdd)
	{
		const int startX = int(std::min(m_pressMouseX, m_currentMouseX));
		const int startY = int(std::min(m_pressMouseY, m_currentMouseY));
		const int sizeX  = int(std::max(m_pressMouseX - m_currentMouseX, m_currentMouseX - m_pressMouseX));
		const int sizeY  = int(std::max(m_pressMouseY - m_currentMouseY, m_currentMouseY - m_pressMouseY));

		GtkWidget* widget = GTK_WIDGET(m_scenarioDrawingArea);
		GdkGC* drawGC     = gdk_gc_new(widget->window);
		gdk_gc_set_function(drawGC, GDK_OR);
		gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_SelectionArea]);
		gdk_draw_rectangle(widget->window, drawGC, TRUE, startX, startY, sizeX, sizeY);
		gdk_gc_set_function(drawGC, GDK_COPY);
		gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_SelectionAreaBorder]);
		gdk_draw_rectangle(widget->window, drawGC, FALSE, startX, startY, sizeX, sizeY);
		g_object_unref(drawGC);
	}

	if (m_currentMode == Mode_Connect)
	{
		GtkWidget* widget = GTK_WIDGET(m_scenarioDrawingArea);
		GdkGC* drawGC     = gdk_gc_new(widget->window);

		gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_Link]);
		gdk_draw_line(widget->window, drawGC, int(m_pressMouseX), int(m_pressMouseY), int(m_currentMouseX), int(m_currentMouseY));
		g_object_unref(drawGC);
	}
}

// This method inserts a box into the scenario upon receiving data
void CInterfacedScenario::scenarioDrawingAreaDragDataReceivedCB(GdkDragContext* dc, const gint x, const gint y, GtkSelectionData* selectionData, guint /*info*/,
																guint /*t*/)
{
	if (this->isLocked()) { return; }

	// two cases: dragged from inside the program = a box ...
	if (dc->protocol == GDK_DRAG_PROTO_LOCAL || dc->protocol == GDK_DRAG_PROTO_XDND)
	{
		CIdentifier boxID;
		CIdentifier boxAlgorithmClassID;

		// The drag data only contains one string, for a normal box this string is its algorithmClassIdentifier
		// However since all metaboxes have the same identifier, we have added the 'identifier' of a metabox after this string
		// The identifier itself is the name of the scenario which created the metabox
		std::string str(reinterpret_cast<const char*>(gtk_selection_data_get_text(selectionData)));

		// check that there is an identifier inside the string, its form is (0xXXXXXXXX, 0xXXXXXXXX)
		if (str.find(')') != string::npos) { boxAlgorithmClassID.fromString(str.substr(0, str.find(')')).c_str()); }

		IBox* box                    = nullptr;
		const IPluginObjectDesc* pod = nullptr;

		if (boxAlgorithmClassID == OV_UndefinedIdentifier)
		{
			m_currentMouseX = x;
			m_currentMouseY = y;
			return;
		}
		if (boxAlgorithmClassID == OVP_ClassId_BoxAlgorithm_Metabox)
		{
			// extract the name of the metabox from the drag data string
			CIdentifier id;
			id.fromString(CString(str.substr(str.find(')') + 1).c_str()));

			//m_kernelCtx.getLogManager() << LogLevel_Info << "This is a metabox with ID " << metaboxID << "\n";
			pod = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(id);

			// insert a box into the scenario, initialize it from the proxy-descriptor from the metabox loader
			m_Scenario.addBox(boxID, *static_cast<const IBoxAlgorithmDesc*>(pod), OV_UndefinedIdentifier);

			box = m_Scenario.getBoxDetails(boxID);
			box->addAttribute(OVP_AttributeId_Metabox_ID, id.toString());
		}
		else
		{
			m_Scenario.addBox(boxID, boxAlgorithmClassID, OV_UndefinedIdentifier);

			box                  = m_Scenario.getBoxDetails(boxID);
			const CIdentifier id = box->getAlgorithmClassIdentifier();
			pod                  = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(id);
		}

		m_SelectedObjects.clear();
		m_SelectedObjects.insert(boxID);

		// If a visualization box was dropped, add it in window manager
		if (pod && pod->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxAdded(box); }
		}

		CBoxProxy proxy(m_kernelCtx, m_Scenario, boxID);
		proxy.setCenter(x - m_viewOffsetX, y - m_viewOffsetY);
		// Aligns boxes on grid
		proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0L), int((proxy.getYCenter() + 8) & 0xfffffff0L));

		// Applies modifications before snapshot
		proxy.apply();

		this->snapshotCB();

		m_currentMouseX = x;
		m_currentMouseY = y;
	}

	// ... or dragged from outside the application = a file
	// ONLY AVAILABLE ON WINDOWS (known d'n'd protocol)
#if defined TARGET_OS_Windows
	if (dc->protocol == GDK_DRAG_PROTO_WIN32_DROPFILES)
	{
		// we get the content of the buffer: the list of files URI:
		// file:///path/to/file.ext\r\n
		// file:///path/to/file.ext\r\n
		// ...
		const std::string draggedFilesPath(reinterpret_cast<const char*>(gtk_selection_data_get_data(selectionData)));
		std::stringstream ss(draggedFilesPath);
		std::string line;
		std::vector<std::string> filesToOpen;
		while (std::getline(ss, line))
		{
			// the path starts with file:/// and ends with \r\n once parsed line after line, a \r remains on Windows
			line = line.substr(8, line.length() - 9);

			// uri to path (to remove %xx escape characters):
			line = g_uri_unescape_string(line.c_str(), nullptr);

			filesToOpen.push_back(line);
		}

		for (auto& file : filesToOpen) { m_Application.openScenario(file.c_str()); }
	}
#endif
}

void CInterfacedScenario::scenarioDrawingAreaMotionNotifyCB(GtkWidget* /*widget*/, GdkEventMotion* event)
{
	// m_kernelCtx.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaMotionNotifyCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* tooltip = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "tooltip"));
	gtk_widget_set_name(tooltip, "gtk-tooltips");
	const size_t objIdx    = pickInterfacedObject(int(event->x), int(event->y));
	CInterfacedObject& obj = m_interfacedObjects[objIdx];
	if (obj.m_ID != OV_UndefinedIdentifier && obj.m_ConnectorType != Box_Link && obj.m_ConnectorType != Box_None)
	{
		IBox* boxDetails = m_Scenario.getBoxDetails(obj.m_ID);
		if (boxDetails)
		{
			CString name;
			CString type;
			if (obj.m_ConnectorType == Box_Input)
			{
				CIdentifier typeID;
				boxDetails->getInputName(obj.m_ConnectorIdx, name);
				boxDetails->getInputType(obj.m_ConnectorIdx, typeID);
				type = m_kernelCtx.getTypeManager().getTypeName(typeID);
				type = CString("[") + type + CString("]");
			}
			else if (obj.m_ConnectorType == Box_Output)
			{
				CIdentifier typeID;
				boxDetails->getOutputName(obj.m_ConnectorIdx, name);
				boxDetails->getOutputType(obj.m_ConnectorIdx, typeID);
				type = m_kernelCtx.getTypeManager().getTypeName(typeID);
				type = CString("[") + type + CString("]");
			}
			else if (obj.m_ConnectorType == Box_Update)
			{
				//m_scenario.updateBox(boxDetails->getIdentifier());
				name = CString("Right click for");
				type = "box update";
			}
			else if (obj.m_ConnectorType == Box_ScenarioInput)
			{
				CIdentifier typeID;
				boxDetails->getInputName(obj.m_ConnectorIdx, name);
				boxDetails->getInputType(obj.m_ConnectorIdx, typeID);

				for (size_t i = 0; i < m_Scenario.getInputCount(); i++)
				{
					CIdentifier id;
					size_t idx;
					m_Scenario.getScenarioInputLink(i, id, idx);
					if (id == boxDetails->getIdentifier() && idx == obj.m_ConnectorIdx)
					{
						m_Scenario.getInputName(i, name);
						name = CString("Connected to \n") + name;
						m_Scenario.getInputType(i, typeID);
					}
				}
				type = m_kernelCtx.getTypeManager().getTypeName(typeID);
				type = CString("[") + type + CString("]");
			}
			else if (obj.m_ConnectorType == Box_ScenarioOutput)
			{
				CIdentifier typeID;
				boxDetails->getOutputName(obj.m_ConnectorIdx, name);
				boxDetails->getOutputType(obj.m_ConnectorIdx, typeID);

				for (size_t i = 0; i < m_Scenario.getOutputCount(); i++)
				{
					CIdentifier id;
					size_t idx;
					m_Scenario.getScenarioOutputLink(i, id, idx);
					if (id == boxDetails->getIdentifier() && idx == obj.m_ConnectorIdx)
					{
						m_Scenario.getOutputName(i, name);
						name = CString("Connected to \n") + name;
						m_Scenario.getOutputType(i, typeID);
					}
				}
				type = m_kernelCtx.getTypeManager().getTypeName(typeID);
				type = CString("[") + type + CString("]");
			}

			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_guiBuilder, "tooltip-label_name_content")), name);
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_guiBuilder, "tooltip-label_type_content")), type);
			gtk_window_move(GTK_WINDOW(tooltip), gint(event->x_root), gint(event->y_root) + 40);
			gtk_widget_show(tooltip);
		}
	}
	else { gtk_widget_hide(tooltip); }

	if (m_currentMode != Mode_None)
	{
		if (m_currentMode == Mode_MoveScenario)
		{
			m_viewOffsetX += int(event->x - m_currentMouseX);
			m_viewOffsetY += int(event->y - m_currentMouseY);
		}
		else if (m_currentMode == Mode_MoveSelection)
		{
			if (m_controlPressed) { m_SelectedObjects.insert(m_currentObject.m_ID); }
			else
			{
				if (!m_SelectedObjects.count(m_currentObject.m_ID))
				{
					m_SelectedObjects.clear();
					m_SelectedObjects.insert(m_currentObject.m_ID);
				}
			}
			for (auto& id : m_SelectedObjects)
			{
				if (m_Scenario.isBox(id))
				{
					CBoxProxy proxy(m_kernelCtx, m_Scenario, id);
					proxy.setCenter(proxy.getXCenter() + int(event->x - m_currentMouseX), proxy.getYCenter() + int(event->y - m_currentMouseY));
				}
				if (m_Scenario.isComment(id))
				{
					CCommentProxy proxy(m_kernelCtx, m_Scenario, id);
					proxy.setCenter(proxy.getXCenter() + int(event->x - m_currentMouseX), proxy.getYCenter() + int(event->y - m_currentMouseY));
				}
			}
		}

		this->redraw();
	}
	m_currentMouseX = event->x;
	m_currentMouseY = event->y;
}

namespace
{
	void gtk_menu_add_separator_menu_item(GtkMenu* menu)
	{
		GtkSeparatorMenuItem* menuitem = GTK_SEPARATOR_MENU_ITEM(gtk_separator_menu_item_new());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menuitem));
	}

	GtkImageMenuItem* gtk_menu_add_new_image_menu_item(GtkMenu* menu, const char* icon, const char* label)
	{
		GtkImageMenuItem* menuitem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label(label));
		gtk_image_menu_item_set_image(menuitem, gtk_image_new_from_stock(icon, GTK_ICON_SIZE_MENU));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menuitem));
		return menuitem;
	}
} // namespace

GtkImageMenuItem* CInterfacedScenario::addNewImageMenuItemWithCBGeneric(GtkMenu* menu, const char* icon, const char* label, const menu_cb_function_t cb,
																		IBox* box, const size_t command, const size_t index, const size_t index2)
{
	GtkImageMenuItem* menuItem = gtk_menu_add_new_image_menu_item(menu, icon, label);
	box_ctx_menu_cb_t menuCB;
	menuCB.command        = command;
	menuCB.index          = index;
	menuCB.secondaryIndex = index2;
	menuCB.box            = box;
	menuCB.scenario       = this;
	const auto idx        = m_boxCtxMenuCBs.size();
	m_boxCtxMenuCBs[idx]  = menuCB;
	g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(cb), &m_boxCtxMenuCBs[idx]);
	return menuItem;
}

void CInterfacedScenario::scenarioDrawingAreaButtonPressedCB(GtkWidget* widget, GdkEventButton* event)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonPressedCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* tooltip = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "tooltip"));
	gtk_widget_hide(tooltip);
	gtk_widget_grab_focus(widget);

	m_buttonPressed |= ((event->type == GDK_BUTTON_PRESS) && (event->button == 1));
	m_pressMouseX = event->x;
	m_pressMouseY = event->y;

	size_t objIdx   = pickInterfacedObject(int(m_pressMouseX), int(m_pressMouseY));
	m_currentObject = m_interfacedObjects[objIdx];

	if (event->button == 1)
	{
		if (event->type == GDK_BUTTON_PRESS)	// Simple click
		{
			if (m_currentObject.m_ID == OV_UndefinedIdentifier)
			{
				if (m_shiftPressed) { m_currentMode = Mode_MoveScenario; }
				else
				{
					if (m_controlPressed) { m_currentMode = Mode_SelectionAdd; }
					else { m_currentMode = Mode_Selection; }
				}
			}
			else
			{
				if (m_currentObject.m_ConnectorType == Box_Input || m_currentObject.m_ConnectorType == Box_Output) { m_currentMode = Mode_Connect; }
				else
				{
					m_currentMode = Mode_MoveSelection;
					/*
					if (m_controlPressed) { m_interfacedObjects[m_currentObject.m_id]=!m_interfacedObjects[m_currentObject.m_id]; }
					else
					{
						m_currentObject.clear();
						m_currentObject[m_oCurrentObject.m_id]=true;
					}
					*/
				}
			}
		}
		else if (event->type == GDK_2BUTTON_PRESS)	// Double click
		{
			if (m_currentObject.m_ID != OV_UndefinedIdentifier)
			{
				m_currentMode    = Mode_EditSettings;
				m_shiftPressed   = false;
				m_controlPressed = false;
				m_altPressed     = false;
				m_aPressed       = false;
				m_wPressed       = false;

				if (m_currentObject.m_ConnectorType == Box_Input || m_currentObject.m_ConnectorType == Box_Output)
				{
					IBox* box = m_Scenario.getBoxDetails(m_currentObject.m_ID);
					if (box)
					{
						if ((m_currentObject.m_ConnectorType == Box_Input && box->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
							|| (m_currentObject.m_ConnectorType == Box_Output && box->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)))
						{
							CConnectorEditor editor(m_kernelCtx, *box, m_currentObject.m_ConnectorType, m_currentObject.m_ConnectorIdx,
													m_currentObject.m_ConnectorType == Box_Input ? "Edit Input" : "Edit Output", m_guiFilename.c_str());
							if (editor.run()) { this->snapshotCB(); }
						}
					}
				}
				else
				{
					if (m_Scenario.isBox(m_currentObject.m_ID))
					{
						IBox* box = m_Scenario.getBoxDetails(m_currentObject.m_ID);
						if (box)
						{
							CBoxConfigurationDialog dialog(m_kernelCtx, *box, m_guiFilename.c_str(), m_guiSettingsFilename.c_str(), false);
							if (dialog.run()) { this->snapshotCB(); }
						}
					}
					if (m_Scenario.isComment(m_currentObject.m_ID))
					{
						IComment* comment = m_Scenario.getCommentDetails(m_currentObject.m_ID);
						if (comment)
						{
							CCommentEditorDialog dialog(m_kernelCtx, *comment, m_guiFilename.c_str());
							if (dialog.run()) { this->snapshotCB(); }
						}
					}
				}
			}
		}
	}
	else if (event->button == 3) // right click
	{
		if (event->type == GDK_BUTTON_PRESS)
		{
			const auto unused = size_t(-1);
			GtkMenu* menu     = GTK_MENU(gtk_menu_new());
			m_boxCtxMenuCBs.clear();

			// -------------- SELECTION -----------

			if (this->hasSelection()) { addNewImageMenuItemWithCB(menu, GTK_STOCK_CUT, "cut", context_menu_cb, nullptr, ContextMenu_SelectionCut, unused); }
			if (this->hasSelection()) { addNewImageMenuItemWithCB(menu, GTK_STOCK_COPY, "copy", context_menu_cb, nullptr, ContextMenu_SelectionCopy, unused); }
			if ((m_Application.m_ClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
				|| (m_Application.m_ClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier))
			{
				addNewImageMenuItemWithCB(menu, GTK_STOCK_PASTE, "paste", context_menu_cb, nullptr, ContextMenu_SelectionPaste, unused);
			}
			if (this->hasSelection())
			{
				addNewImageMenuItemWithCB(menu, GTK_STOCK_DELETE, "delete", context_menu_cb, nullptr, ContextMenu_SelectionDelete, unused);
			}

			if (m_currentObject.m_ID != OV_UndefinedIdentifier && m_Scenario.isBox(m_currentObject.m_ID))
			{
				IBox* box = m_Scenario.getBoxDetails(m_currentObject.m_ID);
				if (box)
				{
					if (!m_boxCtxMenuCBs.empty()) { gtk_menu_add_separator_menu_item(menu); }

					bool toBeUpdated                  = box->hasAttribute(OV_AttributeId_Box_ToBeUpdated);
					bool pendingDeprecatedInterfacors = box->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors);

					// -------------- INPUTS --------------
					bool canAddInput             = box->hasAttribute(OV_AttributeId_Box_FlagCanAddInput);
					bool canModifyInput          = box->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput);
					bool canConnectScenarioInput = (box->getInputCount() > 0 && m_Scenario.getInputCount() > 0);
					if (!pendingDeprecatedInterfacors && !toBeUpdated && (canAddInput || canModifyInput || canConnectScenarioInput))
					{
						size_t nFixedInput = 0;
						sscanf(box->getAttributeValue(OV_AttributeId_Box_InitialInputCount).toASCIIString(), "%d", &nFixedInput);
						GtkMenu* menuInput      = GTK_MENU(gtk_menu_new());
						GtkImageMenuItem* input = gtk_menu_add_new_image_menu_item(menu, GTK_STOCK_PROPERTIES, "inputs");
						for (size_t i = 0; i < box->getInputCount(); ++i)
						{
							CString name;
							CIdentifier typeID, id;
							box->getInputName(i, name);
							box->getInputType(i, typeID);
							id                         = box->getIdentifier();
							const string str           = std::to_string(i + 1) + " : " + name.toASCIIString();
							GtkImageMenuItem* menuItem = gtk_menu_add_new_image_menu_item(menuInput, GTK_STOCK_PROPERTIES, str.c_str());

							GtkMenu* menuAction = GTK_MENU(gtk_menu_new());

							if (canConnectScenarioInput)
							{
								for (size_t j = 0; j < m_Scenario.getInputCount(); ++j)
								{
									CString scenarioInputName;
									CIdentifier boxID, inputTypeID;
									auto idx = size_t(-1);
									m_Scenario.getInputName(j, scenarioInputName);
									m_Scenario.getInputType(j, inputTypeID);
									m_Scenario.getScenarioInputLink(j, boxID, idx);
									const string str2 = std::to_string(j + 1) + " : " + scenarioInputName.toASCIIString();
									if (boxID == id && idx == i)
									{
										addNewImageMenuItemWithCBGeneric(menuAction, GTK_STOCK_DISCONNECT, ("disconnect from " + str2).c_str(),
																		 context_menu_cb, box, ContextMenu_BoxDisconnectScenarioInput, i, j);
									}
									else
									{
										if (m_kernelCtx.getTypeManager().isDerivedFromStream(inputTypeID, typeID))
										{
											addNewImageMenuItemWithCBGeneric(menuAction, GTK_STOCK_CONNECT, ("connect to " + str2).c_str(),
																			 context_menu_cb, box, ContextMenu_BoxConnectScenarioInput, i, j);
										}
									}
								}
							}

							if (canModifyInput)
							{
								addNewImageMenuItemWithCB(menuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, box,
														  ContextMenu_BoxEditInput, i);
							}

							if (canAddInput && nFixedInput <= i)
							{
								addNewImageMenuItemWithCB(menuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, box,
														  ContextMenu_BoxRemoveInput, i);
							}

							if (gtk_container_get_children_count(GTK_CONTAINER(menuAction)) > 0)
							{
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), GTK_WIDGET(menuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(menuItem), false); }
						}
						gtk_menu_add_separator_menu_item(menuInput);
						if (canAddInput)
						{
							addNewImageMenuItemWithCB(menuInput, GTK_STOCK_ADD, "new...", context_menu_cb, box, ContextMenu_BoxAddInput, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(input), GTK_WIDGET(menuInput));
					}

					// -------------- OUTPUTS --------------

					bool canAddOutput             = box->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput);
					bool canModifyOutput          = box->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput);
					bool canConnectScenarioOutput = (box->getOutputCount() > 0 && m_Scenario.getOutputCount() > 0);
					if (!pendingDeprecatedInterfacors && !toBeUpdated && (canAddOutput || canModifyOutput || canConnectScenarioOutput))
					{
						size_t nFixedOutput = 0;
						sscanf(box->getAttributeValue(OV_AttributeId_Box_InitialOutputCount).toASCIIString(), "%d", &nFixedOutput);
						GtkImageMenuItem* itemOutput = gtk_menu_add_new_image_menu_item(menu, GTK_STOCK_PROPERTIES, "outputs");
						GtkMenu* menuOutput          = GTK_MENU(gtk_menu_new());
						for (size_t i = 0; i < box->getOutputCount(); ++i)
						{
							CString name;
							CIdentifier typeID, id;
							box->getOutputName(i, name);
							box->getOutputType(i, typeID);
							id                         = box->getIdentifier();
							const string str           = std::to_string(i + 1) + " : " + name.toASCIIString();
							GtkImageMenuItem* menuItem = gtk_menu_add_new_image_menu_item(menuOutput, GTK_STOCK_PROPERTIES, str.c_str());

							GtkMenu* menuAction = GTK_MENU(gtk_menu_new());

							if (canConnectScenarioOutput)
							{
								for (size_t j = 0; j < m_Scenario.getOutputCount(); ++j)
								{
									CString scenarioOutputName;
									CIdentifier boxID, outputTypeID;
									auto idx = size_t(-1);
									m_Scenario.getOutputName(j, scenarioOutputName);
									m_Scenario.getOutputType(j, outputTypeID);
									m_Scenario.getScenarioOutputLink(j, boxID, idx);
									const string str2 = std::to_string(j + 1) + " : " + scenarioOutputName.toASCIIString();
									if (boxID == id && idx == i)
									{
										addNewImageMenuItemWithCBGeneric(menuAction, GTK_STOCK_DISCONNECT, ("disconnect from " + str2).c_str(),
																		 context_menu_cb, box, ContextMenu_BoxDisconnectScenarioOutput, i, j);
									}
									else if (m_kernelCtx.getTypeManager().isDerivedFromStream(typeID, outputTypeID))
									{
										addNewImageMenuItemWithCBGeneric(menuAction, GTK_STOCK_CONNECT, ("connect to " + str2).c_str(),
																		 context_menu_cb, box, ContextMenu_BoxConnectScenarioOutput, i, j);
									}
								}
							}

							if (canModifyOutput)
							{
								addNewImageMenuItemWithCB(menuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, box,
														  ContextMenu_BoxEditOutput, i);
							}
							if (canAddOutput && nFixedOutput <= i)
							{
								addNewImageMenuItemWithCB(menuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, box,
														  ContextMenu_BoxRemoveOutput, i);
							}

							if (gtk_container_get_children_count(GTK_CONTAINER(menuAction)) > 0)
							{
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), GTK_WIDGET(menuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(menuItem), false); }
						}
						gtk_menu_add_separator_menu_item(menuOutput);
						if (canAddOutput)
						{
							addNewImageMenuItemWithCB(menuOutput, GTK_STOCK_ADD, "new...", context_menu_cb, box, ContextMenu_BoxAddOutput, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(itemOutput), GTK_WIDGET(menuOutput));
					}

					// -------------- SETTINGS --------------

					bool canAddSetting    = box->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting);
					bool canModifySetting = box->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting);
					if (!pendingDeprecatedInterfacors && !toBeUpdated && (canAddSetting || canModifySetting))
					{
						size_t nFixedSetting = 0;
						sscanf(box->getAttributeValue(OV_AttributeId_Box_InitialSettingCount).toASCIIString(), "%d", &nFixedSetting);
						GtkImageMenuItem* itemSetting = gtk_menu_add_new_image_menu_item(menu, GTK_STOCK_PROPERTIES, "modify settings");
						GtkMenu* menuSetting          = GTK_MENU(gtk_menu_new());
						for (size_t i = 0; i < box->getSettingCount(); ++i)
						{
							CString name;
							CIdentifier typeID;
							box->getSettingName(i, name);
							box->getSettingType(i, typeID);
							const string str           = std::to_string(i + 1) + " : " + name.toASCIIString();
							GtkImageMenuItem* menuItem = gtk_menu_add_new_image_menu_item(menuSetting, GTK_STOCK_PROPERTIES, str.c_str());

							if (canModifySetting || nFixedSetting <= i)
							{
								GtkMenu* menuAction = GTK_MENU(gtk_menu_new());
								if (canModifySetting)
								{
									addNewImageMenuItemWithCB(menuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, box,
															  ContextMenu_BoxEditSetting, i);
								}
								if (canAddSetting && nFixedSetting <= i)
								{
									addNewImageMenuItemWithCB(menuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, box,
															  ContextMenu_BoxRemoveSetting, i);
								}
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), GTK_WIDGET(menuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(menuItem), false); }
						}
						gtk_menu_add_separator_menu_item(menuSetting);
						if (canAddSetting)
						{
							addNewImageMenuItemWithCB(menuSetting, GTK_STOCK_ADD, "new...", context_menu_cb, box, ContextMenu_BoxAddSetting, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(itemSetting), GTK_WIDGET(menuSetting));
					}

					// -------------- ABOUT / RENAME --------------

					if (!m_boxCtxMenuCBs.empty()) { gtk_menu_add_separator_menu_item(menu); }
					if (box->hasAttribute(OV_AttributeId_Box_ToBeUpdated))
					{
						auto updateMenuItem = addNewImageMenuItemWithCB(menu, GTK_STOCK_REFRESH, "update box", context_menu_cb, box, ContextMenu_BoxUpdate, unused);
						if (box->hasAttribute(OV_AttributeId_Box_FlagNeedsManualUpdate)
							|| box->hasAttribute(OV_AttributeId_Box_FlagCanAddInput)
							|| box->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput)
							|| box->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting)
							|| box->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput)
							|| box->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)
							|| box->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
						{
							gtk_widget_set_sensitive(GTK_WIDGET(updateMenuItem), FALSE);
							gtk_widget_set_tooltip_text(GTK_WIDGET(updateMenuItem), "Box must be manually updated due to its complexity.");
						}
					}
					if (box->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors))
					{
						addNewImageMenuItemWithCB(menu, GTK_STOCK_REFRESH, "remove deprecated I/O/S", context_menu_cb, box,
												  ContextMenu_BoxRemoveDeprecatedInterfacors, unused);
					}
					addNewImageMenuItemWithCB(menu, GTK_STOCK_EDIT, "rename box...", context_menu_cb, box, ContextMenu_BoxRename, unused);
					if (box->getSettingCount() != 0)
					{
						addNewImageMenuItemWithCB(menu, GTK_STOCK_PREFERENCES, "configure box...", context_menu_cb, box, ContextMenu_BoxConfigure, unused);
					}
					// Add this option only if the user has the authorization to open a metabox
					if (box->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
					{
						CIdentifier id;
						id.fromString(box->getAttributeValue(OVP_AttributeId_Metabox_ID));

						std::string path(m_kernelCtx.getMetaboxManager().getMetaboxFilePath(id).toASCIIString());
						std::string ext    = boost::filesystem::extension(path);
						bool canImportFile = false;

						CString fileExt;
						while ((fileExt = m_kernelCtx.getScenarioManager().getNextScenarioImporter(OVD_ScenarioImportContext_OpenScenario, fileExt)) !=
							   CString(""))
						{
							if (ext == fileExt.toASCIIString())
							{
								canImportFile = true;
								break;
							}
						}

						if (canImportFile)
						{
							addNewImageMenuItemWithCB(menu, GTK_STOCK_PREFERENCES, "open this meta box in editor", context_menu_cb, box,
													  ContextMenu_BoxEditMetabox, unused);
						}
					}
					addNewImageMenuItemWithCB(menu, GTK_STOCK_CONNECT, "enable box", context_menu_cb, box, ContextMenu_BoxEnable, unused);
					addNewImageMenuItemWithCB(menu, GTK_STOCK_DISCONNECT, "disable box", context_menu_cb, box, ContextMenu_BoxDisable, unused);
					addNewImageMenuItemWithCB(menu, GTK_STOCK_CUT, "delete box", context_menu_cb, box, ContextMenu_BoxDelete, unused);
					addNewImageMenuItemWithCB(menu, GTK_STOCK_HELP, "box documentation...", context_menu_cb, box, ContextMenu_BoxDocumentation, unused);
					addNewImageMenuItemWithCB(menu, GTK_STOCK_ABOUT, "about box...", context_menu_cb, box, ContextMenu_BoxAbout, unused);
				}
			}

			gtk_menu_add_separator_menu_item(menu);
			addNewImageMenuItemWithCB(menu, GTK_STOCK_EDIT, "add comment to scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAddComment, unused);
			addNewImageMenuItemWithCB(menu, GTK_STOCK_ABOUT, "about scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAbout, unused);

			// -------------- RUN --------------

			gtk_widget_show_all(GTK_WIDGET(menu));
			gtk_menu_popup(menu, nullptr, nullptr, nullptr, nullptr, 3, event->time);
			if (m_boxCtxMenuCBs.empty()) { gtk_menu_popdown(menu); }
		}
	}

	this->redraw();
}

void CInterfacedScenario::scenarioDrawingAreaButtonReleasedCB(GtkWidget* /*widget*/, GdkEventButton* event)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonReleasedCB\n";

	if (this->isLocked()) { return; }

	m_buttonPressed &= !((event->type == GDK_BUTTON_RELEASE) && (event->button == 1));
	m_releaseMouseX = event->x;
	m_releaseMouseY = event->y;

	if (m_currentMode != Mode_None)
	{
		const int startX = int(std::min(m_pressMouseX, m_currentMouseX));
		const int startY = int(std::min(m_pressMouseY, m_currentMouseY));
		const int sizeX  = int(std::max(m_pressMouseX - m_currentMouseX, m_currentMouseX - m_pressMouseX));
		const int sizeY  = int(std::max(m_pressMouseY - m_currentMouseY, m_currentMouseY - m_pressMouseY));

		if (m_currentMode == Mode_Selection || m_currentMode == Mode_SelectionAdd)
		{
			if (m_currentMode == Mode_Selection) { m_SelectedObjects.clear(); }
			pickInterfacedObject(startX, startY, sizeX, sizeY);
		}
		if (m_currentMode == Mode_Connect)
		{
			bool isActuallyConnecting             = false;
			const bool connectionIsMessage        = false;
			const size_t interfacedObjectId       = pickInterfacedObject(int(m_releaseMouseX), int(m_releaseMouseY));
			const CInterfacedObject currentObject = m_interfacedObjects[interfacedObjectId];
			CInterfacedObject srcObject;
			CInterfacedObject dstObject;
			if (currentObject.m_ConnectorType == Box_Output && m_currentObject.m_ConnectorType == Box_Input)
			{
				srcObject            = currentObject;
				dstObject            = m_currentObject;
				isActuallyConnecting = true;
			}
			if (currentObject.m_ConnectorType == Box_Input && m_currentObject.m_ConnectorType == Box_Output)
			{
				srcObject            = m_currentObject;
				dstObject            = currentObject;
				isActuallyConnecting = true;
			}
			//
			if (isActuallyConnecting)
			{
				CIdentifier srcTypeID;
				CIdentifier dstTypeID;
				const IBox* srcBox = m_Scenario.getBoxDetails(srcObject.m_ID);
				const IBox* dstBox = m_Scenario.getBoxDetails(dstObject.m_ID);
				if (srcBox && dstBox)
				{
					srcBox->getOutputType(srcObject.m_ConnectorIdx, srcTypeID);
					dstBox->getInputType(dstObject.m_ConnectorIdx, dstTypeID);

					bool hasDeprecatedInput = false;
					srcBox->getInterfacorDeprecatedStatus(Output, srcObject.m_ConnectorIdx, hasDeprecatedInput);
					bool hasDeprecatedOutput = false;
					dstBox->getInterfacorDeprecatedStatus(Input, dstObject.m_ConnectorIdx, hasDeprecatedOutput);

					if ((m_kernelCtx.getTypeManager().isDerivedFromStream(srcTypeID, dstTypeID)
						 || m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_AllowUpCastConnection}", false)) && (!connectionIsMessage))
					{
						if (!hasDeprecatedInput && !hasDeprecatedOutput)
						{
							CIdentifier id;
							m_Scenario.connect(id, srcObject.m_ID, srcObject.m_ConnectorIdx, dstObject.m_ID, dstObject.m_ConnectorIdx, OV_UndefinedIdentifier);
							this->snapshotCB();
						}
						else { m_kernelCtx.getLogManager() << LogLevel_Warning << "Cannot connect to/from deprecated I/O\n"; }
					}
					else { m_kernelCtx.getLogManager() << LogLevel_Warning << "Invalid connection\n"; }
				}
			}
		}
		if (m_currentMode == Mode_MoveSelection)
		{
			if (sizeX == 0 && sizeY == 0)
			{
				if (m_controlPressed)
				{
					if (m_SelectedObjects.count(m_currentObject.m_ID)) { m_SelectedObjects.erase(m_currentObject.m_ID); }
					else { m_SelectedObjects.insert(m_currentObject.m_ID); }
				}
				else
				{
					m_SelectedObjects.clear();
					m_SelectedObjects.insert(m_currentObject.m_ID);
				}
			}
			else
			{
				for (auto& id : m_SelectedObjects)
				{
					if (m_Scenario.isBox(id))
					{
						CBoxProxy proxy(m_kernelCtx, m_Scenario, id);
						proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0), int((proxy.getYCenter() + 8) & 0xfffffff0));
					}
					if (m_Scenario.isComment(id))
					{
						CCommentProxy proxy(m_kernelCtx, m_Scenario, id);
						proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0), int((proxy.getYCenter() + 8) & 0xfffffff0));
					}
				}
				this->snapshotCB();
			}
		}
		this->redraw();
	}

	m_currentMode = Mode_None;
}

void CInterfacedScenario::scenarioDrawingAreaKeyPressEventCB(GtkWidget* /*widget*/, GdkEventKey* event)
{
	m_shiftPressed |= (event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R);
	m_controlPressed |= (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R);
	m_altPressed |= (event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R);
	m_aPressed |= (event->keyval == GDK_a || event->keyval == GDK_A);
	m_wPressed |= (event->keyval == GDK_w || event->keyval == GDK_W);

	// m_kernelCtx.getLogManager() << LogLevel_Info << "Key pressed " << (size_t) event->keyval << "\n";
	/*
		if((event->keyval==GDK_Z || event->keyval==GDK_z) && m_controlPressed) { this->undoCB(); }

		if((event->keyval==GDK_Y || event->keyval==GDK_y) && m_controlPressed) { this->redoCB(); }
	*/
	// CTRL+A = select all
	if (m_aPressed && m_controlPressed && !m_shiftPressed && !m_altPressed)
	{
		CIdentifier id;
		while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(id); }
		while ((id = m_Scenario.getNextLinkIdentifier(id)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(id); }
		while ((id = m_Scenario.getNextCommentIdentifier(id)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(id); }
		this->redraw();
	}

	//CTRL+W : close current scenario
	if (m_wPressed && m_controlPressed && !m_shiftPressed && !m_altPressed)
	{
		m_Application.closeScenarioCB(this);
		return;
	}

	if ((event->keyval == GDK_C || event->keyval == GDK_c) && m_currentMode == Mode_None)
	{
		gint x = 0;
		gint y = 0;
		gdk_window_get_pointer(GTK_WIDGET(m_scenarioDrawingArea)->window, &x, &y, nullptr);

		this->addCommentCB(x, y);
	}

	if (event->keyval == GDK_F12 && m_shiftPressed)
	{
		CIdentifier id;
		while ((id = m_Scenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
		{
			IBox* box               = m_Scenario.getBoxDetails(id);
			CIdentifier algorithmID = box->getAlgorithmClassIdentifier();
			CIdentifier hashValue   = m_kernelCtx.getPluginManager().getPluginObjectHashValue(algorithmID);
			if (box->hasAttribute(OV_AttributeId_Box_InitialPrototypeHashValue))
			{
				box->setAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue, hashValue.toString());
			}
			else { box->addAttribute(OV_AttributeId_Box_InitialPrototypeHashValue, hashValue.toString()); }
		}

		this->redraw();
		this->snapshotCB();
	}

	// F1 : browse documentation
	if (event->keyval == GDK_F1)
	{
		bool hasDoc = false;
		for (auto& objectId : m_SelectedObjects)
		{
			if (m_Scenario.isBox(objectId))
			{
				browseBoxDocumentation(objectId);
				hasDoc = true;
			}
		}

		if (!hasDoc)
		{
			const CString fullUrl = m_Scenario.getAttributeValue(OV_AttributeId_Scenario_DocumentationPage);
			if (fullUrl != CString(""))
			{
				browseURL(fullUrl, m_kernelCtx.getConfigurationManager().expand("${Designer_WebBrowserCommand}"),
						  m_kernelCtx.getConfigurationManager().expand("${Designer_WebBrowserCommandPostfix}"));
			}
			else { m_kernelCtx.getLogManager() << LogLevel_Info << "The scenario does not define a documentation page.\n"; }
		}
	}

	// F2 : rename all selected box(es)
	if (event->keyval == GDK_F2) { contextMenuBoxRenameAllCB(); }

	// F8 : toggle enable/disable on all selected box(es)
	if (event->keyval == GDK_F3)
	{
		contextMenuBoxToggleEnableAllCB();
		this->redraw();
	}

	//The shortcuts respect the order in the toolbar

	// F7 :play/pause
	if (event->keyval == GDK_F7)
	{
		if (m_Application.getCurrentInterfacedScenario()->m_PlayerStatus == PlayerStatus_Play) { m_Application.pauseScenarioCB(); }
		else { m_Application.playScenarioCB(); }
	}
	// F6 : step
	if (event->keyval == GDK_F6) { m_Application.nextScenarioCB(); }
	// F8 :fastforward
	if (event->keyval == GDK_F8) { m_Application.forwardScenarioCB(); }
	// F5 : stop
	if (event->keyval == GDK_F5) { m_Application.stopScenarioCB(); }

	m_kernelCtx.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaKeyPressEventCB (" << (m_shiftPressed ? "true" : "false") << "|"
			<< (m_controlPressed ? "true" : "false") << "|" << (m_altPressed ? "true" : "false") << "|" << (m_aPressed ? "true" : "false") << "|"
			<< (m_wPressed ? "true" : "false") << "|" << ")\n";

	if (this->isLocked()) { return; }

#if defined TARGET_OS_Windows || defined TARGET_OS_Linux
	if (event->keyval == GDK_Delete || event->keyval == GDK_KP_Delete)
#elif defined TARGET_OS_MacOS
	if (event->keyval == GDK_BackSpace)
#endif
	{
		this->deleteSelection();
	}
}

void CInterfacedScenario::scenarioDrawingAreaKeyReleaseEventCB(GtkWidget* /*widget*/, GdkEventKey* event)
{
	m_shiftPressed &= !(event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R);
	m_controlPressed &= !(event->keyval == GDK_Control_L || event->keyval == GDK_Control_R);
	m_altPressed &= !(event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R);
	m_aPressed &= !(event->keyval == GDK_A || event->keyval == GDK_a);
	m_wPressed &= !(event->keyval == GDK_W || event->keyval == GDK_w);

	m_kernelCtx.getLogManager() << LogLevel_Debug
			<< "scenarioDrawingAreaKeyReleaseEventCB ("
			<< (m_shiftPressed ? "true" : "false") << "|"
			<< (m_controlPressed ? "true" : "false") << "|"
			<< (m_altPressed ? "true" : "false") << "|"
			<< (m_aPressed ? "true" : "false") << "|"
			<< (m_wPressed ? "true" : "false") << "|"
			<< ")\n";

	//if (this->isLocked()) { return; }
	// ...
}

void CInterfacedScenario::copySelection()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "copySelection\n";

	// Prepares copy
	map<CIdentifier, CIdentifier> mapping;
	m_Application.m_ClipboardScenario->clear();

	// Copies boxes to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId))
		{
			CIdentifier id;
			const IBox* box = m_Scenario.getBoxDetails(objectId);
			m_Application.m_ClipboardScenario->addBox(id, *box, objectId);
			mapping[objectId] = id;
		}
	}

	// Copies comments to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isComment(objectId))
		{
			CIdentifier id;
			const IComment* comment = m_Scenario.getCommentDetails(objectId);
			m_Application.m_ClipboardScenario->addComment(id, *comment, objectId);
			mapping[objectId] = id;
		}
	}

	// Copies links to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isLink(objectId))
		{
			CIdentifier id;
			const ILink* link = m_Scenario.getLinkDetails(objectId);

			// Connect link only if the source and target boxes are copied
			if (mapping.find(link->getSourceBoxIdentifier()) != mapping.end()
				&& mapping.find(link->getTargetBoxIdentifier()) != mapping.end())
			{
				m_Application.m_ClipboardScenario->connect(id, mapping[link->getSourceBoxIdentifier()], link->getSourceBoxOutputIndex(),
														   mapping[link->getTargetBoxIdentifier()], link->getTargetBoxInputIndex(),
														   link->getIdentifier());
			}
		}
	}
}

void CInterfacedScenario::cutSelection()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "cutSelection\n";

	this->copySelection();
	this->deleteSelection();
}

void CInterfacedScenario::pasteSelection()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "pasteSelection\n";

	// Prepares paste
	CIdentifier id;
	map<CIdentifier, CIdentifier> mapping;
	// int centerX = 0, centerY = 0;
	int mostTLCopiedBoxCenterX = 1 << 15;	// most top most left 
	int mostTLCopiedBoxCenterY = 1 << 15;	// most top most left 
	// std::cout << "Mouse position : " << m_currentMouseX << "/" << m_currentMouseY << std::endl;

	// Pastes boxes from clipboard
	while ((id = m_Application.m_ClipboardScenario->getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
	{
		CIdentifier newID;
		IBox* box = m_Application.m_ClipboardScenario->getBoxDetails(id);
		m_Scenario.addBox(newID, *box, id);
		mapping[id] = newID;

		// Updates visualization manager
		CIdentifier boxAlgorithmID   = box->getAlgorithmClassIdentifier();
		const IPluginObjectDesc* pod = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(boxAlgorithmID);

		// If a visualization box was dropped, add it in window manager
		if (pod && pod->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxAdded(m_Scenario.getBoxDetails(newID)); }
		}

		CBoxProxy proxy(m_kernelCtx, m_Scenario, newID);

		// get the position of the topmost-leftmost box (always position on an actual box so when user pastes he sees something)
		if (proxy.getXCenter() < mostTLCopiedBoxCenterX && proxy.getXCenter() < mostTLCopiedBoxCenterY)
		{
			mostTLCopiedBoxCenterX = proxy.getXCenter();
			mostTLCopiedBoxCenterY = proxy.getYCenter();
		}
	}

	// Pastes comments from clipboard
	while ((id = m_Application.m_ClipboardScenario->getNextCommentIdentifier(id)) != OV_UndefinedIdentifier)
	{
		CIdentifier newID;
		IComment* comment = m_Application.m_ClipboardScenario->getCommentDetails(id);
		m_Scenario.addComment(newID, *comment, id);
		mapping[id] = newID;

		CCommentProxy proxy(m_kernelCtx, m_Scenario, newID);

		if (proxy.getXCenter() < mostTLCopiedBoxCenterX && proxy.getYCenter() < mostTLCopiedBoxCenterY)
		{
			mostTLCopiedBoxCenterX = proxy.getXCenter();
			mostTLCopiedBoxCenterY = proxy.getYCenter();
		}
	}

	// Pastes links from clipboard
	while ((id = m_Application.m_ClipboardScenario->getNextLinkIdentifier(id)) != OV_UndefinedIdentifier)
	{
		CIdentifier newID;
		ILink* link = m_Application.m_ClipboardScenario->getLinkDetails(id);
		m_Scenario.connect(newID, mapping[link->getSourceBoxIdentifier()], link->getSourceBoxOutputIndex(),
						   mapping[link->getTargetBoxIdentifier()], link->getTargetBoxInputIndex(), link->getIdentifier());
	}

	// Makes pasted stuff the default selection
	// Moves boxes under cursor
	// Moves comments under cursor
	if (m_Application.m_ClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier
		|| m_Application.m_ClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
	{
		m_SelectedObjects.clear();
		for (auto& it : mapping)
		{
			m_SelectedObjects.insert(it.second);

			if (m_Scenario.isBox(it.second))
			{
				// Moves boxes under cursor
				CBoxProxy proxy(m_kernelCtx, m_Scenario, it.second);
				proxy.setCenter(int(proxy.getXCenter() + m_currentMouseX) - mostTLCopiedBoxCenterX - m_viewOffsetX,
								int(proxy.getYCenter() + m_currentMouseY) - mostTLCopiedBoxCenterY - m_viewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns boxes on grid
				proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0L), int((proxy.getYCenter() + 8) & 0xfffffff0L));
			}

			if (m_Scenario.isComment(it.second))
			{
				// Moves commentes under cursor
				CCommentProxy proxy(m_kernelCtx, m_Scenario, it.second);
				proxy.setCenter(int(proxy.getXCenter() + m_currentMouseX) - mostTLCopiedBoxCenterX - m_viewOffsetX,
								int(proxy.getYCenter() + m_currentMouseY) - mostTLCopiedBoxCenterY - m_viewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns commentes on grid
				proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0L), int((proxy.getYCenter() + 8) & 0xfffffff0L));
			}
		}
	}

	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::deleteSelection()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "deleteSelection\n";
	for (auto& id : m_SelectedObjects)
	{
		if (m_Scenario.isBox(id)) { this->deleteBox(id); }
		if (m_Scenario.isComment(id))
		{
			// removes comment from scenario
			m_Scenario.removeComment(id);
		}
		if (m_Scenario.isLink(id))
		{
			// removes link from scenario
			m_Scenario.disconnect(id);
		}
	}
	m_SelectedObjects.clear();

	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::deleteBox(const CIdentifier& boxID)
{
	// removes visualization box from window manager
	if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxRemoved(boxID); }

	// removes box from scenario
	m_Scenario.removeBox(boxID);
}


void CInterfacedScenario::contextMenuBoxUpdateCB(IBox& box)
{
	m_Scenario.updateBox(box.getIdentifier());
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxUpdateCB\n";
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRemoveDeprecatedInterfacorsCB(IBox& box)
{
	m_Scenario.removeDeprecatedInterfacorsFromBox(box.getIdentifier());
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveDeprecatedInterfacorsCB\n";
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRenameCB(IBox& box)
{
	const IPluginObjectDesc* pod = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(box.getAlgorithmClassIdentifier());
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRenameCB\n";

	if (box.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier id;
		id.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
		pod = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(id);
	}

	CRenameDialog rename(m_kernelCtx, box.getName(), pod ? pod->getName() : box.getName(), m_guiFilename.c_str());
	if (rename.run())
	{
		box.setName(rename.getResult());

		//check whether it is a visualization box
		const CIdentifier id          = box.getAlgorithmClassIdentifier();
		const IPluginObjectDesc* desc = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(id);

		//if a visualization box was renamed, tell window manager about it
		if (desc && desc->hasFunctionality(OVD_Functionality_Visualization))
		{
			if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxRenamed(box.getIdentifier()); }
		}
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxRenameAllCB()
{
	//we find all selected boxes
	map<CIdentifier, CIdentifier> selectedBoxes; // map(object,class)
	for (auto& id : m_SelectedObjects) { if (m_Scenario.isBox(id)) { selectedBoxes[id] = m_Scenario.getBoxDetails(id)->getAlgorithmClassIdentifier(); } }

	if (!selectedBoxes.empty())
	{
		bool dialogOk   = true;
		bool firstBox   = true;
		CString newName = "";
		for (auto it = selectedBoxes.begin(); it != selectedBoxes.end() && dialogOk; ++it)
		{
			if (it->second != OV_UndefinedIdentifier)
			{
				if (m_kernelCtx.getPluginManager().canCreatePluginObject(it->second) || it->second == OVP_ClassId_BoxAlgorithm_Metabox)
				{
					IBox* box = m_Scenario.getBoxDetails(it->first);
					if (firstBox)
					{
						firstBox                     = false;
						const IPluginObjectDesc* pod = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(
							box->getAlgorithmClassIdentifier());
						if (box->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
						{
							CIdentifier metaboxId;
							metaboxId.fromString(box->getAttributeValue(OVP_AttributeId_Metabox_ID));
							pod = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
						}

						CRenameDialog rename(m_kernelCtx, box->getName(),
											 pod ? pod->getName() : box->getName(), m_guiFilename.c_str());
						if (rename.run()) { newName = rename.getResult(); }
						else
						{
							// no rename at all.
							dialogOk = false;
						}
					}
					if (dialogOk)
					{
						box->setName(newName);

						//check whether it is a visualization box
						CIdentifier id               = box->getAlgorithmClassIdentifier();
						const IPluginObjectDesc* pod = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(id);

						//if a visualization box was renamed, tell window manager about it
						if (pod && pod->hasFunctionality(OVD_Functionality_Visualization))
						{
							if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxRenamed(box->getIdentifier()); }
						}
					}
				}
			}
		}
		if (dialogOk) { this->snapshotCB(); }
	}
}

void CInterfacedScenario::contextMenuBoxToggleEnableAllCB()
{
	//we find all selected boxes
	for (const auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId))
		{
			TAttributeHandler handler(*m_Scenario.getBoxDetails(objectId));
			if (handler.hasAttribute(OV_AttributeId_Box_Disabled)) { handler.removeAttribute(OV_AttributeId_Box_Disabled); }
			else { handler.addAttribute(OV_AttributeId_Box_Disabled, 1); }
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxEnableAllCB()
{
	//we find all selected boxes
	for (const auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId))
		{
			TAttributeHandler handler(*m_Scenario.getBoxDetails(objectId));
			if (handler.hasAttribute(OV_AttributeId_Box_Disabled)) { handler.removeAttribute(OV_AttributeId_Box_Disabled); }
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxDisableAllCB()
{
	//we find all selected boxes
	for (const auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId))
		{
			TAttributeHandler handler(*m_Scenario.getBoxDetails(objectId));
			if (!handler.hasAttribute(OV_AttributeId_Box_Disabled)) { handler.addAttribute(OV_AttributeId_Box_Disabled, 1); }
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddInputCB(IBox& box)
{
	if (box.hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors))
	{
		gtk_dialog_run(GTK_DIALOG(m_errorPendingDeprecatedInterfacorsDialog));
		return;
	}
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxAddInputCB\n";
	box.addInput("New input", OV_TypeId_EBMLStream, m_Scenario.getUnusedInputIdentifier());
	if (box.hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
	{
		CConnectorEditor editor(m_kernelCtx, box, Box_Input, box.getInputCount() - 1, "Add Input", m_guiFilename.c_str());
		if (editor.run()) { this->snapshotCB(); }
		else { box.removeInput(box.getInputCount() - 1); }
	}
	else { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxEditInputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditInputCB\n";

	CConnectorEditor editor(m_kernelCtx, box, Box_Input, index, "Edit Input", m_guiFilename.c_str());
	if (editor.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveInputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveInputCB\n";
	box.removeInput(index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddOutputCB(IBox& box)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxAddOutputCB\n";
	box.addOutput("New output", OV_TypeId_EBMLStream, m_Scenario.getUnusedOutputIdentifier());
	if (box.hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput))
	{
		CConnectorEditor editor(m_kernelCtx, box, Box_Output, box.getOutputCount() - 1, "Add Output", m_guiFilename.c_str());
		if (editor.run()) { this->snapshotCB(); }
		else { box.removeOutput(box.getOutputCount() - 1); }
	}
	else { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxEditOutputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditOutputCB\n";

	CConnectorEditor editor(m_kernelCtx, box, Box_Output, index, "Edit Output", m_guiFilename.c_str());
	if (editor.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveOutputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveOutputCB\n";
	box.removeOutput(index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioInputCB(IBox& box, const size_t boxInputIdx, const size_t scenarioInputIdx)
{
	//	m_kernelCtx.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioInputCB : box = " << box.getIdentifier().str() << " box input = " << boxInputIdx << " , scenario input = " << scenarioInputIdx << "\n";
	m_Scenario.setScenarioInputLink(scenarioInputIdx, box.getIdentifier(), boxInputIdx);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioOutputCB(IBox& box, const size_t boxOutputIdx, const size_t scenarioOutputIdx)
{
	//	m_kernelCtx.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioOutputCB : box = " << box.getIdentifier().str() << " box Output = " << boxOutputIdx << " , scenario Output = " << scenarioOutputIdx << "\n";
	m_Scenario.setScenarioOutputLink(scenarioOutputIdx, box.getIdentifier(), boxOutputIdx);
	this->snapshotCB();
}

// Note: In current implementation only the scenarioInputIdx is necessary as it can only be connected to one input
// but to keep things simpler we give it all the info
void CInterfacedScenario::contextMenuBoxDisconnectScenarioInputCB(IBox& box, const size_t boxInputIdx, const size_t scenarioInputIdx)
{
	m_Scenario.removeScenarioInputLink(scenarioInputIdx, box.getIdentifier(), boxInputIdx);
	this->snapshotCB();
}

// Note: In current implementation only the scenarioOutputIdx is necessary as it can only be connected to one output
// but to keep things simpler we give it all the info
void CInterfacedScenario::contextMenuBoxDisconnectScenarioOutputCB(IBox& box, const size_t boxOutputIdx, const size_t scenarioOutputIdx)
{
	m_Scenario.removeScenarioOutputLink(scenarioOutputIdx, box.getIdentifier(), boxOutputIdx);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddSettingCB(IBox& box)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxAddSettingCB\n";
	// Store setting count in case the custom "onSettingAdded" of the box adds more than one setting
	const size_t nOldSettings = box.getSettingCount();
	box.addSetting("New setting", OV_UndefinedIdentifier, "", size_t(-1), false,
				   m_Scenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));
	const size_t nNewSettings = box.getSettingCount();
	// Check that at least one setting was added
	if (nNewSettings > nOldSettings && box.hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
	{
		CSettingEditorDialog dialog(m_kernelCtx, box, nOldSettings, "Add Setting", m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
		if (dialog.run()) { this->snapshotCB(); }
		else { for (size_t i = nOldSettings; i < nNewSettings; ++i) { box.removeSetting(i); } }
	}
	else
	{
		if (nNewSettings > nOldSettings) { this->snapshotCB(); }
		else
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "No setting could be added to the box.\n";
			return;
		}
	}
	// Add an information message to inform the user about the new settings
	m_kernelCtx.getLogManager() << LogLevel_Info << "[" << nNewSettings - nOldSettings << "] new setting(s) was(were) added to the box ["
			<< box.getName().toASCIIString() << "]: ";
	for (size_t i = nOldSettings; i < nNewSettings; ++i)
	{
		CString name;
		box.getSettingName(i, name);
		m_kernelCtx.getLogManager() << "[" << name << "] ";
	}
	m_kernelCtx.getLogManager() << "\n";
	// After adding setting, open configuration so that the user can see the effects.
	CBoxConfigurationDialog dialog(m_kernelCtx, box, m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
	dialog.run();
}

void CInterfacedScenario::contextMenuBoxEditSettingCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditSettingCB\n";
	CSettingEditorDialog dialog(m_kernelCtx, box, index, "Edit Setting", m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
	if (dialog.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveSettingCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveSettingCB\n";
	const size_t nOldSettings = box.getSettingCount();
	if (box.removeSetting(index))
	{
		const size_t nNewSettings = box.getSettingCount();
		this->snapshotCB();
		m_kernelCtx.getLogManager() << LogLevel_Info << "[" << nOldSettings - nNewSettings << "] setting(s) was(were) removed from box ["
				<< box.getName() << "] \n";
	}
	else
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "The setting with index [" << index << "] could not be removed from box ["
				<< box.getName() << "] \n";
	}
}

void CInterfacedScenario::contextMenuBoxConfigureCB(IBox& box)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxConfigureCB\n";
	CBoxConfigurationDialog dialog(m_kernelCtx, box, m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
	dialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAboutCB(IBox& box) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxAboutCB\n";
	if (box.getAlgorithmClassIdentifier() != OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CAboutPluginDialog dialog(m_kernelCtx, box.getAlgorithmClassIdentifier(), m_guiFilename.c_str());
		dialog.run();
	}
	else
	{
		CIdentifier id;
		id.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
		CAboutPluginDialog dialog(m_kernelCtx, m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(id), m_guiFilename.c_str());
		dialog.run();
	}
}

void CInterfacedScenario::contextMenuBoxEditMetaboxCB(IBox& box) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditMetaboxCB\n";

	CIdentifier id;
	id.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
	const CString path(m_kernelCtx.getMetaboxManager().getMetaboxFilePath(id));

	m_Application.openScenario(path.toASCIIString());
}

bool CInterfacedScenario::browseURL(const CString& url, const CString& browserPrefix, const CString& browserPostfix) const
{
	m_kernelCtx.getLogManager() << LogLevel_Trace << "Requesting web browser on URL " << url << "\n";

	const CString cmd = browserPrefix + CString(" \"") + url + CString("\"") + browserPostfix;
	m_kernelCtx.getLogManager() << LogLevel_Debug << "Launching [" << cmd << "]\n";
	const int result = system(cmd.toASCIIString());
	if (result < 0)
	{
		OV_WARNING("Could not launch command [" << cmd << "]\n", m_kernelCtx.getLogManager());
		return false;
	}
	return true;
}

bool CInterfacedScenario::browseBoxDocumentation(const CIdentifier& boxID) const
{
	const CIdentifier algorithmClassID = m_Scenario.getBoxDetails(boxID)->getAlgorithmClassIdentifier();

	// Do not show documentation for non-metaboxes or boxes that can not be created
	if (!(boxID != OV_UndefinedIdentifier && (m_kernelCtx.getPluginManager().canCreatePluginObject(algorithmClassID) ||
											  m_Scenario.getBoxDetails(boxID)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Box with id " << boxID << " can not create a pluging object\n";
		return false;
	}

	const CString defaultURLBase = m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}");
	CString urlBase              = defaultURLBase;
	CString browser              = m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserCommand}");
	CString browserPostfix       = m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserCommandPostfix}");
	CString boxName;

	CString html = "Doc_BoxAlgorithm_";
	if (m_Scenario.getBoxDetails(boxID)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier id;
		id.fromString(m_Scenario.getBoxDetails(boxID)->getAttributeValue(OVP_AttributeId_Metabox_ID));
		boxName = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(id)->getName();
	}
	else
	{
		const IPluginObjectDesc* pod = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(algorithmClassID);
		boxName                      = pod->getName();
	}
	// The documentation files do not have spaces in their name, so we remove them
	html = html + CString(getBoxAlgorithmURL(boxName.toASCIIString()).c_str());


	if (m_Scenario.getBoxDetails(boxID)->hasAttribute(OV_AttributeId_Box_DocumentationURLBase))
	{
		urlBase = m_kernelCtx.getConfigurationManager().expand(
			m_Scenario.getBoxDetails(boxID)->getAttributeValue(OV_AttributeId_Box_DocumentationURLBase));
	}
	html = html + ".html";

	if (m_Scenario.getBoxDetails(boxID)->hasAttribute(OV_AttributeId_Box_DocumentationCommand))
	{
		browser = m_kernelCtx.getConfigurationManager().expand(
			m_Scenario.getBoxDetails(boxID)->getAttributeValue(OV_AttributeId_Box_DocumentationCommand));
		browserPostfix = "";
	}

	CString fullUrl = urlBase + CString("/") + html;
	if (m_Scenario.getBoxDetails(boxID)->hasAttribute(OV_AttributeId_Box_DocumentationURL))
	{
		fullUrl = m_kernelCtx.getConfigurationManager().expand(m_Scenario.getBoxDetails(boxID)->getAttributeValue(OV_AttributeId_Box_DocumentationURL));
	}

	return browseURL(fullUrl, browser, browserPostfix);
}

void CInterfacedScenario::contextMenuBoxDocumentationCB(IBox& box) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxDocumentationCB\n";
	const CIdentifier id = box.getIdentifier();
	browseBoxDocumentation(id);
}

void CInterfacedScenario::contextMenuBoxEnableCB(IBox& box)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEnableCB\n";
	TAttributeHandler handler(box);
	handler.removeAttribute(OV_AttributeId_Box_Disabled);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxDisableCB(IBox& box)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxDisableCB\n";
	TAttributeHandler handler(box);
	if (!handler.hasAttribute(OV_AttributeId_Box_Disabled)) { handler.addAttribute(OV_AttributeId_Box_Disabled, 1); }
	else { handler.setAttributeValue(OV_AttributeId_Box_Disabled, 1); }
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuScenarioAddCommentCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuScenarioAddCommentCB\n";
	this->addCommentCB();
}

void CInterfacedScenario::contextMenuScenarioAboutCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuScenarioAboutCB\n";
	CAboutScenarioDialog dialog(m_kernelCtx, m_Scenario, m_guiFilename.c_str());
	dialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::toggleDesignerVisualization()
{
	m_designerVisualizationToggled = !m_designerVisualizationToggled;

	if (m_DesignerVisualization)
	{
		if (m_designerVisualizationToggled) { m_DesignerVisualization->show(); }
		else { m_DesignerVisualization->hide(); }
	}
}

void CInterfacedScenario::showCurrentVisualization() const
{
	if (isLocked()) { if (m_playerVisualization != nullptr) { m_playerVisualization->showTopLevelWindows(); } }
	else { if (m_DesignerVisualization != nullptr) { m_DesignerVisualization->show(); } }
}

void CInterfacedScenario::hideCurrentVisualization() const
{
	if (isLocked()) { if (m_playerVisualization != nullptr) { m_playerVisualization->hideTopLevelWindows(); } }
	else { if (m_DesignerVisualization != nullptr) { m_DesignerVisualization->hide(); } }
}

void CInterfacedScenario::createPlayerVisualization(IVisualizationTree* tree)
{
	//hide window manager
	if (m_DesignerVisualization) { m_DesignerVisualization->hide(); }

	if (m_playerVisualization == nullptr)
	{
		if (tree) { m_playerVisualization = new CPlayerVisualization(m_kernelCtx, *tree, *this); }
		else { m_playerVisualization = new CPlayerVisualization(m_kernelCtx, *m_Tree, *this); }


		//we go here when we press start
		//we have to set the modUI here
		//first, find the concerned boxes
		IScenario& runtimeScenario = m_Player->getRuntimeScenarioManager().getScenario(m_Player->getRuntimeScenarioIdentifier());
		CIdentifier id;
		while ((id = runtimeScenario.getNextBoxIdentifier(id)) != OV_UndefinedIdentifier)
		{
			IBox* box = runtimeScenario.getBoxDetails(id);
			if (box->hasModifiableSettings())//if the box has modUI
			{
				//create a BoxConfigurationDialog in mode true
				auto* dialog = new CBoxConfigurationDialog(m_kernelCtx, *box, m_guiFilename.c_str(), m_guiSettingsFilename.c_str(), true);
				//store it
				m_boxConfigDialogs.push_back(dialog);
			}
		}
	}

	//initialize and show windows
	m_playerVisualization->init();
}

void CInterfacedScenario::releasePlayerVisualization()
{
	if (m_playerVisualization != nullptr)
	{
		delete m_playerVisualization;
		m_playerVisualization = nullptr;
	}

	//reload designer visualization
	if (m_DesignerVisualization)
	{
		m_DesignerVisualization->load();
		//show it if it was toggled on
		if (m_designerVisualizationToggled) { m_DesignerVisualization->show(); }
	}
}

void CInterfacedScenario::stopAndReleasePlayer()
{
	m_kernelCtx.getErrorManager().releaseErrors();
	m_Player->stop();
	m_PlayerStatus = m_Player->getStatus();
	// removes idle function
	g_idle_remove_by_data(this);

	if (!m_Player->uninitialize()) { m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to uninitialize the player" << "\n"; }

	for (auto elem : m_boxConfigDialogs)
	{
		elem->restoreState();
		delete elem;
	}
	m_boxConfigDialogs.clear();


	if (!m_kernelCtx.getPlayerManager().releasePlayer(m_PlayerID)) { m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to release the player" << "\n"; }

	m_PlayerID = OV_UndefinedIdentifier;
	m_Player   = nullptr;

	// destroy player windows
	releasePlayerVisualization();

	// redraws scenario
	redraw();
}

//give the PlayerVisualisation the matching between the GtkWidget created by the CBoxConfigurationDialog and the Box CIdentifier
bool CInterfacedScenario::setModifiableSettingsWidgets()
{
	for (auto& elem : m_boxConfigDialogs) { m_playerVisualization->setWidget(elem->getBoxID(), elem->getWidget()); }
	return true;
}

bool CInterfacedScenario::centerOnBox(const CIdentifier& identifier)
{
	//m_kernelCtx.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox" << "\n";
	bool res = false;
	if (m_Scenario.isBox(identifier))
	{
		//m_kernelCtx.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox is box" << "\n";
		IBox* box = m_Scenario.getBoxDetails(identifier);

		//clear previous selection
		m_SelectedObjects.clear();
		//to select the box

		m_SelectedObjects.insert(identifier);
		//		m_bScenarioModified=true;
		redraw();

		//CBoxProxy proxy(m_kernelCtx, *box);
		const CBoxProxy proxy(m_kernelCtx, m_Scenario, box->getIdentifier());
		const double marginX = 5.0 * m_currentScale;
		const double merginY = 5.0 * m_currentScale;
		const int sizeX      = int(round(proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) + marginX * 2.0));
		const int sizeY      = int(round(proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) + merginY * 2.0));
		const double centerX = proxy.getXCenter() * m_currentScale;
		const double centerY = proxy.getYCenter() * m_currentScale;
		int x, y;

		//get the parameters of the current adjustement
		GtkAdjustment* oldAdjustmentH = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent));
		GtkAdjustment* oldAdjustmentV = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent));
		gdouble upper, lower, step, page, pagesize, value;

		g_object_get(oldAdjustmentH, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value",
					 &value, nullptr);
		//create a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		auto* adjustment = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (centerX + m_viewOffsetX < upper / 2) { x = int(round(centerX - 2 * sizeX)) + m_viewOffsetX; }
		else { x = int(round(centerX + 2 * sizeX - pagesize)) + m_viewOffsetX; }
		gtk_adjustment_set_value(adjustment, x);
		gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent), adjustment);

		g_object_get(oldAdjustmentV, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value",
					 &value, nullptr);
		adjustment = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (centerY - m_viewOffsetY < upper / 2) { y = int(round(centerY - 2 * sizeY) + m_viewOffsetY); }
		else { y = int(round(centerY + 2 * sizeY - pagesize)) + m_viewOffsetY; }
		gtk_adjustment_set_value(adjustment, y);
		gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent), adjustment);
		res = true;
	}
	return res;
}

void CInterfacedScenario::setScale(const double scale)
{
	m_currentScale = std::max(scale, 0.1);

	PangoContext* ctx          = gtk_widget_get_pango_context(GTK_WIDGET(m_scenarioDrawingArea));
	PangoFontDescription* desc = pango_context_get_font_description(ctx);
	// not done in constructor because the font size is changed elsewhere after that withour our knowledge
	if (m_normalFontSize == 0) { m_normalFontSize = pango_font_description_get_size(desc); }
	pango_font_description_set_size(desc, gint(round(m_normalFontSize * m_currentScale)));

	//m_scenarioModified = true;
	redraw();
}
