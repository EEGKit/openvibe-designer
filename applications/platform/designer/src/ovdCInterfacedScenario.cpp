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
#include <iostream>
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

extern map<uint32_t, GdkColor> g_vColors;

namespace
{
	GtkTargetEntry g_vTargetEntry[] = {
		{ static_cast<gchar*>("STRING"), 0, 0 },
		{ static_cast<gchar*>("text/plain"), 0, 0 }
	};

	GdkColor colorFromIdentifier(const CIdentifier& identifier)
	{
		GdkColor l_oGdkColor;
		unsigned int l_ui32Value1 = 0;
		unsigned int l_ui32Value2 = 0;
		uint64_t l_ui64Result     = 0;

		sscanf(identifier.toString(), "(0x%08X, 0x%08X)", &l_ui32Value1, &l_ui32Value2);
		l_ui64Result += l_ui32Value1;
		l_ui64Result <<= 32;
		l_ui64Result += l_ui32Value2;

		l_oGdkColor.pixel = guint16(0);
		l_oGdkColor.red   = guint16((l_ui64Result & 0xffff) | 0x8000);
		l_oGdkColor.green = guint16(((l_ui64Result >> 16) & 0xffff) | 0x8000);
		l_oGdkColor.blue  = guint16(((l_ui64Result >> 32) & 0xffff) | 0x8000);

		return l_oGdkColor;
	}

	std::string getBoxAlgorithmURL(const std::string& sInput, const bool bRemoveSlash = false)
	{
		std::string l_sInput(sInput);
		std::string l_sOutput;
		bool l_bLastWasSeparator = true;

		for (char c : l_sInput)
		{
			if (std::isalnum(c) || (!bRemoveSlash && c == '/'))
			{
				if (c == '/') { l_sOutput += "_"; }
				else
				{
					if (l_bLastWasSeparator) { l_sOutput += std::to_string(std::toupper(c)); }
					else { l_sOutput += c; }
				}
				l_bLastWasSeparator = false;
			}
			else
			{
				/*
			if(!l_bLastWasSeparator)
			{
				l_sOutput+="_";
			}
*/
				l_bLastWasSeparator = true;
			}
		}
		return l_sOutput;
	}

	void count_widget_cb(GtkWidget* /*widget*/, gpointer data)
	{
		int* i = reinterpret_cast<int*>(data);
		if (i) { (*i)++; }
	}

	int gtk_container_get_children_count(GtkContainer* pContainer)
	{
		int l_iCount = 0;
		gtk_container_foreach(pContainer, count_widget_cb, &l_iCount);
		return l_iCount;
	}

	gboolean scenario_scrolledwindow_scroll_event_cb(GtkWidget* /*widget*/, GdkEventScroll* event)
	{
		guint l_state = event->state & gtk_accelerator_get_default_mod_mask();

		/* Shift+Wheel scrolls the in the perpendicular direction */
		if (l_state & GDK_SHIFT_MASK)
		{
			if (event->direction == GDK_SCROLL_UP) { event->direction = GDK_SCROLL_LEFT; }
			else if (event->direction == GDK_SCROLL_LEFT) { event->direction = GDK_SCROLL_UP; }
			else if (event->direction == GDK_SCROLL_DOWN) { event->direction = GDK_SCROLL_RIGHT; }
			else if (event->direction == GDK_SCROLL_RIGHT) { event->direction = GDK_SCROLL_DOWN; }

			event->state &= ~GDK_SHIFT_MASK;
			l_state &= ~GDK_SHIFT_MASK;
		}

		return FALSE;
	}

	void scenario_drawing_area_expose_cb(GtkWidget* /*widget*/, GdkEventExpose* event, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaExposeCB(event);
	}

	void scenario_drawing_area_drag_data_received_cb(GtkWidget* /*widget*/, GdkDragContext* pDragContext, const gint x, const gint y,
													 GtkSelectionData* selectionData, const guint info, const guint t, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaDragDataReceivedCB(pDragContext, x, y, selectionData, info, t);
	}

	gboolean scenario_drawing_area_motion_notify_cb(GtkWidget* widget, GdkEventMotion* event, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaMotionNotifyCB(widget, event);
		return FALSE;
	}

	void scenario_drawing_area_button_pressed_cb(GtkWidget* widget, GdkEventButton* event, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaButtonPressedCB(widget, event);
	}

	void scenario_drawing_area_button_released_cb(GtkWidget* widget, GdkEventButton* event, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaButtonReleasedCB(widget, event);
	}

	void scenario_drawing_area_key_press_event_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaKeyPressEventCB(widget, event);
	}

	void scenario_drawing_area_key_release_event_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaKeyReleaseEventCB(widget, event);
	}

	void context_menu_cb(GtkMenuItem* /*pMenuItem*/, CInterfacedScenario::BoxContextMenuCB* pContextMenuCB)
	{
		//CInterfacedScenario::BoxContextMenuCB* pContextMenuCB=static_cast < CInterfacedScenario::BoxContextMenuCB* >(data);
		switch (pContextMenuCB->command)
		{
			case ContextMenu_SelectionCopy: pContextMenuCB->pInterfacedScenario->copySelection();
				break;
			case ContextMenu_SelectionCut: pContextMenuCB->pInterfacedScenario->cutSelection();
				break;
			case ContextMenu_SelectionPaste: pContextMenuCB->pInterfacedScenario->pasteSelection();
				break;
			case ContextMenu_SelectionDelete: pContextMenuCB->pInterfacedScenario->deleteSelection();
				break;

			case ContextMenu_BoxRename: pContextMenuCB->pInterfacedScenario->contextMenuBoxRenameCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxUpdate:
			{
				pContextMenuCB->pInterfacedScenario->snapshotCB();
				pContextMenuCB->pInterfacedScenario->contextMenuBoxUpdateCB(*pContextMenuCB->pBox);
				pContextMenuCB->pInterfacedScenario->redraw();
				break;
			}
			case ContextMenu_BoxRemoveDeprecatedInterfacors:
			{
				pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveDeprecatedInterfacorsCB(*pContextMenuCB->pBox);
				pContextMenuCB->pInterfacedScenario->redraw();
				break;
			}
				//case ContextMenu_BoxRename:        l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRenameAllCB(); break;
			case ContextMenu_BoxDelete:
			{
				// If selection is empty delete the box under cursor
				if (pContextMenuCB->pInterfacedScenario->m_SelectedObjects.empty())
				{
					pContextMenuCB->pInterfacedScenario->deleteBox(pContextMenuCB->pBox->getIdentifier());
					pContextMenuCB->pInterfacedScenario->redraw();
					pContextMenuCB->pInterfacedScenario->snapshotCB();
				}
				else { pContextMenuCB->pInterfacedScenario->deleteSelection(); }
				break;
			}
			case ContextMenu_BoxAddInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxAddInputCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEditInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditInputCB(*pContextMenuCB->pBox, pContextMenuCB->index);
				break;
			case ContextMenu_BoxRemoveInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveInputCB(*pContextMenuCB->pBox, pContextMenuCB->index);
				break;
			case ContextMenu_BoxAddOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxAddOutputCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEditOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditOutputCB(*pContextMenuCB->pBox, pContextMenuCB->index);
				break;
			case ContextMenu_BoxRemoveOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveOutputCB(*pContextMenuCB->pBox, pContextMenuCB->index);
				break;

			case ContextMenu_BoxConnectScenarioInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxConnectScenarioInputCB(
					*pContextMenuCB->pBox, pContextMenuCB->index, pContextMenuCB->secondaryIndex);
				break;
			case ContextMenu_BoxConnectScenarioOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxConnectScenarioOutputCB(
					*pContextMenuCB->pBox, pContextMenuCB->index, pContextMenuCB->secondaryIndex);
				break;

			case ContextMenu_BoxDisconnectScenarioInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxDisconnectScenarioInputCB(
					*pContextMenuCB->pBox, pContextMenuCB->index, pContextMenuCB->secondaryIndex);
				break;
			case ContextMenu_BoxDisconnectScenarioOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxDisconnectScenarioOutputCB(
					*pContextMenuCB->pBox, pContextMenuCB->index, pContextMenuCB->secondaryIndex);
				break;

			case ContextMenu_BoxAddSetting: pContextMenuCB->pInterfacedScenario->contextMenuBoxAddSettingCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEditSetting: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditSettingCB(*pContextMenuCB->pBox, pContextMenuCB->index);
				break;
			case ContextMenu_BoxRemoveSetting: pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveSettingCB(*pContextMenuCB->pBox, pContextMenuCB->index);
				break;
			case ContextMenu_BoxConfigure: pContextMenuCB->pInterfacedScenario->contextMenuBoxConfigureCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxAbout: pContextMenuCB->pInterfacedScenario->contextMenuBoxAboutCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEnable:
			{
				if (pContextMenuCB->pInterfacedScenario->m_SelectedObjects.empty())
				{
					pContextMenuCB->pInterfacedScenario->contextMenuBoxEnableCB(*pContextMenuCB->pBox);
				}
				else { pContextMenuCB->pInterfacedScenario->contextMenuBoxEnableAllCB(); }
				break;
			}
			case ContextMenu_BoxDisable:
			{
				if (pContextMenuCB->pInterfacedScenario->m_SelectedObjects.empty())
				{
					pContextMenuCB->pInterfacedScenario->contextMenuBoxDisableCB(*pContextMenuCB->pBox);
					break;
				}
				pContextMenuCB->pInterfacedScenario->contextMenuBoxDisableAllCB();
				break;
			}
			case ContextMenu_BoxDocumentation: pContextMenuCB->pInterfacedScenario->contextMenuBoxDocumentationCB(*pContextMenuCB->pBox);
				break;

			case ContextMenu_BoxEditMetabox: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditMetaboxCB(*pContextMenuCB->pBox);
				break;

			case ContextMenu_ScenarioAbout: pContextMenuCB->pInterfacedScenario->contextMenuScenarioAboutCB();
				break;
			case ContextMenu_ScenarioAddComment: pContextMenuCB->pInterfacedScenario->contextMenuScenarioAddCommentCB();
				break;
			default: break;
		}
		// Redraw in any case, as some of the actual callbacks can forget to redraw. As this callback is only called after the user has accessed
		// the right-click menu, so its not a large overhead to do it in general. @TODO might remove the individual redraws.
		pContextMenuCB->pInterfacedScenario->redraw();
	}

	void gdk_draw_rounded_rectangle(GdkDrawable* pDrawable, GdkGC* pDrawGC, const gboolean bFill, const gint x, const gint y, const gint width,
									const gint height, const gint radius = 8)
	{
		if (bFill != 0)
		{
#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			gdk_draw_rectangle(pDrawable, pDrawGC, TRUE, x + radius, y, width - 2 * radius, height);
			gdk_draw_rectangle(pDrawable, pDrawGC, TRUE, x, y + radius, width, height - 2 * radius);
#elif defined TARGET_OS_Windows
			gdk_draw_rectangle(pDrawable, pDrawGC, TRUE, x + radius, y, width - 2 * radius + 1, height + 1);
			gdk_draw_rectangle(pDrawable, pDrawGC, TRUE, x, y + radius, width + 1, height - 2 * radius + 1);
#else
#pragma error("you should give a version of this function for your OS")
#endif
		}
		else
		{
			gdk_draw_line(pDrawable, pDrawGC, x + radius, y, x + width - radius, y);
			gdk_draw_line(pDrawable, pDrawGC, x + radius, y + height, x + width - radius, y + height);
			gdk_draw_line(pDrawable, pDrawGC, x, y + radius, x, y + height - radius);
			gdk_draw_line(pDrawable, pDrawGC, x + width, y + radius, x + width, y + height - radius);
		}
#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x + width - radius * 2, y, radius * 2, radius * 2, 0 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x, y, radius * 2, radius * 2, 90 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x, y + height - radius * 2, radius * 2, radius * 2, 180 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x + width - radius * 2, y + height - radius * 2, radius * 2, radius * 2, 270 * 64, 90 * 64);
#elif defined TARGET_OS_Windows
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x + width - radius * 2, y, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 0 * 64,
					 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x, y, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 90 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x, y + height - radius * 2, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 180 * 64,
					 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x + width - radius * 2, y + height - radius * 2, radius * 2 + (bFill != 0 ? 2 : 1),
					 radius * 2 + (bFill != 0 ? 2 : 1), 270 * 64, 90 * 64);
#else
#pragma error("you should give a version of this function for your OS")
#endif
	}

	void scenario_title_button_close_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->m_rApplication.closeScenarioCB(static_cast<CInterfacedScenario*>(data));
	}

	gboolean editable_widget_focus_in_cb(GtkWidget*, GdkEvent*, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), 0);
		return 0;
	}

	gboolean editable_widget_focus_out_cb(GtkWidget*, GdkEvent*, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), 1);

		return 0;
	}

	//void scenario_configuration_add_setting_cb(GtkWidget*, CInterfacedScenario* pInterfacedScenario) { pInterfacedScenario->addScenarioSettingCB(); }

	void modify_scenario_setting_value_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* data)
	{
		CIdentifier l_oSettingType = OV_UndefinedIdentifier;
		data->interfacedScenario->m_rScenario.getSettingType(data->settingIndex, l_oSettingType);
		data->interfacedScenario->m_rScenario.setSettingValue(data->settingIndex,
															  data->interfacedScenario->m_pSettingHelper->getValue(l_oSettingType, data->widgetValue));
		data->interfacedScenario->m_hasBeenModified = true;
		data->interfacedScenario->updateScenarioLabel();
	}

	void modify_scenario_setting_default_value_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* data)
	{
		CIdentifier l_oSettingType = OV_UndefinedIdentifier;
		data->interfacedScenario->m_rScenario.getSettingType(data->settingIndex, l_oSettingType);
		data->interfacedScenario->m_rScenario.setSettingDefaultValue(data->settingIndex,
																	 data->interfacedScenario->m_pSettingHelper->getValue(l_oSettingType, data->widgetValue));

		// We also se the 'actual' value to this
		data->interfacedScenario->m_rScenario.setSettingValue(data->settingIndex,
															  data->interfacedScenario->m_pSettingHelper->getValue(l_oSettingType, data->widgetValue));
		data->interfacedScenario->m_hasBeenModified = true;
		data->interfacedScenario->updateScenarioLabel();
	}

	void modify_scenario_setting_move_up_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* data)
	{
		if (data->settingIndex == 0) { return; }

		data->interfacedScenario->swapScenarioSettings(data->settingIndex - 1, data->settingIndex);
	}

	void modify_scenario_setting_move_down_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* data)
	{
		if (data->settingIndex >= data->interfacedScenario->m_rScenario.getSettingCount() - 1) { return; }

		data->interfacedScenario->swapScenarioSettings(data->settingIndex, data->settingIndex + 1);
	}

	void modify_scenario_setting_revert_to_default_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* data)
	{
		CString l_sSettingDefaultValue;
		data->interfacedScenario->m_rScenario.getSettingDefaultValue(data->settingIndex, l_sSettingDefaultValue);

		data->interfacedScenario->m_rScenario.setSettingValue(data->settingIndex, l_sSettingDefaultValue);
		data->interfacedScenario->redrawScenarioSettings();
	}

	void copy_scenario_setting_token_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* data)
	{
		CString settingName;
		data->interfacedScenario->m_rScenario.getSettingName(data->settingIndex, settingName);
		settingName = CString("$var{") + settingName + CString("}");

		GtkClipboard* l_pDefaultClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text(l_pDefaultClipboard, settingName.toASCIIString(), -1);

		// On X11 there is another clipboard that it is useful to set as well
		GtkClipboard* l_pX11Clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
		gtk_clipboard_set_text(l_pX11Clipboard, settingName.toASCIIString(), -1);
	}

	void modify_scenario_setting_type_cb(GtkWidget* pCombobox, CInterfacedScenario::SSettingCallbackData* data)
	{
		GtkBuilder* settingsGuiBuilder = gtk_builder_new();
		gtk_builder_add_from_string(settingsGuiBuilder, data->interfacedScenario->m_sSerializedSettingGUIXML.c_str(),
									data->interfacedScenario->m_sSerializedSettingGUIXML.length(), nullptr);

		gtk_widget_destroy(data->widgetValue);

		const CIdentifier settingType = data->interfacedScenario->m_vSettingType[gtk_combo_box_get_active_text(GTK_COMBO_BOX(pCombobox))];
		data->interfacedScenario->m_rScenario.setSettingType(data->settingIndex, settingType);

		const CString settingWidgetName = data->interfacedScenario->m_pSettingHelper->getSettingWidgetName(settingType);

		GtkWidget* widgetValue = GTK_WIDGET(gtk_builder_get_object(settingsGuiBuilder, settingWidgetName.toASCIIString()));

		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widgetValue)), widgetValue);
		gtk_table_attach_defaults(GTK_TABLE(data->container), widgetValue, 1, 5, 1, 2);

		// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
		CString l_sSettingValue;
		data->interfacedScenario->m_rScenario.getSettingDefaultValue(data->settingIndex, l_sSettingValue);
		data->interfacedScenario->m_pSettingHelper->setValue(settingType, widgetValue, l_sSettingValue);

		// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
		const CString settingEntryWidgetName = data->interfacedScenario->m_pSettingHelper->getSettingEntryWidgetName(settingType);
		GtkWidget* widgetEntryValue          = GTK_WIDGET(gtk_builder_get_object(settingsGuiBuilder, settingEntryWidgetName.toASCIIString()));

		data->widgetValue      = widgetValue;
		data->widgetEntryValue = widgetEntryValue;

		g_signal_connect(widgetEntryValue, "changed", G_CALLBACK(modify_scenario_setting_default_value_cb), data);

		g_object_unref(settingsGuiBuilder);
	}

	void delete_scenario_setting_cb(GtkWidget* /*pDeleteButton*/, CInterfacedScenario::SSettingCallbackData* data)
	{
		data->interfacedScenario->m_rScenario.removeSetting(data->settingIndex);
		data->interfacedScenario->redrawConfigureScenarioSettingsDialog();
	}

	void modify_scenario_setting_name_cb(GtkWidget* pEntry, CInterfacedScenario::SSettingCallbackData* data)
	{
		data->interfacedScenario->m_rScenario.setSettingName(data->settingIndex, gtk_entry_get_text(GTK_ENTRY(pEntry)));
	}

	void reset_scenario_setting_identifier_cb(GtkWidget* /*button*/, CInterfacedScenario::SSettingCallbackData* data)
	{
		const CIdentifier newID = data->interfacedScenario->m_rScenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier);
		if (newID != OV_UndefinedIdentifier)
		{
			data->interfacedScenario->m_rScenario.updateInterfacorIdentifier(BoxInterfacorType::Setting, uint32_t(data->settingIndex), newID);
			data->interfacedScenario->redrawConfigureScenarioSettingsDialog();
		}
	}

	void modify_scenario_setting_identifier_cb(GtkWidget* entry, CInterfacedScenario::SSettingCallbackData* data)
	{
		CIdentifier newID;
		if (newID.fromString(gtk_entry_get_text(GTK_ENTRY(entry))))
		{
			data->interfacedScenario->m_rScenario.updateInterfacorIdentifier(BoxInterfacorType::Setting, uint32_t(data->settingIndex), newID);
		}
	}

	void edit_scenario_link_cb(GtkWidget*, CInterfacedScenario::SLinkCallbackData* data)
	{
		if (data->m_bIsInput) { data->m_pInterfacedScenario->editScenarioInputCB(data->m_uiLinkIndex); }
		else { data->m_pInterfacedScenario->editScenarioOutputCB(data->m_uiLinkIndex); }
		data->m_pInterfacedScenario->redraw();
	}

	void modify_scenario_link_move_up_cb(GtkWidget*, CInterfacedScenario::SLinkCallbackData* data)
	{
		if (data->m_uiLinkIndex == 0) { return; }
		if (data->m_bIsInput) { data->m_pInterfacedScenario->swapScenarioInputs(data->m_uiLinkIndex - 1, data->m_uiLinkIndex); }
		else { data->m_pInterfacedScenario->swapScenarioOutputs(data->m_uiLinkIndex - 1, data->m_uiLinkIndex); }

		data->m_pInterfacedScenario->snapshotCB();
	}

	void modify_scenario_link_move_down_cb(GtkWidget*, CInterfacedScenario::SLinkCallbackData* data)
	{
		const auto interfacorType = data->m_bIsInput ? Input : Output;
		if (data->m_pInterfacedScenario->m_rScenario.getInterfacorCount(interfacorType) < 2
			|| data->m_uiLinkIndex >= data->m_pInterfacedScenario->m_rScenario.getInterfacorCount(interfacorType) - 1) { return; }

		if (data->m_bIsInput) { data->m_pInterfacedScenario->swapScenarioInputs(data->m_uiLinkIndex, data->m_uiLinkIndex + 1); }
		else { data->m_pInterfacedScenario->swapScenarioOutputs(data->m_uiLinkIndex, data->m_uiLinkIndex + 1); }
		data->m_pInterfacedScenario->snapshotCB();
	}

	void delete_scenario_link_cb(GtkButton*, CInterfacedScenario::SLinkCallbackData* data)
	{
		if (data->m_bIsInput)
		{
			data->m_pInterfacedScenario->m_rScenario.removeScenarioInput(data->m_uiLinkIndex);
			data->m_pInterfacedScenario->redrawScenarioInputSettings();
		}
		else
		{
			data->m_pInterfacedScenario->m_rScenario.removeScenarioOutput(data->m_uiLinkIndex);
			data->m_pInterfacedScenario->redrawScenarioOutputSettings();
		}

		data->m_pInterfacedScenario->snapshotCB();
		data->m_pInterfacedScenario->redraw();
	}

	/*
	void modify_scenario_link_name_cb(GtkWidget* pEntry, CInterfacedScenario::SLinkCallbackData* data)
	{
		if (data->m_bIsInput) { data->m_pInterfacedScenario->m_rScenario.setInputName(data->m_uiLinkIndex, gtk_entry_get_text(GTK_ENTRY(pEntry))); }
		else { data->m_pInterfacedScenario->m_rScenario.setOutputName(data->m_uiLinkIndex, gtk_entry_get_text(GTK_ENTRY(pEntry))); }
	}

	void modify_scenario_link_type_cb(GtkWidget* pComboBox, CInterfacedScenario::SLinkCallbackData* data)
	{
		const CIdentifier l_oStreamType = data->m_pInterfacedScenario->m_mStreamType[gtk_combo_box_get_active_text(GTK_COMBO_BOX(pComboBox))];
		if (data->m_bIsInput) { data->m_pInterfacedScenario->m_rScenario.setInputType(data->m_uiLinkIndex, l_oStreamType); }
		else { data->m_pInterfacedScenario->m_rScenario.setOutputType(data->m_uiLinkIndex, l_oStreamType); }
		data->m_pInterfacedScenario->redraw();
	}
	//*/
} // namespace

