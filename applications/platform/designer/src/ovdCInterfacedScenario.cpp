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
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace OpenViBEVisualizationToolkit;
using namespace std;

extern map<size_t, GdkColor> gColors;

static GtkTargetEntry targets[] = { { static_cast<gchar*>("STRING"), 0, 0 }, { static_cast<gchar*>("text/plain"), 0, 0 } };

static GdkColor colorFromIdentifier(const CIdentifier& identifier)
{
	GdkColor color;
	uint32_t value1 = 0;
	uint32_t value2 = 0;
	uint64_t res    = 0;

	sscanf(identifier.toString(), "(0x%08X, 0x%08X)", &value1, &value2);
	res += value1;
	res <<= 32;
	res += value2;

	color.pixel = guint16(0);
	color.red   = guint16((res & 0xffff) | 0x8000);
	color.green = guint16(((res >> 16) & 0xffff) | 0x8000);
	color.blue  = guint16(((res >> 32) & 0xffff) | 0x8000);

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
			// if(!l_bLastWasSeparator) { out += "_"; }
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

static void context_menu_cb(GtkMenuItem* /*pMenuItem*/, CInterfacedScenario::box_ctx_menu_cb_t* cb)
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
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(app->m_pBuilderInterface, "openvibe-menu_edit")), 0);
	return 0;
}

static gboolean editable_widget_focus_out_cb(GtkWidget* /*widget*/, GdkEvent* /*event*/, CApplication* app)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(app->m_pBuilderInterface, "openvibe-menu_edit")), 1);

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
	if (data->m_isInput) { data->m_pInterfacedScenario->m_scenario.setInputName(data->m_uiLinkIdx, gtk_entry_get_text(GTK_ENTRY(entry))); }
	else { data->m_pInterfacedScenario->m_scenario.setOutputName(data->m_uiLinkIdx, gtk_entry_get_text(GTK_ENTRY(entry))); }
}

static void modify_scenario_link_type_cb(GtkWidget* comboBox, CInterfacedScenario::link_cb_data_t* data)
{
	const CIdentifier typeID = data->m_pInterfacedScenario->m_mStreamType[gtk_combo_box_get_active_text(GTK_COMBO_BOX(comboBox))];
	if (data->m_isInput) { data->m_pInterfacedScenario->m_scenario.setInputType(data->m_uiLinkIdx, typeID); }
	else { data->m_pInterfacedScenario->m_scenario.setOutputType(data->m_uiLinkIdx, typeID); }
	data->m_pInterfacedScenario->redraw();
}
//*/

CInterfacedScenario::CInterfacedScenario(const IKernelContext& ctx, CApplication& application, IScenario& scenario, CIdentifier& scenarioID,
										 GtkNotebook& notebook, const char* guiFilename, const char* guiSettingsFilename)
	: m_PlayerStatus(PlayerStatus_Stop), m_ScenarioID(scenarioID), m_Application(application), m_kernelCtx(ctx), m_Scenario(scenario), m_notebook(notebook),
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

	m_Application.m_pVisualizationManager->createVisualizationTree(m_TreeID);
	m_Tree = &m_Application.m_pVisualizationManager->getVisualizationTree(m_TreeID);
	m_Tree->init(&m_Scenario);

	//create window manager
	m_DesignerVisualization = new CDesignerVisualization(m_kernelCtx, *m_Tree, *this);
	m_DesignerVisualization->init(string(guiFilename));

	m_configureSettingsDialog = GTK_WIDGET(gtk_builder_get_object(m_Application.m_pBuilderInterface, "dialog_scenario_configuration"));

	m_settingsVBox = GTK_WIDGET(gtk_builder_get_object(m_Application.m_pBuilderInterface, "dialog_scenario_configuration-vbox"));

	m_noHelpDialog = GTK_WIDGET(gtk_builder_get_object(m_Application.m_pBuilderInterface, "dialog_no_help"));

	m_errorPendingDeprecatedInterfacorsDialog =
			GTK_WIDGET(gtk_builder_get_object(m_Application.m_pBuilderInterface, "dialog_pending_deprecated_interfacors"));

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
		//const IBox *l_pBox = m_scenario.getBoxDetails(l_oBoxID);
		//const CBoxProxy proxy(m_kernelCtx, *l_pBox);
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
	g_object_unref(m_pBuilder);
	g_object_unref(m_pBuilder);
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
	GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_Application.m_pBuilderInterface, "openvibe-scenario_configuration_vbox"));

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

	this->redrawScenarioLinkSettings(m_Application.m_pTableInputs, true, m_scenarioInputCBDatas, getNLink, getLinkName, getLinkType);
}