CInterfacedScenario::CInterfacedScenario(const IKernelContext& ctx, CApplication& rApplication, IScenario& rScenario, CIdentifier& scenarioID,
										 GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename)
	: m_ePlayerStatus(PlayerStatus_Stop), m_oScenarioIdentifier(scenarioID), m_rApplication(rApplication), m_kernelContext(ctx),
	  m_rScenario(rScenario), m_rNotebook(rNotebook), m_sGUIFilename(sGUIFilename), m_sGUISettingsFilename(sGUISettingsFilename)
{
	m_pGUIBuilder = gtk_builder_new();
	gtk_builder_add_from_file(m_pGUIBuilder, m_sGUIFilename.c_str(), nullptr);
	gtk_builder_connect_signals(m_pGUIBuilder, nullptr);

	std::ifstream l_oSettingGUIFilestream;
	FS::Files::openIFStream(l_oSettingGUIFilestream, m_sGUISettingsFilename.c_str());
	m_sSerializedSettingGUIXML = std::string((std::istreambuf_iterator<char>(l_oSettingGUIFilestream)), std::istreambuf_iterator<char>());

	m_pSettingHelper = new CSettingCollectionHelper(m_kernelContext, m_sGUISettingsFilename.c_str());

	// We will need to access setting types by their name later
	CIdentifier l_oCurrentTypeIdentifier;
	while ((l_oCurrentTypeIdentifier = m_kernelContext.getTypeManager().getNextTypeIdentifier(l_oCurrentTypeIdentifier)) != OV_UndefinedIdentifier)
	{
		if (!m_kernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier))
		{
			m_vSettingType[m_kernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString()] = l_oCurrentTypeIdentifier;
		}
		else { m_mStreamType[m_kernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString()] = l_oCurrentTypeIdentifier; }
	}

	m_pNotebookPageTitle   = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "openvibe_scenario_notebook_title"));
	m_pNotebookPageContent = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "openvibe_scenario_notebook_scrolledwindow"));

	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_notebook")), 0);
	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_notebook")), 0);
	gtk_notebook_append_page(&m_rNotebook, m_pNotebookPageContent, m_pNotebookPageTitle);
	gtk_notebook_set_tab_reorderable(&m_rNotebook, m_pNotebookPageContent, 1);

	GtkWidget* l_pCloseWidget = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_button_close"));
	g_signal_connect(G_OBJECT(l_pCloseWidget), "clicked", G_CALLBACK(scenario_title_button_close_cb), this);

	m_pScenarioDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_drawing_area"));
	m_pScenarioViewport    = GTK_VIEWPORT(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_viewport"));
	gtk_drag_dest_set(GTK_WIDGET(m_pScenarioDrawingArea), GTK_DEST_DEFAULT_ALL, g_vTargetEntry, sizeof(g_vTargetEntry) / sizeof(GtkTargetEntry),
					  GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "expose_event", G_CALLBACK(scenario_drawing_area_expose_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "drag_data_received", G_CALLBACK(scenario_drawing_area_drag_data_received_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "motion_notify_event", G_CALLBACK(scenario_drawing_area_motion_notify_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "button_press_event", G_CALLBACK(scenario_drawing_area_button_pressed_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "button_release_event", G_CALLBACK(scenario_drawing_area_button_released_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "key-press-event", G_CALLBACK(scenario_drawing_area_key_press_event_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "key-release-event", G_CALLBACK(scenario_drawing_area_key_release_event_cb), this);
	g_signal_connect(G_OBJECT(m_pNotebookPageContent), "scroll-event", G_CALLBACK(scenario_scrolledwindow_scroll_event_cb), this);

	m_pMensiaLogoPixbuf = gdk_pixbuf_new_from_file(Directories::getDataDir() + "/applications/designer/mensia-decoration.png", nullptr);

#if defined TARGET_OS_Windows
	// add drag-n-drop capabilities onto the scenario notebook to open new scenario
	gtk_drag_dest_add_uri_targets(GTK_WIDGET(m_pScenarioDrawingArea));
#endif

	//retrieve visualization tree

	m_rApplication.m_pVisualizationManager->createVisualizationTree(m_oVisualizationTreeIdentifier);
	m_pVisualizationTree = &m_rApplication.m_pVisualizationManager->getVisualizationTree(m_oVisualizationTreeIdentifier);
	m_pVisualizationTree->init(&m_rScenario);

	//create window manager
	m_pDesignerVisualization = new CDesignerVisualization(m_kernelContext, *m_pVisualizationTree, *this);
	m_pDesignerVisualization->init(string(sGUIFilename));

	m_pConfigureSettingsDialog = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_scenario_configuration"));

	m_pSettingsVBox = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_scenario_configuration-vbox"));

	m_pNoHelpDialog = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_no_help"));

	m_pErrorPendingDeprecatedInterfacorsDialog =
			GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_pending_deprecated_interfacors"));

	this->redrawScenarioSettings();
	this->redrawScenarioInputSettings();
	this->redrawScenarioOutputSettings();

	m_oStateStack.reset(new CScenarioStateStack(ctx, *this, rScenario));

	CInterfacedScenario::updateScenarioLabel();

	// Output a log message if any box of the scenario is in some special state
	CIdentifier l_oBoxIdentifier = OV_UndefinedIdentifier;
	bool warningUpdate           = false;
	bool warningDeprecated       = false;
	bool warningUnknown          = false;
	while ((l_oBoxIdentifier = m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
	{
		//const IBox *l_pBox = m_rScenario.getBoxDetails(l_oBoxIdentifier);
		//const CBoxProxy l_oBoxProxy(m_kernelContext, *l_pBox);
		const CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, l_oBoxIdentifier);

		if (!warningUpdate && !l_oBoxProxy.isUpToDate())
		{
			m_kernelContext.getLogManager() << LogLevel_Warning <<
					"Scenario requires 'update' of some box(es). You need to replace these boxes or the scenario may not work correctly.\n";
			warningUpdate = true;
		}
		if (!warningDeprecated && l_oBoxProxy.isDeprecated())
		{
			m_kernelContext.getLogManager() << LogLevel_Warning << "Scenario constains deprecated box(es). Please consider using other boxes instead.\n";
			warningDeprecated = true;
		}
		//		if (!noteUnstable && l_oBoxProxy.isUnstable())
		//		{
		//			m_kernelContext.getLogManager() << LogLevel_Debug << "Scenario contains unstable box(es).\n";
		//			noteUnstable = true;
		//		}
		if (!warningUnknown && !l_oBoxProxy.isBoxAlgorithmPluginPresent())
		{
			m_kernelContext.getLogManager() << LogLevel_Warning << "Scenario contains unknown box algorithm(s).\n";
			if (l_oBoxProxy.isMetabox())
			{
				CString mPath = m_kernelContext.getConfigurationManager().expand("${Kernel_Metabox}");
				m_kernelContext.getLogManager() << LogLevel_Warning << "Some Metaboxes could not be found in [" << mPath << "]\n";
			}
			warningUnknown = true;
		}
	}
}

CInterfacedScenario::~CInterfacedScenario()

{
	//delete window manager


	delete m_pDesignerVisualization;


	if (m_pStencilBuffer != nullptr) { g_object_unref(m_pStencilBuffer); }

	g_object_unref(m_pGUIBuilder);
	/*
	g_object_unref(m_pBuilder);
	g_object_unref(m_pBuilder);
	*/

	gtk_notebook_remove_page(&m_rNotebook, gtk_notebook_page_num(&m_rNotebook, m_pNotebookPageContent));
}

void CInterfacedScenario::redraw()
{
	if (GDK_IS_WINDOW(GTK_WIDGET(m_pScenarioDrawingArea)->window)) { gdk_window_invalidate_rect(GTK_WIDGET(m_pScenarioDrawingArea)->window, nullptr, 1); }
}

// This function repaints the dialog which opens when configuring settings
void CInterfacedScenario::redrawConfigureScenarioSettingsDialog()
{
	if (m_hasFileName)
	{
		char l_sScenarioFilename[1024];
		FS::Files::getFilename(m_sFileName.c_str(), l_sScenarioFilename);
		char l_sWindowTitle[2048];
		sprintf(l_sWindowTitle, "Settings for \"%s\"", l_sScenarioFilename);
		gtk_window_set_title(GTK_WINDOW(m_pConfigureSettingsDialog), l_sWindowTitle);
	}
	else { gtk_window_set_title(GTK_WINDOW(m_pConfigureSettingsDialog), "Settings for an unnamed scenario"); }

	GList* l_pSettingWidgets = gtk_container_get_children(GTK_CONTAINER(m_pSettingsVBox));
	for (GList* l_pSettingIterator = l_pSettingWidgets; l_pSettingIterator != nullptr; l_pSettingIterator = g_list_next(l_pSettingIterator))
	{
		gtk_widget_destroy(GTK_WIDGET(l_pSettingIterator->data));
	}
	g_list_free(l_pSettingWidgets);

	m_vSettingConfigurationCallbackData.clear();
	m_vSettingConfigurationCallbackData.resize(m_rScenario.getSettingCount());

	if (m_rScenario.getSettingCount() == 0)
	{
		GtkWidget* l_pSettingPlaceholderLabel = gtk_label_new("This scenario has no settings");
		gtk_box_pack_start(GTK_BOX(m_pSettingsVBox), l_pSettingPlaceholderLabel, TRUE, TRUE, 5);
	}
	else
	{
		for (uint32_t l_ui32SettingIndex = 0; l_ui32SettingIndex < m_rScenario.getSettingCount(); l_ui32SettingIndex++)
		{
			GtkBuilder* l_pSettingsGUIBuilder = gtk_builder_new();
			gtk_builder_add_from_string(l_pSettingsGUIBuilder, m_sSerializedSettingGUIXML.c_str(), m_sSerializedSettingGUIXML.length(), nullptr);

			GtkWidget* l_pSettingContainerWidget = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(::gtk_widget_get_parent(l_pSettingContainerWidget)), l_pSettingContainerWidget);
			gtk_box_pack_start(GTK_BOX(m_pSettingsVBox), l_pSettingContainerWidget, FALSE, FALSE, 5);

			GtkWidget* l_pSettingEntryName    = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-entry_name"));
			GtkWidget* l_pSettingComboboxType = GTK_WIDGET(
				gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-combobox_type"));
			GtkWidget* l_pSettingButtonUp = GTK_WIDGET(
				gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_move_up"));
			GtkWidget* l_pSettingButtonDown = GTK_WIDGET(
				gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_move_down"));
			GtkWidget* l_pSettingButtonDelete = GTK_WIDGET(
				gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_delete"));
			GtkWidget* l_pSettingEntryIdentifier = GTK_WIDGET(
				gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-entry_identifier"));
			GtkWidget* l_pSettingButtonResetIdentifier = GTK_WIDGET(
				gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_reset_identifier"));

			// fill the type dropdown
			CIdentifier l_oSettingTypeIdentifier = OV_UndefinedIdentifier;
			m_rScenario.getSettingType(l_ui32SettingIndex, l_oSettingTypeIdentifier);

			CIdentifier l_oCurrentTypeIdentifier;
			gint l_iCurrentSettingIndex = 0;
			while ((l_oCurrentTypeIdentifier = m_kernelContext.getTypeManager().getNextTypeIdentifier(l_oCurrentTypeIdentifier)) != OV_UndefinedIdentifier)
			{
				if (!m_kernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier))
				{
					gtk_combo_box_append_text(
						GTK_COMBO_BOX(l_pSettingComboboxType), m_kernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString());
					if (l_oCurrentTypeIdentifier == l_oSettingTypeIdentifier)
					{
						gtk_combo_box_set_active(GTK_COMBO_BOX(l_pSettingComboboxType), l_iCurrentSettingIndex);
					}
					l_iCurrentSettingIndex++;
				}
			}
			// Set name
			CString l_sSettingLabel;
			m_rScenario.getSettingName(l_ui32SettingIndex, l_sSettingLabel);
			gtk_entry_set_text(GTK_ENTRY(l_pSettingEntryName), l_sSettingLabel.toASCIIString());

			// Set the identifer
			CIdentifier settingIdentifier;
			m_rScenario.getInterfacorIdentifier(BoxInterfacorType::Setting, l_ui32SettingIndex, settingIdentifier);
			gtk_entry_set_text(GTK_ENTRY(l_pSettingEntryIdentifier), settingIdentifier.toString().toASCIIString());

			// Add widget for the actual setting
			CString l_sSettingWidgetName = m_pSettingHelper->getSettingWidgetName(l_oSettingTypeIdentifier);

			GtkWidget* l_widgetDefaultValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingWidgetName.toASCIIString()));

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_widgetDefaultValue)), l_widgetDefaultValue);
			gtk_table_attach_defaults(GTK_TABLE(l_pSettingContainerWidget), l_widgetDefaultValue, 1, 5, 1, 2);

			// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
			CString l_sSettingDefaultValue;
			m_rScenario.getSettingDefaultValue(l_ui32SettingIndex, l_sSettingDefaultValue);
			m_pSettingHelper->setValue(l_oSettingTypeIdentifier, l_widgetDefaultValue, l_sSettingDefaultValue);

			// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
			CString l_sSettingEntryWidgetName    = m_pSettingHelper->getSettingEntryWidgetName(l_oSettingTypeIdentifier);
			GtkWidget* l_widgetEntryDefaultValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingEntryWidgetName.toASCIIString()));

			// Set the callbacks
			SSettingCallbackData l_oCallbackData;
			l_oCallbackData.interfacedScenario = this;
			l_oCallbackData.settingIndex       = l_ui32SettingIndex;
			l_oCallbackData.widgetValue        = l_widgetDefaultValue;
			l_oCallbackData.widgetEntryValue   = l_widgetEntryDefaultValue;
			l_oCallbackData.container          = l_pSettingContainerWidget;

			m_vSettingConfigurationCallbackData[l_ui32SettingIndex] = l_oCallbackData;

			// Connect signals of the container
			g_signal_connect(G_OBJECT(l_pSettingComboboxType), "changed", G_CALLBACK(modify_scenario_setting_type_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonDelete), "clicked", G_CALLBACK(delete_scenario_setting_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonUp), "clicked", G_CALLBACK(modify_scenario_setting_move_up_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonDown), "clicked", G_CALLBACK(modify_scenario_setting_move_down_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingEntryName), "changed", G_CALLBACK(modify_scenario_setting_name_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingEntryIdentifier), "activate", G_CALLBACK(modify_scenario_setting_identifier_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonResetIdentifier), "clicked", G_CALLBACK(reset_scenario_setting_identifier_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);

			// these callbacks assure that we can use copy/paste and undo within editable fields
			// as otherwise the keyboard shortucts are stolen by the designer
			g_signal_connect(G_OBJECT(l_pSettingEntryName), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_pSettingEntryName), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_widgetEntryDefaultValue), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_widgetEntryDefaultValue), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_rApplication);

			// add callbacks for setting the settings
			g_signal_connect(l_widgetEntryDefaultValue, "changed", G_CALLBACK(modify_scenario_setting_default_value_cb),
							 &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);

			g_object_unref(l_pSettingsGUIBuilder);
		}
	}
}

// This function, similar to the previous one, repaints the settings handling sidebar
void CInterfacedScenario::redrawScenarioSettings()
{
	GtkWidget* l_pSettingsVBox = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "openvibe-scenario_configuration_vbox"));

	GList* l_pSettingWidgets = gtk_container_get_children(GTK_CONTAINER(l_pSettingsVBox));
	for (GList* l_pSettingIterator = l_pSettingWidgets; l_pSettingIterator != nullptr; l_pSettingIterator = g_list_next(l_pSettingIterator))
	{
		gtk_widget_destroy(GTK_WIDGET(l_pSettingIterator->data));
	}
	g_list_free(l_pSettingWidgets);

	m_vSettingCallbackData.clear();
	m_vSettingCallbackData.resize(m_rScenario.getSettingCount());

	if (m_rScenario.getSettingCount() == 0)
	{
		GtkWidget* l_pSettingPlaceholderLabel = gtk_label_new("This scenario has no settings");
		gtk_box_pack_start(GTK_BOX(l_pSettingsVBox), l_pSettingPlaceholderLabel, TRUE, TRUE, 5);
	}
	else
	{
		for (uint32_t l_ui32SettingIndex = 0; l_ui32SettingIndex < m_rScenario.getSettingCount(); l_ui32SettingIndex++)
		{
			GtkBuilder* l_pSettingsGUIBuilder = gtk_builder_new();
			gtk_builder_add_from_string(l_pSettingsGUIBuilder, m_sSerializedSettingGUIXML.c_str(), m_sSerializedSettingGUIXML.length(), nullptr);

			GtkWidget* l_pSettingContainerWidget = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingContainerWidget)), l_pSettingContainerWidget);
			gtk_box_pack_start(GTK_BOX(l_pSettingsVBox), l_pSettingContainerWidget, FALSE, FALSE, 5);

			GtkWidget* l_pSettingLabelName     = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-label"));
			GtkWidget* l_pSettingButtonDefault = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-button_default"));
			GtkWidget* l_pSettingButtonCopy    = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-button_copy"));

			// Set name
			CString l_sSettingLabel;
			m_rScenario.getSettingName(l_ui32SettingIndex, l_sSettingLabel);
			gtk_label_set_text(GTK_LABEL(l_pSettingLabelName), l_sSettingLabel.toASCIIString());
			gtk_misc_set_alignment(GTK_MISC(l_pSettingLabelName), 0.0, 0.5);

			// Add widget for the actual setting
			CIdentifier l_oSettingTypeIdentifier = OV_UndefinedIdentifier;
			m_rScenario.getSettingType(l_ui32SettingIndex, l_oSettingTypeIdentifier);
			CString l_sSettingWidgetName = m_pSettingHelper->getSettingWidgetName(l_oSettingTypeIdentifier);

			GtkWidget* l_widgetValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingWidgetName.toASCIIString()));

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_widgetValue)), l_widgetValue);
			gtk_table_attach_defaults(GTK_TABLE(l_pSettingContainerWidget), l_widgetValue, 0, 1, 1, 2);

			// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
			CString l_sSettingValue;
			m_rScenario.getSettingValue(l_ui32SettingIndex, l_sSettingValue);
			m_pSettingHelper->setValue(l_oSettingTypeIdentifier, l_widgetValue, l_sSettingValue);

			// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
			CString l_sSettingEntryWidgetName = m_pSettingHelper->getSettingEntryWidgetName(l_oSettingTypeIdentifier);
			GtkWidget* l_widgetEntryValue     = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingEntryWidgetName.toASCIIString()));

			// Set the callbacks
			SSettingCallbackData l_oCallbackData;
			l_oCallbackData.interfacedScenario = this;
			l_oCallbackData.settingIndex       = l_ui32SettingIndex;
			l_oCallbackData.widgetValue        = l_widgetValue;
			l_oCallbackData.widgetEntryValue   = l_widgetEntryValue;
			l_oCallbackData.container          = l_pSettingContainerWidget;

			m_vSettingCallbackData[l_ui32SettingIndex] = l_oCallbackData;

			// these callbacks assure that we can use copy/paste and undo within editable fields
			// as otherwise the keyboard shortucts are stolen by the designer
			g_signal_connect(G_OBJECT(l_widgetEntryValue), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_widgetEntryValue), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_rApplication);

			// add callbacks for setting the settings
			g_signal_connect(l_widgetEntryValue, "changed", G_CALLBACK(modify_scenario_setting_value_cb), &m_vSettingCallbackData[l_ui32SettingIndex]);
			g_signal_connect(l_pSettingButtonDefault, "clicked", G_CALLBACK(modify_scenario_setting_revert_to_default_cb),
							 &m_vSettingCallbackData[l_ui32SettingIndex]);
			g_signal_connect(l_pSettingButtonCopy, "clicked", G_CALLBACK(copy_scenario_setting_token_cb), &m_vSettingCallbackData[l_ui32SettingIndex]);

			g_object_unref(l_pSettingsGUIBuilder);
		}
	}
	gtk_widget_show_all(l_pSettingsVBox);
}

void CInterfacedScenario::redrawScenarioInputSettings()
{
	uint32_t (IScenario::* l_pfGetLinkCount)() const                  = &IScenario::getInputCount;
	bool (IScenario::* l_pfGetLinkName)(uint32_t, CString&) const     = &IScenario::getInputName;
	bool (IScenario::* l_pfGetLinkType)(uint32_t, CIdentifier&) const = &IScenario::getInputType;

	this->redrawScenarioLinkSettings(m_rApplication.m_pTableInputs, true, m_vScenarioInputCallbackData, l_pfGetLinkCount, l_pfGetLinkName, l_pfGetLinkType);
}

void CInterfacedScenario::redrawScenarioOutputSettings()
{
	uint32_t (IScenario::* l_pfGetLinkCount)() const                  = &IScenario::getOutputCount;
	bool (IScenario::* l_pfGetLinkName)(uint32_t, CString&) const     = &IScenario::getOutputName;
	bool (IScenario::* l_pfGetLinkType)(uint32_t, CIdentifier&) const = &IScenario::getOutputType;

	this->redrawScenarioLinkSettings(m_rApplication.m_pTableOutputs, false, m_vScenarioOutputCallbackData, l_pfGetLinkCount, l_pfGetLinkName, l_pfGetLinkType);
}

// Redraws the tab containing inputs or outputs of the scenario
// This method receives pointers to methods that manipulate either intpus or outputs so it can be generic
void CInterfacedScenario::redrawScenarioLinkSettings(GtkWidget* pLinkTable, const bool bIsInput,
													 std::vector<SLinkCallbackData>& vLinkCallbackData, uint32_t (IScenario::* pfGetLinkCount)() const,
													 bool (IScenario::* pfGetLinkName)(uint32_t, CString&) const,
													 bool (IScenario::* pfGetLinkType)(uint32_t, CIdentifier&) const
)
{
	GList* l_pSettingWidgets = gtk_container_get_children(GTK_CONTAINER(pLinkTable));
	for (GList* l_pSettingIterator = l_pSettingWidgets; l_pSettingIterator != nullptr; l_pSettingIterator = g_list_next(l_pSettingIterator))
	{
		gtk_widget_destroy(GTK_WIDGET(l_pSettingIterator->data));
	}
	g_list_free(l_pSettingWidgets);

	const uint32_t linkCount = (m_rScenario.*pfGetLinkCount)();

	vLinkCallbackData.clear();
	vLinkCallbackData.resize(linkCount);

	gtk_table_resize(GTK_TABLE(pLinkTable), linkCount == 0 ? 1 : linkCount, 7);

	if (linkCount == 0)
	{
		GtkWidget* l_pSettingPlaceholderLabel = gtk_label_new("This scenario has none");
		gtk_table_attach_defaults(GTK_TABLE(pLinkTable), l_pSettingPlaceholderLabel, 0, 1, 0, 1);
	}
	else
	{
		for (uint32_t linkIndex = 0; linkIndex < linkCount; linkIndex++)
		{
			GtkBuilder* ioSettingsGUIBuilder = gtk_builder_new();
			gtk_builder_add_from_string(ioSettingsGUIBuilder, m_sSerializedSettingGUIXML.c_str(), m_sSerializedSettingGUIXML.length(), nullptr);

			GtkWidget* l_pSettingContainerWidget = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingContainerWidget)), l_pSettingContainerWidget);

			GtkWidget* l_pEntryLinkName = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-label"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pEntryLinkName)), l_pEntryLinkName);

			GtkWidget* ioSettingComboboxType = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-combobox_type"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(ioSettingComboboxType)), ioSettingComboboxType);

			// fill the type dropdown
			CIdentifier l_oLinkTypeIdentifier = OV_UndefinedIdentifier;
			(m_rScenario.*pfGetLinkType)(linkIndex, l_oLinkTypeIdentifier);

			CIdentifier l_oCurrentTypeIdentifier;
			gint l_iCurrentLinkIndex = 0;
			while ((l_oCurrentTypeIdentifier = m_kernelContext.getTypeManager().getNextTypeIdentifier(l_oCurrentTypeIdentifier)) != OV_UndefinedIdentifier)
			{
				if (m_kernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier))
				{
					gtk_combo_box_append_text(
						GTK_COMBO_BOX(ioSettingComboboxType), m_kernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString());
					if (l_oCurrentTypeIdentifier == l_oLinkTypeIdentifier)
					{
						gtk_combo_box_set_active(GTK_COMBO_BOX(ioSettingComboboxType), l_iCurrentLinkIndex);
					}

					l_iCurrentLinkIndex++;
				}
			}
			gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(ioSettingComboboxType), GTK_SENSITIVITY_OFF);

			GtkWidget* ioSettingButtonUp = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-button_move_up"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(ioSettingButtonUp)), ioSettingButtonUp);
			GtkWidget* ioSettingButtonDown = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-button_move_down"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(ioSettingButtonDown)), ioSettingButtonDown);
			GtkWidget* l_pSettingButtonEdit = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-button_edit"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingButtonEdit)), l_pSettingButtonEdit);
			GtkWidget* l_pSettingButtonDelete = GTK_WIDGET(gtk_builder_get_object(ioSettingsGUIBuilder, "scenario_io_setting-button_delete"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingButtonDelete)), l_pSettingButtonDelete);

			// Set name
			CString l_sLinkName;
			(m_rScenario.*pfGetLinkName)(linkIndex, l_sLinkName);
			gtk_label_set_text(GTK_LABEL(l_pEntryLinkName), l_sLinkName.toASCIIString());
			gtk_misc_set_alignment(GTK_MISC(l_pEntryLinkName), 0.0, 0.5);
			gtk_widget_set_sensitive(GTK_WIDGET(l_pEntryLinkName), GTK_SENSITIVITY_OFF);

			gtk_table_attach(GTK_TABLE(pLinkTable), l_pEntryLinkName, 0, 1, linkIndex, linkIndex + 1, GtkAttachOptions(GTK_EXPAND | GTK_FILL), GTK_SHRINK, 4,
							 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), ioSettingComboboxType, 1, 2, linkIndex, linkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), ioSettingButtonUp, 3, 4, linkIndex, linkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), ioSettingButtonDown, 4, 5, linkIndex, linkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pSettingButtonEdit, 5, 6, linkIndex, linkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pSettingButtonDelete, 6, 7, linkIndex, linkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);

			// Set the callbacks
			SLinkCallbackData l_oCallbackData;
			l_oCallbackData.m_pInterfacedScenario = this;
			l_oCallbackData.m_uiLinkIndex         = linkIndex;
			l_oCallbackData.m_bIsInput            = bIsInput;

			vLinkCallbackData[linkIndex] = l_oCallbackData;

			g_signal_connect(G_OBJECT(l_pSettingButtonDelete), "clicked", G_CALLBACK(delete_scenario_link_cb), &vLinkCallbackData[linkIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonEdit), "clicked", G_CALLBACK(edit_scenario_link_cb), &vLinkCallbackData[linkIndex]);
			g_signal_connect(G_OBJECT(ioSettingButtonUp), "clicked", G_CALLBACK(modify_scenario_link_move_up_cb), &vLinkCallbackData[linkIndex]);
			g_signal_connect(G_OBJECT(ioSettingButtonDown), "clicked", G_CALLBACK(modify_scenario_link_move_down_cb), &vLinkCallbackData[linkIndex]);

			g_object_unref(ioSettingsGUIBuilder);
		}
	}

	gtk_widget_show_all(pLinkTable);
}

void CInterfacedScenario::updateScenarioLabel()