void CInterfacedScenario::redrawScenarioOutputSettings()
{
	size_t (IScenario::* getNLink)() const                      = &IScenario::getOutputCount;
	bool (IScenario::* getLinkName)(size_t, CString&) const     = &IScenario::getOutputName;
	bool (IScenario::* getLinkType)(size_t, CIdentifier&) const = &IScenario::getOutputType;

	this->redrawScenarioLinkSettings(m_Application.m_pTableOutputs, false, m_scenarioOutputCBDatas, getNLink, getLinkName, getLinkType);
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
		if (trimLimit > 3) trimLimit -= 3; // limit should include the '...'
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

	const int marginX      = int(round(5 * m_currentScale));
	const int marginY      = int(round(5 * m_currentScale));
	const int iCircleSize  = int(round(11 * m_currentScale));
	const int iCircleSpace = int(round(4 * m_currentScale));

	//CBoxProxy proxy(m_kernelCtx, box);
	CBoxProxy proxy(m_kernelCtx, m_Scenario, box.getIdentifier());

	if (box.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
		proxy.setBoxAlgorithmDescriptorOverride(
			static_cast<const IBoxAlgorithmDesc*>(m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId)));
	}

	int xSize  = int(round(proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) * m_currentScale) + marginX * 2);
	int ySize  = int(round(proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) * m_currentScale) + marginY * 2);
	int xStart = int(round(proxy.getXCenter() * m_currentScale + m_viewOffsetX - (xSize >> 1)));
	int yStart = int(round(proxy.getYCenter() * m_currentScale + m_viewOffsetY - (ySize >> 1)));

	UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_currentScale)));
	m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier());

	bool l_bCanCreate                    = proxy.isBoxAlgorithmPluginPresent();
	bool l_bUpToDate                     = l_bCanCreate ? proxy.isUpToDate() : true;
	bool l_bPendingDeprecatedInterfacors = proxy.hasPendingDeprecatedInterfacors();
	bool l_bDeprecated                   = l_bCanCreate && proxy.isDeprecated();
	bool l_bMetabox                      = l_bCanCreate && proxy.isMetabox();
	bool l_bDisabled                     = proxy.isDisabled();


	// Check if this is a mensia box
	auto l_pPOD    = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(box.getAlgorithmClassIdentifier());
	bool l_bMensia = (l_pPOD && l_pPOD->hasFunctionality(M_Functionality_IsMensia));

	// Add a thick dashed border around selected boxes
	if (m_SelectedObjects.count(box.getIdentifier()))
	{
		int l_iTopLeftOffset = 2;
#if defined TARGET_OS_Windows
		int l_iBottomRightOffset = 4;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		int l_iBottomRightOffset = 5;
#else
		int l_iBottomRightOffset = 4;
#endif
		if (l_bMetabox)
		{
			l_iTopLeftOffset     = 3;
			l_iBottomRightOffset = 6;
		}

		gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBorderSelected]);
		gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(widget->window, drawGC, TRUE, xStart - l_iTopLeftOffset, yStart - l_iTopLeftOffset, xSize + l_iBottomRightOffset,
								   ySize + l_iBottomRightOffset);
	}

	if (!this->isLocked() || !m_DebugCPUUsage)
	{
		/*if(m_vCurrentObject[box.getIdentifier()])
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &gColors[Color_BoxBackgroundSelected]);
		}
		else*/
		if (!l_bCanCreate) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundMissing]); }
		else if (l_bDisabled) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundDisabled]); }
		else if (l_bDeprecated) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundDeprecated]); }
		else if (!l_bUpToDate || l_bPendingDeprecatedInterfacors) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundOutdated]); }
		else if (l_bMensia) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackgroundMensia]); }
			/*
					else if(l_bMetabox)
					{
						gdk_gc_set_rgb_fg_color(l_pDrawGC, &gColors[Color_BoxBackgroundMetabox]);
					}
			*/
		else { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_BoxBackground]); }
	}
	else
	{
		CIdentifier l_oComputationTime;
		l_oComputationTime.fromString(box.getAttributeValue(OV_AttributeId_Box_ComputationTimeLastSecond));
		uint64_t l_ui64ComputationTime          = (l_oComputationTime == OV_UndefinedIdentifier ? 0 : l_oComputationTime.toUInteger());
		uint64_t l_ui64ComputationTimeReference = (1LL << 32) / (m_nBox == 0 ? 1 : m_nBox);

		GdkColor l_oColor;
		if (l_ui64ComputationTime < l_ui64ComputationTimeReference)
		{
			l_oColor.pixel = 0;
			l_oColor.red   = guint16((l_ui64ComputationTime << 16) / l_ui64ComputationTimeReference);
			l_oColor.green = 32768;
			l_oColor.blue  = 0;
		}
		else
		{
			if (l_ui64ComputationTime < l_ui64ComputationTimeReference * 4)
			{
				l_oColor.pixel = 0;
				l_oColor.red   = 65535;
				l_oColor.green = guint16(32768 - ((l_ui64ComputationTime << 15) / (l_ui64ComputationTimeReference * 4)));
				l_oColor.blue  = 0;
			}
			else
			{
				l_oColor.pixel = 0;
				l_oColor.red   = 65535;
				l_oColor.green = 0;
				l_oColor.blue  = 0;
			}
		}
		gdk_gc_set_rgb_fg_color(drawGC, &l_oColor);
	}

	gdk_draw_rounded_rectangle(widget->window, drawGC, TRUE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_currentScale)));

	if (l_bMensia)
	{
		gdk_draw_pixbuf(widget->window, drawGC, m_mensiaLogoPixbuf, 5, 5, xStart, yStart, 80, (ySize < 50) ? ySize : 50, GDK_RGB_DITHER_NONE, 0, 0);
	}

	int l_iBorderColor = Color_BoxBorder;
	if (l_bMensia) { l_iBorderColor = Color_BoxBorderMensia; }
	gdk_gc_set_rgb_fg_color(drawGC, &gColors[l_iBorderColor]);
	gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
	gdk_draw_rounded_rectangle(widget->window, drawGC, FALSE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_currentScale)));

	if (l_bMetabox)
	{
		gdk_gc_set_rgb_fg_color(drawGC, &gColors[l_iBorderColor]);
		gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(widget->window, drawGC, FALSE, xStart - 3, yStart - 3, xSize + 6, ySize + 6, gint(round(8.0 * m_currentScale)));
	}

	TAttributeHandler handler(box);

	int l_iInputOffset = xSize / 2 - int(box.getInputCount()) * (iCircleSpace + iCircleSize) / 2 + iCircleSize / 4;
	for (size_t i = 0; i < box.getInterfacorCountIncludingDeprecated(Input); ++i)
	{
		CIdentifier InputID;
		bool isDeprecated;
		box.getInputType(i, InputID);
		box.getInterfacorDeprecatedStatus(Input, i, isDeprecated);
		GdkColor l_oInputColor = colorFromIdentifier(InputID);


		GdkPoint l_vPoint[4];
		l_vPoint[0].x = iCircleSize >> 1;
		l_vPoint[0].y = iCircleSize;
		l_vPoint[1].x = 0;
		l_vPoint[1].y = 0;
		l_vPoint[2].x = iCircleSize - 1;
		l_vPoint[2].y = 0;
		for (int j = 0; j < 3; ++j)
		{
			l_vPoint[j].x += xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset;
			l_vPoint[j].y += yStart - (iCircleSize >> 1);
		}

		UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, l_vPoint, 3);
		m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Input, i);

		if (isDeprecated)
		{
			l_oInputColor.blue  = 2 * l_oInputColor.blue / 3;
			l_oInputColor.red   = 2 * l_oInputColor.red / 3;
			l_oInputColor.green = 2 * l_oInputColor.green / 3;
		}

		gdk_gc_set_rgb_fg_color(drawGC, &l_oInputColor);

		gdk_draw_polygon(widget->window, drawGC, TRUE, l_vPoint, 3);
		int l_iBoxInputBorderColor = Color_BoxInputBorder;
		if (isDeprecated) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkInvalid]); }
		else { gdk_gc_set_rgb_fg_color(drawGC, &gColors[l_iBoxInputBorderColor]); }
		gdk_draw_polygon(widget->window, drawGC, FALSE, l_vPoint, 3);

		int x                 = xStart + i * (iCircleSpace + iCircleSize) + (iCircleSize >> 1) - m_viewOffsetX + l_iInputOffset;
		int y                 = yStart - (iCircleSize >> 1) - m_viewOffsetY;
		CIdentifier l_oLinkID = m_Scenario.getNextLinkIdentifierToBoxInput(OV_UndefinedIdentifier, box.getIdentifier(), i);
		while (l_oLinkID != OV_UndefinedIdentifier)
		{
			ILink* l_pLink = m_Scenario.getLinkDetails(l_oLinkID);
			if (l_pLink)
			{
				TAttributeHandler attributeHandler(*l_pLink);

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_XDst)) { attributeHandler.addAttribute<int>(OV_AttributeId_Link_XDst, x); }
				else { attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XDst, x); }

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_YDst)) { attributeHandler.addAttribute<int>(OV_AttributeId_Link_YDst, y); }
				else { attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YDst, y); }
			}
			l_oLinkID = m_Scenario.getNextLinkIdentifierToBoxInput(l_oLinkID, box.getIdentifier(), i);
		}

		// Display a circle above inputs that are linked to the box inputs
		for (size_t j = 0; j < m_Scenario.getInputCount(); j++)
		{
			CIdentifier linkBoxID;
			size_t linkBoxInputIdx;

			m_Scenario.getScenarioInputLink(j, linkBoxID, linkBoxInputIdx);

			if (linkBoxID == box.getIdentifier() && linkBoxInputIdx == i)
			{
				// Since the circle representing the input is quite large, we are going to offset each other one
				int l_iInputDiscOffset = int(i % 2) * iCircleSize * 2;

				int l_iScenarioInputIndicatorLeft = xStart + int(i) * (iCircleSpace + iCircleSize) + l_iInputOffset - int(iCircleSize * 0.5);
				int l_iScenarioInputIndicatorTop  = yStart - (iCircleSize >> 1) - iCircleSize * 3 - l_iInputDiscOffset;

				CIdentifier scenarioInputTypeIdentifier;
				this->m_Scenario.getInputType(j, scenarioInputTypeIdentifier);
				GdkColor inputColor = colorFromIdentifier(scenarioInputTypeIdentifier);

				UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, l_iScenarioInputIndicatorLeft, l_iScenarioInputIndicatorTop, iCircleSize * 2,
							 iCircleSize * 2, 0, 64 * 360);
				m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_ScenarioInput, i);

				gdk_gc_set_rgb_fg_color(drawGC, &inputColor);

				gdk_draw_arc(widget->window, drawGC, TRUE, l_iScenarioInputIndicatorLeft, l_iScenarioInputIndicatorTop, iCircleSize * 2, iCircleSize * 2,
							 0, 64 * 360);
				gdk_gc_set_rgb_fg_color(drawGC, &gColors[l_iBoxInputBorderColor]);
				gdk_draw_arc(widget->window, drawGC, FALSE, l_iScenarioInputIndicatorLeft, l_iScenarioInputIndicatorTop, iCircleSize * 2, iCircleSize * 2,
							 0, 64 * 360);

				// Draw the text indicating the scenario input index
				PangoContext* l_pPangoContext = nullptr;
				PangoLayout* l_pPangoLayout   = nullptr;
				l_pPangoContext               = gtk_widget_get_pango_context(widget);
				l_pPangoLayout                = pango_layout_new(l_pPangoContext);
				pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
				pango_layout_set_markup(l_pPangoLayout, std::to_string(static_cast<long long int>(j + 1)).c_str(), -1);
				gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], l_iScenarioInputIndicatorLeft + marginX,
								l_iScenarioInputIndicatorTop + marginY, l_pPangoLayout);
				g_object_unref(l_pPangoLayout);
				gdk_draw_line(widget->window, drawGC, xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset + (iCircleSize >> 1),
							  l_iScenarioInputIndicatorTop + iCircleSize * 2, xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset + (iCircleSize >> 1),
							  yStart - (iCircleSize >> 1));
			}
		}
	}

	gdk_gc_set_line_attributes(drawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);

	int l_iOutputOffset = xSize / 2 - int(box.getOutputCount()) * (iCircleSpace + iCircleSize) / 2 + iCircleSize / 4;
	for (size_t i = 0; i < box.getInterfacorCountIncludingDeprecated(Output); ++i)
	{
		CIdentifier OutputID;
		bool isDeprecated;
		box.getOutputType(i, OutputID);
		box.getInterfacorDeprecatedStatus(Output, i, isDeprecated);
		GdkColor l_oOutputColor = colorFromIdentifier(OutputID);

		if (isDeprecated)
		{
			l_oOutputColor.blue  = 2 * l_oOutputColor.blue / 3;
			l_oOutputColor.red   = 2 * l_oOutputColor.red / 3;
			l_oOutputColor.green = 2 * l_oOutputColor.green / 3;
		}

		GdkPoint l_vPoint[4];
		l_vPoint[0].x = iCircleSize >> 1;
		l_vPoint[0].y = iCircleSize;
		l_vPoint[1].x = 0;
		l_vPoint[1].y = 0;
		l_vPoint[2].x = iCircleSize - 1;
		l_vPoint[2].y = 0;
		for (int j = 0; j < 3; ++j)
		{
			l_vPoint[j].x += xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset;
			l_vPoint[j].y += yStart - (iCircleSize >> 1) + ySize;
		}

		UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, l_vPoint, 3);
		m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Output, i);

		gdk_gc_set_rgb_fg_color(drawGC, &l_oOutputColor);
		gdk_draw_polygon(widget->window, drawGC, TRUE, l_vPoint, 3);
		int l_iBoxOutputBorderColor = Color_BoxOutputBorder;
		if (isDeprecated) { gdk_gc_set_rgb_fg_color(drawGC, &gColors[Color_LinkInvalid]); }
		else { gdk_gc_set_rgb_fg_color(drawGC, &gColors[l_iBoxOutputBorderColor]); }

		gdk_draw_polygon(widget->window, drawGC, FALSE, l_vPoint, 3);

		int x                 = xStart + i * (iCircleSpace + iCircleSize) + (iCircleSize >> 1) - m_viewOffsetX + l_iOutputOffset;
		int y                 = yStart + ySize + (iCircleSize >> 1) + 1 - m_viewOffsetY;
		CIdentifier l_oLinkID = m_Scenario.getNextLinkIdentifierFromBoxOutput(OV_UndefinedIdentifier, box.getIdentifier(), i);
		while (l_oLinkID != OV_UndefinedIdentifier)
		{
			ILink* l_pLink = m_Scenario.getLinkDetails(l_oLinkID);
			if (l_pLink)
			{
				TAttributeHandler attributeHandler(*l_pLink);

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_XSrc)) { attributeHandler.addAttribute<int>(OV_AttributeId_Link_XSrc, x); }
				else { attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XSrc, x); }

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_YSrc)) { attributeHandler.addAttribute<int>(OV_AttributeId_Link_YSrc, y); }
				else attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YSrc, y);
			}
			l_oLinkID = m_Scenario.getNextLinkIdentifierFromBoxOutput(l_oLinkID, box.getIdentifier(), i);
		}

		// Display a circle below outputs that are linked to the box outputs
		for (size_t j = 0; j < m_Scenario.getOutputCount(); j++)
		{
			CIdentifier linkBoxID;
			size_t linkBoxOutputIdx;

			m_Scenario.getScenarioOutputLink(j, linkBoxID, linkBoxOutputIdx);

			if (linkBoxID == box.getIdentifier() && linkBoxOutputIdx == i)
			{
				// Since the circle representing the Output is quite large, we are going to offset each other one
				int l_iOutputDiscOffset = (int(i) % 2) * iCircleSize * 2;

				int l_iScenarioOutputIndicatorLeft = xStart + int(i) * (iCircleSpace + iCircleSize) + l_iOutputOffset - int(iCircleSize * 0.5);
				int l_iScenarioOutputIndicatorTop  = yStart - (iCircleSize >> 1) + ySize + l_iOutputDiscOffset + iCircleSize * 2;

				CIdentifier scenarioOutputTypeIdentifier;
				this->m_Scenario.getOutputType(j, scenarioOutputTypeIdentifier);
				GdkColor oOutputColor = colorFromIdentifier(scenarioOutputTypeIdentifier);

				UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, l_iScenarioOutputIndicatorLeft, l_iScenarioOutputIndicatorTop, iCircleSize * 2,
							 iCircleSize * 2, 0, 64 * 360);
				m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_ScenarioOutput, i);

				gdk_gc_set_rgb_fg_color(drawGC, &oOutputColor);
				gdk_draw_arc(widget->window, drawGC, TRUE, l_iScenarioOutputIndicatorLeft, l_iScenarioOutputIndicatorTop, iCircleSize * 2, iCircleSize * 2,
							 0, 64 * 360);
				gdk_gc_set_rgb_fg_color(drawGC, &gColors[l_iBoxOutputBorderColor]);
				gdk_draw_arc(widget->window, drawGC, FALSE, l_iScenarioOutputIndicatorLeft, l_iScenarioOutputIndicatorTop, iCircleSize * 2,
							 iCircleSize * 2, 0, 64 * 360);

				PangoContext* l_pPangoContext = nullptr;
				PangoLayout* l_pPangoLayout   = nullptr;
				l_pPangoContext               = gtk_widget_get_pango_context(widget);
				l_pPangoLayout                = pango_layout_new(l_pPangoContext);
				pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
				pango_layout_set_markup(l_pPangoLayout, std::to_string(static_cast<long long int>(j + 1)).c_str(), -1);
				gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], l_iScenarioOutputIndicatorLeft + marginX,
								l_iScenarioOutputIndicatorTop + marginY, l_pPangoLayout);
				g_object_unref(l_pPangoLayout);
				gdk_draw_line(widget->window, drawGC, xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset + (iCircleSize >> 1),
							  l_iScenarioOutputIndicatorTop, xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset + (iCircleSize >> 1),
							  yStart + (iCircleSize >> 2) + ySize + 2); // This is somewhat the bottom of the triangle indicating a box output
			}
		}
	}

	/*
		::GdkPixbuf* l_pPixbuf=gtk_widget_render_icon(l_widget, GTK_STOCK_EXECUTE, GTK_ICON_SIZE_SMALL_TOOLBAR, "openvibe");
		if(l_pPixbuf)
		{
			gdk_draw_pixbuf(l_widget->window, l_pDrawGC, l_pPixbuf, 0, 0, 10, 10, 64, 64, GDK_RGB_DITHER_NONE, 0, 0);
			g_object_unref(l_pPixbuf);
		}
	*/

	// Draw labels

	PangoContext* l_pPangoContext = nullptr;
	PangoLayout* l_pPangoLayout   = nullptr;
	l_pPangoContext               = gtk_widget_get_pango_context(widget);
	l_pPangoLayout                = pango_layout_new(l_pPangoContext);

	// Draw box label
	PangoRectangle l_oPangoLabelRect;
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(l_pPangoLayout, proxy.getLabel(), -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoLabelRect);
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], xStart + marginX, yStart + marginY, l_pPangoLayout);

	// Draw box status label
	PangoRectangle l_oPangoStatusRect;
	pango_layout_set_markup(l_pPangoLayout, proxy.getStatusLabel(), -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoStatusRect);
	int xShift = (max(l_oPangoLabelRect.width, l_oPangoStatusRect.width) -
				  min(l_oPangoLabelRect.width, l_oPangoStatusRect.width)) / 2;

	UPDATE_STENCIL_IDX(m_interfacedObjectId, stencilGC);
	gdk_draw_rectangle(GDK_DRAWABLE(m_stencilBuffer), stencilGC, TRUE, xStart + xShift + marginX, yStart + l_oPangoLabelRect.height + marginY,
					   l_oPangoStatusRect.width, l_oPangoStatusRect.height);
	m_interfacedObjects[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Update, 0);
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], xStart + xShift + marginX,
					yStart + l_oPangoLabelRect.height + marginY, l_pPangoLayout);

	g_object_unref(l_pPangoLayout);
	g_object_unref(drawGC);
	g_object_unref(stencilGC);

	/*
		CLinkPositionSetterEnum l_oLinkPositionSetterInput(Connector_Input, l_vInputPosition);
		CLinkPositionSetterEnum l_oLinkPositionSetterOutput(Connector_Output, l_vOutputPosition);
		scenario.enumerateLinksToBox(l_oLinkPositionSetterInput, box.getIdentifier());
		scenario.enumerateLinksFromBox(l_oLinkPositionSetterOutput, box.getIdentifier());
	*/
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

	PangoContext* pangoCtx   = gtk_widget_get_pango_context(widget);
	PangoLayout* pangoLayout = pango_layout_new(pangoCtx);
	pango_layout_set_alignment(pangoLayout, PANGO_ALIGN_CENTER);
	if (pango_parse_markup(comment.getText().toASCIIString(), -1, 0, nullptr, nullptr, nullptr, nullptr))
	{
		pango_layout_set_markup(pangoLayout, comment.getText().toASCIIString(), -1);
	}
	else { pango_layout_set_text(pangoLayout, comment.getText().toASCIIString(), -1); }
	gdk_draw_layout(widget->window, widget->style->text_gc[GTK_WIDGET_STATE(widget)], startX + marginX, startY + marginY, pangoLayout);
	g_object_unref(pangoLayout);

	g_object_unref(drawGC);
	g_object_unref(stencilGC);
}