{
	GtkLabel* l_pTitleLabel = GTK_LABEL(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_label"));
	string l_sLabel;
	string l_sTempFileName        = m_sFileName;
	string l_sTitleLabelUntrimmed = "unsaved document";
	string::size_type l_iBackSlashIndex;
	while ((l_iBackSlashIndex = l_sTempFileName.find('\\')) != string::npos) { l_sTempFileName[l_iBackSlashIndex] = '/'; }

	l_sLabel += m_hasBeenModified ? "*" : "";
	l_sLabel += " ";

	// trimming file name if the number of character is above ${Designer_ScenarioFileNameTrimmingLimit}
	// trim only unselected scenarios
	if (m_hasFileName)
	{
		l_sTitleLabelUntrimmed = l_sTempFileName;
		l_sTempFileName        = l_sTempFileName.substr(l_sTempFileName.rfind('/') + 1);
		uint32_t trimLimit     = uint32_t(m_kernelContext.getConfigurationManager().expandAsUInteger("${Designer_ScenarioFileNameTrimmingLimit}", 25));
		if (trimLimit > 3) trimLimit -= 3; // limit should include the '...'
		// default = we trim everything but the current scenario filename
		// if  {we are stacking horizontally the scenarios, we trim also } current filename to avoid losing too much of the edition panel.
		if (l_sTempFileName.size() > trimLimit)
		{
			if (m_rApplication.getCurrentInterfacedScenario() == this && m_kernelContext
																		 .getConfigurationManager().expandAsBoolean(
																			 "${Designer_ScenarioTabsVerticalStack}", false))
			{
				l_sTempFileName = "..." + l_sTempFileName.substr(l_sTempFileName.size() - trimLimit, trimLimit);
			}
			if (m_rApplication.getCurrentInterfacedScenario() != this)
			{
				l_sTempFileName = l_sTempFileName.substr(0, trimLimit);
				l_sTempFileName += "...";
			}
		}
		l_sLabel += l_sTempFileName;
	}
	else { l_sLabel += "(untitled)"; }

	l_sLabel += " ";
	l_sLabel += m_hasBeenModified ? "*" : "";

	gtk_label_set_text(l_pTitleLabel, l_sLabel.c_str());

	std::string tooltipLabel = l_sTitleLabelUntrimmed;
	size_t index             = 0;
	while ((index = tooltipLabel.find('&', index)) != std::string::npos)
	{
		tooltipLabel.replace(index, 1, "&amp;");
		index += 5;
	}
	gtk_widget_set_tooltip_markup(GTK_WIDGET(l_pTitleLabel), ("<i>" + tooltipLabel + (m_hasBeenModified ? " - unsaved" : "") + "</i>").c_str());
}

#define updateStencilIndex(id,stencilgc) { (id)++; ::GdkColor sc={0, guint16(((id)&0xff0000)>>8), guint16((id)&0xff00), guint16(((id)&0xff)<<8) }; gdk_gc_set_rgb_fg_color(stencilgc, &sc); }

void CInterfacedScenario::redraw(IBox& box)
{
	GtkWidget* l_widget = GTK_WIDGET(m_pScenarioDrawingArea);
	GdkGC* l_pStencilGC = gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);

	const int marginX      = int(round(5 * m_currentScale));
	const int marginY      = int(round(5 * m_currentScale));
	const int iCircleSize  = int(round(11 * m_currentScale));
	const int iCircleSpace = int(round(4 * m_currentScale));

	//CBoxProxy l_oBoxProxy(m_kernelContext, box);
	CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, box.getIdentifier());

	if (box.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		l_oBoxProxy.setBoxAlgorithmDescriptorOverride(
			static_cast<const IBoxAlgorithmDesc*>(m_kernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId)));
	}

	int xSize  = int(round(l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) * m_currentScale) + marginX * 2);
	int ySize  = int(round(l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) * m_currentScale) + marginY * 2);
	int xStart = int(round(l_oBoxProxy.getXCenter() * m_currentScale + m_viewOffsetX - (xSize >> 1)));
	int yStart = int(round(l_oBoxProxy.getYCenter() * m_currentScale + m_viewOffsetY - (ySize >> 1)));

	updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_currentScale)));
	m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier());

	bool l_bCanCreate                    = l_oBoxProxy.isBoxAlgorithmPluginPresent();
	bool l_bUpToDate                     = l_bCanCreate ? l_oBoxProxy.isUpToDate() : true;
	bool l_bPendingDeprecatedInterfacors = l_oBoxProxy.hasPendingDeprecatedInterfacors();
	bool l_bDeprecated                   = l_bCanCreate && l_oBoxProxy.isDeprecated();
	bool l_bMetabox                      = l_bCanCreate && l_oBoxProxy.isMetabox();
	bool l_bDisabled                     = l_oBoxProxy.isDisabled();


	// Check if this is a mensia box
	auto l_pPOD    = m_kernelContext.getPluginManager().getPluginObjectDescCreating(box.getAlgorithmClassIdentifier());
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

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBorderSelected]);
		gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(l_widget->window, l_pDrawGC, TRUE, xStart - l_iTopLeftOffset, yStart - l_iTopLeftOffset, xSize + l_iBottomRightOffset,
								   ySize + l_iBottomRightOffset);
	}

	if (!this->isLocked() || !m_debugCPUUsage)
	{
		/*if(m_vCurrentObject[box.getIdentifier()])
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundSelected]);
		}
		else*/
		if (!l_bCanCreate) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundMissing]); }
		else if (l_bDisabled) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundDisabled]); }
		else if (l_bDeprecated) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundDeprecated]); }
		else if (!l_bUpToDate || l_bPendingDeprecatedInterfacors) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundOutdated]); }
		else if (l_bMensia) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundMensia]); }
			/*
					else if(l_bMetabox)
					{
						gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundMetabox]);
					}
			*/
		else { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackground]); }
	}
	else
	{
		CIdentifier l_oComputationTime;
		l_oComputationTime.fromString(box.getAttributeValue(OV_AttributeId_Box_ComputationTimeLastSecond));
		uint64_t l_ui64ComputationTime          = (l_oComputationTime == OV_UndefinedIdentifier ? 0 : l_oComputationTime.toUInteger());
		uint64_t l_ui64ComputationTimeReference = (1LL << 32) / (m_boxCount == 0 ? 1 : m_boxCount);

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
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
	}

	gdk_draw_rounded_rectangle(l_widget->window, l_pDrawGC, TRUE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_currentScale)));

	if (l_bMensia)
	{
		gdk_draw_pixbuf(l_widget->window, l_pDrawGC, m_pMensiaLogoPixbuf, 5, 5, xStart, yStart, 80, (ySize < 50) ? ySize : 50, GDK_RGB_DITHER_NONE, 0, 0);
	}

	int l_iBorderColor = Color_BoxBorder;
	if (l_bMensia) { l_iBorderColor = Color_BoxBorderMensia; }
	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBorderColor]);
	gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
	gdk_draw_rounded_rectangle(l_widget->window, l_pDrawGC, FALSE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_currentScale)));

	if (l_bMetabox)
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBorderColor]);
		gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(l_widget->window, l_pDrawGC, FALSE, xStart - 3, yStart - 3, xSize + 6, ySize + 6, gint(round(8.0 * m_currentScale)));
	}

	TAttributeHandler l_oAttributeHandler(box);

	int l_iInputOffset = xSize / 2 - int(box.getInputCount()) * (iCircleSpace + iCircleSize) / 2 + iCircleSize / 4;
	for (uint32_t i = 0; i < box.getInterfacorCountIncludingDeprecated(Input); ++i)
	{
		CIdentifier l_oInputIdentifier;
		bool isDeprecated;
		box.getInputType(i, l_oInputIdentifier);
		box.getInterfacorDeprecatedStatus(Input, i, isDeprecated);
		GdkColor l_oInputColor = colorFromIdentifier(l_oInputIdentifier);


		GdkPoint l_vPoint[4];
		l_vPoint[0].x = iCircleSize >> 1;
		l_vPoint[0].y = iCircleSize;
		l_vPoint[1].x = 0;
		l_vPoint[1].y = 0;
		l_vPoint[2].x = iCircleSize - 1;
		l_vPoint[2].y = 0;
		for (int j = 0; j < 3; j++)
		{
			l_vPoint[j].x += xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset;
			l_vPoint[j].y += yStart - (iCircleSize >> 1);
		}

		updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, l_vPoint, 3);
		m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Input, i);

		if (isDeprecated)
		{
			l_oInputColor.blue  = 2 * l_oInputColor.blue / 3;
			l_oInputColor.red   = 2 * l_oInputColor.red / 3;
			l_oInputColor.green = 2 * l_oInputColor.green / 3;
		}

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oInputColor);

		gdk_draw_polygon(l_widget->window, l_pDrawGC, TRUE, l_vPoint, 3);
		int l_iBoxInputBorderColor = Color_BoxInputBorder;
		if (isDeprecated) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]); }
		else { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxInputBorderColor]); }
		gdk_draw_polygon(l_widget->window, l_pDrawGC, FALSE, l_vPoint, 3);

		int x                         = xStart + i * (iCircleSpace + iCircleSize) + (iCircleSize >> 1) - m_viewOffsetX + l_iInputOffset;
		int y                         = yStart - (iCircleSize >> 1) - m_viewOffsetY;
		CIdentifier l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierToBoxInput(OV_UndefinedIdentifier, box.getIdentifier(), i);
		while (l_oLinkIdentifier != OV_UndefinedIdentifier)
		{
			ILink* l_pLink = m_rScenario.getLinkDetails(l_oLinkIdentifier);
			if (l_pLink)
			{
				TAttributeHandler attributeHandler(*l_pLink);

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_XTargetPosition))
				{
					attributeHandler.addAttribute<int>(OV_AttributeId_Link_XTargetPosition, x);
				}
				else { attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XTargetPosition, x); }

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
				{
					attributeHandler.addAttribute<int>(OV_AttributeId_Link_YTargetPosition, y);
				}
				else { attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YTargetPosition, y); }
			}
			l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierToBoxInput(l_oLinkIdentifier, box.getIdentifier(), i);
		}

		// Display a circle above inputs that are linked to the box inputs
		for (uint32_t l_scenarioInputIndex = 0; l_scenarioInputIndex < m_rScenario.getInputCount(); l_scenarioInputIndex++)
		{
			CIdentifier scenarioInputLinkBoxIdentifier;
			uint32_t l_scenarioInputLinkBoxInputIndex;

			m_rScenario.getScenarioInputLink(l_scenarioInputIndex, scenarioInputLinkBoxIdentifier, l_scenarioInputLinkBoxInputIndex);

			if (scenarioInputLinkBoxIdentifier == box.getIdentifier() && l_scenarioInputLinkBoxInputIndex == i)
			{
				// Since the circle representing the input is quite large, we are going to offset each other one
				int l_iInputDiscOffset = int(i % 2) * iCircleSize * 2;

				int l_iScenarioInputIndicatorLeft = xStart + int(i) * (iCircleSpace + iCircleSize) + l_iInputOffset - int(iCircleSize * 0.5);
				int l_iScenarioInputIndicatorTop  = yStart - (iCircleSize >> 1) - iCircleSize * 3 - l_iInputDiscOffset;

				CIdentifier scenarioInputTypeIdentifier;
				this->m_rScenario.getInputType(l_scenarioInputIndex, scenarioInputTypeIdentifier);
				GdkColor inputColor = colorFromIdentifier(scenarioInputTypeIdentifier);

				updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE,
							 l_iScenarioInputIndicatorLeft,
							 l_iScenarioInputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_ScenarioInput, i);

				gdk_gc_set_rgb_fg_color(l_pDrawGC, &inputColor);

				gdk_draw_arc(l_widget->window, l_pDrawGC, TRUE,
							 l_iScenarioInputIndicatorLeft,
							 l_iScenarioInputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxInputBorderColor]);
				gdk_draw_arc(l_widget->window, l_pDrawGC, FALSE,
							 l_iScenarioInputIndicatorLeft,
							 l_iScenarioInputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);

				// Draw the text indicating the scenario input index
				PangoContext* l_pPangoContext = nullptr;
				PangoLayout* l_pPangoLayout   = nullptr;
				l_pPangoContext               = gtk_widget_get_pango_context(l_widget);
				l_pPangoLayout                = pango_layout_new(l_pPangoContext);
				pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
				pango_layout_set_markup(l_pPangoLayout, std::to_string(static_cast<long long int>(l_scenarioInputIndex + 1)).c_str(), -1);
				gdk_draw_layout(l_widget->window, l_widget->style->text_gc[GTK_WIDGET_STATE(l_widget)],
								l_iScenarioInputIndicatorLeft + marginX, l_iScenarioInputIndicatorTop + marginY, l_pPangoLayout);
				g_object_unref(l_pPangoLayout);
				gdk_draw_line(l_widget->window, l_pDrawGC,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset + (iCircleSize >> 1),
							  l_iScenarioInputIndicatorTop + iCircleSize * 2,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset + (iCircleSize >> 1),
							  yStart - (iCircleSize >> 1));
			}
		}
	}

	gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);

	int l_iOutputOffset = xSize / 2 - int(box.getOutputCount()) * (iCircleSpace + iCircleSize) / 2 + iCircleSize / 4;
	for (uint32_t i = 0; i < box.getInterfacorCountIncludingDeprecated(Output); ++i)
	{
		CIdentifier l_oOutputIdentifier;
		bool isDeprecated;
		box.getOutputType(i, l_oOutputIdentifier);
		box.getInterfacorDeprecatedStatus(Output, i, isDeprecated);
		GdkColor l_oOutputColor = colorFromIdentifier(l_oOutputIdentifier);

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
		for (int j = 0; j < 3; j++)
		{
			l_vPoint[j].x += xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset;
			l_vPoint[j].y += yStart - (iCircleSize >> 1) + ySize;
		}

		updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, l_vPoint, 3);
		m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Output, i);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oOutputColor);
		gdk_draw_polygon(l_widget->window, l_pDrawGC, TRUE, l_vPoint, 3);
		int l_iBoxOutputBorderColor = Color_BoxOutputBorder;
		if (isDeprecated) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]); }
		else { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxOutputBorderColor]); }

		gdk_draw_polygon(l_widget->window, l_pDrawGC, FALSE, l_vPoint, 3);

		int x                         = xStart + i * (iCircleSpace + iCircleSize) + (iCircleSize >> 1) - m_viewOffsetX + l_iOutputOffset;
		int y                         = yStart + ySize + (iCircleSize >> 1) + 1 - m_viewOffsetY;
		CIdentifier l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierFromBoxOutput(OV_UndefinedIdentifier, box.getIdentifier(), i);
		while (l_oLinkIdentifier != OV_UndefinedIdentifier)
		{
			ILink* l_pLink = m_rScenario.getLinkDetails(l_oLinkIdentifier);
			if (l_pLink)
			{
				TAttributeHandler attributeHandler(*l_pLink);

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_XSourcePosition))
				{
					attributeHandler.addAttribute<int>(OV_AttributeId_Link_XSourcePosition, x);
				}
				else { attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XSourcePosition, x); }

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
				{
					attributeHandler.addAttribute<int>(OV_AttributeId_Link_YSourcePosition, y);
				}
				else attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YSourcePosition, y);
			}
			l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierFromBoxOutput(l_oLinkIdentifier, box.getIdentifier(), i);
		}

		// Display a circle below outputs that are linked to the box outputs
		for (uint32_t l_scenarioOutputIndex = 0; l_scenarioOutputIndex < m_rScenario.getOutputCount(); l_scenarioOutputIndex++)
		{
			CIdentifier scenarioOutputLinkBoxIdentifier;
			uint32_t l_scenarioOutputLinkBoxOutputIndex;

			m_rScenario.getScenarioOutputLink(l_scenarioOutputIndex, scenarioOutputLinkBoxIdentifier, l_scenarioOutputLinkBoxOutputIndex);

			if (scenarioOutputLinkBoxIdentifier == box.getIdentifier() && l_scenarioOutputLinkBoxOutputIndex == i)
			{
				// Since the circle representing the Output is quite large, we are going to offset each other one
				int l_iOutputDiscOffset = (int(i) % 2) * iCircleSize * 2;

				int l_iScenarioOutputIndicatorLeft = xStart + int(i) * (iCircleSpace + iCircleSize) + l_iOutputOffset - int(iCircleSize * 0.5);
				int l_iScenarioOutputIndicatorTop  = yStart - (iCircleSize >> 1) + ySize + l_iOutputDiscOffset + iCircleSize * 2;

				CIdentifier scenarioOutputTypeIdentifier;
				this->m_rScenario.getOutputType(l_scenarioOutputIndex, scenarioOutputTypeIdentifier);
				GdkColor oOutputColor = colorFromIdentifier(scenarioOutputTypeIdentifier);

				updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE,
							 l_iScenarioOutputIndicatorLeft,
							 l_iScenarioOutputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_ScenarioOutput, i);

				gdk_gc_set_rgb_fg_color(l_pDrawGC, &oOutputColor);
				gdk_draw_arc(l_widget->window, l_pDrawGC, TRUE,
							 l_iScenarioOutputIndicatorLeft,
							 l_iScenarioOutputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxOutputBorderColor]);
				gdk_draw_arc(l_widget->window, l_pDrawGC, FALSE,
							 l_iScenarioOutputIndicatorLeft,
							 l_iScenarioOutputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);

				PangoContext* l_pPangoContext = nullptr;
				PangoLayout* l_pPangoLayout   = nullptr;
				l_pPangoContext               = gtk_widget_get_pango_context(l_widget);
				l_pPangoLayout                = pango_layout_new(l_pPangoContext);
				pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
				pango_layout_set_markup(l_pPangoLayout, std::to_string(static_cast<long long int>(l_scenarioOutputIndex + 1)).c_str(), -1);
				gdk_draw_layout(l_widget->window, l_widget->style->text_gc[GTK_WIDGET_STATE(l_widget)],
								l_iScenarioOutputIndicatorLeft + marginX, l_iScenarioOutputIndicatorTop + marginY, l_pPangoLayout);
				g_object_unref(l_pPangoLayout);
				gdk_draw_line(l_widget->window, l_pDrawGC,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset + (iCircleSize >> 1),
							  l_iScenarioOutputIndicatorTop,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset + (iCircleSize >> 1),
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
	l_pPangoContext               = gtk_widget_get_pango_context(l_widget);
	l_pPangoLayout                = pango_layout_new(l_pPangoContext);

	// Draw box label
	PangoRectangle l_oPangoLabelRect;
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(l_pPangoLayout, l_oBoxProxy.getLabel(), -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoLabelRect);
	gdk_draw_layout(l_widget->window, l_widget->style->text_gc[GTK_WIDGET_STATE(l_widget)], xStart + marginX, yStart + marginY, l_pPangoLayout);

	// Draw box status label
	PangoRectangle l_oPangoStatusRect;
	pango_layout_set_markup(l_pPangoLayout, l_oBoxProxy.getStatusLabel(), -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoStatusRect);
	int xShift = (max(l_oPangoLabelRect.width, l_oPangoStatusRect.width) -
				  min(l_oPangoLabelRect.width, l_oPangoStatusRect.width)) / 2;

	updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
	gdk_draw_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE,
					   xStart + xShift + marginX, yStart + l_oPangoLabelRect.height + marginY,
					   l_oPangoStatusRect.width, l_oPangoStatusRect.height);
	m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(box.getIdentifier(), Box_Update, 0);
	gdk_draw_layout(l_widget->window, l_widget->style->text_gc[GTK_WIDGET_STATE(l_widget)], xStart + xShift + marginX,
					yStart + l_oPangoLabelRect.height + marginY, l_pPangoLayout);

	g_object_unref(l_pPangoLayout);
	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);

	/*
		CLinkPositionSetterEnum l_oLinkPositionSetterInput(Connector_Input, l_vInputPosition);
		CLinkPositionSetterEnum l_oLinkPositionSetterOutput(Connector_Output, l_vOutputPosition);
		rScenario.enumerateLinksToBox(l_oLinkPositionSetterInput, box.getIdentifier());
		rScenario.enumerateLinksFromBox(l_oLinkPositionSetterOutput, box.getIdentifier());
	*/
}

void CInterfacedScenario::redraw(IComment& rComment)
{
	GtkWidget* l_widget = GTK_WIDGET(m_pScenarioDrawingArea);
	GdkGC* l_pStencilGC = gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);

	// uint32_t i;
	const int marginX = static_cast<const int>(round(16 * m_currentScale));
	const int marginY = static_cast<const int>(round(16 * m_currentScale));

	const CCommentProxy l_oCommentProxy(m_kernelContext, rComment);
	const int sizeX  = l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) + marginX * 2;
	const int sizeY  = l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) + marginY * 2;
	const int startX = int(round(l_oCommentProxy.getXCenter() * m_currentScale + m_viewOffsetX - (sizeX >> 1)));
	const int startY = int(round(l_oCommentProxy.getYCenter() * m_currentScale + m_viewOffsetY - (sizeY >> 1)));

	updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, startX, startY, sizeX, sizeY, gint(round(16.0 * m_currentScale)));
	m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(rComment.getIdentifier());

	gdk_gc_set_rgb_fg_color(
		l_pDrawGC, &g_vColors[m_SelectedObjects.count(rComment.getIdentifier()) ? Color_CommentBackgroundSelected : Color_CommentBackground]);
	gdk_draw_rounded_rectangle(l_widget->window, l_pDrawGC, TRUE, startX, startY, sizeX, sizeY, gint(round(16.0 * m_currentScale)));
	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[m_SelectedObjects.count(rComment.getIdentifier()) ? Color_CommentBorderSelected : Color_CommentBorder]);
	gdk_draw_rounded_rectangle(l_widget->window, l_pDrawGC, FALSE, startX, startY, sizeX, sizeY, gint(round(16.0 * m_currentScale)));

	PangoContext* l_pPangoContext = gtk_widget_get_pango_context(l_widget);
	PangoLayout* l_pPangoLayout   = pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	if (pango_parse_markup(rComment.getText().toASCIIString(), -1, 0, nullptr, nullptr, nullptr, nullptr))
	{
		pango_layout_set_markup(l_pPangoLayout, rComment.getText().toASCIIString(), -1);
	}
	else { pango_layout_set_text(l_pPangoLayout, rComment.getText().toASCIIString(), -1); }
	gdk_draw_layout(l_widget->window, l_widget->style->text_gc[GTK_WIDGET_STATE(l_widget)], startX + marginX, startY + marginY, l_pPangoLayout);
	g_object_unref(l_pPangoLayout);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);
}

void CInterfacedScenario::redraw(ILink& rLink)
{
	GtkWidget* l_widget = GTK_WIDGET(m_pScenarioDrawingArea);
	GdkGC* l_pStencilGC = gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);

	CLinkProxy l_oLinkProxy(rLink);

	CIdentifier l_oSourceOutputTypeIdentifier;
	CIdentifier l_oTargetInputTypeIdentifier;

	m_rScenario.getBoxDetails(rLink.getSourceBoxIdentifier())->getOutputType(rLink.getSourceBoxOutputIndex(), l_oSourceOutputTypeIdentifier);
	m_rScenario.getBoxDetails(rLink.getTargetBoxIdentifier())->getInputType(rLink.getTargetBoxInputIndex(), l_oTargetInputTypeIdentifier);

	if (rLink.hasAttribute(OV_AttributeId_Link_Invalid)) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]); }
	else if (m_SelectedObjects.count(rLink.getIdentifier())) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkSelected]); }
	else if (l_oTargetInputTypeIdentifier == l_oSourceOutputTypeIdentifier) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_Link]); }
	else
	{
		if (m_kernelContext.getTypeManager().isDerivedFromStream(l_oSourceOutputTypeIdentifier, l_oTargetInputTypeIdentifier))
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkDownCast]);
		}
		else if (m_kernelContext.getTypeManager().isDerivedFromStream(l_oTargetInputTypeIdentifier, l_oSourceOutputTypeIdentifier))
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkUpCast]);
		}
		else { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]); }
	}

	updateStencilIndex(m_interfacedObjectId, l_pStencilGC);
	gdk_draw_line(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC,
				  l_oLinkProxy.getXSource() + m_viewOffsetX, l_oLinkProxy.getYSource() + m_viewOffsetY,
				  l_oLinkProxy.getXTarget() + m_viewOffsetX, l_oLinkProxy.getYTarget() + m_viewOffsetY);
	gdk_draw_line(l_widget->window, l_pDrawGC,
				  l_oLinkProxy.getXSource() + m_viewOffsetX, l_oLinkProxy.getYSource() + m_viewOffsetY,
				  l_oLinkProxy.getXTarget() + m_viewOffsetX, l_oLinkProxy.getYTarget() + m_viewOffsetY);
	m_vInterfacedObject[m_interfacedObjectId] = CInterfacedObject(rLink.getIdentifier(), Box_Link, 0);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);
}

#undef updateStencilIndex

uint32_t CInterfacedScenario::pickInterfacedObject(const int x, const int y) const
{
	if (!GDK_DRAWABLE(m_pStencilBuffer))
	{
		// m_kernelContext.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return 0xffffffff;
	}

	int l_iMaxX;
	int l_iMaxY;
	uint32_t l_interfacedObjectId = 0xffffffff;
	gdk_drawable_get_size(GDK_DRAWABLE(m_pStencilBuffer), &l_iMaxX, &l_iMaxY);
	if (x >= 0 && y >= 0 && x < l_iMaxX && y < l_iMaxY)
	{
		GdkPixbuf* l_pPixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_pStencilBuffer), nullptr, x, y, 0, 0, 1, 1);
		if (!l_pPixbuf)
		{
			m_kernelContext.getLogManager() << LogLevel_ImportantWarning <<
					"Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		guchar* l_pPixels = gdk_pixbuf_get_pixels(l_pPixbuf);
		if (!l_pPixels)
		{
			m_kernelContext.getLogManager() << LogLevel_ImportantWarning <<
					"Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		l_interfacedObjectId = 0;
		l_interfacedObjectId += (l_pPixels[0] << 16);
		l_interfacedObjectId += (l_pPixels[1] << 8);
		l_interfacedObjectId += (l_pPixels[2]);
		g_object_unref(l_pPixbuf);
	}
	return l_interfacedObjectId;
}

bool CInterfacedScenario::pickInterfacedObject(const int x, const int y, int iSizeX, int iSizeY)
{
	if (!GDK_DRAWABLE(m_pStencilBuffer))
	{
		// m_kernelContext.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return false;
	}

	int l_iMaxX;
	int l_iMaxY;
	gdk_drawable_get_size(GDK_DRAWABLE(m_pStencilBuffer), &l_iMaxX, &l_iMaxY);

	int iStartX = x;
	int iStartY = y;
	int iEndX   = x + iSizeX;
	int iEndY   = y + iSizeY;

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
	iSizeX = iEndX - iStartX + 1;
	iSizeY = iEndY - iStartY + 1;

	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_pStencilBuffer), nullptr, iStartX, iStartY, 0, 0, iSizeX, iSizeY);
	if (!pixbuf)
	{
		m_kernelContext.getLogManager() << LogLevel_ImportantWarning <<
				"Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
		return false;
	}

	guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
	if (!pixels)
	{
		m_kernelContext.getLogManager() << LogLevel_ImportantWarning <<
				"Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
		return false;
	}

	const int rowBytesCount = gdk_pixbuf_get_rowstride(pixbuf);
	const int nChannel  = gdk_pixbuf_get_n_channels(pixbuf);
	for (int j = 0; j < iSizeY; j++)
	{
		for (int i = 0; i < iSizeX; ++i)
		{
			uint32_t interfacedObjectId = 0;
			interfacedObjectId += (pixels[j * rowBytesCount + i * nChannel + 0] << 16);
			interfacedObjectId += (pixels[j * rowBytesCount + i * nChannel + 1] << 8);
			interfacedObjectId += (pixels[j * rowBytesCount + i * nChannel + 2]);
			if (m_vInterfacedObject[interfacedObjectId].m_oIdentifier != OV_UndefinedIdentifier)
			{
				m_SelectedObjects.insert(m_vInterfacedObject[interfacedObjectId].m_oIdentifier);
			}
		}
	}

	g_object_unref(pixbuf);
	return true;
}

#define OV_ClassId_Selected OpenViBE::CIdentifier(0xC67A01DC, 0x28CE06C1)

void CInterfacedScenario::undoCB(const bool bManageModifiedStatusFlag)
{
	// When a box gets updated we generate a snapshot beforehand to enable undo in all cases
	// This will result in two indentical undo states, in order to avoid weird Redo, we drop the
	// reduntant state at this moment
	bool shouldDropLastState = false;
	if (m_rScenario.containsBoxWithDeprecatedInterfacors()) { shouldDropLastState = true; }

	if (m_oStateStack->undo())
	{
		CIdentifier identifier;
		m_SelectedObjects.clear();
		while ((identifier = m_rScenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getBoxDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}
		while ((identifier = m_rScenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getLinkDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}

		if (m_pDesignerVisualization) { m_pDesignerVisualization->load(); }
		if (bManageModifiedStatusFlag) { m_hasBeenModified = true; }

		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();

		if (shouldDropLastState) { m_oStateStack->dropLastState(); }

		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), m_oStateStack->isRedoPossible());
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo")), m_oStateStack->isUndoPossible());
	}
	else
	{
		m_kernelContext.getLogManager() << LogLevel_Trace << "Can not undo\n";
		GtkWidget* l_pUndoButton = GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo"));
		gtk_widget_set_sensitive(l_pUndoButton, false);
	}
}

void CInterfacedScenario::redoCB(const bool bManageModifiedStatusFlag)
{
	if (m_oStateStack->redo())
	{
		CIdentifier identifier;
		m_SelectedObjects.clear();
		while ((identifier = m_rScenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getBoxDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}
		while ((identifier = m_rScenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getLinkDetails(identifier)->hasAttribute(OV_ClassId_Selected)) { m_SelectedObjects.insert(identifier); }
		}

		if (m_pDesignerVisualization) { m_pDesignerVisualization->load(); }

		if (bManageModifiedStatusFlag) { m_hasBeenModified = true; }
		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), m_oStateStack->isRedoPossible());
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo")), m_oStateStack->isUndoPossible());
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), false);
		m_kernelContext.getLogManager() << LogLevel_Trace << "Can not redo\n";
	}
}

void CInterfacedScenario::snapshotCB(const bool bManageModifiedStatusFlag)
{
	if (m_rScenario.containsBoxWithDeprecatedInterfacors())
	{
		OV_WARNING("Scenario containing boxes with deprecated I/O or Settings does not support undo", m_kernelContext.getLogManager());
	}
	else
	{
		CIdentifier identifier;

		while ((identifier = m_rScenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(identifier)) { m_rScenario.getBoxDetails(identifier)->addAttribute(OV_ClassId_Selected, ""); }
			else { m_rScenario.getBoxDetails(identifier)->removeAttribute(OV_ClassId_Selected); }
		}
		while ((identifier = m_rScenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(identifier)) m_rScenario.getLinkDetails(identifier)->addAttribute(OV_ClassId_Selected, "");
			else m_rScenario.getLinkDetails(identifier)->removeAttribute(OV_ClassId_Selected);
		}

		if (bManageModifiedStatusFlag) { m_hasBeenModified = true; }
		this->updateScenarioLabel();
		m_oStateStack->snapshot();
	}
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), m_oStateStack->isRedoPossible());
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo")), m_oStateStack->isUndoPossible());
}

void CInterfacedScenario::addCommentCB(int x, int y)
{
	CIdentifier identifier;
	m_rScenario.addComment(identifier, OV_UndefinedIdentifier);
	if (x == -1 || y == -1)
	{
		GtkWidget* l_pScrolledWindow  = gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(m_pScenarioDrawingArea)));
		GtkAdjustment* l_pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(l_pScrolledWindow));
		GtkAdjustment* l_pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(l_pScrolledWindow));

#if defined TARGET_OS_Linux && !defined TARGET_OS_MacOS
		x = int(gtk_adjustment_get_value(l_pHAdjustment) + gtk_adjustment_get_page_size(l_pHAdjustment) / 2);
		y = int(gtk_adjustment_get_value(l_pVAdjustment) + gtk_adjustment_get_page_size(l_pVAdjustment) / 2);
#elif defined TARGET_OS_Windows
		gint wx, wy;
		::gdk_window_get_size(gtk_widget_get_parent(GTK_WIDGET(m_pScenarioDrawingArea))->window, &wx, &wy);
		x = int(gtk_adjustment_get_value(l_pHAdjustment) + int(wx / 2));
		y = int(gtk_adjustment_get_value(l_pVAdjustment) + int(wy / 2));
#else
		x = int(gtk_adjustment_get_value(l_pHAdjustment) + 32);
		y = int(gtk_adjustment_get_value(l_pVAdjustment) + 32);
#endif
	}

	CCommentProxy l_oCommentProxy(m_kernelContext, m_rScenario, identifier);
	l_oCommentProxy.setCenter(x - m_viewOffsetX, y - m_viewOffsetY);

	// Aligns comemnts on grid
	l_oCommentProxy.setCenter(int((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0L), int((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0L));

	// Applies modifications before snapshot
	l_oCommentProxy.apply();

	CCommentEditorDialog l_oCommentEditorDialog(m_kernelContext, *m_rScenario.getCommentDetails(identifier), m_sGUIFilename.c_str());
	if (!l_oCommentEditorDialog.run()) { m_rScenario.removeComment(identifier); }
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

	gtk_widget_show_all(m_pSettingsVBox);

	const gint response = gtk_dialog_run(GTK_DIALOG(m_pConfigureSettingsDialog));

	if (response == GTK_RESPONSE_CANCEL) { this->undoCB(false); }
	else { this->snapshotCB(); }

	gtk_widget_hide(m_pConfigureSettingsDialog);
	this->redrawScenarioSettings();
}

void CInterfacedScenario::addScenarioSettingCB()

{
	char l_sName[1024];
	sprintf(l_sName, "Setting %u", m_rScenario.getSettingCount() + 1);
	m_rScenario.addSetting(l_sName, OVTK_TypeId_Integer, "0", OV_Value_UndefinedIndexUInt, false,
						   m_rScenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));

	this->redrawConfigureScenarioSettingsDialog();
}

void CInterfacedScenario::addScenarioInputCB()