void CInterfacedScenario::redraw(ILink& link)
{
	GtkWidget* widget = GTK_WIDGET(m_scenarioDrawingArea);
	GdkGC* stencilGC  = gdk_gc_new(GDK_DRAWABLE(m_stencilBuffer));
	GdkGC* drawGC     = gdk_gc_new(widget->window);

	CLinkProxy proxy(link);

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
		GdkPixbuf* l_pPixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_stencilBuffer), nullptr, x, y, 0, 0, 1, 1);
		if (!l_pPixbuf)
		{
			m_kernelCtx.getLogManager() << LogLevel_ImportantWarning <<
					"Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
			return size_t(0xffffffff);
		}

		guchar* l_pPixels = gdk_pixbuf_get_pixels(l_pPixbuf);
		if (!l_pPixels)
		{
			m_kernelCtx.getLogManager() << LogLevel_ImportantWarning <<
					"Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		res = 0;
		res += (l_pPixels[0] << 16);
		res += (l_pPixels[1] << 8);
		res += (l_pPixels[2]);
		g_object_unref(l_pPixbuf);
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

	int l_iMaxX;
	int l_iMaxY;
	gdk_drawable_get_size(GDK_DRAWABLE(m_stencilBuffer), &l_iMaxX, &l_iMaxY);

	int iStartX = x;
	int iStartY = y;
	int iEndX   = x + sizeX;
	int iEndY   = y + sizeY;

	// crops according to drawing area boundings
	if (iStartX < 0) { iStartX = 0; }
	if (iStartY < 0) { iStartY = 0; }
	if (iEndX < 0) { iEndX = 0; }
	if (iEndY < 0) { iEndY = 0; }
	if (iStartX >= l_iMaxX - 1) { iStartX = l_iMaxX - 1; }
	if (iStartY >= l_iMaxY - 1) { iStartY = l_iMaxY - 1; }
	if (iEndX >= l_iMaxX - 1) { iEndX = l_iMaxX - 1; }
	if (iEndY >= l_iMaxY - 1) { iEndY = l_iMaxY - 1; }

	// recompute new size
	sizeX = iEndX - iStartX + 1;
	sizeY = iEndY - iStartY + 1;

	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_stencilBuffer), nullptr, iStartX, iStartY, 0, 0, sizeX, sizeY);
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

	const int rowBytesCount = gdk_pixbuf_get_rowstride(pixbuf);
	const int nChannel      = gdk_pixbuf_get_n_channels(pixbuf);
	for (int j = 0; j < sizeY; ++j)
	{
		for (int i = 0; i < sizeX; ++i)
		{
			size_t interfacedObjectId = 0;
			interfacedObjectId += (pixels[j * rowBytesCount + i * nChannel + 0] << 16);
			interfacedObjectId += (pixels[j * rowBytesCount + i * nChannel + 1] << 8);
			interfacedObjectId += (pixels[j * rowBytesCount + i * nChannel + 2]);
			if (m_interfacedObjects[interfacedObjectId].m_ID != OV_UndefinedIdentifier)
			{
				m_SelectedObjects.insert(m_interfacedObjects[interfacedObjectId].m_ID);
			}
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
		CIdentifier identifier;
		m_SelectedObjects.clear();
		while ((identifier = m_Scenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getBoxDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}
		while ((identifier = m_Scenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getLinkDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}

		if (m_DesignerVisualization) { m_DesignerVisualization->load(); }
		if (manageModifiedStatusFlag) { m_HasBeenModified = true; }

		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();

		if (shouldDropLastState) { m_StateStack->dropLastState(); }

		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_redo")), m_StateStack->isRedoPossible());
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_undo")), m_StateStack->isUndoPossible());
	}
	else
	{
		m_kernelCtx.getLogManager() << LogLevel_Trace << "Can not undo\n";
		GtkWidget* l_pUndoButton = GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_undo"));
		gtk_widget_set_sensitive(l_pUndoButton, false);
	}
}

void CInterfacedScenario::redoCB(const bool manageModifiedStatusFlag)
{
	if (m_StateStack->redo())
	{
		CIdentifier identifier;
		m_SelectedObjects.clear();
		while ((identifier = m_Scenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getBoxDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}
		while ((identifier = m_Scenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_Scenario.getLinkDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}

		if (m_DesignerVisualization) { m_DesignerVisualization->load(); }

		if (manageModifiedStatusFlag) { m_HasBeenModified = true; }
		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_redo")), m_StateStack->isRedoPossible());
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_undo")), m_StateStack->isUndoPossible());
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_redo")), false);
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
		CIdentifier identifier;

		while ((identifier = m_Scenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(identifier)) { m_Scenario.getBoxDetails(identifier)->addAttribute(OV_ClassId_Selected, ""); }
			else { m_Scenario.getBoxDetails(identifier)->removeAttribute(OV_ClassId_Selected); }
		}
		while ((identifier = m_Scenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(identifier)) m_Scenario.getLinkDetails(identifier)->addAttribute(OV_ClassId_Selected, "");
			else m_Scenario.getLinkDetails(identifier)->removeAttribute(OV_ClassId_Selected);
		}

		if (manageModifiedStatusFlag) { m_HasBeenModified = true; }
		this->updateScenarioLabel();
		m_StateStack->snapshot();
	}
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_redo")), m_StateStack->isRedoPossible());
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(this->m_Application.m_pBuilderInterface, "openvibe-button_undo")), m_StateStack->isUndoPossible());
}

void CInterfacedScenario::addCommentCB(int x, int y)
{
	CIdentifier identifier;
	m_Scenario.addComment(identifier, OV_UndefinedIdentifier);
	if (x == -1 || y == -1)
	{
		GtkWidget* l_pScrolledWindow  = gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(m_scenarioDrawingArea)));
		GtkAdjustment* l_pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(l_pScrolledWindow));
		GtkAdjustment* l_pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(l_pScrolledWindow));

#if defined TARGET_OS_Linux && !defined TARGET_OS_MacOS
		x = int(gtk_adjustment_get_value(l_pHAdjustment) + gtk_adjustment_get_page_size(l_pHAdjustment) / 2);
		y = int(gtk_adjustment_get_value(l_pVAdjustment) + gtk_adjustment_get_page_size(l_pVAdjustment) / 2);
#elif defined TARGET_OS_Windows
		gint wx, wy;
		::gdk_window_get_size(gtk_widget_get_parent(GTK_WIDGET(m_scenarioDrawingArea))->window, &wx, &wy);
		x = int(gtk_adjustment_get_value(l_pHAdjustment) + int(wx / 2));
		y = int(gtk_adjustment_get_value(l_pVAdjustment) + int(wy / 2));
#else
		x = int(gtk_adjustment_get_value(l_pHAdjustment) + 32);
		y = int(gtk_adjustment_get_value(l_pVAdjustment) + 32);