{
	char l_sName[1024];
	sprintf(l_sName, "Input %u", m_rScenario.getInputCount() + 1);

	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the inputs of the box.
	m_rScenario.addInput(l_sName, OVTK_TypeId_StreamedMatrix, m_rScenario.getUnusedInputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor l_oConnectorEditor(m_kernelContext, m_rScenario, Box_Input, m_rScenario.getInputCount() - 1, "Add Input", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
	else { m_rScenario.removeInput(m_rScenario.getInputCount() - 1); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::editScenarioInputCB(const unsigned int index)

{
	CConnectorEditor l_oConnectorEditor(m_kernelContext, m_rScenario, Box_Input, index, "Edit Input", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::addScenarioOutputCB()

{
	char l_sName[1024];
	sprintf(l_sName, "Output %u", m_rScenario.getOutputCount() + 1);

	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the outputs of the box.
	m_rScenario.addOutput(l_sName, OVTK_TypeId_StreamedMatrix, m_rScenario.getUnusedOutputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor l_oConnectorEditor(m_kernelContext, m_rScenario, Box_Output, m_rScenario.getOutputCount() - 1, "Add Output", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
	else { m_rScenario.removeOutput(m_rScenario.getOutputCount() - 1); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::editScenarioOutputCB(const unsigned int outputIdx)

{
	CConnectorEditor l_oConnectorEditor(m_kernelContext, m_rScenario, Box_Output, outputIdx, "Edit Output", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::swapScenarioSettings(const unsigned int settingAIndex, const unsigned int settingBIndex)
{
	m_rScenario.swapSettings(settingAIndex, settingBIndex);
	this->redrawConfigureScenarioSettingsDialog();
}


void CInterfacedScenario::swapScenarioInputs(const unsigned int inputAIndex, const unsigned int inputBIndex)
{
	CIdentifier ABoxIdentifier;
	unsigned int ABoxInputIndex;
	CIdentifier BBoxIdentifier;
	unsigned int BBoxInputIndex;

	m_rScenario.getScenarioInputLink(inputAIndex, ABoxIdentifier, ABoxInputIndex);
	m_rScenario.getScenarioInputLink(inputBIndex, BBoxIdentifier, BBoxInputIndex);

	m_rScenario.swapInputs(inputAIndex, inputBIndex);

	m_rScenario.setScenarioInputLink(inputBIndex, ABoxIdentifier, ABoxInputIndex);
	m_rScenario.setScenarioInputLink(inputAIndex, BBoxIdentifier, BBoxInputIndex);

	this->redrawScenarioInputSettings();
	this->redraw();
}

void CInterfacedScenario::swapScenarioOutputs(const unsigned int outputAIndex, const unsigned int outputBIndex)
{
	CIdentifier ABoxIdentifier;
	unsigned int ABoxOutputIndex;
	CIdentifier BBoxIdentifier;
	unsigned int BBoxOutputIndex;

	m_rScenario.getScenarioOutputLink(outputAIndex, ABoxIdentifier, ABoxOutputIndex);
	m_rScenario.getScenarioOutputLink(outputBIndex, BBoxIdentifier, BBoxOutputIndex);

	m_rScenario.swapOutputs(outputAIndex, outputBIndex);

	m_rScenario.setScenarioOutputLink(outputBIndex, ABoxIdentifier, ABoxOutputIndex);
	m_rScenario.setScenarioOutputLink(outputAIndex, BBoxIdentifier, BBoxOutputIndex);

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

		CIdentifier l_oBoxIdentifier;
		while ((l_oBoxIdentifier = m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
		{
			//CBoxProxy l_oBoxProxy(m_kernelContext, *m_rScenario.getBoxDetails(l_oBoxIdentifier));
			CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, l_oBoxIdentifier);
			l_iMinX = std::min(l_iMinX, gint((l_oBoxProxy.getXCenter() - 1.0 * l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxX = std::max(l_iMaxX, gint((l_oBoxProxy.getXCenter() + 1.0 * l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
			l_iMinY = std::min(
				l_iMinY, gint((l_oBoxProxy.getYCenter() - 1.0 * l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxY = std::max(
				l_iMaxY, gint((l_oBoxProxy.getYCenter() + 1.0 * l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
		}

		CIdentifier l_oCommentIdentifier;
		while ((l_oCommentIdentifier = m_rScenario.getNextCommentIdentifier(l_oCommentIdentifier)) != OV_UndefinedIdentifier)
		{
			CCommentProxy l_oCommentProxy(m_kernelContext, *m_rScenario.getCommentDetails(l_oCommentIdentifier));
			l_iMinX = std::min(
				l_iMinX, gint((l_oCommentProxy.getXCenter() - 1.0 * l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxX = std::max(
				l_iMaxX, gint((l_oCommentProxy.getXCenter() + 1.0 * l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
			l_iMinY = std::min(
				l_iMinY, gint((l_oCommentProxy.getYCenter() - 1.0 * l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
			l_iMaxY = std::max(
				l_iMaxY, gint((l_oCommentProxy.getYCenter() + 1.0 * l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_currentScale));
		}

		const gint l_iNewScenarioSizeX = l_iMaxX - l_iMinX;
		const gint l_iNewScenarioSizeY = l_iMaxY - l_iMinY;
		gint l_iOldScenarioSizeX       = -1;
		gint l_iOldScenarioSizeY       = -1;

		gdk_window_get_size(GTK_WIDGET(m_pScenarioViewport)->window, &l_iViewportX, &l_iViewportY);
		gtk_widget_get_size_request(GTK_WIDGET(m_pScenarioDrawingArea), &l_iOldScenarioSizeX, &l_iOldScenarioSizeY);

		if (l_iNewScenarioSizeX >= 0 && l_iNewScenarioSizeY >= 0)
		{
			if (l_iOldScenarioSizeX != l_iNewScenarioSizeX + 2 * l_iMarginX || l_iOldScenarioSizeY != l_iNewScenarioSizeY + 2 * l_iMarginY)
			{
				gtk_widget_set_size_request(GTK_WIDGET(m_pScenarioDrawingArea), l_iNewScenarioSizeX + 2 * l_iMarginX, l_iNewScenarioSizeY + 2 * l_iMarginY);
			}
			m_viewOffsetX = std::min(m_viewOffsetX, -l_iMaxX - l_iMarginX + std::max(l_iViewportX, l_iNewScenarioSizeX + 2 * l_iMarginX));
			m_viewOffsetX = std::max(m_viewOffsetX, -l_iMinX + l_iMarginX);
			m_viewOffsetY = std::min(m_viewOffsetY, -l_iMaxY - l_iMarginY + std::max(l_iViewportY, l_iNewScenarioSizeY + 2 * l_iMarginY));
			m_viewOffsetY = std::max(m_viewOffsetY, -l_iMinY + l_iMarginY);
		}
	}

	gint x, y;

	gdk_window_get_size(GTK_WIDGET(m_pScenarioDrawingArea)->window, &x, &y);
	if (m_pStencilBuffer) { g_object_unref(m_pStencilBuffer); }
	m_pStencilBuffer = gdk_pixmap_new(GTK_WIDGET(m_pScenarioDrawingArea)->window, x, y, -1);

	GdkGC* l_pStencilGC = gdk_gc_new(m_pStencilBuffer);
	GdkColor l_oColor   = { 0, 0, 0, 0 };
	gdk_gc_set_rgb_fg_color(l_pStencilGC, &l_oColor);
	gdk_draw_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, 0, 0, x, y);
	g_object_unref(l_pStencilGC);

	if (this->isLocked())
	{
		l_oColor.pixel = 0;
		l_oColor.red   = 0x0f00;
		l_oColor.green = 0x0f00;
		l_oColor.blue  = 0x0f00;

		GdkGC* l_pDrawGC = gdk_gc_new(GTK_WIDGET(m_pScenarioDrawingArea)->window);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
		gdk_gc_set_function(l_pDrawGC, GDK_XOR);
		gdk_draw_rectangle(GTK_WIDGET(m_pScenarioDrawingArea)->window, l_pDrawGC, TRUE, 0, 0, x, y);
		g_object_unref(l_pDrawGC);
	}
	// TODO: optimize this as this will be called endlessly
	/*
	else if (false) //m_rScenario.containsBoxWithDeprecatedInterfacors() 
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
	m_vInterfacedObject.clear();

	uint32_t commentCount = 0;
	CIdentifier l_oCommentIdentifier;
	while ((l_oCommentIdentifier = m_rScenario.getNextCommentIdentifier(l_oCommentIdentifier)) != OV_UndefinedIdentifier)
	{
		redraw(*m_rScenario.getCommentDetails(l_oCommentIdentifier));
		commentCount++;
	}
	m_commentCount = commentCount;

	uint32_t l_ui32BoxCount = 0;
	CIdentifier l_oBoxIdentifier;
	while ((l_oBoxIdentifier = m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
	{
		redraw(*m_rScenario.getBoxDetails(l_oBoxIdentifier));
		l_ui32BoxCount++;
	}
	m_boxCount = l_ui32BoxCount;

	uint32_t linkCount = 0;
	CIdentifier l_oLinkIdentifier;
	while ((l_oLinkIdentifier = m_rScenario.getNextLinkIdentifier(l_oLinkIdentifier)) != OV_UndefinedIdentifier)
	{
		redraw(*m_rScenario.getLinkDetails(l_oLinkIdentifier));
		linkCount++;
	}
	m_linkCount = linkCount;

	if (m_currentMode == Mode_Selection || m_currentMode == Mode_SelectionAdd)
	{
		const int startX = int(std::min(m_pressMouseX, m_currentMouseX));
		const int startY = int(std::min(m_pressMouseY, m_currentMouseY));
		const int sizeX  = int(std::max(m_pressMouseX - m_currentMouseX, m_currentMouseX - m_pressMouseX));
		const int sizeY  = int(std::max(m_pressMouseY - m_currentMouseY, m_currentMouseY - m_pressMouseY));

		GtkWidget* l_widget = GTK_WIDGET(m_pScenarioDrawingArea);
		GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);
		gdk_gc_set_function(l_pDrawGC, GDK_OR);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_SelectionArea]);
		gdk_draw_rectangle(l_widget->window, l_pDrawGC, TRUE, startX, startY, sizeX, sizeY);
		gdk_gc_set_function(l_pDrawGC, GDK_COPY);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_SelectionAreaBorder]);
		gdk_draw_rectangle(l_widget->window, l_pDrawGC, FALSE, startX, startY, sizeX, sizeY);
		g_object_unref(l_pDrawGC);
	}

	if (m_currentMode == Mode_Connect)
	{
		GtkWidget* l_widget = GTK_WIDGET(m_pScenarioDrawingArea);
		GdkGC* l_pDrawGC    = gdk_gc_new(l_widget->window);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_Link]);
		gdk_draw_line(l_widget->window, l_pDrawGC, int(m_pressMouseX), int(m_pressMouseY), int(m_currentMouseX), int(m_currentMouseY));
		g_object_unref(l_pDrawGC);
	}
}

// This method inserts a box into the scenario upon receiving data
void CInterfacedScenario::scenarioDrawingAreaDragDataReceivedCB(GdkDragContext* pDragContext, const gint iX, const gint iY,
																GtkSelectionData* pSelectionData, guint /*info*/, guint /*t*/)
{
	if (this->isLocked()) { return; }

	// two cases: dragged from inside the program = a box ...
	if (pDragContext->protocol == GDK_DRAG_PROTO_LOCAL || pDragContext->protocol == GDK_DRAG_PROTO_XDND)
	{
		CIdentifier l_oBoxIdentifier;
		CIdentifier l_oBoxAlgorithmClassIdentifier;

		// The drag data only contains one string, for a normal box this string is its algorithmClassIdentifier
		// However since all metaboxes have the same identifier, we have added the 'identifier' of a metabox after this string
		// The identifier itself is the name of the scenario which created the metabox
		std::string l_sSelectionData(reinterpret_cast<const char*>(gtk_selection_data_get_text(pSelectionData)));

		// check that there is an identifier inside the string, its form is (0xXXXXXXXX, 0xXXXXXXXX)
		if (l_sSelectionData.find(')') != string::npos)
		{
			l_oBoxAlgorithmClassIdentifier.fromString(l_sSelectionData.substr(0, l_sSelectionData.find(')')).c_str());
		}

		IBox* box                    = nullptr;
		const IPluginObjectDesc* POD = nullptr;

		if (l_oBoxAlgorithmClassIdentifier == OV_UndefinedIdentifier)
		{
			m_currentMouseX = iX;
			m_currentMouseY = iY;
			return;
		}
		if (l_oBoxAlgorithmClassIdentifier == OVP_ClassId_BoxAlgorithm_Metabox)
		{
			// extract the name of the metabox from the drag data string
			CIdentifier metaboxId;
			metaboxId.fromString(CString(l_sSelectionData.substr(l_sSelectionData.find(')') + 1).c_str()));

			//m_kernelContext.getLogManager() << LogLevel_Info << "This is a metabox with ID " << l_sMetaboxIdentifier.c_str() << "\n";
			POD = m_kernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId);

			// insert a box into the scenario, initialize it from the proxy-descriptor from the metabox loader
			m_rScenario.addBox(l_oBoxIdentifier, *static_cast<const IBoxAlgorithmDesc*>(POD), OV_UndefinedIdentifier);

			box = m_rScenario.getBoxDetails(l_oBoxIdentifier);
			box->addAttribute(OVP_AttributeId_Metabox_Identifier, metaboxId.toString());
		}
		else
		{
			m_rScenario.addBox(l_oBoxIdentifier, l_oBoxAlgorithmClassIdentifier, OV_UndefinedIdentifier);

			box                  = m_rScenario.getBoxDetails(l_oBoxIdentifier);
			const CIdentifier id = box->getAlgorithmClassIdentifier();
			POD                  = m_kernelContext.getPluginManager().getPluginObjectDescCreating(id);
		}

		m_SelectedObjects.clear();
		m_SelectedObjects.insert(l_oBoxIdentifier);

		// If a visualization box was dropped, add it in window manager
		if (POD && POD->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_pDesignerVisualization) { m_pDesignerVisualization->onVisualizationBoxAdded(box); }
		}

		CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, l_oBoxIdentifier);
		l_oBoxProxy.setCenter(iX - m_viewOffsetX, iY - m_viewOffsetY);
		// Aligns boxes on grid
		l_oBoxProxy.setCenter(int((l_oBoxProxy.getXCenter() + 8) & 0xfffffff0L), int((l_oBoxProxy.getYCenter() + 8) & 0xfffffff0L));

		// Applies modifications before snapshot
		l_oBoxProxy.apply();

		this->snapshotCB();

		m_currentMouseX = iX;
		m_currentMouseY = iY;
	}

	// ... or dragged from outside the application = a file
	// ONLY AVAILABLE ON WINDOWS (known d'n'd protocol)
#if defined TARGET_OS_Windows
	if (pDragContext->protocol == GDK_DRAG_PROTO_WIN32_DROPFILES)
	{
		// we get the content of the buffer: the list of files URI:
		// file:///path/to/file.ext\r\n
		// file:///path/to/file.ext\r\n
		// ...
		const std::string draggedFilesPath(reinterpret_cast<const char*>(gtk_selection_data_get_data(pSelectionData)));
		std::stringstream l_oStringStream(draggedFilesPath);
		std::string l_sLine;
		std::vector<std::string> l_vFilesToOpen;
		while (std::getline(l_oStringStream, l_sLine))
		{
			// the path starts with file:/// and ends with \r\n,
			// once parsed line after line, a \r remains on Windows
			l_sLine = l_sLine.substr(8, l_sLine.length() - 9);

			// uri to path (to remove %xx escape characters):
			l_sLine = g_uri_unescape_string(l_sLine.c_str(), nullptr);

			l_vFilesToOpen.push_back(l_sLine);
		}

		for (auto& fileName : l_vFilesToOpen) { m_rApplication.openScenario(fileName.c_str()); }
	}
#endif
}

void CInterfacedScenario::scenarioDrawingAreaMotionNotifyCB(GtkWidget* /*widget*/, GdkEventMotion* event)
{
	// m_kernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaMotionNotifyCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* l_pTooltip = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "tooltip"));
	gtk_widget_set_name(l_pTooltip, "gtk-tooltips");
	const uint32_t l_interfacedObjectId = pickInterfacedObject(int(event->x), int(event->y));
	CInterfacedObject& l_rObject        = m_vInterfacedObject[l_interfacedObjectId];
	if (l_rObject.m_oIdentifier != OV_UndefinedIdentifier
		&& l_rObject.m_connectorType != Box_Link
		&& l_rObject.m_connectorType != Box_None)
	{
		IBox* l_pBoxDetails = m_rScenario.getBoxDetails(l_rObject.m_oIdentifier);
		if (l_pBoxDetails)
		{
			CString l_sName;
			CString l_sType;
			if (l_rObject.m_connectorType == Box_Input)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_connectorIndex, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_connectorIndex, l_oType);
				l_sType = m_kernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_connectorType == Box_Output)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_connectorIndex, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_connectorIndex, l_oType);
				l_sType = m_kernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_connectorType == Box_Update)
			{
				//m_rScenario.updateBox(l_pBoxDetails->getIdentifier());
				l_sName = CString("Right click for");
				l_sType = "box update";
			}
			else if (l_rObject.m_connectorType == Box_ScenarioInput)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_connectorIndex, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_connectorIndex, l_oType);

				for (uint32_t l_scenarioInputIndex = 0; l_scenarioInputIndex < m_rScenario.getInputCount(); l_scenarioInputIndex++)
				{
					CIdentifier scenarioInputLinkBoxIdentifier;
					uint32_t l_scenarioInputLinkBoxInputIndex;

					m_rScenario.getScenarioInputLink(l_scenarioInputIndex, scenarioInputLinkBoxIdentifier, l_scenarioInputLinkBoxInputIndex);

					if (scenarioInputLinkBoxIdentifier == l_pBoxDetails->getIdentifier() && l_scenarioInputLinkBoxInputIndex == l_rObject.m_connectorIndex)
					{
						m_rScenario.getInputName(l_scenarioInputIndex, l_sName);
						l_sName = CString("Connected to \n") + l_sName;
						m_rScenario.getInputType(l_scenarioInputIndex, l_oType);
					}
				}
				l_sType = m_kernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_connectorType == Box_ScenarioOutput)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_connectorIndex, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_connectorIndex, l_oType);

				for (uint32_t l_scenarioOutputIndex = 0; l_scenarioOutputIndex < m_rScenario.getOutputCount(); l_scenarioOutputIndex++)
				{
					CIdentifier scenarioOutputLinkBoxIdentifier;
					uint32_t l_scenarioOutputLinkBoxOutputIndex;

					m_rScenario.getScenarioOutputLink(l_scenarioOutputIndex, scenarioOutputLinkBoxIdentifier, l_scenarioOutputLinkBoxOutputIndex);

					if (scenarioOutputLinkBoxIdentifier == l_pBoxDetails->getIdentifier() && l_scenarioOutputLinkBoxOutputIndex == l_rObject.m_connectorIndex)
					{
						m_rScenario.getOutputName(l_scenarioOutputIndex, l_sName);
						l_sName = CString("Connected to \n") + l_sName;
						m_rScenario.getOutputType(l_scenarioOutputIndex, l_oType);
					}
				}
				l_sType = m_kernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}

			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_pGUIBuilder, "tooltip-label_name_content")), l_sName);
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_pGUIBuilder, "tooltip-label_type_content")), l_sType);
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
			if (m_controlPressed) { m_SelectedObjects.insert(m_oCurrentObject.m_oIdentifier); }
			else
			{
				if (!m_SelectedObjects.count(m_oCurrentObject.m_oIdentifier))
				{
					m_SelectedObjects.clear();
					m_SelectedObjects.insert(m_oCurrentObject.m_oIdentifier);
				}
			}
			for (auto& objectId : m_SelectedObjects)
			{
				if (m_rScenario.isBox(objectId))
				{
					CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, objectId);
					l_oBoxProxy.setCenter(l_oBoxProxy.getXCenter() + int(event->x - m_currentMouseX),
										  l_oBoxProxy.getYCenter() + int(event->y - m_currentMouseY));
				}
				if (m_rScenario.isComment(objectId))
				{
					CCommentProxy l_oCommentProxy(m_kernelContext, m_rScenario, objectId);
					l_oCommentProxy.setCenter(l_oCommentProxy.getXCenter() + int(event->x - m_currentMouseX),
											  l_oCommentProxy.getYCenter() + int(event->y - m_currentMouseY));
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

GtkImageMenuItem* CInterfacedScenario::gtk_menu_add_new_image_menu_item_with_cb_generic(GtkMenu* menu, const char* icon, const char* label,
																						const menu_callback_function cb, IBox* box, const uint32_t command,
																						const uint32_t index, const uint32_t index2)
{
	GtkImageMenuItem* menuitem = gtk_menu_add_new_image_menu_item(menu, icon, label);
	BoxContextMenuCB l_oBoxContextMenuCB;
	l_oBoxContextMenuCB.command             = command;
	l_oBoxContextMenuCB.index               = index;
	l_oBoxContextMenuCB.secondaryIndex      = index2;
	l_oBoxContextMenuCB.pBox                = box;
	l_oBoxContextMenuCB.pInterfacedScenario = this;
	const auto mapIndex                     = uint32_t(m_vBoxContextMenuCB.size());
	m_vBoxContextMenuCB[mapIndex]           = l_oBoxContextMenuCB;
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(cb), &m_vBoxContextMenuCB[mapIndex]);
	return menuitem;
}

void CInterfacedScenario::scenarioDrawingAreaButtonPressedCB(GtkWidget* widget, GdkEventButton* event)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonPressedCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* l_pTooltip = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "tooltip"));
	gtk_widget_hide(l_pTooltip);
	gtk_widget_grab_focus(widget);

	m_buttonPressed |= ((event->type == GDK_BUTTON_PRESS) && (event->button == 1));
	m_pressMouseX = event->x;
	m_pressMouseY = event->y;

	uint32_t l_interfacedObjectId = pickInterfacedObject(int(m_pressMouseX), int(m_pressMouseY));
	m_oCurrentObject              = m_vInterfacedObject[l_interfacedObjectId];

	if (event->button == 1)
	{
		if (event->type == GDK_BUTTON_PRESS)
		{
			if (m_oCurrentObject.m_oIdentifier == OV_UndefinedIdentifier)
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
				if (m_oCurrentObject.m_connectorType == Box_Input || m_oCurrentObject.m_connectorType == Box_Output) { m_currentMode = Mode_Connect; }
				else
				{
					m_currentMode = Mode_MoveSelection;
					if (m_controlPressed)
					{
						// m_vCurrentObject[m_oCurrentObject.m_oIdentifier]=!m_vCurrentObject[m_oCurrentObject.m_oIdentifier];
					}
					else
					{
						// m_vCurrentObject.clear();
						// m_vCurrentObject[m_oCurrentObject.m_oIdentifier]=true;
					}
				}
			}
		}
		else if (event->type == GDK_2BUTTON_PRESS)
		{
			if (m_oCurrentObject.m_oIdentifier != OV_UndefinedIdentifier)
			{
				m_currentMode    = Mode_EditSettings;
				m_shiftPressed   = false;
				m_controlPressed = false;
				m_altPressed     = false;
				m_aPressed       = false;
				m_wPressed       = false;

				if (m_oCurrentObject.m_connectorType == Box_Input || m_oCurrentObject.m_connectorType == Box_Output)
				{
					IBox* l_pBox = m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
					if (l_pBox)
					{
						if ((m_oCurrentObject.m_connectorType == Box_Input && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
							|| (m_oCurrentObject.m_connectorType == Box_Output && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)))
						{
							CConnectorEditor l_oConnectorEditor(m_kernelContext, *l_pBox, m_oCurrentObject.m_connectorType, m_oCurrentObject.m_connectorIndex,
																m_oCurrentObject.m_connectorType == Box_Input ? "Edit Input" : "Edit Output",
																m_sGUIFilename.c_str());
							if (l_oConnectorEditor.run()) { this->snapshotCB(); }
						}
					}
				}
				else
				{
					if (m_rScenario.isBox(m_oCurrentObject.m_oIdentifier))
					{
						IBox* l_pBox = m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
						if (l_pBox)
						{
							CBoxConfigurationDialog l_oBoxConfigurationDialog(m_kernelContext, *l_pBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str(),
																			  false);
							if (l_oBoxConfigurationDialog.run()) { this->snapshotCB(); }
						}
					}
					if (m_rScenario.isComment(m_oCurrentObject.m_oIdentifier))
					{
						IComment* l_pComment = m_rScenario.getCommentDetails(m_oCurrentObject.m_oIdentifier);
						if (l_pComment)
						{
							CCommentEditorDialog l_oCommentEditorDialog(m_kernelContext, *l_pComment, m_sGUIFilename.c_str());
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
			const auto unused = uint32_t(-1);
			GtkMenu* l_pMenu  = GTK_MENU(gtk_menu_new());
			m_vBoxContextMenuCB.clear();

			// -------------- SELECTION -----------

			if (this->hasSelection())
			{
				gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_CUT, "cut", context_menu_cb, nullptr, ContextMenu_SelectionCut, unused);
			}
			if (this->hasSelection())
			{
				gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_COPY, "copy", context_menu_cb, nullptr, ContextMenu_SelectionCopy, unused);
			}
			if ((m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
				|| (m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier))
			{
				gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_PASTE, "paste", context_menu_cb, nullptr, ContextMenu_SelectionPaste, unused);
			}
			if (this->hasSelection())
			{
				gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_DELETE, "delete", context_menu_cb, nullptr, ContextMenu_SelectionDelete, unused);
			}

			if (m_oCurrentObject.m_oIdentifier != OV_UndefinedIdentifier && m_rScenario.isBox(m_oCurrentObject.m_oIdentifier))
			{
				IBox* l_pBox = m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
				if (l_pBox)
				{
					uint32_t i, j;
					char l_sCompleteName[1024];

					if (!m_vBoxContextMenuCB.empty()) { gtk_menu_add_separator_menu_item(l_pMenu); }

					bool l_bFlagToBeUpdated                  = l_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated);
					bool l_bFlagPendingDeprecatedInterfacors = l_pBox->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors);

					// -------------- INPUTS --------------
					bool l_bFlagCanAddInput         = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput);
					bool l_bFlagCanModifyInput      = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput);
					bool l_bCanConnectScenarioInput = (l_pBox->getInputCount() > 0 && m_rScenario.getInputCount() > 0);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (
							l_bFlagCanAddInput || l_bFlagCanModifyInput || l_bCanConnectScenarioInput))
					{
						uint32_t l_ui32FixedInputCount = 0;
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
								for (j = 0; j < m_rScenario.getInputCount(); j++)
								{
									char l_sScenarioInputNameComplete[1024];
									CString l_sScenarioInputName;
									CIdentifier scenarioLinkBoxIdentifier;
									CIdentifier scenarioInputType;
									auto l_scenarioLinkInputIndex = uint32_t(-1);
									m_rScenario.getInputName(j, l_sScenarioInputName);
									m_rScenario.getInputType(j, scenarioInputType);
									m_rScenario.getScenarioInputLink(j, scenarioLinkBoxIdentifier, l_scenarioLinkInputIndex);
									sprintf(l_sScenarioInputNameComplete, "%u: %s", j + 1, l_sScenarioInputName.toASCIIString());
									if (scenarioLinkBoxIdentifier == identifier && l_scenarioLinkInputIndex == i)
									{
										gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuInputMenuAction, GTK_STOCK_DISCONNECT,
																						 (CString("disconnect from ") + CString(l_sScenarioInputNameComplete)).
																						 toASCIIString(), context_menu_cb, l_pBox,
																						 ContextMenu_BoxDisconnectScenarioInput, i, j);
									}
									else
									{
										if (m_kernelContext.getTypeManager().isDerivedFromStream(scenarioInputType, l_oType))
										{
											gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuInputMenuAction, GTK_STOCK_CONNECT,
																							 (CString("connect to ") + CString(l_sScenarioInputNameComplete)).
																							 toASCIIString(), context_menu_cb, l_pBox,
																							 ContextMenu_BoxConnectScenarioInput, i, j);
										}
									}
								}
							}

							if (l_bFlagCanModifyInput)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuInputMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox,
																		 ContextMenu_BoxEditInput, i);
							}

							if (l_bFlagCanAddInput && l_ui32FixedInputCount <= i)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuInputMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox,
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
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenuInput, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddInput,
																	 unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemInput), GTK_WIDGET(l_pMenuInput));
					}

					// -------------- OUTPUTS --------------

					bool l_bFlagCanAddOutput         = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput);
					bool l_bFlagCanModifyOutput      = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput);
					bool l_bCanConnectScenarioOutput = (l_pBox->getOutputCount() > 0 && m_rScenario.getOutputCount() > 0);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (
							l_bFlagCanAddOutput || l_bFlagCanModifyOutput || l_bCanConnectScenarioOutput))
					{
						uint32_t l_ui32FixedOutputCount = 0;
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
								for (j = 0; j < m_rScenario.getOutputCount(); j++)
								{
									char l_sScenarioOutputNameComplete[1024];
									CString l_sScenarioOutputName;
									CIdentifier scenarioLinkBoxIdentifier;
									CIdentifier scenarioOutputType;
									auto l_scenarioLinkOutputIndex = uint32_t(-1);
									m_rScenario.getOutputName(j, l_sScenarioOutputName);
									m_rScenario.getOutputType(j, scenarioOutputType);
									m_rScenario.getScenarioOutputLink(j, scenarioLinkBoxIdentifier, l_scenarioLinkOutputIndex);
									sprintf(l_sScenarioOutputNameComplete, "%u: %s", j + 1, l_sScenarioOutputName.toASCIIString());
									if (scenarioLinkBoxIdentifier == identifier && l_scenarioLinkOutputIndex == i)
									{
										gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuOutputMenuAction, GTK_STOCK_DISCONNECT,
																						 (CString("disconnect from ") + CString(l_sScenarioOutputNameComplete)).
																						 toASCIIString(), context_menu_cb, l_pBox,
																						 ContextMenu_BoxDisconnectScenarioOutput, i, j);
									}
									else if (m_kernelContext.getTypeManager().isDerivedFromStream(l_oType, scenarioOutputType))
									{
										gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuOutputMenuAction, GTK_STOCK_CONNECT,
																						 (CString("connect to ") + CString(l_sScenarioOutputNameComplete)).
																						 toASCIIString(), context_menu_cb, l_pBox,
																						 ContextMenu_BoxConnectScenarioOutput, i, j);
									}
								}
							}

							if (l_bFlagCanModifyOutput)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuOutputMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox,
																		 ContextMenu_BoxEditOutput, i);
							}
							if (l_bFlagCanAddOutput && l_ui32FixedOutputCount <= i)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuOutputMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox,
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
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenuOutput, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddOutput,
																	 unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemOutput), GTK_WIDGET(l_pMenuOutput));
					}

					// -------------- SETTINGS --------------

					bool l_bFlagCanAddSetting    = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting);
					bool l_bFlagCanModifySetting = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (l_bFlagCanAddSetting || l_bFlagCanModifySetting))
					{
						uint32_t l_ui32FixedSettingCount = 0;
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
									gtk_menu_add_new_image_menu_item_with_cb(l_pMenuSettingMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox,
																			 ContextMenu_BoxEditSetting, i);
								}
								if (l_bFlagCanAddSetting && l_ui32FixedSettingCount <= i)
								{
									gtk_menu_add_new_image_menu_item_with_cb(l_pMenuSettingMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox,
																			 ContextMenu_BoxRemoveSetting, i);
								}
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuSettingMenuItem), GTK_WIDGET(l_pMenuSettingMenuAction));
							}
							else { gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuSettingMenuItem), false); }
						}
						gtk_menu_add_separator_menu_item(l_pMenuSetting);
						if (l_bFlagCanAddSetting)
						{
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenuSetting, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox,
																	 ContextMenu_BoxAddSetting, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemSetting), GTK_WIDGET(l_pMenuSetting));
					}

					// -------------- ABOUT / RENAME --------------

					if (!m_vBoxContextMenuCB.empty()) { gtk_menu_add_separator_menu_item(l_pMenu); }
					if (l_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated))
					{
						auto updateMenuItem = gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_REFRESH, "update box", context_menu_cb, l_pBox,
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
						gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_REFRESH, "remove deprecated I/O/S", context_menu_cb, l_pBox,
																 ContextMenu_BoxRemoveDeprecatedInterfacors, unused);
					}
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_EDIT, "rename box...", context_menu_cb, l_pBox, ContextMenu_BoxRename, unused);
					if (l_pBox->getSettingCount() != 0)
					{
						gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_PREFERENCES, "configure box...", context_menu_cb, l_pBox,
																 ContextMenu_BoxConfigure, unused);
					}
					// Add this option only if the user has the authorization to open a metabox
					if (l_pBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
					{
						CIdentifier metaboxId;
						metaboxId.fromString(l_pBox->getAttributeValue(OVP_AttributeId_Metabox_Identifier));

						std::string metaboxScenarioPathString(m_kernelContext.getMetaboxManager().getMetaboxFilePath(metaboxId).toASCIIString());
						std::string metaboxScenarioExtension = boost::filesystem::extension(metaboxScenarioPathString);
						bool canImportFile                   = false;

						CString fileNameExtension;
						while ((fileNameExtension = m_kernelContext
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
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_PREFERENCES, "open this meta box in editor", context_menu_cb, l_pBox,
																	 ContextMenu_BoxEditMetabox, unused);
						}
					}
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_CONNECT, "enable box", context_menu_cb, l_pBox, ContextMenu_BoxEnable, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_DISCONNECT, "disable box", context_menu_cb, l_pBox, ContextMenu_BoxDisable,
															 unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_CUT, "delete box", context_menu_cb, l_pBox, ContextMenu_BoxDelete, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_HELP, "box documentation...", context_menu_cb, l_pBox,
															 ContextMenu_BoxDocumentation, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_ABOUT, "about box...", context_menu_cb, l_pBox, ContextMenu_BoxAbout, unused);
				}
			}

			gtk_menu_add_separator_menu_item(l_pMenu);
			gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_EDIT, "add comment to scenario...", context_menu_cb, nullptr,
													 ContextMenu_ScenarioAddComment, unused);
			gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_ABOUT, "about scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAbout,
													 unused);

			// -------------- RUN --------------

			gtk_widget_show_all(GTK_WIDGET(l_pMenu));
			gtk_menu_popup(l_pMenu, nullptr, nullptr, nullptr, nullptr, 3, event->time);
			if (m_vBoxContextMenuCB.empty()) { gtk_menu_popdown(l_pMenu); }
		}
	}

	this->redraw();
}

void CInterfacedScenario::scenarioDrawingAreaButtonReleasedCB(GtkWidget* /*widget*/, GdkEventButton* event)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonReleasedCB\n";

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
			const uint32_t l_interfacedObjectId      = pickInterfacedObject(int(m_releaseMouseX), int(m_releaseMouseY));
			const CInterfacedObject l_oCurrentObject = m_vInterfacedObject[l_interfacedObjectId];
			CInterfacedObject l_oSourceObject;
			CInterfacedObject l_oTargetObject;
			if (l_oCurrentObject.m_connectorType == Box_Output && m_oCurrentObject.m_connectorType == Box_Input)
			{
				l_oSourceObject         = l_oCurrentObject;
				l_oTargetObject         = m_oCurrentObject;
				l_bIsActuallyConnecting = true;
			}
			if (l_oCurrentObject.m_connectorType == Box_Input && m_oCurrentObject.m_connectorType == Box_Output)
			{
				l_oSourceObject         = m_oCurrentObject;
				l_oTargetObject         = l_oCurrentObject;
				l_bIsActuallyConnecting = true;
			}
			//
			if (l_bIsActuallyConnecting)
			{
				CIdentifier l_oSourceTypeIdentifier;
				CIdentifier l_oTargetTypeIdentifier;
				const IBox* l_pSourceBox = m_rScenario.getBoxDetails(l_oSourceObject.m_oIdentifier);
				const IBox* l_pTargetBox = m_rScenario.getBoxDetails(l_oTargetObject.m_oIdentifier);
				if (l_pSourceBox && l_pTargetBox)
				{
					l_pSourceBox->getOutputType(l_oSourceObject.m_connectorIndex, l_oSourceTypeIdentifier);
					l_pTargetBox->getInputType(l_oTargetObject.m_connectorIndex, l_oTargetTypeIdentifier);

					bool hasDeprecatedInput = false;
					l_pSourceBox->getInterfacorDeprecatedStatus(Output, l_oSourceObject.m_connectorIndex, hasDeprecatedInput);
					bool hasDeprecatedOutput = false;
					l_pTargetBox->getInterfacorDeprecatedStatus(Input, l_oTargetObject.m_connectorIndex, hasDeprecatedOutput);

					if ((m_kernelContext.getTypeManager().isDerivedFromStream(l_oSourceTypeIdentifier, l_oTargetTypeIdentifier)
						 || m_kernelContext.getConfigurationManager().expandAsBoolean("${Designer_AllowUpCastConnection}", false)) && (!l_bConnectionIsMessage))
					{
						if (!hasDeprecatedInput && !hasDeprecatedOutput)
						{
							CIdentifier l_oLinkIdentifier;
							m_rScenario.connect(l_oLinkIdentifier, l_oSourceObject.m_oIdentifier, l_oSourceObject.m_connectorIndex,
												l_oTargetObject.m_oIdentifier, l_oTargetObject.m_connectorIndex, OV_UndefinedIdentifier);
							this->snapshotCB();
						}
						else { m_kernelContext.getLogManager() << LogLevel_Warning << "Cannot connect to/from deprecated I/O\n"; }
					}
					else { m_kernelContext.getLogManager() << LogLevel_Warning << "Invalid connection\n"; }
				}
			}
		}
		if (m_currentMode == Mode_MoveSelection)
		{
			if (l_iSizeX == 0 && l_iSizeY == 0)
			{
				if (m_controlPressed)
				{
					if (m_SelectedObjects.count(m_oCurrentObject.m_oIdentifier)) { m_SelectedObjects.erase(m_oCurrentObject.m_oIdentifier); }
					else { m_SelectedObjects.insert(m_oCurrentObject.m_oIdentifier); }
				}
				else
				{
					m_SelectedObjects.clear();
					m_SelectedObjects.insert(m_oCurrentObject.m_oIdentifier);
				}
			}
			else
			{
				for (auto& objectId : m_SelectedObjects)
				{
					if (m_rScenario.isBox(objectId))
					{
						CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, objectId);
						l_oBoxProxy.setCenter(((l_oBoxProxy.getXCenter() + 8) & 0xfffffff0), ((l_oBoxProxy.getYCenter() + 8) & 0xfffffff0));
					}
					if (m_rScenario.isComment(objectId))
					{
						CCommentProxy l_oCommentProxy(m_kernelContext, m_rScenario, objectId);
						l_oCommentProxy.setCenter(((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0), ((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0));
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

	// m_kernelContext.getLogManager() << LogLevel_Info << "Key pressed " << (uint32_t) event->keyval << "\n";
	/*
		if((event->keyval==GDK_Z || event->keyval==GDK_z) && m_controlPressed) { this->undoCB(); }

		if((event->keyval==GDK_Y || event->keyval==GDK_y) && m_controlPressed) { this->redoCB(); }
	*/
	// CTRL+A = select all
	if (m_aPressed && m_controlPressed && !m_shiftPressed && !m_altPressed)
	{
		CIdentifier identifier;
		while ((identifier = m_rScenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(identifier); }
		while ((identifier = m_rScenario.getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(identifier); }
		while ((identifier = m_rScenario.getNextCommentIdentifier(identifier)) != OV_UndefinedIdentifier) { m_SelectedObjects.insert(identifier); }
		this->redraw();
	}

	//CTRL+W : close current scenario
	if (m_wPressed && m_controlPressed && !m_shiftPressed && !m_altPressed)
	{
		m_rApplication.closeScenarioCB(this);
		return;
	}

	if ((event->keyval == GDK_C || event->keyval == GDK_c) && m_currentMode == Mode_None)
	{
		gint iX = 0;
		gint iY = 0;
		gdk_window_get_pointer(GTK_WIDGET(m_pScenarioDrawingArea)->window, &iX, &iY, nullptr);

		this->addCommentCB(iX, iY);
	}

	if (event->keyval == GDK_F12 && m_shiftPressed)
	{
		CIdentifier identifier;
		while ((identifier = m_rScenario.getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			IBox* l_pBox                       = m_rScenario.getBoxDetails(identifier);
			CIdentifier l_oAlgorithmIdentifier = l_pBox->getAlgorithmClassIdentifier();
			CIdentifier l_oHashValue           = m_kernelContext.getPluginManager().getPluginObjectHashValue(l_oAlgorithmIdentifier);
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
			if (m_rScenario.isBox(objectId))
			{
				browseBoxDocumentation(objectId);
				hasDoc = true;
			}
		}

		if (!hasDoc)
		{
			const CString fullUrl = m_rScenario.getAttributeValue(OV_AttributeId_Scenario_DocumentationPage);
			if (fullUrl != CString(""))
			{
				browseURL(fullUrl, m_kernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommand}"),
						  m_kernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommandPostfix}"));
			}
			else { m_kernelContext.getLogManager() << LogLevel_Info << "The scenario does not define a documentation page.\n"; }
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
		if (m_rApplication.getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Play) { m_rApplication.pauseScenarioCB(); }
		else { m_rApplication.playScenarioCB(); }
	}
	// F6 : step
	if (event->keyval == GDK_F6) { m_rApplication.nextScenarioCB(); }
	// F8 :fastforward
	if (event->keyval == GDK_F8) { m_rApplication.forwardScenarioCB(); }
	// F5 : stop
	if (event->keyval == GDK_F5) { m_rApplication.stopScenarioCB(); }

	m_kernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaKeyPressEventCB (" << (m_shiftPressed ? "true" : "false") << "|"
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

	m_kernelContext.getLogManager() << LogLevel_Debug
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
	m_kernelContext.getLogManager() << LogLevel_Debug << "copySelection\n";

	// Prepares copy
	map<CIdentifier, CIdentifier> l_vIdMapping;
	m_rApplication.m_pClipboardScenario->clear();

	// Copies boxes to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isBox(objectId))
		{
			CIdentifier l_oNewIdentifier;
			const IBox* l_pBox = m_rScenario.getBoxDetails(objectId);
			m_rApplication.m_pClipboardScenario->addBox(l_oNewIdentifier, *l_pBox, objectId);
			l_vIdMapping[objectId] = l_oNewIdentifier;
		}
	}

	// Copies comments to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isComment(objectId))
		{
			CIdentifier l_oNewIdentifier;
			const IComment* l_pComment = m_rScenario.getCommentDetails(objectId);
			m_rApplication.m_pClipboardScenario->addComment(l_oNewIdentifier, *l_pComment, objectId);
			l_vIdMapping[objectId] = l_oNewIdentifier;
		}
	}

	// Copies links to clipboard
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isLink(objectId))
		{
			CIdentifier l_oNewIdentifier;
			const ILink* l_pLink = m_rScenario.getLinkDetails(objectId);

			// Connect link only if the source and target boxes are copied
			if (l_vIdMapping.find(l_pLink->getSourceBoxIdentifier()) != l_vIdMapping.end()
				&& l_vIdMapping.find(l_pLink->getTargetBoxIdentifier()) != l_vIdMapping.end())
			{
				m_rApplication.m_pClipboardScenario->connect(l_oNewIdentifier, l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
															 l_pLink->getSourceBoxOutputIndex(), l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
															 l_pLink->getTargetBoxInputIndex(), l_pLink->getIdentifier());
			}
		}
	}
}

void CInterfacedScenario::cutSelection()

{
	m_kernelContext.getLogManager() << LogLevel_Debug << "cutSelection\n";

	this->copySelection();
	this->deleteSelection();
}

void CInterfacedScenario::pasteSelection()

{
	m_kernelContext.getLogManager() << LogLevel_Debug << "pasteSelection\n";

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
	while ((identifier = m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		IBox* l_pBox = m_rApplication.m_pClipboardScenario->getBoxDetails(identifier);
		m_rScenario.addBox(l_oNewIdentifier, *l_pBox, identifier);
		l_vIdMapping[identifier] = l_oNewIdentifier;

		// Updates visualization manager
		CIdentifier l_oBoxAlgorithmIdentifier = l_pBox->getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD       = m_kernelContext.getPluginManager().getPluginObjectDescCreating(l_oBoxAlgorithmIdentifier);

		// If a visualization box was dropped, add it in window manager
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_pDesignerVisualization) { m_pDesignerVisualization->onVisualizationBoxAdded(m_rScenario.getBoxDetails(l_oNewIdentifier)); }
		}

		CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, l_oNewIdentifier);

		// get the position of the topmost-leftmost box (always position on an actual box so when user pastes he sees something)
		if (l_oBoxProxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterX && l_oBoxProxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterY)
		{
			l_iTopmostLeftmostCopiedBoxCenterX = l_oBoxProxy.getXCenter();
			l_iTopmostLeftmostCopiedBoxCenterY = l_oBoxProxy.getYCenter();
		}
	}

	// Pastes comments from clipboard
	while ((identifier = m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		IComment* l_pComment = m_rApplication.m_pClipboardScenario->getCommentDetails(identifier);
		m_rScenario.addComment(l_oNewIdentifier, *l_pComment, identifier);
		l_vIdMapping[identifier] = l_oNewIdentifier;

		CCommentProxy l_oCommentProxy(m_kernelContext, m_rScenario, l_oNewIdentifier);

		if (l_oCommentProxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterX && l_oCommentProxy.getYCenter() < l_iTopmostLeftmostCopiedBoxCenterY)
		{
			l_iTopmostLeftmostCopiedBoxCenterX = l_oCommentProxy.getXCenter();
			l_iTopmostLeftmostCopiedBoxCenterY = l_oCommentProxy.getYCenter();
		}
	}

	// Pastes links from clipboard
	while ((identifier = m_rApplication.m_pClipboardScenario->getNextLinkIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		ILink* l_pLink = m_rApplication.m_pClipboardScenario->getLinkDetails(identifier);
		m_rScenario.connect(l_oNewIdentifier, l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
							l_pLink->getSourceBoxOutputIndex(), l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
							l_pLink->getTargetBoxInputIndex(), l_pLink->getIdentifier());
	}

	// Makes pasted stuff the default selection
	// Moves boxes under cursor
	// Moves comments under cursor
	if (m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier || m_rApplication
																													   .m_pClipboardScenario->
																													   getNextCommentIdentifier(
																														   OV_UndefinedIdentifier) !=
		OV_UndefinedIdentifier)
	{
		m_SelectedObjects.clear();
		for (auto& it : l_vIdMapping)
		{
			m_SelectedObjects.insert(it.second);

			if (m_rScenario.isBox(it.second))
			{
				// Moves boxes under cursor
				CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, it.second);
				l_oBoxProxy.setCenter(int(l_oBoxProxy.getXCenter() + m_currentMouseX) - l_iTopmostLeftmostCopiedBoxCenterX - m_viewOffsetX,
									  int(l_oBoxProxy.getYCenter() + m_currentMouseY) - l_iTopmostLeftmostCopiedBoxCenterY - m_viewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns boxes on grid
				l_oBoxProxy.setCenter(int((l_oBoxProxy.getXCenter() + 8) & 0xfffffff0L), int((l_oBoxProxy.getYCenter() + 8) & 0xfffffff0L));
			}

			if (m_rScenario.isComment(it.second))
			{
				// Moves commentes under cursor
				CCommentProxy l_oCommentProxy(m_kernelContext, m_rScenario, it.second);
				l_oCommentProxy.setCenter(int(l_oCommentProxy.getXCenter() + m_currentMouseX) - l_iTopmostLeftmostCopiedBoxCenterX - m_viewOffsetX,
										  int(l_oCommentProxy.getYCenter() + m_currentMouseY) - l_iTopmostLeftmostCopiedBoxCenterY - m_viewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns commentes on grid
				l_oCommentProxy.setCenter(int((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0L), int((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0L));
			}
		}
	}

	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::deleteSelection()

{
	m_kernelContext.getLogManager() << LogLevel_Debug << "deleteSelection\n";
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isBox(objectId)) { this->deleteBox(objectId); }
		if (m_rScenario.isComment(objectId))
		{
			// removes comment from scenario
			m_rScenario.removeComment(objectId);
		}
		if (m_rScenario.isLink(objectId))
		{
			// removes link from scenario
			m_rScenario.disconnect(objectId);
		}
	}
	m_SelectedObjects.clear();

	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::deleteBox(const CIdentifier& boxID)
{
	// removes visualization box from window manager
	if (m_pDesignerVisualization) { m_pDesignerVisualization->onVisualizationBoxRemoved(boxID); }

	// removes box from scenario
	m_rScenario.removeBox(boxID);
}


void CInterfacedScenario::contextMenuBoxUpdateCB(IBox& box)
{
	m_rScenario.updateBox(box.getIdentifier());
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxUpdateCB\n";
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRemoveDeprecatedInterfacorsCB(IBox& box)
{
	m_rScenario.removeDeprecatedInterfacorsFromBox(box.getIdentifier());
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveDeprecatedInterfacorsCB\n";
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRenameCB(IBox& box)
{
	const IPluginObjectDesc* l_pPluginObjectDescriptor = m_kernelContext.getPluginManager().getPluginObjectDescCreating(box.getAlgorithmClassIdentifier());
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRenameCB\n";

	if (box.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		l_pPluginObjectDescriptor = m_kernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
	}

	CRenameDialog l_oRename(m_kernelContext, box.getName(), l_pPluginObjectDescriptor ? l_pPluginObjectDescriptor->getName() : box.getName(),
							m_sGUIFilename.c_str());
	if (l_oRename.run())
	{
		box.setName(l_oRename.getResult());

		//check whether it is a visualization box
		const CIdentifier id            = box.getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_kernelContext.getPluginManager().getPluginObjectDescCreating(id);

		//if a visualization box was renamed, tell window manager about it
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			if (m_pDesignerVisualization) { m_pDesignerVisualization->onVisualizationBoxRenamed(box.getIdentifier()); }
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
		if (m_rScenario.isBox(objectId)) { l_vSelectedBox[objectId] = m_rScenario.getBoxDetails(objectId)->getAlgorithmClassIdentifier(); }
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
				if (m_kernelContext.getPluginManager().canCreatePluginObject(it->second) || it->second == OVP_ClassId_BoxAlgorithm_Metabox)
				{
					IBox* l_pBox = m_rScenario.getBoxDetails(it->first);
					if (l_bFirstBox)
					{
						l_bFirstBox                                        = false;
						const IPluginObjectDesc* l_pPluginObjectDescriptor = m_kernelContext.getPluginManager().getPluginObjectDescCreating(
							l_pBox->getAlgorithmClassIdentifier());
						if (l_pBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
						{
							CIdentifier metaboxId;
							metaboxId.fromString(l_pBox->getAttributeValue(OVP_AttributeId_Metabox_Identifier));
							l_pPluginObjectDescriptor = m_kernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
						}

						CRenameDialog l_oRename(m_kernelContext, l_pBox->getName(),
												l_pPluginObjectDescriptor ? l_pPluginObjectDescriptor->getName() : l_pBox->getName(), m_sGUIFilename.c_str());
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
						const IPluginObjectDesc* l_pPOD = m_kernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);

						//if a visualization box was renamed, tell window manager about it
						if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
						{
							if (m_pDesignerVisualization) { m_pDesignerVisualization->onVisualizationBoxRenamed(l_pBox->getIdentifier()); }
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
		if (m_rScenario.isBox(objectId))
		{
			TAttributeHandler l_oAttributeHandler(*m_rScenario.getBoxDetails(objectId));
			if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled)) { l_oAttributeHandler.removeAttribute(OV_AttributeId_Box_Disabled); }
			else { l_oAttributeHandler.addAttribute(OV_AttributeId_Box_Disabled, 1); }
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxEnableAllCB()

{
	//we find all selected boxes
	for (const auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isBox(objectId))
		{
			TAttributeHandler l_oAttributeHandler(*m_rScenario.getBoxDetails(objectId));
			if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled)) { l_oAttributeHandler.removeAttribute(OV_AttributeId_Box_Disabled); }
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxDisableAllCB()

{
	//we find all selected boxes
	for (const auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isBox(objectId))
		{
			TAttributeHandler l_oAttributeHandler(*m_rScenario.getBoxDetails(objectId));
			if (!l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled)) { l_oAttributeHandler.addAttribute(OV_AttributeId_Box_Disabled, 1); }
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddInputCB(IBox& box)
{
	if (box.hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors))
	{
		gtk_dialog_run(GTK_DIALOG(m_pErrorPendingDeprecatedInterfacorsDialog));
		return;
	}
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddInputCB\n";
	box.addInput("New input", OV_TypeId_EBMLStream, m_rScenario.getUnusedInputIdentifier());
	if (box.hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
	{
		CConnectorEditor l_oConnectorEditor(m_kernelContext, box, Box_Input, box.getInputCount() - 1, "Add Input", m_sGUIFilename.c_str());
		if (l_oConnectorEditor.run()) { this->snapshotCB(); }
		else { box.removeInput(box.getInputCount() - 1); }
	}
	else { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxEditInputCB(IBox& box, const uint32_t index)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditInputCB\n";

	CConnectorEditor l_oConnectorEditor(m_kernelContext, box, Box_Input, index, "Edit Input", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveInputCB(IBox& box, const uint32_t index)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveInputCB\n";
	box.removeInput(index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddOutputCB(IBox& box)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddOutputCB\n";
	box.addOutput("New output", OV_TypeId_EBMLStream, m_rScenario.getUnusedOutputIdentifier());
	if (box.hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput))
	{
		CConnectorEditor l_oConnectorEditor(m_kernelContext, box, Box_Output, box.getOutputCount() - 1, "Add Output", m_sGUIFilename.c_str());
		if (l_oConnectorEditor.run()) { this->snapshotCB(); }
		else { box.removeOutput(box.getOutputCount() - 1); }
	}
	else { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxEditOutputCB(IBox& box, const uint32_t index)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditOutputCB\n";

	CConnectorEditor l_oConnectorEditor(m_kernelContext, box, Box_Output, index, "Edit Output", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveOutputCB(IBox& box, const uint32_t index)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveOutputCB\n";
	box.removeOutput(index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioInputCB(IBox& box, const uint32_t boxInputIdx, const uint32_t scenarioInputIdx)
{
	//	m_kernelContext.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioInputCB : box = " << box.getIdentifier().toString() << " box input = " << boxInputIdx << " , scenario input = " << scenarioInputIdx << "\n";
	m_rScenario.setScenarioInputLink(scenarioInputIdx, box.getIdentifier(), boxInputIdx);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioOutputCB(IBox& box, const uint32_t boxOutputIdx, const uint32_t scenarioOutputIdx)
{
	//	m_kernelContext.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioOutputCB : box = " << box.getIdentifier().toString() << " box Output = " << boxOutputIdx << " , scenario Output = " << scenarioOutputIdx << "\n";
	m_rScenario.setScenarioOutputLink(scenarioOutputIdx, box.getIdentifier(), boxOutputIdx);
	this->snapshotCB();
}

// Note: In current implementation only the scenarioInputIdx is necessary as it can only be connected to one input
// but to keep things simpler we give it all the info
void CInterfacedScenario::contextMenuBoxDisconnectScenarioInputCB(IBox& box, const uint32_t boxInputIdx, const uint32_t scenarioInputIdx)
{
	m_rScenario.removeScenarioInputLink(scenarioInputIdx, box.getIdentifier(), boxInputIdx);
	this->snapshotCB();
}

// Note: In current implementation only the scenarioOutputIdx is necessary as it can only be connected to one output
// but to keep things simpler we give it all the info
void CInterfacedScenario::contextMenuBoxDisconnectScenarioOutputCB(IBox& box, const uint32_t boxOutputIdx, const uint32_t scenarioOutputIdx)
{
	m_rScenario.removeScenarioOutputLink(scenarioOutputIdx, box.getIdentifier(), boxOutputIdx);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddSettingCB(IBox& box)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddSettingCB\n";
	// Store setting count in case the custom "onSettingAdded" of the box adds more than one setting
	const uint32_t oldSettingsCount = box.getSettingCount();
	box.addSetting("New setting", OV_UndefinedIdentifier, "", OV_Value_UndefinedIndexUInt, false,
				   m_rScenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));
	const uint32_t newSettingsCount = box.getSettingCount();
	// Check that at least one setting was added
	if (newSettingsCount > oldSettingsCount && box.hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
	{
		CSettingEditorDialog l_oSettingEditorDialog(m_kernelContext, box, oldSettingsCount, "Add Setting", m_sGUIFilename.c_str(),
													m_sGUISettingsFilename.c_str());
		if (l_oSettingEditorDialog.run()) { this->snapshotCB(); }
		else { for (uint32_t i = oldSettingsCount; i < newSettingsCount; ++i) { box.removeSetting(i); } }
	}
	else
	{
		if (newSettingsCount > oldSettingsCount) { this->snapshotCB(); }
		else
		{
			m_kernelContext.getLogManager() << LogLevel_Error << "No setting could be added to the box.\n";
			return;
		}
	}
	// Add an information message to inform the user about the new settings
	m_kernelContext.getLogManager() << LogLevel_Info << "[" << newSettingsCount - oldSettingsCount << "] new setting(s) was(were) added to the box ["
			<< box.getName().toASCIIString() << "]: ";
	for (uint32_t i = oldSettingsCount; i < newSettingsCount; ++i)
	{
		CString l_sSettingName;
		box.getSettingName(i, l_sSettingName);
		m_kernelContext.getLogManager() << "[" << l_sSettingName << "] ";
	}
	m_kernelContext.getLogManager() << "\n";
	// After adding setting, open configuration so that the user can see the effects.
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_kernelContext, box, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	l_oBoxConfigurationDialog.run();
}

void CInterfacedScenario::contextMenuBoxEditSettingCB(IBox& box, const uint32_t index)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditSettingCB\n";
	CSettingEditorDialog l_oSettingEditorDialog(m_kernelContext, box, index, "Edit Setting", m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	if (l_oSettingEditorDialog.run()) { this->snapshotCB(); }
}

void CInterfacedScenario::contextMenuBoxRemoveSettingCB(IBox& box, const uint32_t index)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveSettingCB\n";
	const uint32_t oldSettingsCount = box.getSettingCount();
	if (box.removeSetting(index))
	{
		const uint32_t newSettingsCount = box.getSettingCount();
		this->snapshotCB();
		m_kernelContext.getLogManager() << LogLevel_Info << "[" << oldSettingsCount - newSettingsCount << "] setting(s) was(were) removed from box ["
				<< box.getName().toASCIIString() << "] \n";
	}
	else
	{
		m_kernelContext.getLogManager() << LogLevel_Error << "The setting with index [" << index << "] could not be removed from box ["
				<< box.getName().toASCIIString() << "] \n";
	}
}

void CInterfacedScenario::contextMenuBoxConfigureCB(IBox& box)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxConfigureCB\n";
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_kernelContext, box, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	l_oBoxConfigurationDialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAboutCB(IBox& box) const
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAboutCB\n";
	if (box.getAlgorithmClassIdentifier() != OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CAboutPluginDialog l_oAboutPluginDialog(m_kernelContext, box.getAlgorithmClassIdentifier(), m_sGUIFilename.c_str());
		l_oAboutPluginDialog.run();
	}
	else
	{
		CIdentifier metaboxId;
		metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		CAboutPluginDialog l_oAboutPluginDialog(m_kernelContext, m_kernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId), m_sGUIFilename.c_str());
		l_oAboutPluginDialog.run();
	}
}

void CInterfacedScenario::contextMenuBoxEditMetaboxCB(IBox& box) const
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditMetaboxCB\n";

	CIdentifier metaboxId;
	metaboxId.fromString(box.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
	const CString metaboxScenarioPath(m_kernelContext.getMetaboxManager().getMetaboxFilePath(metaboxId));

	m_rApplication.openScenario(metaboxScenarioPath.toASCIIString());
}

bool CInterfacedScenario::browseURL(const CString& url, const CString& browserPrefix, const CString& browserPostfix) const
{
	m_kernelContext.getLogManager() << LogLevel_Trace << "Requesting web browser on URL " << url << "\n";

	const CString command = browserPrefix + CString(" \"") + url + CString("\"") + browserPostfix;
	m_kernelContext.getLogManager() << LogLevel_Debug << "Launching [" << command << "]\n";
	const int result = system(command.toASCIIString());
	if (result < 0)
	{
		OV_WARNING("Could not launch command [" << command << "]\n", m_kernelContext.getLogManager());
		return false;
	}
	return true;
}

bool CInterfacedScenario::browseBoxDocumentation(const CIdentifier& oBoxId) const
{
	const CIdentifier algorithmClassIdentifier = m_rScenario.getBoxDetails(oBoxId)->getAlgorithmClassIdentifier();

	// Do not show documentation for non-metaboxes or boxes that can not be created
	if (!(oBoxId != OV_UndefinedIdentifier && (m_kernelContext.getPluginManager().canCreatePluginObject(algorithmClassIdentifier) ||
											   m_rScenario.getBoxDetails(oBoxId)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)))
	{
		m_kernelContext.getLogManager() << LogLevel_Warning << "Box with id " << oBoxId << " can not create a pluging object\n";
		return false;
	}

	const CString defaultURLBase = m_kernelContext.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}");
	CString URLBase              = defaultURLBase;
	CString browser              = m_kernelContext.getConfigurationManager().expand("${Designer_HelpBrowserCommand}");
	CString browserPostfix       = m_kernelContext.getConfigurationManager().expand("${Designer_HelpBrowserCommandPostfix}");
	CString boxName;

	CString l_sHTMLName = "Doc_BoxAlgorithm_";
	if (m_rScenario.getBoxDetails(oBoxId)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		boxName = m_kernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId)->getName();
	}
	else
	{
		const IPluginObjectDesc* l_pPluginObjectDesc = m_kernelContext.getPluginManager().getPluginObjectDescCreating(algorithmClassIdentifier);
		boxName                                      = l_pPluginObjectDesc->getName();
	}
	// The documentation files do not have spaces in their name, so we remove them
	l_sHTMLName = l_sHTMLName + CString(getBoxAlgorithmURL(boxName.toASCIIString()).c_str());


	if (m_rScenario.getBoxDetails(oBoxId)->hasAttribute(OV_AttributeId_Box_DocumentationURLBase))
	{
		URLBase = m_kernelContext.getConfigurationManager().expand(
			m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OV_AttributeId_Box_DocumentationURLBase));
	}
	l_sHTMLName = l_sHTMLName + ".html";

	if (m_rScenario.getBoxDetails(oBoxId)->hasAttribute(OV_AttributeId_Box_DocumentationCommand))
	{
		browser = m_kernelContext.getConfigurationManager().expand(
			m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OV_AttributeId_Box_DocumentationCommand));
		browserPostfix = "";
	}

	CString fullUrl = URLBase + CString("/") + l_sHTMLName;
	if (m_rScenario.getBoxDetails(oBoxId)->hasAttribute(OV_AttributeId_Box_DocumentationURL))
	{
		fullUrl = m_kernelContext.getConfigurationManager().expand(m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OV_AttributeId_Box_DocumentationURL));
	}

	return browseURL(fullUrl, browser, browserPostfix);
}

void CInterfacedScenario::contextMenuBoxDocumentationCB(IBox& box) const
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxDocumentationCB\n";
	const CIdentifier l_oBoxId = box.getIdentifier();
	browseBoxDocumentation(l_oBoxId);
}

void CInterfacedScenario::contextMenuBoxEnableCB(IBox& box)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEnableCB\n";
	TAttributeHandler l_oAttributeHandler(box);
	l_oAttributeHandler.removeAttribute(OV_AttributeId_Box_Disabled);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxDisableCB(IBox& box)
{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxDisableCB\n";
	TAttributeHandler l_oAttributeHandler(box);
	if (!l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled)) { l_oAttributeHandler.addAttribute(OV_AttributeId_Box_Disabled, 1); }
	else { l_oAttributeHandler.setAttributeValue(OV_AttributeId_Box_Disabled, 1); }
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuScenarioAddCommentCB()

{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuScenarioAddCommentCB\n";
	this->addCommentCB();
}

void CInterfacedScenario::contextMenuScenarioAboutCB()

{
	m_kernelContext.getLogManager() << LogLevel_Debug << "contextMenuScenarioAboutCB\n";
	CAboutScenarioDialog l_oAboutScenarioDialog(m_kernelContext, m_rScenario, m_sGUIFilename.c_str());
	l_oAboutScenarioDialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::toggleDesignerVisualization()
{
	m_designerVisualizationToggled = !m_designerVisualizationToggled;

	if (m_pDesignerVisualization)
	{
		if (m_designerVisualizationToggled) { m_pDesignerVisualization->show(); }
		else { m_pDesignerVisualization->hide(); }
	}
}

void CInterfacedScenario::showCurrentVisualization() const
{
	if (isLocked()) { if (m_pPlayerVisualization != nullptr) { m_pPlayerVisualization->showTopLevelWindows(); } }
	else { if (m_pDesignerVisualization != nullptr) { m_pDesignerVisualization->show(); } }
}

void CInterfacedScenario::hideCurrentVisualization() const
{
	if (isLocked()) { if (m_pPlayerVisualization != nullptr) { m_pPlayerVisualization->hideTopLevelWindows(); } }
	else { if (m_pDesignerVisualization != nullptr) { m_pDesignerVisualization->hide(); } }
}

void CInterfacedScenario::createPlayerVisualization(IVisualizationTree* pVisualizationTree)
{
	//hide window manager
	if (m_pDesignerVisualization) { m_pDesignerVisualization->hide(); }

	if (m_pPlayerVisualization == nullptr)
	{
		if (pVisualizationTree) { m_pPlayerVisualization = new CPlayerVisualization(m_kernelContext, *pVisualizationTree, *this); }
		else { m_pPlayerVisualization = new CPlayerVisualization(m_kernelContext, *m_pVisualizationTree, *this); }


		//we go here when we press start
		//we have to set the modUI here
		//first, find the concerned boxes
		IScenario& runtimeScenario = m_pPlayer->getRuntimeScenarioManager().getScenario(m_pPlayer->getRuntimeScenarioIdentifier());
		CIdentifier objectId;
		while ((objectId = runtimeScenario.getNextBoxIdentifier(objectId)) != OV_UndefinedIdentifier)
		{
			IBox* l_oBox = runtimeScenario.getBoxDetails(objectId);
			if (l_oBox->hasModifiableSettings())//if the box has modUI
			{
				//create a BoxConfigurationDialog in mode true
				auto* l_oBoxConfigurationDialog = new CBoxConfigurationDialog(m_kernelContext, *l_oBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str(),
																			  true);
				//store it
				m_vBoxConfigurationDialog.push_back(l_oBoxConfigurationDialog);
			}
		}
	}

	//initialize and show windows
	m_pPlayerVisualization->init();
}

void CInterfacedScenario::releasePlayerVisualization()

{
	if (m_pPlayerVisualization != nullptr)
	{
		delete m_pPlayerVisualization;
		m_pPlayerVisualization = nullptr;
	}

	//reload designer visualization
	if (m_pDesignerVisualization)
	{
		m_pDesignerVisualization->load();
		//show it if it was toggled on
		if (m_designerVisualizationToggled) { m_pDesignerVisualization->show(); }
	}
}

void CInterfacedScenario::stopAndReleasePlayer()
{
	m_kernelContext.getErrorManager().releaseErrors();
	m_pPlayer->stop();
	m_ePlayerStatus = m_pPlayer->getStatus();
	// removes idle function
	g_idle_remove_by_data(this);

	if (!m_pPlayer->uninitialize()) { m_kernelContext.getLogManager() << LogLevel_Error << "Failed to uninitialize the player" << "\n"; }

	for (auto elem : m_vBoxConfigurationDialog)
	{
		elem->restoreState();
		delete elem;
	}
	m_vBoxConfigurationDialog.clear();


	if (!m_kernelContext.getPlayerManager().releasePlayer(m_oPlayerIdentifier))
	{
		m_kernelContext.getLogManager() << LogLevel_Error << "Failed to release the player" << "\n";
	}

	m_oPlayerIdentifier = OV_UndefinedIdentifier;
	m_pPlayer           = nullptr;

	// destroy player windows
	releasePlayerVisualization();

	// redraws scenario
	redraw();
}

//give the PlayerVisualisation the matching between the GtkWidget created by the CBoxConfigurationDialog and the Box CIdentifier
bool CInterfacedScenario::setModifiableSettingsWidgets()

{
	for (auto& elem : m_vBoxConfigurationDialog) { m_pPlayerVisualization->setWidget(elem->getBoxID(), elem->getWidget()); }

	return true;
}

bool CInterfacedScenario::centerOnBox(const CIdentifier& identifier)
{
	//m_kernelContext.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox" << "\n";
	bool ret_val = false;
	if (m_rScenario.isBox(identifier))
	{
		//m_kernelContext.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox is box" << "\n";
		IBox* box = m_rScenario.getBoxDetails(identifier);

		//clear previous selection
		m_SelectedObjects.clear();
		//to select the box

		m_SelectedObjects.insert(identifier);
		//		m_bScenarioModified=true;
		redraw();

		//CBoxProxy l_oBoxProxy(m_kernelContext, *box);
		const CBoxProxy l_oBoxProxy(m_kernelContext, m_rScenario, box->getIdentifier());
		const double marginX = 5.0 * m_currentScale;
		const double merginY = 5.0 * m_currentScale;
		const int sizeX      = int(round(l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) + marginX * 2.0));
		const int sizeY      = int(round(l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) + merginY * 2.0));
		const double centerX = l_oBoxProxy.getXCenter() * m_currentScale;
		const double centerY = l_oBoxProxy.getYCenter() * m_currentScale;
		int x, y;

		//get the parameters of the current adjustement
		GtkAdjustment* l_pOldHAdjustement =
				gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent));//gtk_viewport_get_vadjustment(m_pScenarioViewport);
		GtkAdjustment* l_pOldVAdjustement = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent));
		gdouble upper, lower, step, page, pagesize, value;

		g_object_get(l_pOldHAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value",
					 &value, nullptr);
		//create a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		auto* l_pAdjustement = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (centerX + m_viewOffsetX < upper / 2) { x = int(round(centerX - 2 * sizeX)) + m_viewOffsetX; }
		else { x = int(round(centerX + 2 * sizeX - pagesize)) + m_viewOffsetX; }
		gtk_adjustment_set_value(l_pAdjustement, x);
		gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent), l_pAdjustement);

		g_object_get(l_pOldVAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value",
					 &value, nullptr);
		l_pAdjustement = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (centerY - m_viewOffsetY < upper / 2) { y = int(round(centerY - 2 * sizeY) + m_viewOffsetY); }
		else { y = int(round(centerY + 2 * sizeY - pagesize)) + m_viewOffsetY; }
		gtk_adjustment_set_value(l_pAdjustement, y);
		gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent), l_pAdjustement);
		ret_val = true;
	}
	return ret_val;
}

void CInterfacedScenario::setScale(const double scale)
{
	m_currentScale = std::max(scale, 0.1);

	PangoContext* l_pPangoContext                 = gtk_widget_get_pango_context(GTK_WIDGET(m_pScenarioDrawingArea));
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