#endif
	}

	CCommentProxy l_oCommentProxy(m_kernelCtx, m_Scenario, identifier);
	l_oCommentProxy.setCenter(x - m_viewOffsetX, y - m_viewOffsetY);

	// Aligns comemnts on grid
	l_oCommentProxy.setCenter(int((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0L), int((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0L));

	// Applies modifications before snapshot
	l_oCommentProxy.apply();

	CCommentEditorDialog l_oCommentEditorDialog(m_kernelCtx, *m_Scenario.getCommentDetails(identifier), m_guiFilename.c_str());
	if (!l_oCommentEditorDialog.run()) { m_Scenario.removeComment(identifier); }
	else
	{
		m_SelectedObjects.clear();
		m_SelectedObjects.insert(identifier);

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
	char l_sName[1024];
	sprintf(l_sName, "Setting %u", m_Scenario.getSettingCount() + 1);
	m_Scenario.addSetting(l_sName, OVTK_TypeId_Integer, "0", OV_Value_UndefinedIndexUInt, false, m_Scenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));

	this->redrawConfigureScenarioSettingsDialog();
}

void CInterfacedScenario::addScenarioInputCB()
{
	char l_sName[1024];
	sprintf(l_sName, "Input %u", m_Scenario.getInputCount() + 1);

	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the inputs of the box.
	m_Scenario.addInput(l_sName, OVTK_TypeId_StreamedMatrix, m_Scenario.getUnusedInputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor l_oConnectorEditor(m_kernelCtx, m_Scenario, Box_Input, m_Scenario.getInputCount() - 1, "Add Input", m_guiFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
	else { m_Scenario.removeInput(m_Scenario.getInputCount() - 1); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::editScenarioInputCB(const size_t index)
{
	CConnectorEditor l_oConnectorEditor(m_kernelCtx, m_Scenario, Box_Input, index, "Edit Input", m_guiFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::addScenarioOutputCB()
{
	char l_sName[1024];
	sprintf(l_sName, "Output %u", m_Scenario.getOutputCount() + 1);

	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the outputs of the box.
	m_Scenario.addOutput(l_sName, OVTK_TypeId_StreamedMatrix, m_Scenario.getUnusedOutputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor l_oConnectorEditor(m_kernelCtx, m_Scenario, Box_Output, m_Scenario.getOutputCount() - 1, "Add Output", m_guiFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
	else { m_Scenario.removeOutput(m_Scenario.getOutputCount() - 1); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::editScenarioOutputCB(const size_t index)
{
	CConnectorEditor l_oConnectorEditor(m_kernelCtx, m_Scenario, Box_Output, index, "Edit Output", m_guiFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::swapScenarioSettings(const size_t indexA, const size_t indexB)
{
	m_Scenario.swapSettings(indexA, indexB);
	this->redrawConfigureScenarioSettingsDialog();
}


void CInterfacedScenario::swapScenarioInputs(const size_t indexA, const size_t indexB)
{
	CIdentifier ABoxIdentifier;
	size_t ABoxInputIndex;
	CIdentifier BBoxIdentifier;
	size_t BBoxInputIndex;

	m_Scenario.getScenarioInputLink(indexA, ABoxIdentifier, ABoxInputIndex);
	m_Scenario.getScenarioInputLink(indexB, BBoxIdentifier, BBoxInputIndex);

	m_Scenario.swapInputs(indexA, indexB);

	m_Scenario.setScenarioInputLink(indexB, ABoxIdentifier, ABoxInputIndex);
	m_Scenario.setScenarioInputLink(indexA, BBoxIdentifier, BBoxInputIndex);

	this->redrawScenarioInputSettings();
	this->redraw();
}

void CInterfacedScenario::swapScenarioOutputs(const size_t indexA, const size_t indexB)
{
	CIdentifier ABoxIdentifier;
	size_t ABoxOutputIndex;
	CIdentifier BBoxIdentifier;
	size_t BBoxOutputIndex;

	m_Scenario.getScenarioOutputLink(indexA, ABoxIdentifier, ABoxOutputIndex);
	m_Scenario.getScenarioOutputLink(indexB, BBoxIdentifier, BBoxOutputIndex);

	m_Scenario.swapOutputs(indexA, indexB);

	m_Scenario.setScenarioOutputLink(indexB, ABoxIdentifier, ABoxOutputIndex);
	m_Scenario.setScenarioOutputLink(indexA, BBoxIdentifier, BBoxOutputIndex);

	this->redrawScenarioOutputSettings();
	this->redraw();
}

void CInterfacedScenario::scenarioDrawingAreaExposeCB(GdkEventExpose* /*event*/)
{
	if (m_currentMode == Mode_None)
	{
		gint l_iViewportX = -1;
		gint l_iViewportY = -1;

		gint l_iMinX = 0x7fff;
		gint l_iMaxX = -0x7fff;
		gint l_iMinY = 0x7fff;
		gint l_iMaxY = -0x7fff;

		const gint l_iMarginX = gint(round(32.0 * m_currentScale));
		const gint l_iMarginY = gint(round(32.0 * m_currentScale));

		CIdentifier l_oBoxID;
		while ((l_oBoxID = m_Scenario.getNextBoxIdentifier(l_oBoxID)) != OV_UndefinedIdentifier)
		{
			//CBoxProxy proxy(m_kernelCtx, *m_scenario.getBoxDetails(l_oBoxID));
			CBoxProxy proxy(m_kernelCtx, m_Scenario, l_oBoxID);
			l_iMinX = std::min(l_iMinX, gint((proxy.getXCenter() - 1.0 * proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxX = std::max(l_iMaxX, gint((proxy.getXCenter() + 1.0 * proxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			l_iMinY = std::min(
				l_iMinY, gint((proxy.getYCenter() - 1.0 * proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxY = std::max(
				l_iMaxY, gint((proxy.getYCenter() + 1.0 * proxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
		}

		CIdentifier l_oCommentID;
		while ((l_oCommentID = m_Scenario.getNextCommentIdentifier(l_oCommentID)) != OV_UndefinedIdentifier)
		{
			CCommentProxy l_oCommentProxy(m_kernelCtx, *m_Scenario.getCommentDetails(l_oCommentID));
			l_iMinX = std::min(
				l_iMinX, gint((l_oCommentProxy.getXCenter() - 1.0 * l_oCommentProxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxX = std::max(
				l_iMaxX, gint((l_oCommentProxy.getXCenter() + 1.0 * l_oCommentProxy.getWidth(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			l_iMinY = std::min(
				l_iMinY, gint((l_oCommentProxy.getYCenter() - 1.0 * l_oCommentProxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxY = std::max(
				l_iMaxY, gint((l_oCommentProxy.getYCenter() + 1.0 * l_oCommentProxy.getHeight(GTK_WIDGET(m_scenarioDrawingArea)) / 2) * m_currentScale));
		}

		const gint l_iNewScenarioSizeX = l_iMaxX - l_iMinX;
		const gint l_iNewScenarioSizeY = l_iMaxY - l_iMinY;
		gint l_iOldScenarioSizeX       = -1;
		gint l_iOldScenarioSizeY       = -1;

		gdk_window_get_size(GTK_WIDGET(m_scenarioViewport)->window, &l_iViewportX, &l_iViewportY);
		gtk_widget_get_size_request(GTK_WIDGET(m_scenarioDrawingArea), &l_iOldScenarioSizeX, &l_iOldScenarioSizeY);

		if (l_iNewScenarioSizeX >= 0 && l_iNewScenarioSizeY >= 0)
		{
			if (l_iOldScenarioSizeX != l_iNewScenarioSizeX + 2 * l_iMarginX || l_iOldScenarioSizeY != l_iNewScenarioSizeY + 2 * l_iMarginY)
			{
				gtk_widget_set_size_request(GTK_WIDGET(m_scenarioDrawingArea), l_iNewScenarioSizeX + 2 * l_iMarginX, l_iNewScenarioSizeY + 2 * l_iMarginY);
			}
			m_viewOffsetX = std::min(m_viewOffsetX, -l_iMaxX - l_iMarginX + std::max(l_iViewportX, l_iNewScenarioSizeX + 2 * l_iMarginX));
			m_viewOffsetX = std::max(m_viewOffsetX, -l_iMinX + l_iMarginX);
			m_viewOffsetY = std::min(m_viewOffsetY, -l_iMaxY - l_iMarginY + std::max(l_iViewportY, l_iNewScenarioSizeY + 2 * l_iMarginY));
			m_viewOffsetY = std::max(m_viewOffsetY, -l_iMinY + l_iMarginY);
		}
	}

	gint x, y;

	gdk_window_get_size(GTK_WIDGET(m_scenarioDrawingArea)->window, &x, &y);
	if (m_stencilBuffer) { g_object_unref(m_stencilBuffer); }
	m_stencilBuffer = gdk_pixmap_new(GTK_WIDGET(m_scenarioDrawingArea)->window, x, y, -1);

	GdkGC* l_pStencilGC = gdk_gc_new(m_stencilBuffer);
	GdkColor l_oColor   = { 0, 0, 0, 0 };
	gdk_gc_set_rgb_fg_color(l_pStencilGC, &l_oColor);
	gdk_draw_rectangle(GDK_DRAWABLE(m_stencilBuffer), l_pStencilGC, TRUE, 0, 0, x, y);
	g_object_unref(l_pStencilGC);

	if (this->isLocked())
	{
		l_oColor.pixel = 0;
		l_oColor.red   = 0x0f00;
		l_oColor.green = 0x0f00;
		l_oColor.blue  = 0x0f00;

		GdkGC* l_pDrawGC = gdk_gc_new(GTK_WIDGET(m_scenarioDrawingArea)->window);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
		gdk_gc_set_function(l_pDrawGC, GDK_XOR);
		gdk_draw_rectangle(GTK_WIDGET(m_scenarioDrawingArea)->window, l_pDrawGC, TRUE, 0, 0, x, y);
		g_object_unref(l_pDrawGC);
	}
	// TODO: optimize this as this will be called endlessly
	/*
	else if (false) //m_scenario.containsBoxWithDeprecatedInterfacors() 
	{
		l_oColor.pixel = 0;
		l_oColor.red = 0xffff;
		l_oColor.green = 0xefff;
		l_oColor.blue = 0xefff;

		GdkGC* l_pDrawGC = gdk_gc_new(GTK_WIDGET(m_pScenarioDrawingArea)->window);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
		gdk_gc_set_function(l_pDrawGC, GDK_AND);
		gdk_draw_rectangle(GTK_WIDGET(m_pScenarioDrawingArea)->window, l_pDrawGC, TRUE, 0, 0, x, y);
		g_object_unref(l_pDrawGC);
	}
	*/
	m_interfacedObjectId = 0;
	m_interfacedObjects.clear();

	size_t commentCount = 0;
	CIdentifier l_oCommentID;
	while ((l_oCommentID = m_Scenario.getNextCommentIdentifier(l_oCommentID)) != OV_UndefinedIdentifier)
	{
		redraw(*m_Scenario.getCommentDetails(l_oCommentID));
		commentCount++;
	}
	m_nComment = commentCount;

	size_t l_ui32BoxCount = 0;
	CIdentifier l_oBoxID;
	while ((l_oBoxID = m_Scenario.getNextBoxIdentifier(l_oBoxID)) != OV_UndefinedIdentifier)
	{
		redraw(*m_Scenario.getBoxDetails(l_oBoxID));
		l_ui32BoxCount++;
	}
	m_nBox = l_ui32BoxCount;

	size_t linkCount = 0;
	CIdentifier l_oLinkID;
	while ((l_oLinkID = m_Scenario.getNextLinkIdentifier(l_oLinkID)) != OV_UndefinedIdentifier)
	{
		redraw(*m_Scenario.getLinkDetails(l_oLinkID));
		linkCount++;
	}
	m_nLink = linkCount;

	if (m_currentMode == Mode_Selection || m_currentMode == Mode_SelectionAdd)
	{
		const int startX = int(std::min(m_pressMouseX, m_currentMouseX));
		const int startY = int(std::min(m_pressMouseY, m_currentMouseY));
		const int sizeX  = int(std::max(m_pressMouseX - m_currentMouseX, m_currentMouseX - m_pressMouseX));
		const int sizeY  = int(std::max(m_pressMouseY - m_currentMouseY, m_currentMouseY - m_pressMouseY));

		GtkWidget* l_widget = GTK_WIDGET(m_scenarioDrawingArea);
		GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);
		gdk_gc_set_function(l_pDrawGC, GDK_OR);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &gColors[Color_SelectionArea]);
		gdk_draw_rectangle(l_widget->window, l_pDrawGC, TRUE, startX, startY, sizeX, sizeY);
		gdk_gc_set_function(l_pDrawGC, GDK_COPY);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &gColors[Color_SelectionAreaBorder]);
		gdk_draw_rectangle(l_widget->window, l_pDrawGC, FALSE, startX, startY, sizeX, sizeY);
		g_object_unref(l_pDrawGC);
	}

	if (m_currentMode == Mode_Connect)
	{
		GtkWidget* l_widget = GTK_WIDGET(m_scenarioDrawingArea);
		GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &gColors[Color_Link]);
		gdk_draw_line(l_widget->window, l_pDrawGC, int(m_pressMouseX), int(m_pressMouseY), int(m_currentMouseX), int(m_currentMouseY));
		g_object_unref(l_pDrawGC);
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
		CIdentifier l_oBoxID;
		CIdentifier l_oBoxAlgorithmClassID;

		// The drag data only contains one string, for a normal box this string is its algorithmClassIdentifier
		// However since all metaboxes have the same identifier, we have added the 'identifier' of a metabox after this string
		// The identifier itself is the name of the scenario which created the metabox
		std::string l_sSelectionData(reinterpret_cast<const char*>(gtk_selection_data_get_text(selectionData)));

		// check that there is an identifier inside the string, its form is (0xXXXXXXXX, 0xXXXXXXXX)
		if (l_sSelectionData.find(')') != string::npos) { l_oBoxAlgorithmClassID.fromString(l_sSelectionData.substr(0, l_sSelectionData.find(')')).c_str()); }

		IBox* box                    = nullptr;
		const IPluginObjectDesc* POD = nullptr;

		if (l_oBoxAlgorithmClassID == OV_UndefinedIdentifier)
		{
			m_currentMouseX = x;
			m_currentMouseY = y;
			return;
		}
		if (l_oBoxAlgorithmClassID == OVP_ClassId_BoxAlgorithm_Metabox)
		{
			// extract the name of the metabox from the drag data string
			CIdentifier metaboxId;
			metaboxId.fromString(CString(l_sSelectionData.substr(l_sSelectionData.find(')') + 1).c_str()));

			//m_kernelCtx.getLogManager() << LogLevel_Info << "This is a metabox with ID " << metaboxID.c_str() << "\n";
			POD = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId);

			// insert a box into the scenario, initialize it from the proxy-descriptor from the metabox loader
			m_Scenario.addBox(l_oBoxID, *static_cast<const IBoxAlgorithmDesc*>(POD), OV_UndefinedIdentifier);

			box = m_Scenario.getBoxDetails(l_oBoxID);
			box->addAttribute(OVP_AttributeId_Metabox_ID, metaboxId.toString());
		}
		else
		{
			m_Scenario.addBox(l_oBoxID, l_oBoxAlgorithmClassID, OV_UndefinedIdentifier);

			box                  = m_Scenario.getBoxDetails(l_oBoxID);
			const CIdentifier id = box->getAlgorithmClassIdentifier();
			POD                  = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(id);
		}

		m_SelectedObjects.clear();
		m_SelectedObjects.insert(l_oBoxID);

		// If a visualization box was dropped, add it in window manager
		if (POD && POD->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxAdded(box); }
		}

		CBoxProxy proxy(m_kernelCtx, m_Scenario, l_oBoxID);
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
		std::stringstream l_oStringStream(draggedFilesPath);
		std::string l_sLine;
		std::vector<std::string> l_vFilesToOpen;
		while (std::getline(l_oStringStream, l_sLine))
		{
			// the path starts with file:/// and ends with \r\n once parsed line after line, a \r remains on Windows
			l_sLine = l_sLine.substr(8, l_sLine.length() - 9);

			// uri to path (to remove %xx escape characters):
			l_sLine = g_uri_unescape_string(l_sLine.c_str(), nullptr);

			l_vFilesToOpen.push_back(l_sLine);
		}

		for (auto& fileName : l_vFilesToOpen) { m_Application.openScenario(fileName.c_str()); }
	}
#endif
}

void CInterfacedScenario::scenarioDrawingAreaMotionNotifyCB(GtkWidget* /*widget*/, GdkEventMotion* event)
{
	// m_kernelCtx.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaMotionNotifyCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* l_pTooltip = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "tooltip"));
	gtk_widget_set_name(l_pTooltip, "gtk-tooltips");
	const size_t l_interfacedObjectId = pickInterfacedObject(int(event->x), int(event->y));
	CInterfacedObject& l_rObject      = m_interfacedObjects[l_interfacedObjectId];
	if (l_rObject.m_ID != OV_UndefinedIdentifier && l_rObject.m_ConnectorType != Box_Link && l_rObject.m_ConnectorType != Box_None)
	{
		IBox* l_pBoxDetails = m_Scenario.getBoxDetails(l_rObject.m_ID);
		if (l_pBoxDetails)
		{
			CString l_sName;
			CString l_sType;
			if (l_rObject.m_ConnectorType == Box_Input)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_ConnectorIdx, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_ConnectorIdx, l_oType);
				l_sType = m_kernelCtx.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_ConnectorType == Box_Output)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_ConnectorIdx, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_ConnectorIdx, l_oType);
				l_sType = m_kernelCtx.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_ConnectorType == Box_Update)
			{
				//m_scenario.updateBox(l_pBoxDetails->getIdentifier());
				l_sName = CString("Right click for");
				l_sType = "box update";
			}
			else if (l_rObject.m_ConnectorType == Box_ScenarioInput)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_ConnectorIdx, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_ConnectorIdx, l_oType);

				for (size_t i = 0; i < m_Scenario.getInputCount(); i++)
				{
					CIdentifier linkBoxID;
					size_t linkBoxInputIdx;

					m_Scenario.getScenarioInputLink(i, linkBoxID, linkBoxInputIdx);

					if (linkBoxID == l_pBoxDetails->getIdentifier() && linkBoxInputIdx == l_rObject.m_ConnectorIdx)
					{
						m_Scenario.getInputName(i, l_sName);
						l_sName = CString("Connected to \n") + l_sName;
						m_Scenario.getInputType(i, l_oType);
					}
				}
				l_sType = m_kernelCtx.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_ConnectorType == Box_ScenarioOutput)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_ConnectorIdx, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_ConnectorIdx, l_oType);

				for (size_t i = 0; i < m_Scenario.getOutputCount(); i++)
				{
					CIdentifier linkBoxID;
					size_t linkBoxOutputIdx;

					m_Scenario.getScenarioOutputLink(i, linkBoxID, linkBoxOutputIdx);

					if (linkBoxID == l_pBoxDetails->getIdentifier() && linkBoxOutputIdx == l_rObject.m_ConnectorIdx)
					{
						m_Scenario.getOutputName(i, l_sName);
						l_sName = CString("Connected to \n") + l_sName;
						m_Scenario.getOutputType(i, l_oType);
					}
				}
				l_sType = m_kernelCtx.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}

			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_guiBuilder, "tooltip-label_name_content")), l_sName);
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_guiBuilder, "tooltip-label_type_content")), l_sType);
			gtk_window_move(GTK_WINDOW(l_pTooltip), gint(event->x_root), gint(event->y_root) + 40);
			gtk_widget_show(l_pTooltip);
		}
	}
	else { gtk_widget_hide(l_pTooltip); }

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
			for (auto& objectId : m_SelectedObjects)
			{
				if (m_Scenario.isBox(objectId))
				{
					CBoxProxy proxy(m_kernelCtx, m_Scenario, objectId);
					proxy.setCenter(proxy.getXCenter() + int(event->x - m_currentMouseX), proxy.getYCenter() + int(event->y - m_currentMouseY));
				}
				if (m_Scenario.isComment(objectId))
				{
					CCommentProxy proxy(m_kernelCtx, m_Scenario, objectId);
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
	GtkImageMenuItem* menuitem = gtk_menu_add_new_image_menu_item(menu, icon, label);
	box_ctx_menu_cb_t l_oBoxContextMenuCB;
	l_oBoxContextMenuCB.command        = command;
	l_oBoxContextMenuCB.index          = index;
	l_oBoxContextMenuCB.secondaryIndex = index2;
	l_oBoxContextMenuCB.box            = box;
	l_oBoxContextMenuCB.scenario       = this;
	const auto mapIndex                = m_boxCtxMenuCBs.size();
	m_boxCtxMenuCBs[mapIndex]          = l_oBoxContextMenuCB;
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(cb), &m_boxCtxMenuCBs[mapIndex]);
	return menuitem;
}

void CInterfacedScenario::scenarioDrawingAreaButtonPressedCB(GtkWidget* widget, GdkEventButton* event)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonPressedCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* l_pTooltip = GTK_WIDGET(gtk_builder_get_object(m_guiBuilder, "tooltip"));
	gtk_widget_hide(l_pTooltip);
	gtk_widget_grab_focus(widget);

	m_buttonPressed |= ((event->type == GDK_BUTTON_PRESS) && (event->button == 1));
	m_pressMouseX = event->x;
	m_pressMouseY = event->y;

	size_t l_interfacedObjectId = pickInterfacedObject(int(m_pressMouseX), int(m_pressMouseY));
	m_currentObject             = m_interfacedObjects[l_interfacedObjectId];

	if (event->button == 1)
	{
		if (event->type == GDK_BUTTON_PRESS)
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
					if (m_controlPressed)
					{
						// m_vCurrentObject[m_oCurrentObject.m_id]=!m_vCurrentObject[m_oCurrentObject.m_id];
					}
					else
					{
						// m_vCurrentObject.clear();
						// m_vCurrentObject[m_oCurrentObject.m_id]=true;
					}
				}
			}
		}
		else if (event->type == GDK_2BUTTON_PRESS)
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
					IBox* l_pBox = m_Scenario.getBoxDetails(m_currentObject.m_ID);
					if (l_pBox)
					{
						if ((m_currentObject.m_ConnectorType == Box_Input && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
							|| (m_currentObject.m_ConnectorType == Box_Output && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)))
						{
							CConnectorEditor l_oConnectorEditor(m_kernelCtx, *l_pBox, m_currentObject.m_ConnectorType, m_currentObject.m_ConnectorIdx,
																m_currentObject.m_ConnectorType == Box_Input ? "Edit Input" : "Edit Output",
																m_guiFilename.c_str());
							if (l_oConnectorEditor.run()) { this->snapshotCB(); }
						}
					}
				}
				else
				{
					if (m_Scenario.isBox(m_currentObject.m_ID))
					{
						IBox* l_pBox = m_Scenario.getBoxDetails(m_currentObject.m_ID);
						if (l_pBox)
						{
							CBoxConfigurationDialog
									l_oBoxConfigurationDialog(m_kernelCtx, *l_pBox, m_guiFilename.c_str(), m_guiSettingsFilename.c_str(), false);
							if (l_oBoxConfigurationDialog.run()) { this->snapshotCB(); }
						}
					}
					if (m_Scenario.isComment(m_currentObject.m_ID))
					{
						IComment* l_pComment = m_Scenario.getCommentDetails(m_currentObject.m_ID);
						if (l_pComment)
						{
							CCommentEditorDialog l_oCommentEditorDialog(m_kernelCtx, *l_pComment, m_guiFilename.c_str());
							if (l_oCommentEditorDialog.run()) { this->snapshotCB(); }
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
			GtkMenu* l_pMenu  = GTK_MENU(gtk_menu_new());
			m_boxCtxMenuCBs.clear();

			// -------------- SELECTION -----------

			if (this->hasSelection()) { addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_CUT, "cut", context_menu_cb, nullptr, ContextMenu_SelectionCut, unused); }
			if (this->hasSelection())
			{
				addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_COPY, "copy", context_menu_cb, nullptr, ContextMenu_SelectionCopy, unused);
			}
			if ((m_Application.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
				|| (m_Application.m_pClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier))
			{
				addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_PASTE, "paste", context_menu_cb, nullptr, ContextMenu_SelectionPaste, unused);
			}
			if (this->hasSelection())
			{
				addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_DELETE, "delete", context_menu_cb, nullptr, ContextMenu_SelectionDelete, unused);
			}

			if (m_currentObject.m_ID != OV_UndefinedIdentifier && m_Scenario.isBox(m_currentObject.m_ID))
			{
				IBox* l_pBox = m_Scenario.getBoxDetails(m_currentObject.m_ID);
				if (l_pBox)
				{
					size_t i, j;
					char l_sCompleteName[1024];

					if (!m_boxCtxMenuCBs.empty()) { gtk_menu_add_separator_menu_item(l_pMenu); }

					bool l_bFlagToBeUpdated                  = l_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated);
					bool l_bFlagPendingDeprecatedInterfacors = l_pBox->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors);

					// -------------- INPUTS --------------
					bool l_bFlagCanAddInput         = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput);
					bool l_bFlagCanModifyInput      = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput);
					bool l_bCanConnectScenarioInput = (l_pBox->getInputCount() > 0 && m_Scenario.getInputCount() > 0);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (
							l_bFlagCanAddInput || l_bFlagCanModifyInput || l_bCanConnectScenarioInput))
					{
						size_t l_ui32FixedInputCount = 0;
						sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialInputCount).toASCIIString(), "%d", &l_ui32FixedInputCount);
						GtkMenu* l_pMenuInput              = GTK_MENU(gtk_menu_new());
						GtkImageMenuItem* l_pMenuItemInput = gtk_menu_add_new_image_menu_item(l_pMenu, GTK_STOCK_PROPERTIES, "inputs");
						for (i = 0; i < l_pBox->getInputCount(); ++i)
						{
							CString l_sName;
							CIdentifier l_oType;
							CIdentifier identifier;
							l_pBox->getInputName(i, l_sName);
							l_pBox->getInputType(i, l_oType);
							identifier = l_pBox->getIdentifier();
							sprintf(l_sCompleteName, "%i : %s", int(i + 1), l_sName.toASCIIString());
							GtkImageMenuItem* l_pMenuInputMenuItem = gtk_menu_add_new_image_menu_item(l_pMenuInput, GTK_STOCK_PROPERTIES, l_sCompleteName);

							GtkMenu* l_pMenuInputMenuAction = GTK_MENU(gtk_menu_new());

							if (l_bCanConnectScenarioInput)
							{
								for (j = 0; j < m_Scenario.getInputCount(); ++j)
								{
									char l_sScenarioInputNameComplete[1024];
									CString l_sScenarioInputName;
									CIdentifier scenarioLinkBoxIdentifier;
									CIdentifier scenarioInputType;
									auto linkInputIdx = size_t(-1);
									m_Scenario.getInputName(j, l_sScenarioInputName);
									m_Scenario.getInputType(j, scenarioInputType);
									m_Scenario.getScenarioInputLink(j, scenarioLinkBoxIdentifier, linkInputIdx);
									sprintf(l_sScenarioInputNameComplete, "%u: %s", j + 1, l_sScenarioInputName.toASCIIString());
									if (scenarioLinkBoxIdentifier == identifier && linkInputIdx == i)
									{
										addNewImageMenuItemWithCBGeneric(l_pMenuInputMenuAction, GTK_STOCK_DISCONNECT,
																		 (CString("disconnect from ") + CString(l_sScenarioInputNameComplete)).toASCIIString(),
																		 context_menu_cb, l_pBox, ContextMenu_BoxDisconnectScenarioInput, i, j);
									}
									else
									{
										if (m_kernelCtx.getTypeManager().isDerivedFromStream(scenarioInputType, l_oType))
										{
											addNewImageMenuItemWithCBGeneric(l_pMenuInputMenuAction, GTK_STOCK_CONNECT,
																			 (CString("connect to ") + CString(l_sScenarioInputNameComplete)).toASCIIString(),
																			 context_menu_cb, l_pBox, ContextMenu_BoxConnectScenarioInput, i, j);
										}
									}
								}
							}

							if (l_bFlagCanModifyInput)
							{
								addNewImageMenuItemWithCB(l_pMenuInputMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox,
														  ContextMenu_BoxEditInput, i);
							}

							if (l_bFlagCanAddInput && l_ui32FixedInputCount <= i)
							{
								addNewImageMenuItemWithCB(l_pMenuInputMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox,
														  ContextMenu_BoxRemoveInput, i);
							}

							if (gtk_container_get_children_count(GTK_CONTAINER(l_pMenuInputMenuAction)) > 0)
							{
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuInputMenuItem), GTK_WIDGET(l_pMenuInputMenuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuInputMenuItem), false); }
						}
						gtk_menu_add_separator_menu_item(l_pMenuInput);
						if (l_bFlagCanAddInput)
						{
							addNewImageMenuItemWithCB(l_pMenuInput, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddInput, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemInput), GTK_WIDGET(l_pMenuInput));
					}

					// -------------- OUTPUTS --------------

					bool l_bFlagCanAddOutput         = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput);
					bool l_bFlagCanModifyOutput      = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput);
					bool l_bCanConnectScenarioOutput = (l_pBox->getOutputCount() > 0 && m_Scenario.getOutputCount() > 0);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (
							l_bFlagCanAddOutput || l_bFlagCanModifyOutput || l_bCanConnectScenarioOutput))
					{
						size_t l_ui32FixedOutputCount = 0;
						sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialOutputCount).toASCIIString(), "%d", &l_ui32FixedOutputCount);
						GtkImageMenuItem* l_pMenuItemOutput = gtk_menu_add_new_image_menu_item(l_pMenu, GTK_STOCK_PROPERTIES, "outputs");
						GtkMenu* l_pMenuOutput              = GTK_MENU(gtk_menu_new());
						for (i = 0; i < l_pBox->getOutputCount(); ++i)
						{
							CString l_sName;
							CIdentifier l_oType;
							CIdentifier identifier;
							l_pBox->getOutputName(i, l_sName);
							l_pBox->getOutputType(i, l_oType);
							identifier = l_pBox->getIdentifier();
							sprintf(l_sCompleteName, "%i : %s", int(i) + 1, l_sName.toASCIIString());
							GtkImageMenuItem* l_pMenuOutputMenuItem = gtk_menu_add_new_image_menu_item(l_pMenuOutput, GTK_STOCK_PROPERTIES, l_sCompleteName);

							GtkMenu* l_pMenuOutputMenuAction = GTK_MENU(gtk_menu_new());

							if (l_bCanConnectScenarioOutput)
							{
								for (j = 0; j < m_Scenario.getOutputCount(); ++j)
								{
									char l_sScenarioOutputNameComplete[1024];
									CString l_sScenarioOutputName;
									CIdentifier scenarioLinkBoxIdentifier;
									CIdentifier scenarioOutputType;
									auto linkOutputIdx = size_t(-1);
									m_Scenario.getOutputName(j, l_sScenarioOutputName);
									m_Scenario.getOutputType(j, scenarioOutputType);
									m_Scenario.getScenarioOutputLink(j, scenarioLinkBoxIdentifier, linkOutputIdx);
									sprintf(l_sScenarioOutputNameComplete, "%u: %s", j + 1, l_sScenarioOutputName.toASCIIString());
									if (scenarioLinkBoxIdentifier == identifier && linkOutputIdx == i)
									{
										addNewImageMenuItemWithCBGeneric(l_pMenuOutputMenuAction, GTK_STOCK_DISCONNECT,
																		 (CString("disconnect from ") + CString(l_sScenarioOutputNameComplete)).toASCIIString(),
																		 context_menu_cb, l_pBox, ContextMenu_BoxDisconnectScenarioOutput, i, j);
									}
									else if (m_kernelCtx.getTypeManager().isDerivedFromStream(l_oType, scenarioOutputType))
									{
										addNewImageMenuItemWithCBGeneric(l_pMenuOutputMenuAction, GTK_STOCK_CONNECT,
																		 (CString("connect to ") + CString(l_sScenarioOutputNameComplete)).toASCIIString(),
																		 context_menu_cb, l_pBox, ContextMenu_BoxConnectScenarioOutput, i, j);
									}
								}
							}

							if (l_bFlagCanModifyOutput)
							{
								addNewImageMenuItemWithCB(l_pMenuOutputMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox,
														  ContextMenu_BoxEditOutput, i);
							}
							if (l_bFlagCanAddOutput && l_ui32FixedOutputCount <= i)
							{
								addNewImageMenuItemWithCB(l_pMenuOutputMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox,
														  ContextMenu_BoxRemoveOutput, i);
							}

							if (gtk_container_get_children_count(GTK_CONTAINER(l_pMenuOutputMenuAction)) > 0)
							{
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuOutputMenuItem), GTK_WIDGET(l_pMenuOutputMenuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuOutputMenuItem), false); }
						}
						gtk_menu_add_separator_menu_item(l_pMenuOutput);
						if (l_bFlagCanAddOutput)
						{
							addNewImageMenuItemWithCB(l_pMenuOutput, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddOutput, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemOutput), GTK_WIDGET(l_pMenuOutput));
					}

					// -------------- SETTINGS --------------

					bool l_bFlagCanAddSetting    = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting);
					bool l_bFlagCanModifySetting = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (l_bFlagCanAddSetting || l_bFlagCanModifySetting))
					{
						size_t l_ui32FixedSettingCount = 0;
						sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialSettingCount).toASCIIString(), "%d", &l_ui32FixedSettingCount);
						GtkImageMenuItem* l_pMenuItemSetting = gtk_menu_add_new_image_menu_item(l_pMenu, GTK_STOCK_PROPERTIES, "modify settings");
						GtkMenu* l_pMenuSetting              = GTK_MENU(gtk_menu_new());
						for (i = 0; i < l_pBox->getSettingCount(); ++i)
						{
							CString l_sName;
							CIdentifier l_oType;
							l_pBox->getSettingName(i, l_sName);
							l_pBox->getSettingType(i, l_oType);
							sprintf(l_sCompleteName, "%i : %s", int(i + 1), l_sName.toASCIIString());
							GtkImageMenuItem* l_pMenuSettingMenuItem = gtk_menu_add_new_image_menu_item(l_pMenuSetting, GTK_STOCK_PROPERTIES, l_sCompleteName);

							if (l_bFlagCanModifySetting || l_ui32FixedSettingCount <= i)
							{
								GtkMenu* l_pMenuSettingMenuAction = GTK_MENU(gtk_menu_new());
								if (l_bFlagCanModifySetting)
								{
									addNewImageMenuItemWithCB(l_pMenuSettingMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox,
															  ContextMenu_BoxEditSetting, i);
								}
								if (l_bFlagCanAddSetting && l_ui32FixedSettingCount <= i)
								{
									addNewImageMenuItemWithCB(l_pMenuSettingMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox,
															  ContextMenu_BoxRemoveSetting, i);
								}
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuSettingMenuItem), GTK_WIDGET(l_pMenuSettingMenuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuSettingMenuItem), false); }
						}
						gtk_menu_add_separator_menu_item(l_pMenuSetting);
						if (l_bFlagCanAddSetting)
						{
							addNewImageMenuItemWithCB(l_pMenuSetting, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddSetting, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemSetting), GTK_WIDGET(l_pMenuSetting));
					}

					// -------------- ABOUT / RENAME --------------

					if (!m_boxCtxMenuCBs.empty()) { gtk_menu_add_separator_menu_item(l_pMenu); }
					if (l_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated))
					{
						auto updateMenuItem = addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_REFRESH, "update box", context_menu_cb, l_pBox,
																		ContextMenu_BoxUpdate, unused);
						if (l_pBox->hasAttribute(OV_AttributeId_Box_FlagNeedsManualUpdate)
							|| l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput)
							|| l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput)
							|| l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting)
							|| l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput)
							|| l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)
							|| l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
						{
							gtk_widget_set_sensitive(GTK_WIDGET(updateMenuItem), FALSE);
							gtk_widget_set_tooltip_text(GTK_WIDGET(updateMenuItem), "Box must be manually updated due to its complexity.");
						}
					}
					if (l_pBox->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors))
					{
						addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_REFRESH, "remove deprecated I/O/S", context_menu_cb, l_pBox,
												  ContextMenu_BoxRemoveDeprecatedInterfacors, unused);
					}
					addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_EDIT, "rename box...", context_menu_cb, l_pBox, ContextMenu_BoxRename, unused);
					if (l_pBox->getSettingCount() != 0)
					{
						addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_PREFERENCES, "configure box...", context_menu_cb, l_pBox, ContextMenu_BoxConfigure,
												  unused);
					}
					// Add this option only if the user has the authorization to open a metabox
					if (l_pBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
					{
						CIdentifier metaboxId;
						metaboxId.fromString(l_pBox->getAttributeValue(OVP_AttributeId_Metabox_ID));

						std::string metaboxScenarioPathString(m_kernelCtx.getMetaboxManager().getMetaboxFilePath(metaboxId).toASCIIString());
						std::string metaboxScenarioExtension = boost::filesystem::extension(metaboxScenarioPathString);
						bool canImportFile                   = false;

						CString fileNameExtension;
						while ((fileNameExtension = m_kernelCtx
													.getScenarioManager().getNextScenarioImporter(OVD_ScenarioImportContext_OpenScenario, fileNameExtension)) !=
							   CString(""))
						{
							if (metaboxScenarioExtension == fileNameExtension.toASCIIString())
							{
								canImportFile = true;
								break;
							}
						}

						if (canImportFile)
						{
							addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_PREFERENCES, "open this meta box in editor", context_menu_cb, l_pBox,
													  ContextMenu_BoxEditMetabox, unused);
						}
					}
					addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_CONNECT, "enable box", context_menu_cb, l_pBox, ContextMenu_BoxEnable, unused);
					addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_DISCONNECT, "disable box", context_menu_cb, l_pBox, ContextMenu_BoxDisable, unused);
					addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_CUT, "delete box", context_menu_cb, l_pBox, ContextMenu_BoxDelete, unused);
					addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_HELP, "box documentation...", context_menu_cb, l_pBox, ContextMenu_BoxDocumentation, unused);
					addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_ABOUT, "about box...", context_menu_cb, l_pBox, ContextMenu_BoxAbout, unused);
				}
			}

			gtk_menu_add_separator_menu_item(l_pMenu);
			addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_EDIT, "add comment to scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAddComment, unused);
			addNewImageMenuItemWithCB(l_pMenu, GTK_STOCK_ABOUT, "about scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAbout, unused);

			// -------------- RUN --------------

			gtk_widget_show_all(GTK_WIDGET(l_pMenu));
			gtk_menu_popup(l_pMenu, nullptr, nullptr, nullptr, nullptr, 3, event->time);
			if (m_boxCtxMenuCBs.empty()) { gtk_menu_popdown(l_pMenu); }
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
		const int l_iStartX = int(std::min(m_pressMouseX, m_currentMouseX));
		const int l_iStartY = int(std::min(m_pressMouseY, m_currentMouseY));
		const int l_iSizeX  = int(std::max(m_pressMouseX - m_currentMouseX, m_currentMouseX - m_pressMouseX));
		const int l_iSizeY  = int(std::max(m_pressMouseY - m_currentMouseY, m_currentMouseY - m_pressMouseY));

		if (m_currentMode == Mode_Selection || m_currentMode == Mode_SelectionAdd)
		{
			if (m_currentMode == Mode_Selection) { m_SelectedObjects.clear(); }
			pickInterfacedObject(l_iStartX, l_iStartY, l_iSizeX, l_iSizeY);
		}
		if (m_currentMode == Mode_Connect)
		{
			bool l_bIsActuallyConnecting             = false;
			const bool l_bConnectionIsMessage        = false;
			const size_t l_interfacedObjectId        = pickInterfacedObject(int(m_releaseMouseX), int(m_releaseMouseY));
			const CInterfacedObject l_oCurrentObject = m_interfacedObjects[l_interfacedObjectId];
			CInterfacedObject l_oSourceObject;
			CInterfacedObject l_oTargetObject;
			if (l_oCurrentObject.m_ConnectorType == Box_Output && m_currentObject.m_ConnectorType == Box_Input)
			{
				l_oSourceObject         = l_oCurrentObject;
				l_oTargetObject         = m_currentObject;
				l_bIsActuallyConnecting = true;
			}
			if (l_oCurrentObject.m_ConnectorType == Box_Input && m_currentObject.m_ConnectorType == Box_Output)
			{
				l_oSourceObject         = m_currentObject;
				l_oTargetObject         = l_oCurrentObject;
				l_bIsActuallyConnecting = true;
			}
			//
			if (l_bIsActuallyConnecting)
			{
				CIdentifier l_oSourceTypeID;
				CIdentifier l_oTargetTypeID;
				const IBox* l_pSourceBox = m_Scenario.getBoxDetails(l_oSourceObject.m_ID);
				const IBox* l_pTargetBox = m_Scenario.getBoxDetails(l_oTargetObject.m_ID);
				if (l_pSourceBox && l_pTargetBox)
				{
					l_pSourceBox->getOutputType(l_oSourceObject.m_ConnectorIdx, l_oSourceTypeID);
					l_pTargetBox->getInputType(l_oTargetObject.m_ConnectorIdx, l_oTargetTypeID);

					bool hasDeprecatedInput = false;
					l_pSourceBox->getInterfacorDeprecatedStatus(Output, l_oSourceObject.m_ConnectorIdx, hasDeprecatedInput);
					bool hasDeprecatedOutput = false;
					l_pTargetBox->getInterfacorDeprecatedStatus(Input, l_oTargetObject.m_ConnectorIdx, hasDeprecatedOutput);

					if ((m_kernelCtx.getTypeManager().isDerivedFromStream(l_oSourceTypeID, l_oTargetTypeID)
						 || m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_AllowUpCastConnection}", false)) && (!l_bConnectionIsMessage))
					{
						if (!hasDeprecatedInput && !hasDeprecatedOutput)
						{
							CIdentifier l_oLinkID;
							m_Scenario.connect(l_oLinkID, l_oSourceObject.m_ID, l_oSourceObject.m_ConnectorIdx, l_oTargetObject.m_ID,
											   l_oTargetObject.m_ConnectorIdx, OV_UndefinedIdentifier);
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
			if (l_iSizeX == 0 && l_iSizeY == 0)
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
				for (auto& objectId : m_SelectedObjects)
				{
					if (m_Scenario.isBox(objectId))
					{
						CBoxProxy proxy(m_kernelCtx, m_Scenario, objectId);
						proxy.setCenter(((proxy.getXCenter() + 8) & 0xfffffff0), ((proxy.getYCenter() + 8) & 0xfffffff0));
					}
					if (m_Scenario.isComment(objectId))
					{
						CCommentProxy proxy(m_kernelCtx, m_Scenario, objectId);
						proxy.setCenter(((proxy.getXCenter() + 8) & 0xfffffff0), ((proxy.getYCenter() + 8) & 0xfffffff0));
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
		CIdentifier identifier;
		while ((identifier = m_Scenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(identifier); }
		while ((identifier = m_Scenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(identifier); }
		while ((identifier = m_Scenario.getNextCommentIdentifier(identifier)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(identifier); }
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
		gint iX = 0;
		gint iY = 0;
		gdk_window_get_pointer(GTK_WIDGET(m_scenarioDrawingArea)->window, &iX, &iY, nullptr);

		this->addCommentCB(iX, iY);
	}

	if (event->keyval == GDK_F12 && m_shiftPressed)
	{
		CIdentifier identifier;
		while ((identifier = m_Scenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			IBox* l_pBox               = m_Scenario.getBoxDetails(identifier);
			CIdentifier l_oAlgorithmID = l_pBox->getAlgorithmClassIdentifier();
			CIdentifier l_oHashValue   = m_kernelCtx.getPluginManager().getPluginObjectHashValue(l_oAlgorithmID);
			if (l_pBox->hasAttribute(OV_AttributeId_Box_InitialPrototypeHashValue))
			{
				l_pBox->setAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue, l_oHashValue.toString());
			}
			else { l_pBox->addAttribute(OV_AttributeId_Box_InitialPrototypeHashValue, l_oHashValue.toString()); }
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
	map<CIdentifier, CIdentifier> l_vIdMapping;
	m_Application.m_pClipboardScenario->clear();

	// Copies boxes to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId))
		{
			CIdentifier l_oNewID;
			const IBox* l_pBox = m_Scenario.getBoxDetails(objectId);
			m_Application.m_pClipboardScenario->addBox(l_oNewID, *l_pBox, objectId);
			l_vIdMapping[objectId] = l_oNewID;
		}
	}

	// Copies comments to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isComment(objectId))
		{
			CIdentifier l_oNewID;
			const IComment* l_pComment = m_Scenario.getCommentDetails(objectId);
			m_Application.m_pClipboardScenario->addComment(l_oNewID, *l_pComment, objectId);
			l_vIdMapping[objectId] = l_oNewID;
		}
	}

	// Copies links to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isLink(objectId))
		{
			CIdentifier l_oNewID;
			const ILink* l_pLink = m_Scenario.getLinkDetails(objectId);

			// Connect link only if the source and target boxes are copied
			if (l_vIdMapping.find(l_pLink->getSourceBoxIdentifier()) != l_vIdMapping.end()
				&& l_vIdMapping.find(l_pLink->getTargetBoxIdentifier()) != l_vIdMapping.end())
			{
				m_Application.m_pClipboardScenario->connect(l_oNewID, l_vIdMapping[l_pLink->getSourceBoxIdentifier()], l_pLink->getSourceBoxOutputIndex(),
															l_vIdMapping[l_pLink->getTargetBoxIdentifier()], l_pLink->getTargetBoxInputIndex(),
															l_pLink->getIdentifier());
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
	CIdentifier identifier;
	map<CIdentifier, CIdentifier> l_vIdMapping;
	/*
	int l_iCenterX=0;
	int l_iCenterY=0;
	*/
	int l_iTopmostLeftmostCopiedBoxCenterX = 1 << 15;
	int l_iTopmostLeftmostCopiedBoxCenterY = 1 << 15;
	// std::cout << "Mouse position : " << m_currentMouseX << "/" << m_currentMouseY << std::endl;

	// Pastes boxes from clipboard
	while ((identifier = m_Application.m_pClipboardScenario->getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewID;
		IBox* l_pBox = m_Application.m_pClipboardScenario->getBoxDetails(identifier);
		m_Scenario.addBox(l_oNewID, *l_pBox, identifier);
		l_vIdMapping[identifier] = l_oNewID;

		// Updates visualization manager
		CIdentifier l_oBoxAlgorithmID   = l_pBox->getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(l_oBoxAlgorithmID);

		// If a visualization box was dropped, add it in window manager
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxAdded(m_Scenario.getBoxDetails(l_oNewID)); }
		}

		CBoxProxy proxy(m_kernelCtx, m_Scenario, l_oNewID);

		// get the position of the topmost-leftmost box (always position on an actual box so when user pastes he sees something)
		if (proxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterX && proxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterY)
		{
			l_iTopmostLeftmostCopiedBoxCenterX = proxy.getXCenter();
			l_iTopmostLeftmostCopiedBoxCenterY = proxy.getYCenter();
		}
	}

	// Pastes comments from clipboard
	while ((identifier = m_Application.m_pClipboardScenario->getNextCommentIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewID;
		IComment* l_pComment = m_Application.m_pClipboardScenario->getCommentDetails(identifier);
		m_Scenario.addComment(l_oNewID, *l_pComment, identifier);
		l_vIdMapping[identifier] = l_oNewID;

		CCommentProxy proxy(m_kernelCtx, m_Scenario, l_oNewID);

		if (proxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterX && proxy.getYCenter() < l_iTopmostLeftmostCopiedBoxCenterY)
		{
			l_iTopmostLeftmostCopiedBoxCenterX = proxy.getXCenter();
			l_iTopmostLeftmostCopiedBoxCenterY = proxy.getYCenter();
		}
	}

	// Pastes links from clipboard
	while ((identifier = m_Application.m_pClipboardScenario->getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewID;
		ILink* l_pLink = m_Application.m_pClipboardScenario->getLinkDetails(identifier);
		m_Scenario.connect(l_oNewID, l_vIdMapping[l_pLink->getSourceBoxIdentifier()], l_pLink->getSourceBoxOutputIndex(),
						   l_vIdMapping[l_pLink->getTargetBoxIdentifier()], l_pLink->getTargetBoxInputIndex(), l_pLink->getIdentifier());
	}

	// Makes pasted stuff the default selection
	// Moves boxes under cursor
	// Moves comments under cursor
	if (m_Application.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier
		|| m_Application.m_pClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
	{
		m_SelectedObjects.clear();
		for (auto& it : l_vIdMapping)
		{
			m_SelectedObjects.insert(it.second);

			if (m_Scenario.isBox(it.second))
			{
				// Moves boxes under cursor
				CBoxProxy proxy(m_kernelCtx, m_Scenario, it.second);
				proxy.setCenter(int(proxy.getXCenter() + m_currentMouseX) - l_iTopmostLeftmostCopiedBoxCenterX - m_viewOffsetX,
								int(proxy.getYCenter() + m_currentMouseY) - l_iTopmostLeftmostCopiedBoxCenterY - m_viewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns boxes on grid
				proxy.setCenter(int((proxy.getXCenter() + 8) & 0xfffffff0L), int((proxy.getYCenter() + 8) & 0xfffffff0L));
			}

			if (m_Scenario.isComment(it.second))
			{
				// Moves commentes under cursor
				CCommentProxy proxy(m_kernelCtx, m_Scenario, it.second);
				proxy.setCenter(int(proxy.getXCenter() + m_currentMouseX) - l_iTopmostLeftmostCopiedBoxCenterX - m_viewOffsetX,
								int(proxy.getYCenter() + m_currentMouseY) - l_iTopmostLeftmostCopiedBoxCenterY - m_viewOffsetY);
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
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId)) { this->deleteBox(objectId); }
		if (m_Scenario.isComment(objectId))
		{
			// removes comment from scenario
			m_Scenario.removeComment(objectId);
		}
		if (m_Scenario.isLink(objectId))
		{
			// removes link from scenario
			m_Scenario.disconnect(objectId);
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
	const IPluginObjectDesc* l_pPluginObjectDescriptor = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(box.getAlgorithmClassIdentifier());
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRenameCB\n";

	if (box.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
		l_pPluginObjectDescriptor = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
	}

	CRenameDialog l_oRename(m_kernelCtx, box.getName(), l_pPluginObjectDescriptor ? l_pPluginObjectDescriptor->getName() : box.getName(),
							m_guiFilename.c_str());
	if (l_oRename.run())
	{
		box.setName(l_oRename.getResult());

		//check whether it is a visualization box
		const CIdentifier id            = box.getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(id);

		//if a visualization box was renamed, tell window manager about it
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxRenamed(box.getIdentifier()); }
		}
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxRenameAllCB()
{
	//we find all selected boxes
	map<CIdentifier, CIdentifier> l_vSelectedBox; // map(object,class)
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_Scenario.isBox(objectId)) { l_vSelectedBox[objectId] = m_Scenario.getBoxDetails(objectId)->getAlgorithmClassIdentifier(); }
	}

	if (!l_vSelectedBox.empty())
	{
		bool l_bDialogOk   = true;
		bool l_bFirstBox   = true;
		CString l_sNewName = "";
		for (auto it = l_vSelectedBox.begin(); it != l_vSelectedBox.end() && l_bDialogOk; ++it)
		{
			if (it->second != OV_UndefinedIdentifier)
			{
				if (m_kernelCtx.getPluginManager().canCreatePluginObject(it->second) || it->second == OVP_ClassId_BoxAlgorithm_Metabox)
				{
					IBox* l_pBox = m_Scenario.getBoxDetails(it->first);
					if (l_bFirstBox)
					{
						l_bFirstBox                                        = false;
						const IPluginObjectDesc* l_pPluginObjectDescriptor = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(
							l_pBox->getAlgorithmClassIdentifier());
						if (l_pBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
						{
							CIdentifier metaboxId;
							metaboxId.fromString(l_pBox->getAttributeValue(OVP_AttributeId_Metabox_ID));
							l_pPluginObjectDescriptor = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
						}

						CRenameDialog l_oRename(m_kernelCtx, l_pBox->getName(),
												l_pPluginObjectDescriptor ? l_pPluginObjectDescriptor->getName() : l_pBox->getName(), m_guiFilename.c_str());
						if (l_oRename.run()) { l_sNewName = l_oRename.getResult(); }
						else
						{
							// no rename at all.
							l_bDialogOk = false;
						}
					}
					if (l_bDialogOk)
					{
						l_pBox->setName(l_sNewName);

						//check whether it is a visualization box
						CIdentifier l_oId               = l_pBox->getAlgorithmClassIdentifier();
						const IPluginObjectDesc* l_pPOD = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(l_oId);

						//if a visualization box was renamed, tell window manager about it
						if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
						{
							if (m_DesignerVisualization) { m_DesignerVisualization->onVisualizationBoxRenamed(l_pBox->getIdentifier()); }
						}
					}
				}
			}
		}
		if (l_bDialogOk) { this->snapshotCB(); }
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
		CConnectorEditor l_oConnectorEditor(m_kernelCtx, box, Box_Input, box.getInputCount() - 1, "Add Input", m_guiFilename.c_str());
		if (l_oConnectorEditor.run()) { this->snapshotCB(); }
		else { box.removeInput(box.getInputCount() - 1); }
	}
	else { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxEditInputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditInputCB\n";

	CConnectorEditor l_oConnectorEditor(m_kernelCtx, box, Box_Input, index, "Edit Input", m_guiFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
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
		CConnectorEditor l_oConnectorEditor(m_kernelCtx, box, Box_Output, box.getOutputCount() - 1, "Add Output", m_guiFilename.c_str());
		if (l_oConnectorEditor.run()) { this->snapshotCB(); }
		else { box.removeOutput(box.getOutputCount() - 1); }
	}
	else { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxEditOutputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditOutputCB\n";

	CConnectorEditor l_oConnectorEditor(m_kernelCtx, box, Box_Output, index, "Edit Output", m_guiFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveOutputCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveOutputCB\n";
	box.removeOutput(index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioInputCB(IBox& box, const size_t boxInputIdx, const size_t scenarioInputIdx)
{
	//	m_kernelCtx.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioInputCB : box = " << box.getIdentifier().toString() << " box input = " << boxInputIdx << " , scenario input = " << scenarioInputIdx << "\n";
	m_Scenario.setScenarioInputLink(scenarioInputIdx, box.getIdentifier(), boxInputIdx);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioOutputCB(IBox& box, const size_t boxOutputIdx, const size_t scenarioOutputIdx)
{
	//	m_kernelCtx.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioOutputCB : box = " << box.getIdentifier().toString() << " box Output = " << boxOutputIdx << " , scenario Output = " << scenarioOutputIdx << "\n";
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
	const size_t oldSettingsCount = box.getSettingCount();
	box.addSetting("New setting", OV_UndefinedIdentifier, "", OV_Value_UndefinedIndexUInt, false,
				   m_Scenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));
	const size_t newSettingsCount = box.getSettingCount();
	// Check that at least one setting was added
	if (newSettingsCount > oldSettingsCount && box.hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
	{
		CSettingEditorDialog l_oSettingEditorDialog(m_kernelCtx, box, oldSettingsCount, "Add Setting", m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
		if (l_oSettingEditorDialog.run()) { this->snapshotCB(); }
		else { for (size_t i = oldSettingsCount; i < newSettingsCount; ++i) { box.removeSetting(i); } }
	}
	else
	{
		if (newSettingsCount > oldSettingsCount) { this->snapshotCB(); }
		else
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "No setting could be added to the box.\n";
			return;
		}
	}
	// Add an information message to inform the user about the new settings
	m_kernelCtx.getLogManager() << LogLevel_Info << "[" << newSettingsCount - oldSettingsCount << "] new setting(s) was(were) added to the box ["
			<< box.getName().toASCIIString() << "]: ";
	for (size_t i = oldSettingsCount; i < newSettingsCount; ++i)
	{
		CString l_sSettingName;
		box.getSettingName(i, l_sSettingName);
		m_kernelCtx.getLogManager() << "[" << l_sSettingName << "] ";
	}
	m_kernelCtx.getLogManager() << "\n";
	// After adding setting, open configuration so that the user can see the effects.
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_kernelCtx, box, m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
	l_oBoxConfigurationDialog.run();
}

void CInterfacedScenario::contextMenuBoxEditSettingCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditSettingCB\n";
	CSettingEditorDialog l_oSettingEditorDialog(m_kernelCtx, box, index, "Edit Setting", m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
	if (l_oSettingEditorDialog.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveSettingCB(IBox& box, const size_t index)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveSettingCB\n";
	const size_t oldSettingsCount = box.getSettingCount();
	if (box.removeSetting(index))
	{
		const size_t newSettingsCount = box.getSettingCount();
		this->snapshotCB();
		m_kernelCtx.getLogManager() << LogLevel_Info << "[" << oldSettingsCount - newSettingsCount << "] setting(s) was(were) removed from box ["
				<< box.getName().toASCIIString() << "] \n";
	}
	else
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "The setting with index [" << index << "] could not be removed from box ["
				<< box.getName().toASCIIString() << "] \n";
	}
}

void CInterfacedScenario::contextMenuBoxConfigureCB(IBox& box)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxConfigureCB\n";
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_kernelCtx, box, m_guiFilename.c_str(), m_guiSettingsFilename.c_str());
	l_oBoxConfigurationDialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAboutCB(IBox& box) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxAboutCB\n";
	if (box.getAlgorithmClassIdentifier() != OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CAboutPluginDialog l_oAboutPluginDialog(m_kernelCtx, box.getAlgorithmClassIdentifier(), m_guiFilename.c_str());
		l_oAboutPluginDialog.run();
	}
	else
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
		CAboutPluginDialog l_oAboutPluginDialog(m_kernelCtx, m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId), m_guiFilename.c_str());
		l_oAboutPluginDialog.run();
	}
}

void CInterfacedScenario::contextMenuBoxEditMetaboxCB(IBox& box) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxEditMetaboxCB\n";

	CIdentifier metaboxId;
	metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_ID));
	const CString metaboxScenarioPath(m_kernelCtx.getMetaboxManager().getMetaboxFilePath(metaboxId));

	m_Application.openScenario(metaboxScenarioPath.toASCIIString());
}

bool CInterfacedScenario::browseURL(const CString& url, const CString& browserPrefix, const CString& browserPostfix) const
{
	m_kernelCtx.getLogManager() << LogLevel_Trace << "Requesting web browser on URL " << url << "\n";

	const CString command = browserPrefix + CString(" \"") + url + CString("\"") + browserPostfix;
	m_kernelCtx.getLogManager() << LogLevel_Debug << "Launching [" << command << "]\n";
	const int result = system(command.toASCIIString());
	if (result < 0)
	{
		OV_WARNING("Could not launch command [" << command << "]\n", m_kernelCtx.getLogManager());
		return false;
	}
	return true;
}

bool CInterfacedScenario::browseBoxDocumentation(const CIdentifier& boxID) const
{
	const CIdentifier algorithmClassIdentifier = m_Scenario.getBoxDetails(boxID)->getAlgorithmClassIdentifier();

	// Do not show documentation for non-metaboxes or boxes that can not be created
	if (!(boxID != OV_UndefinedIdentifier && (m_kernelCtx.getPluginManager().canCreatePluginObject(algorithmClassIdentifier) ||
											  m_Scenario.getBoxDetails(boxID)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Box with id " << boxID << " can not create a pluging object\n";
		return false;
	}

	const CString defaultURLBase = m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}");
	CString URLBase              = defaultURLBase;
	CString browser              = m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserCommand}");
	CString browserPostfix       = m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserCommandPostfix}");
	CString boxName;

	CString l_sHTMLName = "Doc_BoxAlgorithm_";
	if (m_Scenario.getBoxDetails(boxID)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(m_Scenario.getBoxDetails(boxID)->getAttributeValue(OVP_AttributeId_Metabox_ID));
		boxName = m_kernelCtx.getMetaboxManager().getMetaboxObjectDesc(metaboxId)->getName();
	}
	else
	{
		const IPluginObjectDesc* l_pPluginObjectDesc = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(algorithmClassIdentifier);
		boxName                                      = l_pPluginObjectDesc->getName();
	}
	// The documentation files do not have spaces in their name, so we remove them
	l_sHTMLName = l_sHTMLName + CString(getBoxAlgorithmURL(boxName.toASCIIString()).c_str());


	if (m_Scenario.getBoxDetails(boxID)->hasAttribute(OV_AttributeId_Box_DocumentationURLBase))
	{
		URLBase = m_kernelCtx.getConfigurationManager().expand(
			m_Scenario.getBoxDetails(boxID)->getAttributeValue(OV_AttributeId_Box_DocumentationURLBase));
	}
	l_sHTMLName = l_sHTMLName + ".html";

	if (m_Scenario.getBoxDetails(boxID)->hasAttribute(OV_AttributeId_Box_DocumentationCommand))
	{
		browser = m_kernelCtx.getConfigurationManager().expand(
			m_Scenario.getBoxDetails(boxID)->getAttributeValue(OV_AttributeId_Box_DocumentationCommand));
		browserPostfix = "";
	}

	CString fullUrl = URLBase + CString("/") + l_sHTMLName;
	if (m_Scenario.getBoxDetails(boxID)->hasAttribute(OV_AttributeId_Box_DocumentationURL))
	{
		fullUrl = m_kernelCtx.getConfigurationManager().expand(m_Scenario.getBoxDetails(boxID)->getAttributeValue(OV_AttributeId_Box_DocumentationURL));
	}

	return browseURL(fullUrl, browser, browserPostfix);
}

void CInterfacedScenario::contextMenuBoxDocumentationCB(IBox& box) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "contextMenuBoxDocumentationCB\n";
	const CIdentifier l_oBoxId = box.getIdentifier();
	browseBoxDocumentation(l_oBoxId);
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
	CAboutScenarioDialog l_oAboutScenarioDialog(m_kernelCtx, m_Scenario, m_guiFilename.c_str());
	l_oAboutScenarioDialog.run();
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
	if (isLocked()) { if (m_playerVisu != nullptr) { m_playerVisu->showTopLevelWindows(); } }
	else { if (m_DesignerVisualization != nullptr) { m_DesignerVisualization->show(); } }
}

void CInterfacedScenario::hideCurrentVisualization() const
{
	if (isLocked()) { if (m_playerVisu != nullptr) { m_playerVisu->hideTopLevelWindows(); } }
	else { if (m_DesignerVisualization != nullptr) { m_DesignerVisualization->hide(); } }
}

void CInterfacedScenario::createPlayerVisualization(IVisualizationTree* tree)
{
	//hide window manager
	if (m_DesignerVisualization) { m_DesignerVisualization->hide(); }

	if (m_playerVisu == nullptr)
	{
		if (tree) { m_playerVisu = new CPlayerVisualization(m_kernelCtx, *tree, *this); }
		else { m_playerVisu = new CPlayerVisualization(m_kernelCtx, *m_Tree, *this); }


		//we go here when we press start
		//we have to set the modUI here
		//first, find the concerned boxes
		IScenario& runtimeScenario = m_Player->getRuntimeScenarioManager().getScenario(m_Player->getRuntimeScenarioIdentifier());
		CIdentifier objectId;
		while ((objectId = runtimeScenario.getNextBoxIdentifier(objectId)) != OV_UndefinedIdentifier)
		{
			IBox* l_oBox = runtimeScenario.getBoxDetails(objectId);
			if (l_oBox->hasModifiableSettings())//if the box has modUI
			{
				//create a BoxConfigurationDialog in mode true
				auto* l_oBoxConfigurationDialog = new CBoxConfigurationDialog(m_kernelCtx, *l_oBox, m_guiFilename.c_str(), m_guiSettingsFilename.c_str(), true);
				//store it
				m_boxConfigDialogs.push_back(l_oBoxConfigurationDialog);
			}
		}
	}

	//initialize and show windows
	m_playerVisu->init();
}

void CInterfacedScenario::releasePlayerVisualization()
{
	if (m_playerVisu != nullptr)
	{
		delete m_playerVisu;
		m_playerVisu = nullptr;
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
	for (auto& elem : m_boxConfigDialogs) { m_playerVisu->setWidget(elem->getBoxID(), elem->getWidget()); }

	return true;
}

bool CInterfacedScenario::centerOnBox(const CIdentifier& identifier)
{
	//m_kernelCtx.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox" << "\n";
	bool ret_val = false;
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
		GtkAdjustment* l_pOldHAdjustement =
				gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent));//gtk_viewport_get_vadjustment(m_pScenarioViewport);
		GtkAdjustment* l_pOldVAdjustement = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent));
		gdouble upper, lower, step, page, pagesize, value;

		g_object_get(l_pOldHAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value",
					 &value, nullptr);
		//create a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		auto* l_pAdjustement = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (centerX + m_viewOffsetX < upper / 2) { x = int(round(centerX - 2 * sizeX)) + m_viewOffsetX; }
		else { x = int(round(centerX + 2 * sizeX - pagesize)) + m_viewOffsetX; }
		gtk_adjustment_set_value(l_pAdjustement, x);
		gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent), l_pAdjustement);

		g_object_get(l_pOldVAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value",
					 &value, nullptr);
		l_pAdjustement = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (centerY - m_viewOffsetY < upper / 2) { y = int(round(centerY - 2 * sizeY) + m_viewOffsetY); }
		else { y = int(round(centerY + 2 * sizeY - pagesize)) + m_viewOffsetY; }
		gtk_adjustment_set_value(l_pAdjustement, y);
		gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(m_notebookPageContent), l_pAdjustement);
		ret_val = true;
	}
	return ret_val;
}

void CInterfacedScenario::setScale(const double scale)
{
	m_currentScale = std::max(scale, 0.1);

	PangoContext* l_pPangoContext                 = gtk_widget_get_pango_context(GTK_WIDGET(m_scenarioDrawingArea));
	PangoFontDescription* l_pPangoFontDescription = pango_context_get_font_description(l_pPangoContext);
	if (m_normalFontSize == 0)
	{
		// not done in constructor because the font size is changed elsewhere after that withour our knowledge
		m_normalFontSize = pango_font_description_get_size(l_pPangoFontDescription);
	}
	pango_font_description_set_size(l_pPangoFontDescription, gint(round(m_normalFontSize * m_currentScale)));

	//m_bScenarioModified = true;
	redraw();
}
