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

	GdkColor colorFromIdentifier(const CIdentifier& rIdentifier)
	{
		GdkColor l_oGdkColor;
		unsigned int l_ui32Value1 = 0;
		unsigned int l_ui32Value2 = 0;
		uint64_t l_ui64Result = 0;

		sscanf(rIdentifier.toString(), "(0x%08X, 0x%08X)", &l_ui32Value1, &l_ui32Value2);
		l_ui64Result += l_ui32Value1;
		l_ui64Result <<= 32;
		l_ui64Result += l_ui32Value2;

		l_oGdkColor.pixel = guint16(0);
		l_oGdkColor.red = guint16((l_ui64Result & 0xffff) | 0x8000);
		l_oGdkColor.green = guint16(((l_ui64Result >> 16) & 0xffff) | 0x8000);
		l_oGdkColor.blue = guint16(((l_ui64Result >> 32) & 0xffff) | 0x8000);

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
				if (c == '/')
				{
					l_sOutput += "_";
				}
				else
				{
					if (l_bLastWasSeparator)
					{
						l_sOutput += std::to_string(std::toupper(c));
					}
					else
					{
						l_sOutput += c;
					}
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

	void count_widget_cb(GtkWidget* /*pWidget*/, gpointer data)
	{
		int* i = reinterpret_cast<int*>(data);
		if (i)
		{
			(*i)++;
		}
	}

	int gtk_container_get_children_count(GtkContainer* pContainer)
	{
		int l_iCount = 0;
		gtk_container_foreach(pContainer, count_widget_cb, &l_iCount);
		return l_iCount;
	}

	gboolean scenario_scrolledwindow_scroll_event_cb(GtkWidget* /*pWidget*/, GdkEventScroll* pEvent)
	{
		guint l_uiState = pEvent->state & gtk_accelerator_get_default_mod_mask();

		/* Shift+Wheel scrolls the in the perpendicular direction */
		if (l_uiState & GDK_SHIFT_MASK)
		{
			if (pEvent->direction == GDK_SCROLL_UP) { pEvent->direction = GDK_SCROLL_LEFT; }
			else if (pEvent->direction == GDK_SCROLL_LEFT) { pEvent->direction = GDK_SCROLL_UP; }
			else if (pEvent->direction == GDK_SCROLL_DOWN) { pEvent->direction = GDK_SCROLL_RIGHT; }
			else if (pEvent->direction == GDK_SCROLL_RIGHT) { pEvent->direction = GDK_SCROLL_DOWN; }

			pEvent->state &= ~GDK_SHIFT_MASK;
			l_uiState &= ~GDK_SHIFT_MASK;
		}

		return FALSE;
	}

	void scenario_drawing_area_expose_cb(GtkWidget* /*pWidget*/, GdkEventExpose* pEvent, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaExposeCB(pEvent);
	}

	void scenario_drawing_area_drag_data_received_cb(GtkWidget* /*pWidget*/, GdkDragContext* pDragContext, const gint iX, const gint iY, GtkSelectionData* pSelectionData, const guint uiInfo, const guint uiT, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaDragDataReceivedCB(pDragContext, iX, iY, pSelectionData, uiInfo, uiT);
	}

	gboolean scenario_drawing_area_motion_notify_cb(GtkWidget* pWidget, GdkEventMotion* pEvent, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaMotionNotifyCB(pWidget, pEvent);
		return FALSE;
	}

	void scenario_drawing_area_button_pressed_cb(GtkWidget* pWidget, GdkEventButton* pEvent, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaButtonPressedCB(pWidget, pEvent);
	}

	void scenario_drawing_area_button_released_cb(GtkWidget* pWidget, GdkEventButton* pEvent, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaButtonReleasedCB(pWidget, pEvent);
	}

	void scenario_drawing_area_key_press_event_cb(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaKeyPressEventCB(pWidget, pEvent);
	}

	void scenario_drawing_area_key_release_event_cb(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer data)
	{
		static_cast<CInterfacedScenario*>(data)->scenarioDrawingAreaKeyReleaseEventCB(pWidget, pEvent);
	}

	void context_menu_cb(GtkMenuItem* /*pMenuItem*/, CInterfacedScenario::BoxContextMenuCB* pContextMenuCB)
	{
		//CInterfacedScenario::BoxContextMenuCB* pContextMenuCB=static_cast < CInterfacedScenario::BoxContextMenuCB* >(data);
		switch (pContextMenuCB->ui32Command)
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
					else
					{
						pContextMenuCB->pInterfacedScenario->deleteSelection();
					}
					break;
				}
			case ContextMenu_BoxAddInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxAddInputCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEditInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditInputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index);
				break;
			case ContextMenu_BoxRemoveInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveInputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index);
				break;
			case ContextMenu_BoxAddOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxAddOutputCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEditOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditOutputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index);
				break;
			case ContextMenu_BoxRemoveOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveOutputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index);
				break;

			case ContextMenu_BoxConnectScenarioInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxConnectScenarioInputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index, pContextMenuCB->ui32SecondaryIndex);
				break;
			case ContextMenu_BoxConnectScenarioOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxConnectScenarioOutputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index, pContextMenuCB->ui32SecondaryIndex);
				break;

			case ContextMenu_BoxDisconnectScenarioInput: pContextMenuCB->pInterfacedScenario->contextMenuBoxDisconnectScenarioInputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index, pContextMenuCB->ui32SecondaryIndex);
				break;
			case ContextMenu_BoxDisconnectScenarioOutput: pContextMenuCB->pInterfacedScenario->contextMenuBoxDisconnectScenarioOutputCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index, pContextMenuCB->ui32SecondaryIndex);
				break;

			case ContextMenu_BoxAddSetting: pContextMenuCB->pInterfacedScenario->contextMenuBoxAddSettingCB(*pContextMenuCB->pBox);
				break;
			case ContextMenu_BoxEditSetting: pContextMenuCB->pInterfacedScenario->contextMenuBoxEditSettingCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index);
				break;
			case ContextMenu_BoxRemoveSetting: pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveSettingCB(*pContextMenuCB->pBox, pContextMenuCB->ui32Index);
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
					else
					{
						pContextMenuCB->pInterfacedScenario->contextMenuBoxEnableAllCB();
					}
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

	void gdk_draw_rounded_rectangle(GdkDrawable* pDrawable, GdkGC* pDrawGC, const gboolean bFill, const gint x, const gint y, const gint width, const gint height, const gint radius = 8)
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
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x + width - radius * 2, y, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 0 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x, y, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 90 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x, y + height - radius * 2, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 180 * 64, 90 * 64);
		gdk_draw_arc(pDrawable, pDrawGC, bFill, x + width - radius * 2, y + height - radius * 2, radius * 2 + (bFill != 0 ? 2 : 1), radius * 2 + (bFill != 0 ? 2 : 1), 270 * 64, 90 * 64);
#else
#pragma error("you should give a version of this function for your OS")
#endif
	}

	void scenario_title_button_close_cb(GtkButton* /*pButton*/, gpointer data)
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

	void scenario_configuration_add_setting_cb(GtkWidget*, CInterfacedScenario* pInterfacedScenario)
	{
		pInterfacedScenario->addScenarioSettingCB();
	}

	void modify_scenario_setting_value_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* pData)
	{
		CIdentifier l_oSettingType = OV_UndefinedIdentifier;
		pData->interfacedScenario->m_rScenario.getSettingType(pData->settingIndex, l_oSettingType);
		pData->interfacedScenario->m_rScenario.setSettingValue(pData->settingIndex, pData->interfacedScenario->m_pSettingHelper->getValue(l_oSettingType, pData->widgetValue));
		pData->interfacedScenario->m_bHasBeenModified = true;
		pData->interfacedScenario->updateScenarioLabel();
	}

	void modify_scenario_setting_default_value_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* pData)
	{
		CIdentifier l_oSettingType = OV_UndefinedIdentifier;
		pData->interfacedScenario->m_rScenario.getSettingType(pData->settingIndex, l_oSettingType);
		pData->interfacedScenario->m_rScenario.setSettingDefaultValue(pData->settingIndex, pData->interfacedScenario->m_pSettingHelper->getValue(l_oSettingType, pData->widgetValue));

		// We also se the 'actual' value to this
		pData->interfacedScenario->m_rScenario.setSettingValue(pData->settingIndex, pData->interfacedScenario->m_pSettingHelper->getValue(l_oSettingType, pData->widgetValue));
		pData->interfacedScenario->m_bHasBeenModified = true;
		pData->interfacedScenario->updateScenarioLabel();
	}

	void modify_scenario_setting_move_up_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* pData)
	{
		if (pData->settingIndex == 0) { return; }

		pData->interfacedScenario->swapScenarioSettings(pData->settingIndex - 1, pData->settingIndex);
	}

	void modify_scenario_setting_move_down_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* pData)
	{
		if (pData->settingIndex >= pData->interfacedScenario->m_rScenario.getSettingCount() - 1) { return; }

		pData->interfacedScenario->swapScenarioSettings(pData->settingIndex, pData->settingIndex + 1);
	}

	void modify_scenario_setting_revert_to_default_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* pData)
	{
		CString l_sSettingDefaultValue;
		pData->interfacedScenario->m_rScenario.getSettingDefaultValue(pData->settingIndex, l_sSettingDefaultValue);

		pData->interfacedScenario->m_rScenario.setSettingValue(pData->settingIndex, l_sSettingDefaultValue);
		pData->interfacedScenario->redrawScenarioSettings();
	}

	void copy_scenario_setting_token_cb(GtkWidget*, CInterfacedScenario::SSettingCallbackData* pData)
	{
		CString l_sSettingName;
		pData->interfacedScenario->m_rScenario.getSettingName(pData->settingIndex, l_sSettingName);
		l_sSettingName = CString("$var{") + l_sSettingName + CString("}");

		GtkClipboard* l_pDefaultClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text(l_pDefaultClipboard, l_sSettingName.toASCIIString(), -1);

		// On X11 there is another clipboard that it is useful to set as well
		GtkClipboard* l_pX11Clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
		gtk_clipboard_set_text(l_pX11Clipboard, l_sSettingName.toASCIIString(), -1);
	}

	void modify_scenario_setting_type_cb(GtkWidget* pCombobox, CInterfacedScenario::SSettingCallbackData* pData)
	{
		GtkBuilder* l_pSettingsGUIBuilder = gtk_builder_new();
		gtk_builder_add_from_string(l_pSettingsGUIBuilder, pData->interfacedScenario->m_sSerializedSettingGUIXML.c_str(), pData->interfacedScenario->m_sSerializedSettingGUIXML.length(), nullptr);

		gtk_widget_destroy(pData->widgetValue);

		CIdentifier l_oSettingType = pData->interfacedScenario->m_vSettingType[gtk_combo_box_get_active_text(GTK_COMBO_BOX(pCombobox))];
		pData->interfacedScenario->m_rScenario.setSettingType(pData->settingIndex, l_oSettingType);

		CString l_sSettingWidgetName = pData->interfacedScenario->m_pSettingHelper->getSettingWidgetName(l_oSettingType);

		GtkWidget* l_pWidgetValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingWidgetName.toASCIIString()));

		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidgetValue)), l_pWidgetValue);
		gtk_table_attach_defaults(GTK_TABLE(pData->container), l_pWidgetValue, 1, 5, 1, 2);

		// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
		CString l_sSettingValue;
		pData->interfacedScenario->m_rScenario.getSettingDefaultValue(pData->settingIndex, l_sSettingValue);
		pData->interfacedScenario->m_pSettingHelper->setValue(l_oSettingType, l_pWidgetValue, l_sSettingValue);

		// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
		CString l_sSettingEntryWidgetName = pData->interfacedScenario->m_pSettingHelper->getSettingEntryWidgetName(l_oSettingType);
		GtkWidget* l_pWidgetEntryValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingEntryWidgetName.toASCIIString()));

		pData->widgetValue = l_pWidgetValue;
		pData->widgetEntryValue = l_pWidgetEntryValue;

		g_signal_connect(l_pWidgetEntryValue, "changed", G_CALLBACK(modify_scenario_setting_default_value_cb), pData);

		g_object_unref(l_pSettingsGUIBuilder);
	}

	void delete_scenario_setting_cb(GtkWidget* /*pDeleteButton*/, CInterfacedScenario::SSettingCallbackData* pData)
	{
		pData->interfacedScenario->m_rScenario.removeSetting(pData->settingIndex);
		pData->interfacedScenario->redrawConfigureScenarioSettingsDialog();
	}

	void modify_scenario_setting_name_cb(GtkWidget* pEntry, CInterfacedScenario::SSettingCallbackData* pData)
	{
		pData->interfacedScenario->m_rScenario.setSettingName(pData->settingIndex, gtk_entry_get_text(GTK_ENTRY(pEntry)));
	}

	void reset_scenario_setting_identifier_cb(GtkWidget* /*button*/, CInterfacedScenario::SSettingCallbackData* data)
	{
		CIdentifier newIdentifier = data->interfacedScenario->m_rScenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier);
		if (newIdentifier != OV_UndefinedIdentifier)
		{
			data->interfacedScenario->m_rScenario.updateInterfacorIdentifier(BoxInterfacorType::Setting, uint32_t(data->settingIndex), newIdentifier);
			data->interfacedScenario->redrawConfigureScenarioSettingsDialog();
		}
	}

	void modify_scenario_setting_identifier_cb(GtkWidget* entry, CInterfacedScenario::SSettingCallbackData* data)
	{
		CIdentifier newIdentifier;
		if (newIdentifier.fromString(gtk_entry_get_text(GTK_ENTRY(entry))))
		{
			data->interfacedScenario->m_rScenario.updateInterfacorIdentifier(BoxInterfacorType::Setting, uint32_t(data->settingIndex), newIdentifier);
		}
	}

	void edit_scenario_link_cb(GtkWidget*, CInterfacedScenario::SLinkCallbackData* pData)
	{
		if (pData->m_bIsInput)
		{
			pData->m_pInterfacedScenario->editScenarioInputCB(pData->m_uiLinkIndex);
		}
		else
		{
			pData->m_pInterfacedScenario->editScenarioOutputCB(pData->m_uiLinkIndex);
		}
		pData->m_pInterfacedScenario->redraw();
	}

	void modify_scenario_link_move_up_cb(GtkWidget*, CInterfacedScenario::SLinkCallbackData* pData)
	{
		if (pData->m_uiLinkIndex == 0) { return; }
		if (pData->m_bIsInput)
		{
			pData->m_pInterfacedScenario->swapScenarioInputs(pData->m_uiLinkIndex - 1, pData->m_uiLinkIndex);
		}
		else
		{
			pData->m_pInterfacedScenario->swapScenarioOutputs(pData->m_uiLinkIndex - 1, pData->m_uiLinkIndex);
		}

		pData->m_pInterfacedScenario->snapshotCB();
	}

	void modify_scenario_link_move_down_cb(GtkWidget*, CInterfacedScenario::SLinkCallbackData* pData)
	{
		auto interfacorType = pData->m_bIsInput ? Input : Output;
		if (pData->m_pInterfacedScenario->m_rScenario.getInterfacorCount(interfacorType) < 2
			|| pData->m_uiLinkIndex >= pData->m_pInterfacedScenario->m_rScenario.getInterfacorCount(interfacorType) - 1)
		{
			return;
		}

		if (pData->m_bIsInput)
		{
			pData->m_pInterfacedScenario->swapScenarioInputs(pData->m_uiLinkIndex, pData->m_uiLinkIndex + 1);
		}
		else
		{
			pData->m_pInterfacedScenario->swapScenarioOutputs(pData->m_uiLinkIndex, pData->m_uiLinkIndex + 1);
		}
		pData->m_pInterfacedScenario->snapshotCB();
	}

	void delete_scenario_link_cb(GtkButton*, CInterfacedScenario::SLinkCallbackData* pData)
	{
		if (pData->m_bIsInput)
		{
			pData->m_pInterfacedScenario->m_rScenario.removeScenarioInput(pData->m_uiLinkIndex);
			pData->m_pInterfacedScenario->redrawScenarioInputSettings();
		}
		else
		{
			pData->m_pInterfacedScenario->m_rScenario.removeScenarioOutput(pData->m_uiLinkIndex);
			pData->m_pInterfacedScenario->redrawScenarioOutputSettings();
		}

		pData->m_pInterfacedScenario->snapshotCB();
		pData->m_pInterfacedScenario->redraw();
	}

	void modify_scenario_link_name_cb(GtkWidget* pEntry, CInterfacedScenario::SLinkCallbackData* pData)
	{
		if (pData->m_bIsInput)
		{
			pData->m_pInterfacedScenario->m_rScenario.setInputName(pData->m_uiLinkIndex, gtk_entry_get_text(GTK_ENTRY(pEntry)));
		}
		else
		{
			pData->m_pInterfacedScenario->m_rScenario.setOutputName(pData->m_uiLinkIndex, gtk_entry_get_text(GTK_ENTRY(pEntry)));
		}
	}

	void modify_scenario_link_type_cb(GtkWidget* pComboBox, CInterfacedScenario::SLinkCallbackData* pData)
	{
		const CIdentifier l_oStreamType = pData->m_pInterfacedScenario->m_mStreamType[gtk_combo_box_get_active_text(GTK_COMBO_BOX(pComboBox))];

		if (pData->m_bIsInput)
		{
			pData->m_pInterfacedScenario->m_rScenario.setInputType(pData->m_uiLinkIndex, l_oStreamType);
		}
		else
		{
			pData->m_pInterfacedScenario->m_rScenario.setOutputType(pData->m_uiLinkIndex, l_oStreamType);
		}

		pData->m_pInterfacedScenario->redraw();
	}
}  // namespace

CInterfacedScenario::CInterfacedScenario(const IKernelContext& rKernelContext, CApplication& rApplication, IScenario& rScenario, CIdentifier& rScenarioIdentifier, GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename)
	: m_ePlayerStatus(PlayerStatus_Stop), m_oScenarioIdentifier(rScenarioIdentifier), m_rApplication(rApplication), m_rKernelContext(rKernelContext),
	  m_rScenario(rScenario), m_rNotebook(rNotebook), m_sGUIFilename(sGUIFilename), m_sGUISettingsFilename(sGUISettingsFilename)
{
	m_pGUIBuilder = gtk_builder_new();
	gtk_builder_add_from_file(m_pGUIBuilder, m_sGUIFilename.c_str(), nullptr);
	gtk_builder_connect_signals(m_pGUIBuilder, nullptr);

	std::ifstream l_oSettingGUIFilestream;
	FS::Files::openIFStream(l_oSettingGUIFilestream, m_sGUISettingsFilename.c_str());
	m_sSerializedSettingGUIXML = std::string((std::istreambuf_iterator<char>(l_oSettingGUIFilestream)), std::istreambuf_iterator<char>());

	m_pSettingHelper = new CSettingCollectionHelper(m_rKernelContext, m_sGUISettingsFilename.c_str());

	// We will need to access setting types by their name later
	CIdentifier l_oCurrentTypeIdentifier;
	while ((l_oCurrentTypeIdentifier = m_rKernelContext.getTypeManager().getNextTypeIdentifier(l_oCurrentTypeIdentifier)) != OV_UndefinedIdentifier)
	{
		if (!m_rKernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier))
		{
			m_vSettingType[m_rKernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString()] = l_oCurrentTypeIdentifier;
		}
		else
		{
			m_mStreamType[m_rKernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString()] = l_oCurrentTypeIdentifier;
		}
	}

	m_pNotebookPageTitle = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "openvibe_scenario_notebook_title"));
	m_pNotebookPageContent = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "openvibe_scenario_notebook_scrolledwindow"));

	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_notebook")), 0);
	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_notebook")), 0);
	gtk_notebook_append_page(&m_rNotebook, m_pNotebookPageContent, m_pNotebookPageTitle);
	gtk_notebook_set_tab_reorderable(&m_rNotebook, m_pNotebookPageContent, 1);

	GtkWidget* l_pCloseWidget = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_button_close"));
	g_signal_connect(G_OBJECT(l_pCloseWidget), "clicked", G_CALLBACK(scenario_title_button_close_cb), this);

	m_pScenarioDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_drawing_area"));
	m_pScenarioViewport = GTK_VIEWPORT(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_viewport"));
	gtk_drag_dest_set(GTK_WIDGET(m_pScenarioDrawingArea), GTK_DEST_DEFAULT_ALL, g_vTargetEntry, sizeof(g_vTargetEntry) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
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
	m_pDesignerVisualization = new CDesignerVisualization(m_rKernelContext, *m_pVisualizationTree, *this);
	m_pDesignerVisualization->init(string(sGUIFilename));

	m_pConfigureSettingsDialog = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_scenario_configuration"));

	m_pSettingsVBox = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_scenario_configuration-vbox"));

	m_pNoHelpDialog = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_no_help"));

	m_pErrorPendingDeprecatedInterfacorsDialog = GTK_WIDGET(gtk_builder_get_object(m_rApplication.m_pBuilderInterface, "dialog_pending_deprecated_interfacors"));

	this->redrawScenarioSettings();
	this->redrawScenarioInputSettings();
	this->redrawScenarioOutputSettings();

	m_oStateStack.reset(new CScenarioStateStack(rKernelContext, *this, rScenario));

	this->updateScenarioLabel();

	// Output a log message if any box of the scenario is in some special state
	CIdentifier l_oBoxIdentifier = OV_UndefinedIdentifier;
	bool warningUpdate = false;
	bool warningDeprecated = false;
	bool warningUnknown = false;
	while ((l_oBoxIdentifier = m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
	{
		//const IBox *l_pBox = m_rScenario.getBoxDetails(l_oBoxIdentifier);
		//const CBoxProxy l_oBoxProxy(m_rKernelContext, *l_pBox);
		const CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, l_oBoxIdentifier);

		if (!warningUpdate && !l_oBoxProxy.isUpToDate())
		{
			m_rKernelContext.getLogManager() << LogLevel_Warning << "Scenario requires 'update' of some box(es). You need to replace these boxes or the scenario may not work correctly.\n";
			warningUpdate = true;
		}
		if (!warningDeprecated && l_oBoxProxy.isDeprecated())
		{
			m_rKernelContext.getLogManager() << LogLevel_Warning << "Scenario constains deprecated box(es). Please consider using other boxes instead.\n";
			warningDeprecated = true;
		}
		//		if (!noteUnstable && l_oBoxProxy.isUnstable())
		//		{
		//			m_rKernelContext.getLogManager() << LogLevel_Debug << "Scenario contains unstable box(es).\n";
		//			noteUnstable = true;
		//		}
		if (!warningUnknown && !l_oBoxProxy.isBoxAlgorithmPluginPresent())
		{
			m_rKernelContext.getLogManager() << LogLevel_Warning << "Scenario contains unknown box algorithm(s).\n";
			if (l_oBoxProxy.isMetabox())
			{
				CString mPath = m_rKernelContext.getConfigurationManager().expand("${Kernel_Metabox}");
				m_rKernelContext.getLogManager() << LogLevel_Warning << "Some Metaboxes could not be found in [" << mPath << "]\n";
			}
			warningUnknown = true;
		}
	}
}

CInterfacedScenario::~CInterfacedScenario()

{
	//delete window manager


	delete m_pDesignerVisualization;


	if (m_pStencilBuffer != nullptr)
	{
		g_object_unref(m_pStencilBuffer);
	}

	g_object_unref(m_pGUIBuilder);
	/*
	g_object_unref(m_pBuilder);
	g_object_unref(m_pBuilder);
	*/

	gtk_notebook_remove_page(&m_rNotebook, gtk_notebook_page_num(&m_rNotebook, m_pNotebookPageContent));
}

bool CInterfacedScenario::isLocked() const { return m_pPlayer != nullptr; }

void CInterfacedScenario::redraw()

{
	if (GDK_IS_WINDOW(GTK_WIDGET(m_pScenarioDrawingArea)->window))
	{
		gdk_window_invalidate_rect(GTK_WIDGET(m_pScenarioDrawingArea)->window, nullptr, 1);
	}
}

// This function repaints the dialog which opens when configuring settings
void CInterfacedScenario::redrawConfigureScenarioSettingsDialog()
{
	if (m_bHasFileName)
	{
		char l_sScenarioFilename[1024];
		FS::Files::getFilename(m_sFileName.c_str(), l_sScenarioFilename);
		char l_sWindowTitle[2048];
		sprintf(l_sWindowTitle, "Settings for \"%s\"", l_sScenarioFilename);
		gtk_window_set_title(GTK_WINDOW(m_pConfigureSettingsDialog), l_sWindowTitle);
	}
	else
	{
		gtk_window_set_title(GTK_WINDOW(m_pConfigureSettingsDialog), "Settings for an unnamed scenario");
	}

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

			GtkWidget* l_pSettingEntryName = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-entry_name"));
			GtkWidget* l_pSettingComboboxType = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-combobox_type"));
			GtkWidget* l_pSettingButtonUp = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_move_up"));
			GtkWidget* l_pSettingButtonDown = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_move_down"));
			GtkWidget* l_pSettingButtonDelete = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_delete"));
			GtkWidget* l_pSettingEntryIdentifier = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-entry_identifier"));
			GtkWidget* l_pSettingButtonResetIdentifier = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_configuration_setting-button_reset_identifier"));

			// fill the type dropdown
			CIdentifier l_oSettingTypeIdentifier = OV_UndefinedIdentifier;
			m_rScenario.getSettingType(l_ui32SettingIndex, l_oSettingTypeIdentifier);

			CIdentifier l_oCurrentTypeIdentifier;
			gint l_iCurrentSettingIndex = 0;
			while ((l_oCurrentTypeIdentifier = m_rKernelContext.getTypeManager().getNextTypeIdentifier(l_oCurrentTypeIdentifier)) != OV_UndefinedIdentifier)
			{
				if (!m_rKernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier))
				{
					gtk_combo_box_append_text(GTK_COMBO_BOX(l_pSettingComboboxType), m_rKernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString());
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

			GtkWidget* l_pWidgetDefaultValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingWidgetName.toASCIIString()));

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidgetDefaultValue)), l_pWidgetDefaultValue);
			gtk_table_attach_defaults(GTK_TABLE(l_pSettingContainerWidget), l_pWidgetDefaultValue, 1, 5, 1, 2);

			// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
			CString l_sSettingDefaultValue;
			m_rScenario.getSettingDefaultValue(l_ui32SettingIndex, l_sSettingDefaultValue);
			m_pSettingHelper->setValue(l_oSettingTypeIdentifier, l_pWidgetDefaultValue, l_sSettingDefaultValue);

			// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
			CString l_sSettingEntryWidgetName = m_pSettingHelper->getSettingEntryWidgetName(l_oSettingTypeIdentifier);
			GtkWidget* l_pWidgetEntryDefaultValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingEntryWidgetName.toASCIIString()));

			// Set the callbacks
			SSettingCallbackData l_oCallbackData;
			l_oCallbackData.interfacedScenario = this;
			l_oCallbackData.settingIndex = l_ui32SettingIndex;
			l_oCallbackData.widgetValue = l_pWidgetDefaultValue;
			l_oCallbackData.widgetEntryValue = l_pWidgetEntryDefaultValue;
			l_oCallbackData.container = l_pSettingContainerWidget;

			m_vSettingConfigurationCallbackData[l_ui32SettingIndex] = l_oCallbackData;

			// Connect signals of the container
			g_signal_connect(G_OBJECT(l_pSettingComboboxType), "changed", G_CALLBACK(modify_scenario_setting_type_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonDelete), "clicked", G_CALLBACK(delete_scenario_setting_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonUp), "clicked", G_CALLBACK(modify_scenario_setting_move_up_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonDown), "clicked", G_CALLBACK(modify_scenario_setting_move_down_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingEntryName), "changed", G_CALLBACK(modify_scenario_setting_name_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingEntryIdentifier), "activate", G_CALLBACK(modify_scenario_setting_identifier_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonResetIdentifier), "clicked", G_CALLBACK(reset_scenario_setting_identifier_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);

			// these callbacks assure that we can use copy/paste and undo within editable fields
			// as otherwise the keyboard shortucts are stolen by the designer
			g_signal_connect(G_OBJECT(l_pSettingEntryName), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_pSettingEntryName), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_pWidgetEntryDefaultValue), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_pWidgetEntryDefaultValue), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_rApplication);

			// add callbacks for setting the settings
			g_signal_connect(l_pWidgetEntryDefaultValue, "changed", G_CALLBACK(modify_scenario_setting_default_value_cb), &m_vSettingConfigurationCallbackData[l_ui32SettingIndex]);

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

			GtkWidget* l_pSettingLabelName = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-label"));
			GtkWidget* l_pSettingButtonDefault = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-button_default"));
			GtkWidget* l_pSettingButtonCopy = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, "scenario_setting-button_copy"));

			// Set name
			CString l_sSettingLabel;
			m_rScenario.getSettingName(l_ui32SettingIndex, l_sSettingLabel);
			gtk_label_set_text(GTK_LABEL(l_pSettingLabelName), l_sSettingLabel.toASCIIString());
			gtk_misc_set_alignment(GTK_MISC(l_pSettingLabelName), 0.0, 0.5);

			// Add widget for the actual setting
			CIdentifier l_oSettingTypeIdentifier = OV_UndefinedIdentifier;
			m_rScenario.getSettingType(l_ui32SettingIndex, l_oSettingTypeIdentifier);
			CString l_sSettingWidgetName = m_pSettingHelper->getSettingWidgetName(l_oSettingTypeIdentifier);

			GtkWidget* l_pWidgetValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingWidgetName.toASCIIString()));

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidgetValue)), l_pWidgetValue);
			gtk_table_attach_defaults(GTK_TABLE(l_pSettingContainerWidget), l_pWidgetValue, 0, 1, 1, 2);

			// Set the value and connect GUI callbacks (because, yes, setValue connects callbacks like a ninja)
			CString l_sSettingValue;
			m_rScenario.getSettingValue(l_ui32SettingIndex, l_sSettingValue);
			m_pSettingHelper->setValue(l_oSettingTypeIdentifier, l_pWidgetValue, l_sSettingValue);

			// add callbacks to disable the Edit menu in openvibe designer, which will in turn enable using stuff like copy-paste inside the widget
			CString l_sSettingEntryWidgetName = m_pSettingHelper->getSettingEntryWidgetName(l_oSettingTypeIdentifier);
			GtkWidget* l_pWidgetEntryValue = GTK_WIDGET(gtk_builder_get_object(l_pSettingsGUIBuilder, l_sSettingEntryWidgetName.toASCIIString()));

			// Set the callbacks
			SSettingCallbackData l_oCallbackData;
			l_oCallbackData.interfacedScenario = this;
			l_oCallbackData.settingIndex = l_ui32SettingIndex;
			l_oCallbackData.widgetValue = l_pWidgetValue;
			l_oCallbackData.widgetEntryValue = l_pWidgetEntryValue;
			l_oCallbackData.container = l_pSettingContainerWidget;

			m_vSettingCallbackData[l_ui32SettingIndex] = l_oCallbackData;

			// these callbacks assure that we can use copy/paste and undo within editable fields
			// as otherwise the keyboard shortucts are stolen by the designer
			g_signal_connect(G_OBJECT(l_pWidgetEntryValue), "focus-in-event", G_CALLBACK(editable_widget_focus_in_cb), &m_rApplication);
			g_signal_connect(G_OBJECT(l_pWidgetEntryValue), "focus-out-event", G_CALLBACK(editable_widget_focus_out_cb), &m_rApplication);

			// add callbacks for setting the settings
			g_signal_connect(l_pWidgetEntryValue, "changed", G_CALLBACK(modify_scenario_setting_value_cb), &m_vSettingCallbackData[l_ui32SettingIndex]);
			g_signal_connect(l_pSettingButtonDefault, "clicked", G_CALLBACK(modify_scenario_setting_revert_to_default_cb), &m_vSettingCallbackData[l_ui32SettingIndex]);
			g_signal_connect(l_pSettingButtonCopy, "clicked", G_CALLBACK(copy_scenario_setting_token_cb), &m_vSettingCallbackData[l_ui32SettingIndex]);

			g_object_unref(l_pSettingsGUIBuilder);
		}
	}
	gtk_widget_show_all(l_pSettingsVBox);
}

void CInterfacedScenario::redrawScenarioInputSettings()
{
	uint32_t (IScenario::* l_pfGetLinkCount)() const = &IScenario::getInputCount;
	bool (IScenario::* l_pfGetLinkName)(uint32_t, CString&) const = &IScenario::getInputName;
	bool (IScenario::* l_pfGetLinkType)(uint32_t, CIdentifier&) const = &IScenario::getInputType;

	this->redrawScenarioLinkSettings(m_rApplication.m_pTableInputs, true, m_vScenarioInputCallbackData, l_pfGetLinkCount, l_pfGetLinkName, l_pfGetLinkType);
}

void CInterfacedScenario::redrawScenarioOutputSettings()
{
	uint32_t (IScenario::* l_pfGetLinkCount)() const = &IScenario::getOutputCount;
	bool (IScenario::* l_pfGetLinkName)(uint32_t, CString&) const = &IScenario::getOutputName;
	bool (IScenario::* l_pfGetLinkType)(uint32_t, CIdentifier&) const = &IScenario::getOutputType;

	this->redrawScenarioLinkSettings(m_rApplication.m_pTableOutputs, false, m_vScenarioOutputCallbackData, l_pfGetLinkCount, l_pfGetLinkName, l_pfGetLinkType);
}

// Redraws the tab containing inputs or outputs of the scenario
// This method receives pointers to methods that manipulate either intpus or outputs so it can be generic
void CInterfacedScenario::redrawScenarioLinkSettings(GtkWidget* pLinkTable, bool bIsInput,
													 std::vector<SLinkCallbackData>& vLinkCallbackData, uint32_t (IScenario::* pfGetLinkCount)() const,
													 bool (IScenario::* pfGetLinkName)(uint32_t, CString&) const, bool (IScenario::* pfGetLinkType)(uint32_t, CIdentifier&) const
)
{
	GList* l_pSettingWidgets = gtk_container_get_children(GTK_CONTAINER(pLinkTable));
	for (GList* l_pSettingIterator = l_pSettingWidgets; l_pSettingIterator != nullptr; l_pSettingIterator = g_list_next(l_pSettingIterator))
	{
		gtk_widget_destroy(GTK_WIDGET(l_pSettingIterator->data));
	}
	g_list_free(l_pSettingWidgets);

	uint32_t l_ui32LinkCount = (m_rScenario.*pfGetLinkCount)();

	vLinkCallbackData.clear();
	vLinkCallbackData.resize(l_ui32LinkCount);

	gtk_table_resize(GTK_TABLE(pLinkTable), l_ui32LinkCount == 0 ? 1 : l_ui32LinkCount, 7);

	if (l_ui32LinkCount == 0)
	{
		GtkWidget* l_pSettingPlaceholderLabel = gtk_label_new("This scenario has none");
		gtk_table_attach_defaults(GTK_TABLE(pLinkTable), l_pSettingPlaceholderLabel, 0, 1, 0, 1);
	}
	else
	{
		for (uint32_t l_ui32LinkIndex = 0; l_ui32LinkIndex < l_ui32LinkCount; l_ui32LinkIndex++)
		{
			GtkBuilder* l_pIoSettingsGUIBuilder = gtk_builder_new();
			gtk_builder_add_from_string(l_pIoSettingsGUIBuilder, m_sSerializedSettingGUIXML.c_str(), m_sSerializedSettingGUIXML.length(), nullptr);

			GtkWidget* l_pSettingContainerWidget = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-table"));
			// this has to be done since the widget is already inside a parent in the gtkbuilder
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingContainerWidget)), l_pSettingContainerWidget);

			GtkWidget* l_pEntryLinkName = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-label"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pEntryLinkName)), l_pEntryLinkName);

			GtkWidget* l_pIoSettingComboboxType = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-combobox_type"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pIoSettingComboboxType)), l_pIoSettingComboboxType);

			// fill the type dropdown
			CIdentifier l_oLinkTypeIdentifier = OV_UndefinedIdentifier;
			(m_rScenario.*pfGetLinkType)(l_ui32LinkIndex, l_oLinkTypeIdentifier);

			CIdentifier l_oCurrentTypeIdentifier;
			gint l_iCurrentLinkIndex = 0;
			while ((l_oCurrentTypeIdentifier = m_rKernelContext.getTypeManager().getNextTypeIdentifier(l_oCurrentTypeIdentifier)) != OV_UndefinedIdentifier)
			{
				if (m_rKernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier))
				{
					gtk_combo_box_append_text(GTK_COMBO_BOX(l_pIoSettingComboboxType), m_rKernelContext.getTypeManager().getTypeName(l_oCurrentTypeIdentifier).toASCIIString());
					if (l_oCurrentTypeIdentifier == l_oLinkTypeIdentifier)
					{
						gtk_combo_box_set_active(GTK_COMBO_BOX(l_pIoSettingComboboxType), l_iCurrentLinkIndex);
					}

					l_iCurrentLinkIndex++;
				}
			}
			gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(l_pIoSettingComboboxType), GTK_SENSITIVITY_OFF);

			GtkWidget* l_pIoSettingButtonUp = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-button_move_up"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pIoSettingButtonUp)), l_pIoSettingButtonUp);
			GtkWidget* l_pIoSettingButtonDown = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-button_move_down"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pIoSettingButtonDown)), l_pIoSettingButtonDown);
			GtkWidget* l_pSettingButtonEdit = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-button_edit"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingButtonEdit)), l_pSettingButtonEdit);
			GtkWidget* l_pSettingButtonDelete = GTK_WIDGET(gtk_builder_get_object(l_pIoSettingsGUIBuilder, "scenario_io_setting-button_delete"));
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingButtonDelete)), l_pSettingButtonDelete);

			// Set name
			CString l_sLinkName;
			(m_rScenario.*pfGetLinkName)(l_ui32LinkIndex, l_sLinkName);
			gtk_label_set_text(GTK_LABEL(l_pEntryLinkName), l_sLinkName.toASCIIString());
			gtk_misc_set_alignment(GTK_MISC(l_pEntryLinkName), 0.0, 0.5);
			gtk_widget_set_sensitive(GTK_WIDGET(l_pEntryLinkName), GTK_SENSITIVITY_OFF);

			gtk_table_attach(GTK_TABLE(pLinkTable), l_pEntryLinkName, 0, 1, l_ui32LinkIndex, l_ui32LinkIndex + 1, static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pIoSettingComboboxType, 1, 2, l_ui32LinkIndex, l_ui32LinkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pIoSettingButtonUp, 3, 4, l_ui32LinkIndex, l_ui32LinkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pIoSettingButtonDown, 4, 5, l_ui32LinkIndex, l_ui32LinkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pSettingButtonEdit, 5, 6, l_ui32LinkIndex, l_ui32LinkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);
			gtk_table_attach(GTK_TABLE(pLinkTable), l_pSettingButtonDelete, 6, 7, l_ui32LinkIndex, l_ui32LinkIndex + 1, GTK_SHRINK, GTK_SHRINK, 4, 4);

			// Set the callbacks
			SLinkCallbackData l_oCallbackData;
			l_oCallbackData.m_pInterfacedScenario = this;
			l_oCallbackData.m_uiLinkIndex = l_ui32LinkIndex;
			l_oCallbackData.m_bIsInput = bIsInput;

			vLinkCallbackData[l_ui32LinkIndex] = l_oCallbackData;

			g_signal_connect(G_OBJECT(l_pSettingButtonDelete), "clicked", G_CALLBACK(delete_scenario_link_cb), &vLinkCallbackData[l_ui32LinkIndex]);
			g_signal_connect(G_OBJECT(l_pSettingButtonEdit), "clicked", G_CALLBACK(edit_scenario_link_cb), &vLinkCallbackData[l_ui32LinkIndex]);
			g_signal_connect(G_OBJECT(l_pIoSettingButtonUp), "clicked", G_CALLBACK(modify_scenario_link_move_up_cb), &vLinkCallbackData[l_ui32LinkIndex]);
			g_signal_connect(G_OBJECT(l_pIoSettingButtonDown), "clicked", G_CALLBACK(modify_scenario_link_move_down_cb), &vLinkCallbackData[l_ui32LinkIndex]);

			g_object_unref(l_pIoSettingsGUIBuilder);
		}
	}

	gtk_widget_show_all(pLinkTable);
}

void CInterfacedScenario::updateScenarioLabel()

{
	GtkLabel* l_pTitleLabel = GTK_LABEL(gtk_builder_get_object(m_pGUIBuilder, "openvibe-scenario_label"));
	string l_sLabel;
	string l_sTempFileName = m_sFileName;
	string l_sTitleLabelUntrimmed = "unsaved document";
	string::size_type l_iBackSlashIndex;
	while ((l_iBackSlashIndex = l_sTempFileName.find('\\')) != string::npos)
	{
		l_sTempFileName[l_iBackSlashIndex] = '/';
	}

	l_sLabel += m_bHasBeenModified ? "*" : "";
	l_sLabel += " ";

	// trimming file name if the number of character is above ${Designer_ScenarioFileNameTrimmingLimit}
	// trim only unselected scenarios
	if (m_bHasFileName)
	{
		l_sTitleLabelUntrimmed = l_sTempFileName;
		l_sTempFileName = l_sTempFileName.substr(l_sTempFileName.rfind('/') + 1);
		uint32_t l_ui32TrimLimit = uint32_t(m_rKernelContext.getConfigurationManager().expandAsUInteger("${Designer_ScenarioFileNameTrimmingLimit}", 25));
		if (l_ui32TrimLimit > 3) l_ui32TrimLimit -= 3; // limit should include the '...'
		// default = we trim everything but the current scenario filename
		// if  {we are stacking horizontally the scenarios, we trim also } current filename to avoid losing too much of the edition panel.
		if (l_sTempFileName.size() > l_ui32TrimLimit)
		{
			if (m_rApplication.getCurrentInterfacedScenario() == this && m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ScenarioTabsVerticalStack}", false))
			{
				l_sTempFileName = "..." + l_sTempFileName.substr(l_sTempFileName.size() - l_ui32TrimLimit, l_ui32TrimLimit);
			}
			if (m_rApplication.getCurrentInterfacedScenario() != this)
			{
				l_sTempFileName = l_sTempFileName.substr(0, l_ui32TrimLimit);
				l_sTempFileName += "...";
			}
		}
		l_sLabel += l_sTempFileName;
	}
	else
	{
		l_sLabel += "(untitled)";
	}

	l_sLabel += " ";
	l_sLabel += m_bHasBeenModified ? "*" : "";

	gtk_label_set_text(l_pTitleLabel, l_sLabel.c_str());

	std::string tooltipLabel = l_sTitleLabelUntrimmed;
	size_t index = 0;
	while ((index = tooltipLabel.find('&', index)) != std::string::npos)
	{
		tooltipLabel.replace(index, 1, "&amp;");
		index += 5;
	}
	gtk_widget_set_tooltip_markup(GTK_WIDGET(l_pTitleLabel), ("<i>" + tooltipLabel + (m_bHasBeenModified ? " - unsaved" : "") + "</i>").c_str());
}

#define updateStencilIndex(id,stencilgc) { (id)++; ::GdkColor sc={0, guint16(((id)&0xff0000)>>8), guint16((id)&0xff00), guint16(((id)&0xff)<<8) }; gdk_gc_set_rgb_fg_color(stencilgc, &sc); }

void CInterfacedScenario::redraw(IBox& rBox)
{
	GtkWidget* l_pWidget = GTK_WIDGET(m_pScenarioDrawingArea);
	GdkGC* l_pStencilGC = gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	GdkGC* l_pDrawGC = gdk_gc_new(l_pWidget->window);

	const int xMargin = int(round(5 * m_f64CurrentScale));
	const int yMargin = int(round(5 * m_f64CurrentScale));
	const int iCircleSize = int(round(11 * m_f64CurrentScale));
	const int iCircleSpace = int(round(4 * m_f64CurrentScale));

	//CBoxProxy l_oBoxProxy(m_rKernelContext, rBox);
	CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, rBox.getIdentifier());

	if (rBox.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(rBox.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		l_oBoxProxy.setBoxAlgorithmDescriptorOverride(static_cast<const IBoxAlgorithmDesc*>(m_rKernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId)));
	}

	int xSize = int(round(l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) * m_f64CurrentScale) + xMargin * 2);
	int ySize = int(round(l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) * m_f64CurrentScale) + yMargin * 2);
	int xStart = int(round(l_oBoxProxy.getXCenter() * m_f64CurrentScale + m_i32ViewOffsetX - (xSize >> 1)));
	int yStart = int(round(l_oBoxProxy.getYCenter() * m_f64CurrentScale + m_i32ViewOffsetY - (ySize >> 1)));

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_f64CurrentScale)));
	m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rBox.getIdentifier());

	bool l_bCanCreate = l_oBoxProxy.isBoxAlgorithmPluginPresent();
	bool l_bUpToDate = l_bCanCreate ? l_oBoxProxy.isUpToDate() : true;
	bool l_bPendingDeprecatedInterfacors = l_oBoxProxy.hasPendingDeprecatedInterfacors();
	bool l_bDeprecated = l_bCanCreate && l_oBoxProxy.isDeprecated();
	bool l_bMetabox = l_bCanCreate && l_oBoxProxy.isMetabox();
	bool l_bDisabled = l_oBoxProxy.isDisabled();


	// Check if this is a mensia box
	auto l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(rBox.getAlgorithmClassIdentifier());
	bool l_bMensia = (l_pPOD && l_pPOD->hasFunctionality(M_Functionality_IsMensia));

	// Add a thick dashed border around selected boxes
	if (m_SelectedObjects.count(rBox.getIdentifier()))
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
			l_iTopLeftOffset = 3;
			l_iBottomRightOffset = 6;
		}

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBorderSelected]);
		gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(l_pWidget->window, l_pDrawGC, TRUE, xStart - l_iTopLeftOffset, yStart - l_iTopLeftOffset, xSize + l_iBottomRightOffset, ySize + l_iBottomRightOffset);
	}

	if (!this->isLocked() || !m_bDebugCPUUsage)
	{
		/*if(m_vCurrentObject[rBox.getIdentifier()])
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
		else
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackground]);
		}
	}
	else
	{
		CIdentifier l_oComputationTime;
		l_oComputationTime.fromString(rBox.getAttributeValue(OV_AttributeId_Box_ComputationTimeLastSecond));
		uint64_t l_ui64ComputationTime = (l_oComputationTime == OV_UndefinedIdentifier ? 0 : l_oComputationTime.toUInteger());
		uint64_t l_ui64ComputationTimeReference = (1LL << 32) / (m_ui32BoxCount == 0 ? 1 : m_ui32BoxCount);

		GdkColor l_oColor;
		if (l_ui64ComputationTime < l_ui64ComputationTimeReference)
		{
			l_oColor.pixel = 0;
			l_oColor.red = guint16((l_ui64ComputationTime << 16) / l_ui64ComputationTimeReference);
			l_oColor.green = 32768;
			l_oColor.blue = 0;
		}
		else
		{
			if (l_ui64ComputationTime < l_ui64ComputationTimeReference * 4)
			{
				l_oColor.pixel = 0;
				l_oColor.red = 65535;
				l_oColor.green = guint16(32768 - ((l_ui64ComputationTime << 15) / (l_ui64ComputationTimeReference * 4)));
				l_oColor.blue = 0;
			}
			else
			{
				l_oColor.pixel = 0;
				l_oColor.red = 65535;
				l_oColor.green = 0;
				l_oColor.blue = 0;
			}
		}
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
	}

	gdk_draw_rounded_rectangle(l_pWidget->window, l_pDrawGC, TRUE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_f64CurrentScale)));

	if (l_bMensia)
	{
		gdk_draw_pixbuf(l_pWidget->window, l_pDrawGC, m_pMensiaLogoPixbuf, 5, 5, xStart, yStart, 80, (ySize < 50) ? ySize : 50, GDK_RGB_DITHER_NONE, 0, 0);
	}

	int l_iBorderColor = Color_BoxBorder;
	if (l_bMensia)
	{
		l_iBorderColor = Color_BoxBorderMensia;
	}
	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBorderColor]);
	gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
	gdk_draw_rounded_rectangle(l_pWidget->window, l_pDrawGC, FALSE, xStart, yStart, xSize, ySize, gint(round(8.0 * m_f64CurrentScale)));

	if (l_bMetabox)
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBorderColor]);
		gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
		gdk_draw_rounded_rectangle(l_pWidget->window, l_pDrawGC, FALSE, xStart - 3, yStart - 3, xSize + 6, ySize + 6, gint(round(8.0 * m_f64CurrentScale)));
	}

	TAttributeHandler l_oAttributeHandler(rBox);

	int l_iInputOffset = xSize / 2 - int(rBox.getInputCount()) * (iCircleSpace + iCircleSize) / 2 + iCircleSize / 4;
	for (uint32_t i = 0; i < rBox.getInterfacorCountIncludingDeprecated(Input); ++i)
	{
		CIdentifier l_oInputIdentifier;
		bool isDeprecated;
		rBox.getInputType(i, l_oInputIdentifier);
		rBox.getInterfacorDeprecatedStatus(Input, i, isDeprecated);
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

		updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, l_vPoint, 3);
		m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rBox.getIdentifier(), Box_Input, i);

		if (isDeprecated)
		{
			l_oInputColor.blue = 2 * l_oInputColor.blue / 3;
			l_oInputColor.red = 2 * l_oInputColor.red / 3;
			l_oInputColor.green = 2 * l_oInputColor.green / 3;
		}

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oInputColor);

		gdk_draw_polygon(l_pWidget->window, l_pDrawGC, TRUE, l_vPoint, 3);
		int l_iBoxInputBorderColor = Color_BoxInputBorder;
		if (isDeprecated) { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]); }
		else { gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxInputBorderColor]); }
		gdk_draw_polygon(l_pWidget->window, l_pDrawGC, FALSE, l_vPoint, 3);

		int32_t x = xStart + i * (iCircleSpace + iCircleSize) + (iCircleSize >> 1) - m_i32ViewOffsetX + l_iInputOffset;
		int32_t y = yStart - (iCircleSize >> 1) - m_i32ViewOffsetY;
		CIdentifier l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierToBoxInput(OV_UndefinedIdentifier, rBox.getIdentifier(), i);
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
				else
				{
					attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XTargetPosition, x);
				}

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
				{
					attributeHandler.addAttribute<int>(OV_AttributeId_Link_YTargetPosition, y);
				}
				else
				{
					attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YTargetPosition, y);
				}
			}
			l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierToBoxInput(l_oLinkIdentifier, rBox.getIdentifier(), i);
		}

		// Display a circle above inputs that are linked to the box inputs
		for (uint32_t l_ui32ScenarioInputIndex = 0; l_ui32ScenarioInputIndex < m_rScenario.getInputCount(); l_ui32ScenarioInputIndex++)
		{
			CIdentifier l_oScenarioInputLinkBoxIdentifier;
			uint32_t l_ui32ScenarioInputLinkBoxInputIndex;

			m_rScenario.getScenarioInputLink(l_ui32ScenarioInputIndex, l_oScenarioInputLinkBoxIdentifier, l_ui32ScenarioInputLinkBoxInputIndex);

			if (l_oScenarioInputLinkBoxIdentifier == rBox.getIdentifier() && l_ui32ScenarioInputLinkBoxInputIndex == i)
			{
				// Since the circle representing the input is quite large, we are going to offset each other one
				int l_iInputDiscOffset = int(i % 2) * iCircleSize * 2;

				int l_iScenarioInputIndicatorLeft = xStart + int(i) * (iCircleSpace + iCircleSize) + l_iInputOffset - int(iCircleSize * 0.5);
				int l_iScenarioInputIndicatorTop = yStart - (iCircleSize >> 1) - iCircleSize * 3 - l_iInputDiscOffset;

				CIdentifier l_oScenarioInputTypeIdentifier;
				this->m_rScenario.getInputType(l_ui32ScenarioInputIndex, l_oScenarioInputTypeIdentifier);
				GdkColor inputColor = colorFromIdentifier(l_oScenarioInputTypeIdentifier);

				updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE,
							 l_iScenarioInputIndicatorLeft,
							 l_iScenarioInputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rBox.getIdentifier(), Box_ScenarioInput, i);

				gdk_gc_set_rgb_fg_color(l_pDrawGC, &inputColor);

				gdk_draw_arc(l_pWidget->window, l_pDrawGC, TRUE,
							 l_iScenarioInputIndicatorLeft,
							 l_iScenarioInputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxInputBorderColor]);
				gdk_draw_arc(l_pWidget->window, l_pDrawGC, FALSE,
							 l_iScenarioInputIndicatorLeft,
							 l_iScenarioInputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);

				// Draw the text indicating the scenario input index
				PangoContext* l_pPangoContext = nullptr;
				PangoLayout* l_pPangoLayout = nullptr;
				l_pPangoContext = gtk_widget_get_pango_context(l_pWidget);
				l_pPangoLayout = pango_layout_new(l_pPangoContext);
				pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
				pango_layout_set_markup(l_pPangoLayout, std::to_string(static_cast<long long int>(l_ui32ScenarioInputIndex + 1)).c_str(), -1);
				gdk_draw_layout(l_pWidget->window, l_pWidget->style->text_gc[GTK_WIDGET_STATE(l_pWidget)],
								l_iScenarioInputIndicatorLeft + xMargin, l_iScenarioInputIndicatorTop + yMargin, l_pPangoLayout);
				g_object_unref(l_pPangoLayout);
				gdk_draw_line(l_pWidget->window, l_pDrawGC,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset + (iCircleSize >> 1),
							  l_iScenarioInputIndicatorTop + iCircleSize * 2,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iInputOffset + (iCircleSize >> 1),
							  yStart - (iCircleSize >> 1));
			}
		}
	}

	gdk_gc_set_line_attributes(l_pDrawGC, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);

	int l_iOutputOffset = xSize / 2 - int(rBox.getOutputCount()) * (iCircleSpace + iCircleSize) / 2 + iCircleSize / 4;
	for (uint32_t i = 0; i < rBox.getInterfacorCountIncludingDeprecated(Output); ++i)
	{
		CIdentifier l_oOutputIdentifier;
		bool isDeprecated;
		rBox.getOutputType(i, l_oOutputIdentifier);
		rBox.getInterfacorDeprecatedStatus(Output, i, isDeprecated);
		GdkColor l_oOutputColor = colorFromIdentifier(l_oOutputIdentifier);

		if (isDeprecated)
		{
			l_oOutputColor.blue = 2 * l_oOutputColor.blue / 3;
			l_oOutputColor.red = 2 * l_oOutputColor.red / 3;
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

		updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, l_vPoint, 3);
		m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rBox.getIdentifier(), Box_Output, i);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oOutputColor);
		gdk_draw_polygon(l_pWidget->window, l_pDrawGC, TRUE, l_vPoint, 3);
		int l_iBoxOutputBorderColor = Color_BoxOutputBorder;
		if (isDeprecated)
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]);
		}
		else
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxOutputBorderColor]);
		}

		gdk_draw_polygon(l_pWidget->window, l_pDrawGC, FALSE, l_vPoint, 3);

		int32_t x = xStart + i * (iCircleSpace + iCircleSize) + (iCircleSize >> 1) - m_i32ViewOffsetX + l_iOutputOffset;
		int32_t y = yStart + ySize + (iCircleSize >> 1) + 1 - m_i32ViewOffsetY;
		CIdentifier l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierFromBoxOutput(OV_UndefinedIdentifier, rBox.getIdentifier(), i);
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
				else
				{
					attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XSourcePosition, x);
				}

				if (!attributeHandler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
				{
					attributeHandler.addAttribute<int>(OV_AttributeId_Link_YSourcePosition, y);
				}
				else
					attributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YSourcePosition, y);
			}
			l_oLinkIdentifier = m_rScenario.getNextLinkIdentifierFromBoxOutput(l_oLinkIdentifier, rBox.getIdentifier(), i);
		}

		// Display a circle below outputs that are linked to the box outputs
		for (uint32_t l_ui32ScenarioOutputIndex = 0; l_ui32ScenarioOutputIndex < m_rScenario.getOutputCount(); l_ui32ScenarioOutputIndex++)
		{
			CIdentifier l_oScenarioOutputLinkBoxIdentifier;
			uint32_t l_ui32ScenarioOutputLinkBoxOutputIndex;

			m_rScenario.getScenarioOutputLink(l_ui32ScenarioOutputIndex, l_oScenarioOutputLinkBoxIdentifier, l_ui32ScenarioOutputLinkBoxOutputIndex);

			if (l_oScenarioOutputLinkBoxIdentifier == rBox.getIdentifier() && l_ui32ScenarioOutputLinkBoxOutputIndex == i)
			{
				// Since the circle representing the Output is quite large, we are going to offset each other one
				int l_iOutputDiscOffset = (int(i) % 2) * iCircleSize * 2;

				int l_iScenarioOutputIndicatorLeft = xStart + int(i) * (iCircleSpace + iCircleSize) + l_iOutputOffset - int(iCircleSize * 0.5);
				int l_iScenarioOutputIndicatorTop = yStart - (iCircleSize >> 1) + ySize + l_iOutputDiscOffset + iCircleSize * 2;

				CIdentifier l_oScenarioOutputTypeIdentifier;
				this->m_rScenario.getOutputType(l_ui32ScenarioOutputIndex, l_oScenarioOutputTypeIdentifier);
				GdkColor oOutputColor = colorFromIdentifier(l_oScenarioOutputTypeIdentifier);

				updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
				gdk_draw_arc(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE,
							 l_iScenarioOutputIndicatorLeft,
							 l_iScenarioOutputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rBox.getIdentifier(), Box_ScenarioOutput, i);

				gdk_gc_set_rgb_fg_color(l_pDrawGC, &oOutputColor);
				gdk_draw_arc(l_pWidget->window, l_pDrawGC, TRUE,
							 l_iScenarioOutputIndicatorLeft,
							 l_iScenarioOutputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);
				gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[l_iBoxOutputBorderColor]);
				gdk_draw_arc(l_pWidget->window, l_pDrawGC, FALSE,
							 l_iScenarioOutputIndicatorLeft,
							 l_iScenarioOutputIndicatorTop,
							 iCircleSize * 2, iCircleSize * 2, 0, 64 * 360);

				PangoContext* l_pPangoContext = nullptr;
				PangoLayout* l_pPangoLayout = nullptr;
				l_pPangoContext = gtk_widget_get_pango_context(l_pWidget);
				l_pPangoLayout = pango_layout_new(l_pPangoContext);
				pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
				pango_layout_set_markup(l_pPangoLayout, std::to_string(static_cast<long long int>(l_ui32ScenarioOutputIndex + 1)).c_str(), -1);
				gdk_draw_layout(l_pWidget->window, l_pWidget->style->text_gc[GTK_WIDGET_STATE(l_pWidget)],
								l_iScenarioOutputIndicatorLeft + xMargin, l_iScenarioOutputIndicatorTop + yMargin, l_pPangoLayout);
				g_object_unref(l_pPangoLayout);
				gdk_draw_line(l_pWidget->window, l_pDrawGC,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset + (iCircleSize >> 1),
							  l_iScenarioOutputIndicatorTop,
							  xStart + i * (iCircleSpace + iCircleSize) + l_iOutputOffset + (iCircleSize >> 1),
							  yStart + (iCircleSize >> 2) + ySize + 2); // This is somewhat the bottom of the triangle indicating a box output
			}
		}
	}

	/*
		::GdkPixbuf* l_pPixbuf=gtk_widget_render_icon(l_pWidget, GTK_STOCK_EXECUTE, GTK_ICON_SIZE_SMALL_TOOLBAR, "openvibe");
		if(l_pPixbuf)
		{
			gdk_draw_pixbuf(l_pWidget->window, l_pDrawGC, l_pPixbuf, 0, 0, 10, 10, 64, 64, GDK_RGB_DITHER_NONE, 0, 0);
			g_object_unref(l_pPixbuf);
		}
	*/

	// Draw labels

	PangoContext* l_pPangoContext = nullptr;
	PangoLayout* l_pPangoLayout = nullptr;
	l_pPangoContext = gtk_widget_get_pango_context(l_pWidget);
	l_pPangoLayout = pango_layout_new(l_pPangoContext);

	// Draw box label
	PangoRectangle l_oPangoLabelRect;
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(l_pPangoLayout, l_oBoxProxy.getLabel(), -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoLabelRect);
	gdk_draw_layout(l_pWidget->window, l_pWidget->style->text_gc[GTK_WIDGET_STATE(l_pWidget)], xStart + xMargin, yStart + yMargin, l_pPangoLayout);

	// Draw box status label
	PangoRectangle l_oPangoStatusRect;
	pango_layout_set_markup(l_pPangoLayout, l_oBoxProxy.getStatusLabel(), -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoStatusRect);
	int xShift = (max(l_oPangoLabelRect.width, l_oPangoStatusRect.width) -
		min(l_oPangoLabelRect.width, l_oPangoStatusRect.width)) / 2;

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE,
					   xStart + xShift + xMargin, yStart + l_oPangoLabelRect.height + yMargin,
					   l_oPangoStatusRect.width, l_oPangoStatusRect.height);
	m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rBox.getIdentifier(), Box_Update, 0);
	gdk_draw_layout(l_pWidget->window, l_pWidget->style->text_gc[GTK_WIDGET_STATE(l_pWidget)], xStart + xShift + xMargin, yStart + l_oPangoLabelRect.height + yMargin, l_pPangoLayout);

	g_object_unref(l_pPangoLayout);
	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);

	/*
		CLinkPositionSetterEnum l_oLinkPositionSetterInput(Connector_Input, l_vInputPosition);
		CLinkPositionSetterEnum l_oLinkPositionSetterOutput(Connector_Output, l_vOutputPosition);
		rScenario.enumerateLinksToBox(l_oLinkPositionSetterInput, rBox.getIdentifier());
		rScenario.enumerateLinksFromBox(l_oLinkPositionSetterOutput, rBox.getIdentifier());
	*/
}

void CInterfacedScenario::redraw(IComment& rComment)
{
	GtkWidget* l_pWidget = GTK_WIDGET(m_pScenarioDrawingArea);
	GdkGC* l_pStencilGC = gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	GdkGC* l_pDrawGC = gdk_gc_new(l_pWidget->window);

	// uint32_t i;
	const int xMargin = static_cast<const int>(round(16 * m_f64CurrentScale));
	const int yMargin = static_cast<const int>(round(16 * m_f64CurrentScale));

	CCommentProxy l_oCommentProxy(m_rKernelContext, rComment);
	int xSize = l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) + xMargin * 2;
	int ySize = l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) + yMargin * 2;
	int xStart = int(round(l_oCommentProxy.getXCenter() * m_f64CurrentScale + m_i32ViewOffsetX - (xSize >> 1)));
	int yStart = int(round(l_oCommentProxy.getYCenter() * m_f64CurrentScale + m_i32ViewOffsetY - (ySize >> 1)));

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_rounded_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, xStart, yStart, xSize, ySize, gint(round(16.0 * m_f64CurrentScale)));
	m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rComment.getIdentifier());

	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[m_SelectedObjects.count(rComment.getIdentifier()) ? Color_CommentBackgroundSelected : Color_CommentBackground]);
	gdk_draw_rounded_rectangle(l_pWidget->window, l_pDrawGC, TRUE, xStart, yStart, xSize, ySize, gint(round(16.0 * m_f64CurrentScale)));
	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[m_SelectedObjects.count(rComment.getIdentifier()) ? Color_CommentBorderSelected : Color_CommentBorder]);
	gdk_draw_rounded_rectangle(l_pWidget->window, l_pDrawGC, FALSE, xStart, yStart, xSize, ySize, gint(round(16.0 * m_f64CurrentScale)));

	PangoContext* l_pPangoContext = nullptr;
	PangoLayout* l_pPangoLayout = nullptr;
	l_pPangoContext = gtk_widget_get_pango_context(l_pWidget);
	l_pPangoLayout = pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	if (pango_parse_markup(rComment.getText().toASCIIString(), -1, 0, nullptr, nullptr, nullptr, nullptr))
	{
		pango_layout_set_markup(l_pPangoLayout, rComment.getText().toASCIIString(), -1);
	}
	else
	{
		pango_layout_set_text(l_pPangoLayout, rComment.getText().toASCIIString(), -1);
	}
	gdk_draw_layout(l_pWidget->window, l_pWidget->style->text_gc[GTK_WIDGET_STATE(l_pWidget)], xStart + xMargin, yStart + yMargin, l_pPangoLayout);
	g_object_unref(l_pPangoLayout);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);
}

void CInterfacedScenario::redraw(ILink& rLink)
{
	GtkWidget* l_pWidget = GTK_WIDGET(m_pScenarioDrawingArea);
	GdkGC* l_pStencilGC = gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	GdkGC* l_pDrawGC = gdk_gc_new(l_pWidget->window);

	CLinkProxy l_oLinkProxy(rLink);

	CIdentifier l_oSourceOutputTypeIdentifier;
	CIdentifier l_oTargetInputTypeIdentifier;

	m_rScenario.getBoxDetails(rLink.getSourceBoxIdentifier())->getOutputType(rLink.getSourceBoxOutputIndex(), l_oSourceOutputTypeIdentifier);
	m_rScenario.getBoxDetails(rLink.getTargetBoxIdentifier())->getInputType(rLink.getTargetBoxInputIndex(), l_oTargetInputTypeIdentifier);

	if (rLink.hasAttribute(OV_AttributeId_Link_Invalid))
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]);
	}
	else if (m_SelectedObjects.count(rLink.getIdentifier()))
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkSelected]);
	}
	else if (l_oTargetInputTypeIdentifier == l_oSourceOutputTypeIdentifier)
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_Link]);
	}
	else
	{
		if (m_rKernelContext.getTypeManager().isDerivedFromStream(l_oSourceOutputTypeIdentifier, l_oTargetInputTypeIdentifier))
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkDownCast]);
		}
		else if (m_rKernelContext.getTypeManager().isDerivedFromStream(l_oTargetInputTypeIdentifier, l_oSourceOutputTypeIdentifier))
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkUpCast]);
		}
		else
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]);
		}
	}

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_line(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC,
				  l_oLinkProxy.getXSource() + m_i32ViewOffsetX, l_oLinkProxy.getYSource() + m_i32ViewOffsetY,
				  l_oLinkProxy.getXTarget() + m_i32ViewOffsetX, l_oLinkProxy.getYTarget() + m_i32ViewOffsetY);
	gdk_draw_line(l_pWidget->window, l_pDrawGC,
				  l_oLinkProxy.getXSource() + m_i32ViewOffsetX, l_oLinkProxy.getYSource() + m_i32ViewOffsetY,
				  l_oLinkProxy.getXTarget() + m_i32ViewOffsetX, l_oLinkProxy.getYTarget() + m_i32ViewOffsetY);
	m_vInterfacedObject[m_ui32InterfacedObjectId] = CInterfacedObject(rLink.getIdentifier(), Box_Link, 0);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);
}

#undef updateStencilIndex

uint32_t CInterfacedScenario::pickInterfacedObject(const int x, const int y)
{
	if (!GDK_DRAWABLE(m_pStencilBuffer))
	{
		// m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return 0xffffffff;
	}

	int l_iMaxX;
	int l_iMaxY;
	uint32_t l_ui32InterfacedObjectId = 0xffffffff;
	gdk_drawable_get_size(GDK_DRAWABLE(m_pStencilBuffer), &l_iMaxX, &l_iMaxY);
	if (x >= 0 && y >= 0 && x < l_iMaxX && y < l_iMaxY)
	{
		GdkPixbuf* l_pPixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_pStencilBuffer), nullptr, x, y, 0, 0, 1, 1);
		if (!l_pPixbuf)
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		guchar* l_pPixels = gdk_pixbuf_get_pixels(l_pPixbuf);
		if (!l_pPixels)
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		l_ui32InterfacedObjectId = 0;
		l_ui32InterfacedObjectId += (l_pPixels[0] << 16);
		l_ui32InterfacedObjectId += (l_pPixels[1] << 8);
		l_ui32InterfacedObjectId += (l_pPixels[2]);
		g_object_unref(l_pPixbuf);
	}
	return l_ui32InterfacedObjectId;
}

bool CInterfacedScenario::pickInterfacedObject(const int x, const int y, int iSizeX, int iSizeY)
{
	if (!GDK_DRAWABLE(m_pStencilBuffer))
	{
		// m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return false;
	}

	int l_iMaxX;
	int l_iMaxY;
	gdk_drawable_get_size(GDK_DRAWABLE(m_pStencilBuffer), &l_iMaxX, &l_iMaxY);

	int iStartX = x;
	int iStartY = y;
	int iEndX = x + iSizeX;
	int iEndY = y + iSizeY;

	// crops according to drawing area boundings
	if (iStartX < 0) iStartX = 0;
	if (iStartY < 0) iStartY = 0;
	if (iEndX < 0) { iEndX = 0; }
	if (iEndY < 0) { iEndY = 0; }
	if (iStartX >= l_iMaxX - 1) { iStartX = l_iMaxX - 1; }
	if (iStartY >= l_iMaxY - 1) { iStartY = l_iMaxY - 1; }
	if (iEndX >= l_iMaxX - 1) { iEndX = l_iMaxX - 1; }
	if (iEndY >= l_iMaxY - 1) { iEndY = l_iMaxY - 1; }

	// recompute new size
	iSizeX = iEndX - iStartX + 1;
	iSizeY = iEndY - iStartY + 1;

	GdkPixbuf* l_pPixbuf = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(m_pStencilBuffer), nullptr, iStartX, iStartY, 0, 0, iSizeX, iSizeY);
	if (!l_pPixbuf)
	{
		m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
		return false;
	}

	guchar* l_pPixels = gdk_pixbuf_get_pixels(l_pPixbuf);
	if (!l_pPixels)
	{
		m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
		return false;
	}

	int l_iRowBytesCount = gdk_pixbuf_get_rowstride(l_pPixbuf);
	int l_iChannelCount = gdk_pixbuf_get_n_channels(l_pPixbuf);
	for (int j = 0; j < iSizeY; j++)
	{
		for (int i = 0; i < iSizeX; ++i)
		{
			uint32_t l_ui32InterfacedObjectId = 0;
			l_ui32InterfacedObjectId += (l_pPixels[j * l_iRowBytesCount + i * l_iChannelCount + 0] << 16);
			l_ui32InterfacedObjectId += (l_pPixels[j * l_iRowBytesCount + i * l_iChannelCount + 1] << 8);
			l_ui32InterfacedObjectId += (l_pPixels[j * l_iRowBytesCount + i * l_iChannelCount + 2]);
			if (m_vInterfacedObject[l_ui32InterfacedObjectId].m_oIdentifier != OV_UndefinedIdentifier)
			{
				m_SelectedObjects.insert(m_vInterfacedObject[l_ui32InterfacedObjectId].m_oIdentifier);
			}
		}
	}

	g_object_unref(l_pPixbuf);
	return true;
}

#define OV_ClassId_Selected OpenViBE::CIdentifier(0xC67A01DC, 0x28CE06C1)

void CInterfacedScenario::undoCB(const bool bManageModifiedStatusFlag)
{
	// When a box gets updated we generate a snapshot beforehand to enable undo in all cases
	// This will result in two indentical undo states, in order to avoid weird Redo, we drop the
	// reduntant state at this moment
	bool shouldDropLastState = false;
	if (m_rScenario.containsBoxWithDeprecatedInterfacors())
	{
		shouldDropLastState = true;
	}

	if (m_oStateStack->undo())
	{
		CIdentifier l_oIdentifier;
		m_SelectedObjects.clear();
		while ((l_oIdentifier = m_rScenario.getNextBoxIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getBoxDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
			{
				m_SelectedObjects.insert(l_oIdentifier);
			}
		}
		while ((l_oIdentifier = m_rScenario.getNextLinkIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getLinkDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
			{
				m_SelectedObjects.insert(l_oIdentifier);
			}
		}

		if (m_pDesignerVisualization) { m_pDesignerVisualization->load(); }
		if (bManageModifiedStatusFlag) { m_bHasBeenModified = true; }

		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();

		if (shouldDropLastState) { m_oStateStack->dropLastState(); }

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), m_oStateStack->isRedoPossible());
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo")), m_oStateStack->isUndoPossible());
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Can not undo\n";
		GtkWidget* l_pUndoButton = GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo"));
		gtk_widget_set_sensitive(l_pUndoButton, false);
	}
}

void CInterfacedScenario::redoCB(const bool bManageModifiedStatusFlag)
{
	if (m_oStateStack->redo())
	{
		CIdentifier l_oIdentifier;
		m_SelectedObjects.clear();
		while ((l_oIdentifier = m_rScenario.getNextBoxIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getBoxDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
			{
				m_SelectedObjects.insert(l_oIdentifier);
			}
		}
		while ((l_oIdentifier = m_rScenario.getNextLinkIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			if (m_rScenario.getLinkDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
			{
				m_SelectedObjects.insert(l_oIdentifier);
			}
		}

		if (m_pDesignerVisualization)
		{
			m_pDesignerVisualization->load();
		}

		if (bManageModifiedStatusFlag)
		{
			m_bHasBeenModified = true;
		}
		this->redrawScenarioSettings();
		this->redrawScenarioInputSettings();
		this->redrawScenarioOutputSettings();

		this->redraw();
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), m_oStateStack->isRedoPossible());
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo")), m_oStateStack->isUndoPossible());
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), false);
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Can not redo\n";
	}
}

void CInterfacedScenario::snapshotCB(const bool bManageModifiedStatusFlag)
{
	if (m_rScenario.containsBoxWithDeprecatedInterfacors())
	{
		OV_WARNING("Scenario containing boxes with deprecated I/O or Settings does not support undo", m_rKernelContext.getLogManager());
	}
	else
	{
		CIdentifier l_oIdentifier;

		while ((l_oIdentifier = m_rScenario.getNextBoxIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(l_oIdentifier))
			{
				m_rScenario.getBoxDetails(l_oIdentifier)->addAttribute(OV_ClassId_Selected, "");
			}
			else
			{
				m_rScenario.getBoxDetails(l_oIdentifier)->removeAttribute(OV_ClassId_Selected);
			}
		}
		while ((l_oIdentifier = m_rScenario.getNextLinkIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			if (m_SelectedObjects.count(l_oIdentifier))
				m_rScenario.getLinkDetails(l_oIdentifier)->addAttribute(OV_ClassId_Selected, "");
			else
				m_rScenario.getLinkDetails(l_oIdentifier)->removeAttribute(OV_ClassId_Selected);
		}

		if (bManageModifiedStatusFlag)
		{
			m_bHasBeenModified = true;
		}
		this->updateScenarioLabel();
		m_oStateStack->snapshot();
	}
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_redo")), m_oStateStack->isRedoPossible());
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(this->m_rApplication.m_pBuilderInterface, "openvibe-button_undo")), m_oStateStack->isUndoPossible());
}

void CInterfacedScenario::addCommentCB(int x, int y)
{
	CIdentifier l_oIdentifier;
	m_rScenario.addComment(l_oIdentifier, OV_UndefinedIdentifier);
	if (x == -1 || y == -1)
	{
		GtkWidget* l_pScrolledWindow = gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(m_pScenarioDrawingArea)));
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

	CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, l_oIdentifier);
	l_oCommentProxy.setCenter(x - m_i32ViewOffsetX, y - m_i32ViewOffsetY);

	// Aligns comemnts on grid
	l_oCommentProxy.setCenter(int32_t((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0L), int32_t((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0L));

	// Applies modifications before snapshot
	l_oCommentProxy.apply();

	CCommentEditorDialog l_oCommentEditorDialog(m_rKernelContext, *m_rScenario.getCommentDetails(l_oIdentifier), m_sGUIFilename.c_str());
	if (!l_oCommentEditorDialog.run())
	{
		m_rScenario.removeComment(l_oIdentifier);
	}
	else
	{
		m_SelectedObjects.clear();
		m_SelectedObjects.insert(l_oIdentifier);

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

	gint l_iResponse = gtk_dialog_run(GTK_DIALOG(m_pConfigureSettingsDialog));

	if (l_iResponse == GTK_RESPONSE_CANCEL) { this->undoCB(false); }
	else { this->snapshotCB(); }

	gtk_widget_hide(m_pConfigureSettingsDialog);
	this->redrawScenarioSettings();
}

void CInterfacedScenario::addScenarioSettingCB()

{
	char l_sName[1024];
	sprintf(l_sName, "Setting %u", m_rScenario.getSettingCount() + 1);
	m_rScenario.addSetting(l_sName, OVTK_TypeId_Integer, "0", OV_Value_UndefinedIndexUInt, false, m_rScenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));

	this->redrawConfigureScenarioSettingsDialog();
}

void CInterfacedScenario::addScenarioInputCB()

{
	char l_sName[1024];
	sprintf(l_sName, "Input %u", m_rScenario.getInputCount() + 1);

	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the inputs of the box.
	m_rScenario.addInput(l_sName, OVTK_TypeId_StreamedMatrix, m_rScenario.getUnusedInputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor l_oConnectorEditor(m_rKernelContext, m_rScenario, Box_Input, m_rScenario.getInputCount() - 1, "Add Input", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run())
	{
		this->snapshotCB();
	}
	else
	{
		m_rScenario.removeInput(m_rScenario.getInputCount() - 1);
	}

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::editScenarioInputCB(const unsigned int l_ui32InputIndex)

{
	CConnectorEditor l_oConnectorEditor(m_rKernelContext, m_rScenario, Box_Input, l_ui32InputIndex, "Edit Input", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }

	this->redrawScenarioInputSettings();
}

void CInterfacedScenario::addScenarioOutputCB()

{
	char l_sName[1024];
	sprintf(l_sName, "Output %u", m_rScenario.getOutputCount() + 1);

	// scenario I/O are identified by name/type combination value, at worst uniq in the scope of the outputs of the box.
	m_rScenario.addOutput(l_sName, OVTK_TypeId_StreamedMatrix, m_rScenario.getUnusedOutputIdentifier(OV_UndefinedIdentifier));

	CConnectorEditor l_oConnectorEditor(m_rKernelContext, m_rScenario, Box_Output, m_rScenario.getOutputCount() - 1, "Add Output", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }
	else { m_rScenario.removeOutput(m_rScenario.getOutputCount() - 1); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::editScenarioOutputCB(const unsigned int l_ui32OutputIndex)

{
	CConnectorEditor l_oConnectorEditor(m_rKernelContext, m_rScenario, Box_Output, l_ui32OutputIndex, "Edit Output", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run()) { this->snapshotCB(); }

	this->redrawScenarioOutputSettings();
}

void CInterfacedScenario::swapScenarioSettings(const unsigned int uiSettingAIndex, const unsigned int uiSettingBIndex)
{
	m_rScenario.swapSettings(uiSettingAIndex, uiSettingBIndex);
	this->redrawConfigureScenarioSettingsDialog();
}


void CInterfacedScenario::swapScenarioInputs(const unsigned int ui32InputAIndex, const unsigned int ui32InputBIndex)
{
	CIdentifier l_oABoxIdentifier;
	unsigned int l_ui32ABoxInputIndex;
	CIdentifier l_oBBoxIdentifier;
	unsigned int l_ui32BBoxInputIndex;

	m_rScenario.getScenarioInputLink(ui32InputAIndex, l_oABoxIdentifier, l_ui32ABoxInputIndex);
	m_rScenario.getScenarioInputLink(ui32InputBIndex, l_oBBoxIdentifier, l_ui32BBoxInputIndex);

	m_rScenario.swapInputs(ui32InputAIndex, ui32InputBIndex);

	m_rScenario.setScenarioInputLink(ui32InputBIndex, l_oABoxIdentifier, l_ui32ABoxInputIndex);
	m_rScenario.setScenarioInputLink(ui32InputAIndex, l_oBBoxIdentifier, l_ui32BBoxInputIndex);

	this->redrawScenarioInputSettings();
	this->redraw();
}

void CInterfacedScenario::swapScenarioOutputs(const unsigned int ui32OutputAIndex, const unsigned int ui32OutputBIndex)
{
	CIdentifier l_oABoxIdentifier;
	unsigned int l_ui32ABoxOutputIndex;
	CIdentifier l_oBBoxIdentifier;
	unsigned int l_ui32BBoxOutputIndex;

	m_rScenario.getScenarioOutputLink(ui32OutputAIndex, l_oABoxIdentifier, l_ui32ABoxOutputIndex);
	m_rScenario.getScenarioOutputLink(ui32OutputBIndex, l_oBBoxIdentifier, l_ui32BBoxOutputIndex);

	m_rScenario.swapOutputs(ui32OutputAIndex, ui32OutputBIndex);

	m_rScenario.setScenarioOutputLink(ui32OutputBIndex, l_oABoxIdentifier, l_ui32ABoxOutputIndex);
	m_rScenario.setScenarioOutputLink(ui32OutputAIndex, l_oBBoxIdentifier, l_ui32BBoxOutputIndex);

	this->redrawScenarioOutputSettings();
	this->redraw();
}

void CInterfacedScenario::scenarioDrawingAreaExposeCB(GdkEventExpose* /*pEvent*/)
{
	if (m_ui32CurrentMode == Mode_None)
	{
		gint l_iViewportX = -1;
		gint l_iViewportY = -1;

		gint l_iMinX = 0x7fff;
		gint l_iMaxX = -0x7fff;
		gint l_iMinY = 0x7fff;
		gint l_iMaxY = -0x7fff;

		const gint l_iMarginX = gint(round(32.0 * m_f64CurrentScale));
		const gint l_iMarginY = gint(round(32.0 * m_f64CurrentScale));

		CIdentifier l_oBoxIdentifier;
		while ((l_oBoxIdentifier = m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
		{
			//CBoxProxy l_oBoxProxy(m_rKernelContext, *m_rScenario.getBoxDetails(l_oBoxIdentifier));
			CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, l_oBoxIdentifier);
			l_iMinX = std::min(l_iMinX, gint((l_oBoxProxy.getXCenter() - 1.0 * l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
			l_iMaxX = std::max(l_iMaxX, gint((l_oBoxProxy.getXCenter() + 1.0 * l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
			l_iMinY = std::min(l_iMinY, gint((l_oBoxProxy.getYCenter() - 1.0 * l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
			l_iMaxY = std::max(l_iMaxY, gint((l_oBoxProxy.getYCenter() + 1.0 * l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
		}

		CIdentifier l_oCommentIdentifier;
		while ((l_oCommentIdentifier = m_rScenario.getNextCommentIdentifier(l_oCommentIdentifier)) != OV_UndefinedIdentifier)
		{
			CCommentProxy l_oCommentProxy(m_rKernelContext, *m_rScenario.getCommentDetails(l_oCommentIdentifier));
			l_iMinX = std::min(l_iMinX, gint((l_oCommentProxy.getXCenter() - 1.0 * l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
			l_iMaxX = std::max(l_iMaxX, gint((l_oCommentProxy.getXCenter() + 1.0 * l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
			l_iMinY = std::min(l_iMinY, gint((l_oCommentProxy.getYCenter() - 1.0 * l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
			l_iMaxY = std::max(l_iMaxY, gint((l_oCommentProxy.getYCenter() + 1.0 * l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) / 2) * m_f64CurrentScale));
		}

		const gint l_iNewScenarioSizeX = l_iMaxX - l_iMinX;
		const gint l_iNewScenarioSizeY = l_iMaxY - l_iMinY;
		gint l_iOldScenarioSizeX = -1;
		gint l_iOldScenarioSizeY = -1;

		gdk_window_get_size(GTK_WIDGET(m_pScenarioViewport)->window, &l_iViewportX, &l_iViewportY);
		gtk_widget_get_size_request(GTK_WIDGET(m_pScenarioDrawingArea), &l_iOldScenarioSizeX, &l_iOldScenarioSizeY);

		if (l_iNewScenarioSizeX >= 0 && l_iNewScenarioSizeY >= 0)
		{
			if (l_iOldScenarioSizeX != l_iNewScenarioSizeX + 2 * l_iMarginX || l_iOldScenarioSizeY != l_iNewScenarioSizeY + 2 * l_iMarginY)
			{
				gtk_widget_set_size_request(GTK_WIDGET(m_pScenarioDrawingArea), l_iNewScenarioSizeX + 2 * l_iMarginX, l_iNewScenarioSizeY + 2 * l_iMarginY);
			}
			m_i32ViewOffsetX = std::min(m_i32ViewOffsetX, -l_iMaxX - l_iMarginX + std::max(l_iViewportX, l_iNewScenarioSizeX + 2 * l_iMarginX));
			m_i32ViewOffsetX = std::max(m_i32ViewOffsetX, -l_iMinX + l_iMarginX);
			m_i32ViewOffsetY = std::min(m_i32ViewOffsetY, -l_iMaxY - l_iMarginY + std::max(l_iViewportY, l_iNewScenarioSizeY + 2 * l_iMarginY));
			m_i32ViewOffsetY = std::max(m_i32ViewOffsetY, -l_iMinY + l_iMarginY);
		}
	}

	gint x, y;

	gdk_window_get_size(GTK_WIDGET(m_pScenarioDrawingArea)->window, &x, &y);
	if (m_pStencilBuffer) { g_object_unref(m_pStencilBuffer); }
	m_pStencilBuffer = gdk_pixmap_new(GTK_WIDGET(m_pScenarioDrawingArea)->window, x, y, -1);

	GdkGC* l_pStencilGC = gdk_gc_new(m_pStencilBuffer);
	GdkColor l_oColor = { 0, 0, 0, 0 };
	gdk_gc_set_rgb_fg_color(l_pStencilGC, &l_oColor);
	gdk_draw_rectangle(GDK_DRAWABLE(m_pStencilBuffer), l_pStencilGC, TRUE, 0, 0, x, y);
	g_object_unref(l_pStencilGC);

	if (this->isLocked())
	{
		l_oColor.pixel = 0;
		l_oColor.red = 0x0f00;
		l_oColor.green = 0x0f00;
		l_oColor.blue = 0x0f00;

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
	m_ui32InterfacedObjectId = 0;
	m_vInterfacedObject.clear();

	uint32_t l_ui32CommentCount = 0;
	CIdentifier l_oCommentIdentifier;
	while ((l_oCommentIdentifier = m_rScenario.getNextCommentIdentifier(l_oCommentIdentifier)) != OV_UndefinedIdentifier)
	{
		redraw(*m_rScenario.getCommentDetails(l_oCommentIdentifier));
		l_ui32CommentCount++;
	}
	m_ui32CommentCount = l_ui32CommentCount;

	uint32_t l_ui32BoxCount = 0;
	CIdentifier l_oBoxIdentifier;
	while ((l_oBoxIdentifier = m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
	{
		redraw(*m_rScenario.getBoxDetails(l_oBoxIdentifier));
		l_ui32BoxCount++;
	}
	m_ui32BoxCount = l_ui32BoxCount;

	uint32_t l_ui32LinkCount = 0;
	CIdentifier l_oLinkIdentifier;
	while ((l_oLinkIdentifier = m_rScenario.getNextLinkIdentifier(l_oLinkIdentifier)) != OV_UndefinedIdentifier)
	{
		redraw(*m_rScenario.getLinkDetails(l_oLinkIdentifier));
		l_ui32LinkCount++;
	}
	m_ui32LinkCount = l_ui32LinkCount;

	if (m_ui32CurrentMode == Mode_Selection || m_ui32CurrentMode == Mode_SelectionAdd)
	{
		int l_iStartX = int(std::min(m_f64PressMouseX, m_f64CurrentMouseX));
		int l_iStartY = int(std::min(m_f64PressMouseY, m_f64CurrentMouseY));
		int l_iSizeX = int(std::max(m_f64PressMouseX - m_f64CurrentMouseX, m_f64CurrentMouseX - m_f64PressMouseX));
		int l_iSizeY = int(std::max(m_f64PressMouseY - m_f64CurrentMouseY, m_f64CurrentMouseY - m_f64PressMouseY));

		GtkWidget* l_pWidget = GTK_WIDGET(m_pScenarioDrawingArea);
		GdkGC* l_pDrawGC = gdk_gc_new(l_pWidget->window);
		gdk_gc_set_function(l_pDrawGC, GDK_OR);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_SelectionArea]);
		gdk_draw_rectangle(l_pWidget->window, l_pDrawGC, TRUE, l_iStartX, l_iStartY, l_iSizeX, l_iSizeY);
		gdk_gc_set_function(l_pDrawGC, GDK_COPY);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_SelectionAreaBorder]);
		gdk_draw_rectangle(l_pWidget->window, l_pDrawGC, FALSE, l_iStartX, l_iStartY, l_iSizeX, l_iSizeY);
		g_object_unref(l_pDrawGC);
	}

	if (m_ui32CurrentMode == Mode_Connect)
	{
		GtkWidget* l_pWidget = GTK_WIDGET(m_pScenarioDrawingArea);
		GdkGC* l_pDrawGC = gdk_gc_new(l_pWidget->window);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_Link]);
		gdk_draw_line(l_pWidget->window, l_pDrawGC, int(m_f64PressMouseX), int(m_f64PressMouseY), int(m_f64CurrentMouseX), int(m_f64CurrentMouseY));
		g_object_unref(l_pDrawGC);
	}
}

// This method inserts a box into the scenario upon receiving data
void CInterfacedScenario::scenarioDrawingAreaDragDataReceivedCB(GdkDragContext* pDragContext, gint iX, gint iY, GtkSelectionData* pSelectionData, guint /*uiInfo*/, guint /*uiT*/)
{
	if (this->isLocked()) return;

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

		IBox* l_pBox = nullptr;
		const IPluginObjectDesc* l_pPOD = nullptr;

		if (l_oBoxAlgorithmClassIdentifier == OV_UndefinedIdentifier)
		{
			m_f64CurrentMouseX = iX;
			m_f64CurrentMouseY = iY;
			return;
		}
		if (l_oBoxAlgorithmClassIdentifier == OVP_ClassId_BoxAlgorithm_Metabox)
		{
			// extract the name of the metabox from the drag data string
			CIdentifier metaboxId;
			metaboxId.fromString(CString(l_sSelectionData.substr(l_sSelectionData.find(')') + 1).c_str()));

			//m_rKernelContext.getLogManager() << LogLevel_Info << "This is a metabox with ID " << l_sMetaboxIdentifier.c_str() << "\n";
			l_pPOD = m_rKernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId);

			// insert a box into the scenario, initialize it from the proxy-descriptor from the metabox loader
			m_rScenario.addBox(l_oBoxIdentifier, *static_cast<const IBoxAlgorithmDesc*>(l_pPOD), OV_UndefinedIdentifier);

			l_pBox = m_rScenario.getBoxDetails(l_oBoxIdentifier);
			l_pBox->addAttribute(OVP_AttributeId_Metabox_Identifier, metaboxId.toString());
		}
		else
		{
			m_rScenario.addBox(l_oBoxIdentifier, l_oBoxAlgorithmClassIdentifier, OV_UndefinedIdentifier);

			l_pBox = m_rScenario.getBoxDetails(l_oBoxIdentifier);
			CIdentifier l_oId = l_pBox->getAlgorithmClassIdentifier();
			l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);
		}

		m_SelectedObjects.clear();
		m_SelectedObjects.insert(l_oBoxIdentifier);

		// If a visualization box was dropped, add it in window manager
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_pDesignerVisualization)
			{
				m_pDesignerVisualization->onVisualizationBoxAdded(l_pBox);
			}
		}

		CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, l_oBoxIdentifier);
		l_oBoxProxy.setCenter(iX - m_i32ViewOffsetX, iY - m_i32ViewOffsetY);
		// Aligns boxes on grid
		l_oBoxProxy.setCenter(int32_t((l_oBoxProxy.getXCenter() + 8) & 0xfffffff0L), int32_t((l_oBoxProxy.getYCenter() + 8) & 0xfffffff0L));

		// Applies modifications before snapshot
		l_oBoxProxy.apply();

		this->snapshotCB();

		m_f64CurrentMouseX = iX;
		m_f64CurrentMouseY = iY;
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
		std::string l_sDraggedFilesPath((char*)gtk_selection_data_get_data(pSelectionData));
		std::stringstream l_oStringStream(l_sDraggedFilesPath);
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

		for (auto& fileName : l_vFilesToOpen)
		{
			m_rApplication.openScenario(fileName.c_str());
		}
	}
#endif
}

void CInterfacedScenario::scenarioDrawingAreaMotionNotifyCB(GtkWidget* /*pWidget*/, GdkEventMotion* pEvent)
{
	// m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaMotionNotifyCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* l_pTooltip = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "tooltip"));
	gtk_widget_set_name(l_pTooltip, "gtk-tooltips");
	const uint32_t l_ui32InterfacedObjectId = pickInterfacedObject(int(pEvent->x), int(pEvent->y));
	CInterfacedObject& l_rObject = m_vInterfacedObject[l_ui32InterfacedObjectId];
	if (l_rObject.m_oIdentifier != OV_UndefinedIdentifier
		&& l_rObject.m_ui32ConnectorType != Box_Link
		&& l_rObject.m_ui32ConnectorType != Box_None)
	{
		IBox* l_pBoxDetails = m_rScenario.getBoxDetails(l_rObject.m_oIdentifier);
		if (l_pBoxDetails)
		{
			CString l_sName;
			CString l_sType;
			if (l_rObject.m_ui32ConnectorType == Box_Input)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_ui32ConnectorIndex, l_oType);
				l_sType = m_rKernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_ui32ConnectorType == Box_Output)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_ui32ConnectorIndex, l_oType);
				l_sType = m_rKernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_ui32ConnectorType == Box_Update)
			{
				//m_rScenario.updateBox(l_pBoxDetails->getIdentifier());
				l_sName = CString("Right click for");
				l_sType = "box update";
			}
			else if (l_rObject.m_ui32ConnectorType == Box_ScenarioInput)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_ui32ConnectorIndex, l_oType);

				for (uint32_t l_ui32ScenarioInputIndex = 0; l_ui32ScenarioInputIndex < m_rScenario.getInputCount(); l_ui32ScenarioInputIndex++)
				{
					CIdentifier l_oScenarioInputLinkBoxIdentifier;
					uint32_t l_ui32ScenarioInputLinkBoxInputIndex;

					m_rScenario.getScenarioInputLink(l_ui32ScenarioInputIndex, l_oScenarioInputLinkBoxIdentifier, l_ui32ScenarioInputLinkBoxInputIndex);

					if (l_oScenarioInputLinkBoxIdentifier == l_pBoxDetails->getIdentifier() && l_ui32ScenarioInputLinkBoxInputIndex == l_rObject.m_ui32ConnectorIndex)
					{
						m_rScenario.getInputName(l_ui32ScenarioInputIndex, l_sName);
						l_sName = CString("Connected to \n") + l_sName;
						m_rScenario.getInputType(l_ui32ScenarioInputIndex, l_oType);
					}
				}
				l_sType = m_rKernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}
			else if (l_rObject.m_ui32ConnectorType == Box_ScenarioOutput)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_ui32ConnectorIndex, l_oType);

				for (uint32_t l_ui32ScenarioOutputIndex = 0; l_ui32ScenarioOutputIndex < m_rScenario.getOutputCount(); l_ui32ScenarioOutputIndex++)
				{
					CIdentifier l_oScenarioOutputLinkBoxIdentifier;
					uint32_t l_ui32ScenarioOutputLinkBoxOutputIndex;

					m_rScenario.getScenarioOutputLink(l_ui32ScenarioOutputIndex, l_oScenarioOutputLinkBoxIdentifier, l_ui32ScenarioOutputLinkBoxOutputIndex);

					if (l_oScenarioOutputLinkBoxIdentifier == l_pBoxDetails->getIdentifier() && l_ui32ScenarioOutputLinkBoxOutputIndex == l_rObject.m_ui32ConnectorIndex)
					{
						m_rScenario.getOutputName(l_ui32ScenarioOutputIndex, l_sName);
						l_sName = CString("Connected to \n") + l_sName;
						m_rScenario.getOutputType(l_ui32ScenarioOutputIndex, l_oType);
					}
				}
				l_sType = m_rKernelContext.getTypeManager().getTypeName(l_oType);
				l_sType = CString("[") + l_sType + CString("]");
			}

			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_pGUIBuilder, "tooltip-label_name_content")), l_sName);
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_pGUIBuilder, "tooltip-label_type_content")), l_sType);
			gtk_window_move(GTK_WINDOW(l_pTooltip), gint(pEvent->x_root), gint(pEvent->y_root) + 40);
			gtk_widget_show(l_pTooltip);
		}
	}
	else
	{
		gtk_widget_hide(l_pTooltip);
	}

	if (m_ui32CurrentMode != Mode_None)
	{
		if (m_ui32CurrentMode == Mode_MoveScenario)
		{
			m_i32ViewOffsetX += int32_t(pEvent->x - m_f64CurrentMouseX);
			m_i32ViewOffsetY += int32_t(pEvent->y - m_f64CurrentMouseY);
		}
		else if (m_ui32CurrentMode == Mode_MoveSelection)
		{
			if (m_bControlPressed)
			{
				m_SelectedObjects.insert(m_oCurrentObject.m_oIdentifier);
			}
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
					CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, objectId);
					l_oBoxProxy.setCenter(l_oBoxProxy.getXCenter() + int32_t(pEvent->x - m_f64CurrentMouseX),
										  l_oBoxProxy.getYCenter() + int32_t(pEvent->y - m_f64CurrentMouseY));
				}
				if (m_rScenario.isComment(objectId))
				{
					CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, objectId);
					l_oCommentProxy.setCenter(l_oCommentProxy.getXCenter() + int32_t(pEvent->x - m_f64CurrentMouseX),
											  l_oCommentProxy.getYCenter() + int32_t(pEvent->y - m_f64CurrentMouseY));
				}
			}
		}

		this->redraw();
	}
	m_f64CurrentMouseX = pEvent->x;
	m_f64CurrentMouseY = pEvent->y;
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
}  // namespace

GtkImageMenuItem* CInterfacedScenario::gtk_menu_add_new_image_menu_item_with_cb_generic(GtkMenu* menu, const char* icon, const char* label, menu_callback_function cb, IBox* cb_box, uint32_t cb_command, uint32_t cb_index, uint32_t cb_index2)
{
	GtkImageMenuItem* menuitem = gtk_menu_add_new_image_menu_item(menu, icon, label);
	BoxContextMenuCB l_oBoxContextMenuCB;
	l_oBoxContextMenuCB.ui32Command = cb_command;
	l_oBoxContextMenuCB.ui32Index = cb_index;
	l_oBoxContextMenuCB.ui32SecondaryIndex = cb_index2;
	l_oBoxContextMenuCB.pBox = cb_box;
	l_oBoxContextMenuCB.pInterfacedScenario = this;
	const auto l_ui32MapIndex = uint32_t(m_vBoxContextMenuCB.size());
	m_vBoxContextMenuCB[l_ui32MapIndex] = l_oBoxContextMenuCB;
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(cb), &m_vBoxContextMenuCB[l_ui32MapIndex]);
	return menuitem;
}

void CInterfacedScenario::scenarioDrawingAreaButtonPressedCB(GtkWidget* pWidget, GdkEventButton* pEvent)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonPressedCB\n";

	if (this->isLocked()) { return; }

	GtkWidget* l_pTooltip = GTK_WIDGET(gtk_builder_get_object(m_pGUIBuilder, "tooltip"));
	gtk_widget_hide(l_pTooltip);
	gtk_widget_grab_focus(pWidget);

	m_bButtonPressed |= ((pEvent->type == GDK_BUTTON_PRESS) && (pEvent->button == 1));
	m_f64PressMouseX = pEvent->x;
	m_f64PressMouseY = pEvent->y;

	uint32_t l_ui32InterfacedObjectId = pickInterfacedObject(int(m_f64PressMouseX), int(m_f64PressMouseY));
	m_oCurrentObject = m_vInterfacedObject[l_ui32InterfacedObjectId];

	if (pEvent->button == 1)
	{
		if (pEvent->type == GDK_BUTTON_PRESS)
		{
			if (m_oCurrentObject.m_oIdentifier == OV_UndefinedIdentifier)
			{
				if (m_bShiftPressed)
				{
					m_ui32CurrentMode = Mode_MoveScenario;
				}
				else
				{
					if (m_bControlPressed) { m_ui32CurrentMode = Mode_SelectionAdd; }
					else { m_ui32CurrentMode = Mode_Selection; }
				}
			}
			else
			{
				if (m_oCurrentObject.m_ui32ConnectorType == Box_Input || m_oCurrentObject.m_ui32ConnectorType == Box_Output)
				{
					m_ui32CurrentMode = Mode_Connect;
				}
				else
				{
					m_ui32CurrentMode = Mode_MoveSelection;
					if (m_bControlPressed)
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
		else if (pEvent->type == GDK_2BUTTON_PRESS)
		{
			if (m_oCurrentObject.m_oIdentifier != OV_UndefinedIdentifier)
			{
				m_ui32CurrentMode = Mode_EditSettings;
				m_bShiftPressed = false;
				m_bControlPressed = false;
				m_bAltPressed = false;
				m_bAPressed = false;
				m_bWPressed = false;

				if (m_oCurrentObject.m_ui32ConnectorType == Box_Input || m_oCurrentObject.m_ui32ConnectorType == Box_Output)
				{
					IBox* l_pBox = m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
					if (l_pBox)
					{
						if ((m_oCurrentObject.m_ui32ConnectorType == Box_Input && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
							|| (m_oCurrentObject.m_ui32ConnectorType == Box_Output && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)))
						{
							CConnectorEditor l_oConnectorEditor(m_rKernelContext, *l_pBox, m_oCurrentObject.m_ui32ConnectorType, m_oCurrentObject.m_ui32ConnectorIndex, m_oCurrentObject.m_ui32ConnectorType == Box_Input ? "Edit Input" : "Edit Output", m_sGUIFilename.c_str());
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
							CBoxConfigurationDialog l_oBoxConfigurationDialog(m_rKernelContext, *l_pBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str(), false);
							if (l_oBoxConfigurationDialog.run()) { this->snapshotCB(); }
						}
					}
					if (m_rScenario.isComment(m_oCurrentObject.m_oIdentifier))
					{
						IComment* l_pComment = m_rScenario.getCommentDetails(m_oCurrentObject.m_oIdentifier);
						if (l_pComment)
						{
							CCommentEditorDialog l_oCommentEditorDialog(m_rKernelContext, *l_pComment, m_sGUIFilename.c_str());
							if (l_oCommentEditorDialog.run()) { this->snapshotCB(); }
						}
					}
				}
			}
		}
	}
	else if (pEvent->button == 3) // right click
	{
		if (pEvent->type == GDK_BUTTON_PRESS)
		{
			const auto unused = uint32_t(-1);
			GtkMenu* l_pMenu = GTK_MENU(gtk_menu_new());
			m_vBoxContextMenuCB.clear();

			// -------------- SELECTION -----------

			if (this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_CUT, "cut", context_menu_cb, nullptr, ContextMenu_SelectionCut, unused); }
			if (this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_COPY, "copy", context_menu_cb, nullptr, ContextMenu_SelectionCopy, unused); }
			if ((m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
				|| (m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier))
			{
				gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_PASTE, "paste", context_menu_cb, nullptr, ContextMenu_SelectionPaste, unused);
			}
			if (this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_DELETE, "delete", context_menu_cb, nullptr, ContextMenu_SelectionDelete, unused); }

			if (m_oCurrentObject.m_oIdentifier != OV_UndefinedIdentifier && m_rScenario.isBox(m_oCurrentObject.m_oIdentifier))
			{
				IBox* l_pBox = m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
				if (l_pBox)
				{
					uint32_t i, j;
					char l_sCompleteName[1024];

					if (!m_vBoxContextMenuCB.empty()) { gtk_menu_add_separator_menu_item(l_pMenu); }

					bool l_bFlagToBeUpdated = l_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated);
					bool l_bFlagPendingDeprecatedInterfacors = l_pBox->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors);

					// -------------- INPUTS --------------
					bool l_bFlagCanAddInput = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput);
					bool l_bFlagCanModifyInput = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput);
					bool l_bCanConnectScenarioInput = (l_pBox->getInputCount() > 0 && m_rScenario.getInputCount() > 0);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (l_bFlagCanAddInput || l_bFlagCanModifyInput || l_bCanConnectScenarioInput))
					{
						uint32_t l_ui32FixedInputCount = 0;
						sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialInputCount).toASCIIString(), "%d", &l_ui32FixedInputCount);
						GtkMenu* l_pMenuInput = GTK_MENU(gtk_menu_new());
						GtkImageMenuItem* l_pMenuItemInput = gtk_menu_add_new_image_menu_item(l_pMenu, GTK_STOCK_PROPERTIES, "inputs");
						for (i = 0; i < l_pBox->getInputCount(); ++i)
						{
							CString l_sName;
							CIdentifier l_oType;
							CIdentifier l_oIdentifier;
							l_pBox->getInputName(i, l_sName);
							l_pBox->getInputType(i, l_oType);
							l_oIdentifier = l_pBox->getIdentifier();
							sprintf(l_sCompleteName, "%i : %s", int(i + 1), l_sName.toASCIIString());
							GtkImageMenuItem* l_pMenuInputMenuItem = gtk_menu_add_new_image_menu_item(l_pMenuInput, GTK_STOCK_PROPERTIES, l_sCompleteName);

							GtkMenu* l_pMenuInputMenuAction = GTK_MENU(gtk_menu_new());

							if (l_bCanConnectScenarioInput)
							{
								for (j = 0; j < m_rScenario.getInputCount(); j++)
								{
									char l_sScenarioInputNameComplete[1024];
									CString l_sScenarioInputName;
									CIdentifier l_oScenarioLinkBoxIdentifier;
									CIdentifier l_oScenarioInputType;
									auto l_ui32ScenarioLinkInputIndex = uint32_t(-1);
									m_rScenario.getInputName(j, l_sScenarioInputName);
									m_rScenario.getInputType(j, l_oScenarioInputType);
									m_rScenario.getScenarioInputLink(j, l_oScenarioLinkBoxIdentifier, l_ui32ScenarioLinkInputIndex);
									sprintf(l_sScenarioInputNameComplete, "%u: %s", j + 1, l_sScenarioInputName.toASCIIString());
									if (l_oScenarioLinkBoxIdentifier == l_oIdentifier && l_ui32ScenarioLinkInputIndex == i)
									{
										gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuInputMenuAction, GTK_STOCK_DISCONNECT, (CString("disconnect from ") + CString(l_sScenarioInputNameComplete)).toASCIIString(), context_menu_cb, l_pBox, ContextMenu_BoxDisconnectScenarioInput, i, j);
									}
									else
									{
										if (m_rKernelContext.getTypeManager().isDerivedFromStream(l_oScenarioInputType, l_oType))
										{
											gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuInputMenuAction, GTK_STOCK_CONNECT, (CString("connect to ") + CString(l_sScenarioInputNameComplete)).toASCIIString(), context_menu_cb, l_pBox, ContextMenu_BoxConnectScenarioInput, i, j);
										}
									}
								}
							}

							if (l_bFlagCanModifyInput)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuInputMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditInput, i);
							}

							if (l_bFlagCanAddInput && l_ui32FixedInputCount <= i)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuInputMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox, ContextMenu_BoxRemoveInput, i);
							}

							if (gtk_container_get_children_count(GTK_CONTAINER(l_pMenuInputMenuAction)) > 0)
							{
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuInputMenuItem), GTK_WIDGET(l_pMenuInputMenuAction));
							}
							else
							{
								gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuInputMenuItem), false);
							}
						}
						gtk_menu_add_separator_menu_item(l_pMenuInput);
						if (l_bFlagCanAddInput)
						{
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenuInput, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddInput, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemInput), GTK_WIDGET(l_pMenuInput));
					}

					// -------------- OUTPUTS --------------

					bool l_bFlagCanAddOutput = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput);
					bool l_bFlagCanModifyOutput = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput);
					bool l_bCanConnectScenarioOutput = (l_pBox->getOutputCount() > 0 && m_rScenario.getOutputCount() > 0);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (l_bFlagCanAddOutput || l_bFlagCanModifyOutput || l_bCanConnectScenarioOutput))
					{
						uint32_t l_ui32FixedOutputCount = 0;
						sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialOutputCount).toASCIIString(), "%d", &l_ui32FixedOutputCount);
						GtkImageMenuItem* l_pMenuItemOutput = gtk_menu_add_new_image_menu_item(l_pMenu, GTK_STOCK_PROPERTIES, "outputs");
						GtkMenu* l_pMenuOutput = GTK_MENU(gtk_menu_new());
						for (i = 0; i < l_pBox->getOutputCount(); ++i)
						{
							CString l_sName;
							CIdentifier l_oType;
							CIdentifier l_oIdentifier;
							l_pBox->getOutputName(i, l_sName);
							l_pBox->getOutputType(i, l_oType);
							l_oIdentifier = l_pBox->getIdentifier();
							sprintf(l_sCompleteName, "%i : %s", int(i) + 1, l_sName.toASCIIString());
							GtkImageMenuItem* l_pMenuOutputMenuItem = gtk_menu_add_new_image_menu_item(l_pMenuOutput, GTK_STOCK_PROPERTIES, l_sCompleteName);

							GtkMenu* l_pMenuOutputMenuAction = GTK_MENU(gtk_menu_new());

							if (l_bCanConnectScenarioOutput)
							{
								for (j = 0; j < m_rScenario.getOutputCount(); j++)
								{
									char l_sScenarioOutputNameComplete[1024];
									CString l_sScenarioOutputName;
									CIdentifier l_oScenarioLinkBoxIdentifier;
									CIdentifier l_oScenarioOutputType;
									auto l_ui32ScenarioLinkOutputIndex = uint32_t(-1);
									m_rScenario.getOutputName(j, l_sScenarioOutputName);
									m_rScenario.getOutputType(j, l_oScenarioOutputType);
									m_rScenario.getScenarioOutputLink(j, l_oScenarioLinkBoxIdentifier, l_ui32ScenarioLinkOutputIndex);
									sprintf(l_sScenarioOutputNameComplete, "%u: %s", j + 1, l_sScenarioOutputName.toASCIIString());
									if (l_oScenarioLinkBoxIdentifier == l_oIdentifier && l_ui32ScenarioLinkOutputIndex == i)
									{
										gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuOutputMenuAction, GTK_STOCK_DISCONNECT, (CString("disconnect from ") + CString(l_sScenarioOutputNameComplete)).toASCIIString(), context_menu_cb, l_pBox, ContextMenu_BoxDisconnectScenarioOutput, i, j);
									}
									else if (m_rKernelContext.getTypeManager().isDerivedFromStream(l_oType, l_oScenarioOutputType))
									{
										gtk_menu_add_new_image_menu_item_with_cb_generic(l_pMenuOutputMenuAction, GTK_STOCK_CONNECT, (CString("connect to ") + CString(l_sScenarioOutputNameComplete)).toASCIIString(), context_menu_cb, l_pBox, ContextMenu_BoxConnectScenarioOutput, i, j);
									}
								}
							}

							if (l_bFlagCanModifyOutput)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuOutputMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditOutput, i);
							}
							if (l_bFlagCanAddOutput && l_ui32FixedOutputCount <= i)
							{
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenuOutputMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox, ContextMenu_BoxRemoveOutput, i);
							}

							if (gtk_container_get_children_count(GTK_CONTAINER(l_pMenuOutputMenuAction)) > 0)
							{
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuOutputMenuItem), GTK_WIDGET(l_pMenuOutputMenuAction));
							}
							else
							{
								gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuOutputMenuItem), false);
							}
						}
						gtk_menu_add_separator_menu_item(l_pMenuOutput);
						if (l_bFlagCanAddOutput)
						{
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenuOutput, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddOutput, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemOutput), GTK_WIDGET(l_pMenuOutput));
					}

					// -------------- SETTINGS --------------

					bool l_bFlagCanAddSetting = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting);
					bool l_bFlagCanModifySetting = l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting);
					if (!l_bFlagPendingDeprecatedInterfacors && !l_bFlagToBeUpdated && (l_bFlagCanAddSetting || l_bFlagCanModifySetting))
					{
						uint32_t l_ui32FixedSettingCount = 0;
						sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialSettingCount).toASCIIString(), "%d", &l_ui32FixedSettingCount);
						GtkImageMenuItem* l_pMenuItemSetting = gtk_menu_add_new_image_menu_item(l_pMenu, GTK_STOCK_PROPERTIES, "modify settings");
						GtkMenu* l_pMenuSetting = GTK_MENU(gtk_menu_new());
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
									gtk_menu_add_new_image_menu_item_with_cb(l_pMenuSettingMenuAction, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditSetting, i);
								}
								if (l_bFlagCanAddSetting && l_ui32FixedSettingCount <= i)
								{
									gtk_menu_add_new_image_menu_item_with_cb(l_pMenuSettingMenuAction, GTK_STOCK_REMOVE, "delete", context_menu_cb, l_pBox, ContextMenu_BoxRemoveSetting, i);
								}
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuSettingMenuItem), GTK_WIDGET(l_pMenuSettingMenuAction));
							}
							else
							{
								gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuSettingMenuItem), false);
							}
						}
						gtk_menu_add_separator_menu_item(l_pMenuSetting);
						if (l_bFlagCanAddSetting)
						{
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenuSetting, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddSetting, unused);
						}
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemSetting), GTK_WIDGET(l_pMenuSetting));
					}

					// -------------- ABOUT / RENAME --------------

					if (!m_vBoxContextMenuCB.empty()) { gtk_menu_add_separator_menu_item(l_pMenu); }
					if (l_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated))
					{
						auto updateMenuItem = gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_REFRESH, "update box", context_menu_cb, l_pBox, ContextMenu_BoxUpdate, unused);
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
						gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_REFRESH, "remove deprecated I/O/S", context_menu_cb, l_pBox, ContextMenu_BoxRemoveDeprecatedInterfacors, unused);
					}
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_EDIT, "rename box...", context_menu_cb, l_pBox, ContextMenu_BoxRename, unused);
					if (l_pBox->getSettingCount() != 0)
					{
						gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_PREFERENCES, "configure box...", context_menu_cb, l_pBox, ContextMenu_BoxConfigure, unused);
					}
					// Add this option only if the user has the authorization to open a metabox
					if (l_pBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
					{
						CIdentifier metaboxId;
						metaboxId.fromString(l_pBox->getAttributeValue(OVP_AttributeId_Metabox_Identifier));

						std::string metaboxScenarioPathString(m_rKernelContext.getMetaboxManager().getMetaboxFilePath(metaboxId).toASCIIString());
						std::string metaboxScenarioExtension = boost::filesystem::extension(metaboxScenarioPathString);
						bool canImportFile = false;

						CString fileNameExtension;
						while ((fileNameExtension = m_rKernelContext.getScenarioManager().getNextScenarioImporter(OVD_ScenarioImportContext_OpenScenario, fileNameExtension)) != CString(""))
						{
							if (metaboxScenarioExtension == fileNameExtension.toASCIIString())
							{
								canImportFile = true;
								break;
							}
						}

						if (canImportFile)
						{
							gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_PREFERENCES, "open this meta box in editor", context_menu_cb, l_pBox, ContextMenu_BoxEditMetabox, unused);
						}
					}
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_CONNECT, "enable box", context_menu_cb, l_pBox, ContextMenu_BoxEnable, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_DISCONNECT, "disable box", context_menu_cb, l_pBox, ContextMenu_BoxDisable, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_CUT, "delete box", context_menu_cb, l_pBox, ContextMenu_BoxDelete, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_HELP, "box documentation...", context_menu_cb, l_pBox, ContextMenu_BoxDocumentation, unused);
					gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_ABOUT, "about box...", context_menu_cb, l_pBox, ContextMenu_BoxAbout, unused);
				}
			}

			gtk_menu_add_separator_menu_item(l_pMenu);
			gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_EDIT, "add comment to scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAddComment, unused);
			gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, GTK_STOCK_ABOUT, "about scenario...", context_menu_cb, nullptr, ContextMenu_ScenarioAbout, unused);

			// -------------- RUN --------------

			gtk_widget_show_all(GTK_WIDGET(l_pMenu));
			gtk_menu_popup(l_pMenu, nullptr, nullptr, nullptr, nullptr, 3, pEvent->time);
			if (m_vBoxContextMenuCB.empty())
			{
				gtk_menu_popdown(l_pMenu);
			}
		}
	}

	this->redraw();
}

void CInterfacedScenario::scenarioDrawingAreaButtonReleasedCB(GtkWidget* /*pWidget*/, GdkEventButton* pEvent)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonReleasedCB\n";

	if (this->isLocked()) { return; }

	m_bButtonPressed &= !((pEvent->type == GDK_BUTTON_RELEASE) && (pEvent->button == 1));
	m_f64ReleaseMouseX = pEvent->x;
	m_f64ReleaseMouseY = pEvent->y;

	if (m_ui32CurrentMode != Mode_None)
	{
		const int l_iStartX = int(std::min(m_f64PressMouseX, m_f64CurrentMouseX));
		const int l_iStartY = int(std::min(m_f64PressMouseY, m_f64CurrentMouseY));
		const int l_iSizeX = int(std::max(m_f64PressMouseX - m_f64CurrentMouseX, m_f64CurrentMouseX - m_f64PressMouseX));
		const int l_iSizeY = int(std::max(m_f64PressMouseY - m_f64CurrentMouseY, m_f64CurrentMouseY - m_f64PressMouseY));

		if (m_ui32CurrentMode == Mode_Selection || m_ui32CurrentMode == Mode_SelectionAdd)
		{
			if (m_ui32CurrentMode == Mode_Selection) { m_SelectedObjects.clear(); }
			pickInterfacedObject(l_iStartX, l_iStartY, l_iSizeX, l_iSizeY);
		}
		if (m_ui32CurrentMode == Mode_Connect)
		{
			bool l_bIsActuallyConnecting = false;
			const bool l_bConnectionIsMessage = false;
			const uint32_t l_ui32InterfacedObjectId = pickInterfacedObject(int(m_f64ReleaseMouseX), int(m_f64ReleaseMouseY));
			const CInterfacedObject l_oCurrentObject = m_vInterfacedObject[l_ui32InterfacedObjectId];
			CInterfacedObject l_oSourceObject;
			CInterfacedObject l_oTargetObject;
			if (l_oCurrentObject.m_ui32ConnectorType == Box_Output && m_oCurrentObject.m_ui32ConnectorType == Box_Input)
			{
				l_oSourceObject = l_oCurrentObject;
				l_oTargetObject = m_oCurrentObject;
				l_bIsActuallyConnecting = true;
			}
			if (l_oCurrentObject.m_ui32ConnectorType == Box_Input && m_oCurrentObject.m_ui32ConnectorType == Box_Output)
			{
				l_oSourceObject = m_oCurrentObject;
				l_oTargetObject = l_oCurrentObject;
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
					l_pSourceBox->getOutputType(l_oSourceObject.m_ui32ConnectorIndex, l_oSourceTypeIdentifier);
					l_pTargetBox->getInputType(l_oTargetObject.m_ui32ConnectorIndex, l_oTargetTypeIdentifier);

					bool hasDeprecatedInput = false;
					l_pSourceBox->getInterfacorDeprecatedStatus(Output, l_oSourceObject.m_ui32ConnectorIndex, hasDeprecatedInput);
					bool hasDeprecatedOutput = false;
					l_pTargetBox->getInterfacorDeprecatedStatus(Input, l_oTargetObject.m_ui32ConnectorIndex, hasDeprecatedOutput);

					if ((m_rKernelContext.getTypeManager().isDerivedFromStream(l_oSourceTypeIdentifier, l_oTargetTypeIdentifier)
						|| m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_AllowUpCastConnection}", false)) && (!l_bConnectionIsMessage))
					{
						if (!hasDeprecatedInput && !hasDeprecatedOutput)
						{
							CIdentifier l_oLinkIdentifier;
							m_rScenario.connect(l_oLinkIdentifier, l_oSourceObject.m_oIdentifier, l_oSourceObject.m_ui32ConnectorIndex,
												l_oTargetObject.m_oIdentifier, l_oTargetObject.m_ui32ConnectorIndex, OV_UndefinedIdentifier);
							this->snapshotCB();
						}
						else { m_rKernelContext.getLogManager() << LogLevel_Warning << "Cannot connect to/from deprecated I/O\n"; }
					}
					else { m_rKernelContext.getLogManager() << LogLevel_Warning << "Invalid connection\n"; }
				}
			}
		}
		if (m_ui32CurrentMode == Mode_MoveSelection)
		{
			if (l_iSizeX == 0 && l_iSizeY == 0)
			{
				if (m_bControlPressed)
				{
					if (m_SelectedObjects.count(m_oCurrentObject.m_oIdentifier))
					{
						m_SelectedObjects.erase(m_oCurrentObject.m_oIdentifier);
					}
					else
					{
						m_SelectedObjects.insert(m_oCurrentObject.m_oIdentifier);
					}
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
						CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, objectId);
						l_oBoxProxy.setCenter(((l_oBoxProxy.getXCenter() + 8) & 0xfffffff0), ((l_oBoxProxy.getYCenter() + 8) & 0xfffffff0));
					}
					if (m_rScenario.isComment(objectId))
					{
						CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, objectId);
						l_oCommentProxy.setCenter(((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0), ((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0));
					}
				}
				this->snapshotCB();
			}
		}

		this->redraw();
	}

	m_ui32CurrentMode = Mode_None;
}

void CInterfacedScenario::scenarioDrawingAreaKeyPressEventCB(GtkWidget* /*pWidget*/, GdkEventKey* pEvent)
{
	m_bShiftPressed |= (pEvent->keyval == GDK_Shift_L || pEvent->keyval == GDK_Shift_R);
	m_bControlPressed |= (pEvent->keyval == GDK_Control_L || pEvent->keyval == GDK_Control_R);
	m_bAltPressed |= (pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R);
	m_bAPressed |= (pEvent->keyval == GDK_a || pEvent->keyval == GDK_A);
	m_bWPressed |= (pEvent->keyval == GDK_w || pEvent->keyval == GDK_W);

	// m_rKernelContext.getLogManager() << LogLevel_Info << "Key pressed " << (uint32_t) pEvent->keyval << "\n";
	/*
		if((pEvent->keyval==GDK_Z || pEvent->keyval==GDK_z) && m_bControlPressed)
		{
			this->undoCB();
		}

		if((pEvent->keyval==GDK_Y || pEvent->keyval==GDK_y) && m_bControlPressed)
		{
			this->redoCB();
		}
	*/
	// CTRL+A = select all
	if (m_bAPressed && m_bControlPressed && !m_bShiftPressed && !m_bAltPressed)
	{
		CIdentifier l_oIdentifier;
		while ((l_oIdentifier = m_rScenario.getNextBoxIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			m_SelectedObjects.insert(l_oIdentifier);
		}
		while ((l_oIdentifier = m_rScenario.getNextLinkIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			m_SelectedObjects.insert(l_oIdentifier);
		}
		while ((l_oIdentifier = m_rScenario.getNextCommentIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			m_SelectedObjects.insert(l_oIdentifier);
		}
		this->redraw();
	}

	//CTRL+W : close current scenario
	if (m_bWPressed && m_bControlPressed && !m_bShiftPressed && !m_bAltPressed)
	{
		m_rApplication.closeScenarioCB(this);
		return;
	}

	if ((pEvent->keyval == GDK_C || pEvent->keyval == GDK_c) && m_ui32CurrentMode == Mode_None)
	{
		gint iX = 0;
		gint iY = 0;
		gdk_window_get_pointer(GTK_WIDGET(m_pScenarioDrawingArea)->window, &iX, &iY, nullptr);

		this->addCommentCB(iX, iY);
	}

	if (pEvent->keyval == GDK_F12 && m_bShiftPressed)
	{
		CIdentifier l_oIdentifier;
		while ((l_oIdentifier = m_rScenario.getNextBoxIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			IBox* l_pBox = m_rScenario.getBoxDetails(l_oIdentifier);
			CIdentifier l_oAlgorithmIdentifier = l_pBox->getAlgorithmClassIdentifier();
			CIdentifier l_oHashValue = m_rKernelContext.getPluginManager().getPluginObjectHashValue(l_oAlgorithmIdentifier);
			if (l_pBox->hasAttribute(OV_AttributeId_Box_InitialPrototypeHashValue))
			{
				l_pBox->setAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue, l_oHashValue.toString());
			}
			else
			{
				l_pBox->addAttribute(OV_AttributeId_Box_InitialPrototypeHashValue, l_oHashValue.toString());
			}
		}

		this->redraw();
		this->snapshotCB();
	}

	// F1 : browse documentation
	if (pEvent->keyval == GDK_F1)
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
			CString l_sFullURL = m_rScenario.getAttributeValue(OV_AttributeId_Scenario_DocumentationPage);
			if (l_sFullURL != CString(""))
			{
				browseURL(l_sFullURL, m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommand}"), m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommandPostfix}"));
			}
			else
			{
				m_rKernelContext.getLogManager() << LogLevel_Info << "The scenario does not define a documentation page.\n";
			}
		}
	}

	// F2 : rename all selected box(es)
	if (pEvent->keyval == GDK_F2)
	{
		contextMenuBoxRenameAllCB();
	}

	// F8 : toggle enable/disable on all selected box(es)
	if (pEvent->keyval == GDK_F3)
	{
		contextMenuBoxToggleEnableAllCB();
		this->redraw();
	}

	//The shortcuts respect the order in the toolbar

	// F7 :play/pause
	if (pEvent->keyval == GDK_F7)
	{
		if (m_rApplication.getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Play)
		{
			m_rApplication.pauseScenarioCB();
		}
		else
		{
			m_rApplication.playScenarioCB();
		}
	}
	// F6 : step
	if (pEvent->keyval == GDK_F6)
	{
		m_rApplication.nextScenarioCB();
	}
	// F8 :fastforward
	if (pEvent->keyval == GDK_F8)
	{
		m_rApplication.forwardScenarioCB();
	}
	// F5 : stop
	if (pEvent->keyval == GDK_F5)
	{
		m_rApplication.stopScenarioCB();
	}

	m_rKernelContext.getLogManager() << LogLevel_Debug
		<< "scenarioDrawingAreaKeyPressEventCB ("
		<< (m_bShiftPressed ? "true" : "false") << "|"
		<< (m_bControlPressed ? "true" : "false") << "|"
		<< (m_bAltPressed ? "true" : "false") << "|"
		<< (m_bAPressed ? "true" : "false") << "|"
		<< (m_bWPressed ? "true" : "false") << "|"
		<< ")\n";

	if (this->isLocked()) { return; }

#if defined TARGET_OS_Windows || defined TARGET_OS_Linux
	if (pEvent->keyval == GDK_Delete || pEvent->keyval == GDK_KP_Delete)
#elif defined TARGET_OS_MacOS
	if (pEvent->keyval == GDK_BackSpace)
#endif
	{
		this->deleteSelection();
	}
}

void CInterfacedScenario::scenarioDrawingAreaKeyReleaseEventCB(GtkWidget* /*pWidget*/, GdkEventKey* pEvent)
{
	m_bShiftPressed &= !(pEvent->keyval == GDK_Shift_L || pEvent->keyval == GDK_Shift_R);
	m_bControlPressed &= !(pEvent->keyval == GDK_Control_L || pEvent->keyval == GDK_Control_R);
	m_bAltPressed &= !(pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R);
	m_bAPressed &= !(pEvent->keyval == GDK_A || pEvent->keyval == GDK_a);
	m_bWPressed &= !(pEvent->keyval == GDK_W || pEvent->keyval == GDK_w);

	m_rKernelContext.getLogManager() << LogLevel_Debug
		<< "scenarioDrawingAreaKeyReleaseEventCB ("
		<< (m_bShiftPressed ? "true" : "false") << "|"
		<< (m_bControlPressed ? "true" : "false") << "|"
		<< (m_bAltPressed ? "true" : "false") << "|"
		<< (m_bAPressed ? "true" : "false") << "|"
		<< (m_bWPressed ? "true" : "false") << "|"
		<< ")\n";

	//if (this->isLocked()) { return; }
	// ...
}

void CInterfacedScenario::copySelection()

{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "copySelection\n";

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
	m_rKernelContext.getLogManager() << LogLevel_Debug << "cutSelection\n";

	this->copySelection();
	this->deleteSelection();
}

void CInterfacedScenario::pasteSelection()

{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "pasteSelection\n";

	// Prepares paste
	CIdentifier l_oIdentifier;
	map<CIdentifier, CIdentifier> l_vIdMapping;
	/*
	int32_t l_iCenterX=0;
	int32_t l_iCenterY=0;
	*/
	int32_t l_iTopmostLeftmostCopiedBoxCenterX = 1 << 15;
	int32_t l_iTopmostLeftmostCopiedBoxCenterY = 1 << 15;
	// std::cout << "Mouse position : " << m_f64CurrentMouseX << "/" << m_f64CurrentMouseY << std::endl;

	// Pastes boxes from clipboard
	while ((l_oIdentifier = m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		IBox* l_pBox = m_rApplication.m_pClipboardScenario->getBoxDetails(l_oIdentifier);
		m_rScenario.addBox(l_oNewIdentifier, *l_pBox, l_oIdentifier);
		l_vIdMapping[l_oIdentifier] = l_oNewIdentifier;

		// Updates visualization manager
		CIdentifier l_oBoxAlgorithmIdentifier = l_pBox->getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oBoxAlgorithmIdentifier);

		// If a visualization box was dropped, add it in window manager
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			// Let window manager know about new box
			if (m_pDesignerVisualization)
			{
				m_pDesignerVisualization->onVisualizationBoxAdded(m_rScenario.getBoxDetails(l_oNewIdentifier));
			}
		}

		CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, l_oNewIdentifier);

		// get the position of the topmost-leftmost box (always position on an actual box so when user pastes he sees something)
		if (l_oBoxProxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterX && l_oBoxProxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterY)
		{
			l_iTopmostLeftmostCopiedBoxCenterX = l_oBoxProxy.getXCenter();
			l_iTopmostLeftmostCopiedBoxCenterY = l_oBoxProxy.getYCenter();
		}
	}

	// Pastes comments from clipboard
	while ((l_oIdentifier = m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		IComment* l_pComment = m_rApplication.m_pClipboardScenario->getCommentDetails(l_oIdentifier);
		m_rScenario.addComment(l_oNewIdentifier, *l_pComment, l_oIdentifier);
		l_vIdMapping[l_oIdentifier] = l_oNewIdentifier;

		CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, l_oNewIdentifier);

		if (l_oCommentProxy.getXCenter() < l_iTopmostLeftmostCopiedBoxCenterX && l_oCommentProxy.getYCenter() < l_iTopmostLeftmostCopiedBoxCenterY)
		{
			l_iTopmostLeftmostCopiedBoxCenterX = l_oCommentProxy.getXCenter();
			l_iTopmostLeftmostCopiedBoxCenterY = l_oCommentProxy.getYCenter();
		}
	}

	// Pastes links from clipboard
	while ((l_oIdentifier = m_rApplication.m_pClipboardScenario->getNextLinkIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		ILink* l_pLink = m_rApplication.m_pClipboardScenario->getLinkDetails(l_oIdentifier);
		m_rScenario.connect(l_oNewIdentifier, l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
							l_pLink->getSourceBoxOutputIndex(), l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
							l_pLink->getTargetBoxInputIndex(), l_pLink->getIdentifier());
	}

	// Makes pasted stuff the default selection
	// Moves boxes under cursor
	// Moves comments under cursor
	if (m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier || m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier) != OV_UndefinedIdentifier)
	{
		m_SelectedObjects.clear();
		for (auto& it : l_vIdMapping)
		{
			m_SelectedObjects.insert(it.second);

			if (m_rScenario.isBox(it.second))
			{
				// Moves boxes under cursor
				CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, it.second);
				l_oBoxProxy.setCenter(int32_t(l_oBoxProxy.getXCenter() + m_f64CurrentMouseX) - l_iTopmostLeftmostCopiedBoxCenterX - m_i32ViewOffsetX,
									  int32_t(l_oBoxProxy.getYCenter() + m_f64CurrentMouseY) - l_iTopmostLeftmostCopiedBoxCenterY - m_i32ViewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns boxes on grid
				l_oBoxProxy.setCenter(int32_t((l_oBoxProxy.getXCenter() + 8) & 0xfffffff0L), int32_t((l_oBoxProxy.getYCenter() + 8) & 0xfffffff0L));
			}

			if (m_rScenario.isComment(it.second))
			{
				// Moves commentes under cursor
				CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, it.second);
				l_oCommentProxy.setCenter(int32_t(l_oCommentProxy.getXCenter() + m_f64CurrentMouseX) - l_iTopmostLeftmostCopiedBoxCenterX - m_i32ViewOffsetX,
										  int32_t(l_oCommentProxy.getYCenter() + m_f64CurrentMouseY) - l_iTopmostLeftmostCopiedBoxCenterY - m_i32ViewOffsetY);
				// Ok, why 32 would you ask, just because it is fine

				// Aligns commentes on grid
				l_oCommentProxy.setCenter(int32_t((l_oCommentProxy.getXCenter() + 8) & 0xfffffff0L), int32_t((l_oCommentProxy.getYCenter() + 8) & 0xfffffff0L));
			}
		}
	}

	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::deleteSelection()

{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "deleteSelection\n";
	for (auto& objectId : m_SelectedObjects)
	{
		if (m_rScenario.isBox(objectId))
		{
			this->deleteBox(objectId);
		}
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

void CInterfacedScenario::deleteBox(const CIdentifier& rBoxIdentifier)
{
	// removes visualization box from window manager
	if (m_pDesignerVisualization)
	{
		m_pDesignerVisualization->onVisualizationBoxRemoved(rBoxIdentifier);
	}

	// removes box from scenario
	m_rScenario.removeBox(rBoxIdentifier);
}


void CInterfacedScenario::contextMenuBoxUpdateCB(IBox& rBox)
{
	m_rScenario.updateBox(rBox.getIdentifier());
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxUpdateCB\n";
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRemoveDeprecatedInterfacorsCB(IBox& rBox)
{
	m_rScenario.removeDeprecatedInterfacorsFromBox(rBox.getIdentifier());
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveDeprecatedInterfacorsCB\n";
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRenameCB(IBox& rBox)
{
	const IPluginObjectDesc* l_pPluginObjectDescriptor = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(rBox.getAlgorithmClassIdentifier());
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRenameCB\n";

	if (rBox.getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(rBox.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		l_pPluginObjectDescriptor = m_rKernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
	}

	CRenameDialog l_oRename(m_rKernelContext, rBox.getName(), l_pPluginObjectDescriptor ? l_pPluginObjectDescriptor->getName() : rBox.getName(), m_sGUIFilename.c_str());
	if (l_oRename.run())
	{
		rBox.setName(l_oRename.getResult());

		//check whether it is a visualization box
		CIdentifier l_oId = rBox.getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);

		//if a visualization box was renamed, tell window manager about it
		if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
		{
			if (m_pDesignerVisualization)
			{
				m_pDesignerVisualization->onVisualizationBoxRenamed(rBox.getIdentifier());
			}
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
		if (m_rScenario.isBox(objectId))
		{
			l_vSelectedBox[objectId] = m_rScenario.getBoxDetails(objectId)->getAlgorithmClassIdentifier();
		}
	}

	if (!l_vSelectedBox.empty())
	{
		bool l_bDialogOk = true;
		bool l_bFirstBox = true;
		CString l_sNewName = "";
		for (auto it = l_vSelectedBox.begin(); it != l_vSelectedBox.end() && l_bDialogOk; ++it)
		{
			if (it->second != OV_UndefinedIdentifier)
			{
				if (m_rKernelContext.getPluginManager().canCreatePluginObject(it->second) || it->second == OVP_ClassId_BoxAlgorithm_Metabox)
				{
					IBox* l_pBox = m_rScenario.getBoxDetails(it->first);
					if (l_bFirstBox)
					{
						l_bFirstBox = false;
						const IPluginObjectDesc* l_pPluginObjectDescriptor = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_pBox->getAlgorithmClassIdentifier());
						if (l_pBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
						{
							CIdentifier metaboxId;
							metaboxId.fromString(l_pBox->getAttributeValue(OVP_AttributeId_Metabox_Identifier));
							l_pPluginObjectDescriptor = m_rKernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId);
						}

						CRenameDialog l_oRename(m_rKernelContext, l_pBox->getName(), l_pPluginObjectDescriptor ? l_pPluginObjectDescriptor->getName() : l_pBox->getName(), m_sGUIFilename.c_str());
						if (l_oRename.run())
						{
							l_sNewName = l_oRename.getResult();
						}
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
						CIdentifier l_oId = l_pBox->getAlgorithmClassIdentifier();
						const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);

						//if a visualization box was renamed, tell window manager about it
						if (l_pPOD && l_pPOD->hasFunctionality(OVD_Functionality_Visualization))
						{
							if (m_pDesignerVisualization)
							{
								m_pDesignerVisualization->onVisualizationBoxRenamed(l_pBox->getIdentifier());
							}
						}
					}
				}
			}
		}
		if (l_bDialogOk)
		{
			this->snapshotCB();
		}
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
			if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled))
			{
				l_oAttributeHandler.removeAttribute(OV_AttributeId_Box_Disabled);
			}
			else
			{
				l_oAttributeHandler.addAttribute(OV_AttributeId_Box_Disabled, 1);
			}
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
			if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled))
			{
				l_oAttributeHandler.removeAttribute(OV_AttributeId_Box_Disabled);
			}
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
			if (!l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled))
			{
				l_oAttributeHandler.addAttribute(OV_AttributeId_Box_Disabled, 1);
			}
		}
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddInputCB(IBox& rBox)
{
	if (rBox.hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors))
	{
		gtk_dialog_run(GTK_DIALOG(m_pErrorPendingDeprecatedInterfacorsDialog));
		return;
	}
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddInputCB\n";
	rBox.addInput("New input", OV_TypeId_EBMLStream, m_rScenario.getUnusedInputIdentifier());
	if (rBox.hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
	{
		CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Box_Input, rBox.getInputCount() - 1, "Add Input", m_sGUIFilename.c_str());
		if (l_oConnectorEditor.run())
		{
			this->snapshotCB();
		}
		else
		{
			rBox.removeInput(rBox.getInputCount() - 1);
		}
	}
	else
	{
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxEditInputCB(IBox& rBox, const uint32_t ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditInputCB\n";

	CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Box_Input, ui32Index, "Edit Input", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run())
	{
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxRemoveInputCB(IBox& rBox, const uint32_t ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveInputCB\n";
	rBox.removeInput(ui32Index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddOutputCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddOutputCB\n";
	rBox.addOutput("New output", OV_TypeId_EBMLStream, m_rScenario.getUnusedOutputIdentifier());
	if (rBox.hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput))
	{
		CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Box_Output, rBox.getOutputCount() - 1, "Add Output", m_sGUIFilename.c_str());
		if (l_oConnectorEditor.run())
		{
			this->snapshotCB();
		}
		else
		{
			rBox.removeOutput(rBox.getOutputCount() - 1);
		}
	}
	else
	{
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxEditOutputCB(IBox& rBox, const uint32_t ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditOutputCB\n";

	CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Box_Output, ui32Index, "Edit Output", m_sGUIFilename.c_str());
	if (l_oConnectorEditor.run())
	{
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxRemoveOutputCB(IBox& rBox, const uint32_t ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveOutputCB\n";
	rBox.removeOutput(ui32Index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioInputCB(IBox& rBox, const uint32_t ui32BoxInputIndex, const uint32_t ui32ScenarioInputIndex)
{
	//	m_rKernelContext.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioInputCB : box = " << rBox.getIdentifier().toString() << " box input = " << ui32BoxInputIndex << " , scenario input = " << ui32ScenarioInputIndex << "\n";
	m_rScenario.setScenarioInputLink(ui32ScenarioInputIndex, rBox.getIdentifier(), ui32BoxInputIndex);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxConnectScenarioOutputCB(IBox& rBox, const uint32_t ui32BoxOutputIndex, const uint32_t ui32ScenarioOutputIndex)
{
	//	m_rKernelContext.getLogManager() << LogLevel_Info << "contextMenuBoxConnectScenarioOutputCB : box = " << rBox.getIdentifier().toString() << " box Output = " << ui32BoxOutputIndex << " , scenario Output = " << ui32ScenarioOutputIndex << "\n";
	m_rScenario.setScenarioOutputLink(ui32ScenarioOutputIndex, rBox.getIdentifier(), ui32BoxOutputIndex);
	this->snapshotCB();
}

// Note: In current implementation only the scenarioInputIndex is necessary as it can only be connected to one input
// but to keep things simpler we give it all the info
void CInterfacedScenario::contextMenuBoxDisconnectScenarioInputCB(IBox& rBox, const uint32_t ui32BoxInputIndex, uint32_t ui32ScenarioInputIndex)
{
	m_rScenario.removeScenarioInputLink(ui32ScenarioInputIndex, rBox.getIdentifier(), ui32BoxInputIndex);
	this->snapshotCB();
}

// Note: In current implementation only the scenarioOutputIndex is necessary as it can only be connected to one output
// but to keep things simpler we give it all the info
void CInterfacedScenario::contextMenuBoxDisconnectScenarioOutputCB(IBox& rBox, const uint32_t ui32BoxOutputIndex, const uint32_t ui32ScenarioOutputIndex)
{
	m_rScenario.removeScenarioOutputLink(ui32ScenarioOutputIndex, rBox.getIdentifier(), ui32BoxOutputIndex);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddSettingCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddSettingCB\n";
	// Store setting count in case the custom "onSettingAdded" of the box adds more than one setting
	uint32_t l_ui32OldSettingsCount = rBox.getSettingCount();
	rBox.addSetting("New setting", OV_UndefinedIdentifier, "", OV_Value_UndefinedIndexUInt, false, m_rScenario.getUnusedSettingIdentifier(OV_UndefinedIdentifier));
	uint32_t l_ui32NewSettingsCount = rBox.getSettingCount();
	// Check that at least one setting was added
	if (l_ui32NewSettingsCount > l_ui32OldSettingsCount && rBox.hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
	{
		CSettingEditorDialog l_oSettingEditorDialog(m_rKernelContext, rBox, l_ui32OldSettingsCount, "Add Setting", m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
		if (l_oSettingEditorDialog.run())
		{
			this->snapshotCB();
		}
		else
		{
			for (uint32_t i = l_ui32OldSettingsCount; i < l_ui32NewSettingsCount; ++i)
			{
				rBox.removeSetting(i);
			}
		}
	}
	else
	{
		if (l_ui32NewSettingsCount > l_ui32OldSettingsCount)
		{
			this->snapshotCB();
		}
		else
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "No setting could be added to the box.\n";
			return;
		}
	}
	// Add an information message to inform the user about the new settings
	m_rKernelContext.getLogManager() << LogLevel_Info << "[" << l_ui32NewSettingsCount - l_ui32OldSettingsCount << "] new setting(s) was(were) added to the box ["
		<< rBox.getName().toASCIIString() << "]: ";
	for (uint32_t i = l_ui32OldSettingsCount; i < l_ui32NewSettingsCount; ++i)
	{
		CString l_sSettingName;
		rBox.getSettingName(i, l_sSettingName);
		m_rKernelContext.getLogManager() << "[" << l_sSettingName << "] ";
	}
	m_rKernelContext.getLogManager() << "\n";
	// After adding setting, open configuration so that the user can see the effects.
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_rKernelContext, rBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	l_oBoxConfigurationDialog.run();
}

void CInterfacedScenario::contextMenuBoxEditSettingCB(IBox& rBox, const uint32_t ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditSettingCB\n";
	CSettingEditorDialog l_oSettingEditorDialog(m_rKernelContext, rBox, ui32Index, "Edit Setting", m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	if (l_oSettingEditorDialog.run())
	{
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxRemoveSettingCB(IBox& rBox, const uint32_t ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveSettingCB\n";
	uint32_t l_ui32OldSettingsCount = rBox.getSettingCount();
	if (rBox.removeSetting(ui32Index))
	{
		uint32_t l_ui32NewSettingsCount = rBox.getSettingCount();
		this->snapshotCB();
		m_rKernelContext.getLogManager() << LogLevel_Info << "[" << l_ui32OldSettingsCount - l_ui32NewSettingsCount << "] setting(s) was(were) removed from box ["
			<< rBox.getName().toASCIIString() << "] \n";
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "The setting with index [" << ui32Index << "] could not be removed from box ["
			<< rBox.getName().toASCIIString() << "] \n";
	}
}

void CInterfacedScenario::contextMenuBoxConfigureCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxConfigureCB\n";
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_rKernelContext, rBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	l_oBoxConfigurationDialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAboutCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAboutCB\n";
	if (rBox.getAlgorithmClassIdentifier() != OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CAboutPluginDialog l_oAboutPluginDialog(m_rKernelContext, rBox.getAlgorithmClassIdentifier(), m_sGUIFilename.c_str());
		l_oAboutPluginDialog.run();
	}
	else
	{
		CIdentifier metaboxId;
		metaboxId.fromString(rBox.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		CAboutPluginDialog l_oAboutPluginDialog(m_rKernelContext, m_rKernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId), m_sGUIFilename.c_str());
		l_oAboutPluginDialog.run();
	}
}

void CInterfacedScenario::contextMenuBoxEditMetaboxCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditMetaboxCB\n";

	CIdentifier metaboxId;
	metaboxId.fromString(rBox.getAttributeValue(OVP_AttributeId_Metabox_Identifier));
	CString l_sMetaboxScenarioPath(m_rKernelContext.getMetaboxManager().getMetaboxFilePath(metaboxId));

	m_rApplication.openScenario(l_sMetaboxScenarioPath.toASCIIString());
}

bool CInterfacedScenario::browseURL(const CString& url, const CString& browserPrefix, const CString& browserPostfix)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "Requesting web browser on URL " << url << "\n";

	const CString command = browserPrefix + CString(" \"") + url + CString("\"") + browserPostfix;
	m_rKernelContext.getLogManager() << LogLevel_Debug << "Launching [" << command << "]\n";
	const int result = system(command.toASCIIString());
	if (result < 0)
	{
		OV_WARNING("Could not launch command [" << command << "]\n", m_rKernelContext.getLogManager());
		return false;
	}
	return true;
}

bool CInterfacedScenario::browseBoxDocumentation(const CIdentifier& oBoxId)
{
	CIdentifier l_oAlgorithmClassIdentifier = m_rScenario.getBoxDetails(oBoxId)->getAlgorithmClassIdentifier();

	// Do not show documentation for non-metaboxes or boxes that can not be created
	if (!(oBoxId != OV_UndefinedIdentifier && (m_rKernelContext.getPluginManager().canCreatePluginObject(l_oAlgorithmClassIdentifier) ||
		m_rScenario.getBoxDetails(oBoxId)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)))
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Box with id " << oBoxId << " can not create a pluging object\n";
		return false;
	}

	const CString l_sDefaultURLBase = m_rKernelContext.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}");
	CString l_sURLBase = l_sDefaultURLBase;
	CString l_sBrowser = m_rKernelContext.getConfigurationManager().expand("${Designer_HelpBrowserCommand}");
	CString l_sBrowserPostfix = m_rKernelContext.getConfigurationManager().expand("${Designer_HelpBrowserCommandPostfix}");
	CString l_sBoxName;

	CString l_sHTMLName = "Doc_BoxAlgorithm_";
	if (m_rScenario.getBoxDetails(oBoxId)->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CIdentifier metaboxId;
		metaboxId.fromString(m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OVP_AttributeId_Metabox_Identifier));
		l_sBoxName = m_rKernelContext.getMetaboxManager().getMetaboxObjectDesc(metaboxId)->getName();
	}
	else
	{
		const IPluginObjectDesc* l_pPluginObjectDesc = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oAlgorithmClassIdentifier);
		l_sBoxName = l_pPluginObjectDesc->getName();
	}
	// The documentation files do not have spaces in their name, so we remove them
	l_sHTMLName = l_sHTMLName + CString(getBoxAlgorithmURL(l_sBoxName.toASCIIString()).c_str());


	if (m_rScenario.getBoxDetails(oBoxId)->hasAttribute(OV_AttributeId_Box_DocumentationURLBase))
	{
		l_sURLBase = m_rKernelContext.getConfigurationManager().expand(m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OV_AttributeId_Box_DocumentationURLBase));
	}
	l_sHTMLName = l_sHTMLName + ".html";

	if (m_rScenario.getBoxDetails(oBoxId)->hasAttribute(OV_AttributeId_Box_DocumentationCommand))
	{
		l_sBrowser = m_rKernelContext.getConfigurationManager().expand(m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OV_AttributeId_Box_DocumentationCommand));
		l_sBrowserPostfix = "";
	}

	CString l_sFullURL = l_sURLBase + CString("/") + l_sHTMLName;
	if (m_rScenario.getBoxDetails(oBoxId)->hasAttribute(OV_AttributeId_Box_DocumentationURL))
	{
		l_sFullURL = m_rKernelContext.getConfigurationManager().expand(m_rScenario.getBoxDetails(oBoxId)->getAttributeValue(OV_AttributeId_Box_DocumentationURL));
	}

	return browseURL(l_sFullURL, l_sBrowser, l_sBrowserPostfix);
}

void CInterfacedScenario::contextMenuBoxDocumentationCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxDocumentationCB\n";
	const CIdentifier l_oBoxId = rBox.getIdentifier();

	browseBoxDocumentation(l_oBoxId);
}

void CInterfacedScenario::contextMenuBoxEnableCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEnableCB\n";
	TAttributeHandler l_oAttributeHandler(rBox);
	l_oAttributeHandler.removeAttribute(OV_AttributeId_Box_Disabled);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxDisableCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxDisableCB\n";
	TAttributeHandler l_oAttributeHandler(rBox);
	if (!l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled))
	{
		l_oAttributeHandler.addAttribute(OV_AttributeId_Box_Disabled, 1);
	}
	else
	{
		l_oAttributeHandler.setAttributeValue(OV_AttributeId_Box_Disabled, 1);
	}
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuScenarioAddCommentCB()

{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuScenarioAddCommentCB\n";
	this->addCommentCB();
}

void CInterfacedScenario::contextMenuScenarioAboutCB()

{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuScenarioAboutCB\n";
	CAboutScenarioDialog l_oAboutScenarioDialog(m_rKernelContext, m_rScenario, m_sGUIFilename.c_str());
	l_oAboutScenarioDialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::toggleDesignerVisualization()
{
	m_bDesignerVisualizationToggled = !m_bDesignerVisualizationToggled;

	if (m_pDesignerVisualization)
	{
		if (m_bDesignerVisualizationToggled)
		{
			m_pDesignerVisualization->show();
		}
		else
		{
			m_pDesignerVisualization->hide();
		}
	}
}

bool CInterfacedScenario::isDesignerVisualizationToggled() { return m_bDesignerVisualizationToggled; }

void CInterfacedScenario::showCurrentVisualization()
{
	if (isLocked())
	{
		if (m_pPlayerVisualization != nullptr)
		{
			m_pPlayerVisualization->showTopLevelWindows();
		}
	}
	else
	{
		if (m_pDesignerVisualization != nullptr)
		{
			m_pDesignerVisualization->show();
		}
	}
}

void CInterfacedScenario::hideCurrentVisualization()
{
	if (isLocked())
	{
		if (m_pPlayerVisualization != nullptr)
		{
			m_pPlayerVisualization->hideTopLevelWindows();
		}
	}
	else
	{
		if (m_pDesignerVisualization != nullptr)
		{
			m_pDesignerVisualization->hide();
		}
	}
}

void CInterfacedScenario::createPlayerVisualization(IVisualizationTree* pVisualizationTree)
{
	//hide window manager
	if (m_pDesignerVisualization)
	{
		m_pDesignerVisualization->hide();
	}

	if (m_pPlayerVisualization == nullptr)
	{
		if (pVisualizationTree)
		{
			m_pPlayerVisualization = new CPlayerVisualization(m_rKernelContext, *pVisualizationTree, *this);
		}
		else
		{
			m_pPlayerVisualization = new CPlayerVisualization(m_rKernelContext, *m_pVisualizationTree, *this);
		}


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
				auto* l_oBoxConfigurationDialog = new CBoxConfigurationDialog(m_rKernelContext, *l_oBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str(), true);
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
		if (m_bDesignerVisualizationToggled)
		{
			m_pDesignerVisualization->show();
		}
	}
}

bool CInterfacedScenario::hasSelection()
{
	return !m_SelectedObjects.empty();
}


void CInterfacedScenario::stopAndReleasePlayer()

{
	m_rKernelContext.getErrorManager().releaseErrors();
	m_pPlayer->stop();
	m_ePlayerStatus = m_pPlayer->getStatus();
	// removes idle function
	g_idle_remove_by_data(this);

	if (!m_pPlayer->uninitialize())
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to uninitialize the player" << "\n";
	}

	for (auto elem : m_vBoxConfigurationDialog)
	{
		elem->restoreState();
		delete elem;
	}
	m_vBoxConfigurationDialog.clear();


	if (!m_rKernelContext.getPlayerManager().releasePlayer(m_oPlayerIdentifier))
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to release the player" << "\n";
	}

	m_oPlayerIdentifier = OV_UndefinedIdentifier;
	m_pPlayer = nullptr;

	// destroy player windows
	releasePlayerVisualization();

	// redraws scenario
	redraw();
}

//give the PlayerVisualisation the matching between the GtkWidget created by the CBoxConfigurationDialog and the Box CIdentifier
bool CInterfacedScenario::setModifiableSettingsWidgets()

{
	for (auto& elem : m_vBoxConfigurationDialog)
	{
		m_pPlayerVisualization->setWidget(elem->getBoxID(), elem->getWidget());
	}

	return true;
}

bool CInterfacedScenario::centerOnBox(const CIdentifier& rIdentifier)
{
	//m_rKernelContext.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox" << "\n";
	bool ret_val = false;
	if (m_rScenario.isBox(rIdentifier))
	{
		//m_rKernelContext.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox is box" << "\n";
		IBox* rBox = m_rScenario.getBoxDetails(rIdentifier);

		//clear previous selection
		m_SelectedObjects.clear();
		//to select the box

		m_SelectedObjects.insert(rIdentifier);
		//		m_bScenarioModified=true;
		redraw();

		//CBoxProxy l_oBoxProxy(m_rKernelContext, *rBox);
		CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, rBox->getIdentifier());
		const double xMargin = 5.0 * m_f64CurrentScale;
		const double yMargin = 5.0 * m_f64CurrentScale;
		int xSize = int(round(l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) + xMargin * 2.0));
		int ySize = int(round(l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) + yMargin * 2.0));
		const double l_f64XCenter = l_oBoxProxy.getXCenter() * m_f64CurrentScale;
		const double l_f64YCenter = l_oBoxProxy.getYCenter() * m_f64CurrentScale;
		int x, y;

		//get the parameters of the current adjustement
		GtkAdjustment* l_pOldHAdjustement = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent));//gtk_viewport_get_vadjustment(m_pScenarioViewport);
		GtkAdjustment* l_pOldVAdjustement = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent));
		gdouble upper, lower, step, page, pagesize, value;

		g_object_get(l_pOldHAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value", &value, nullptr);
		//create a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		auto* l_pAdjustement = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (l_f64XCenter + m_i32ViewOffsetX < upper / 2)
		{
			x = int(round(l_f64XCenter - 2 * xSize)) + m_i32ViewOffsetX;
		}
		else
		{
			x = int(round(l_f64XCenter + 2 * xSize - pagesize)) + m_i32ViewOffsetX;
		}
		gtk_adjustment_set_value(l_pAdjustement, x);
		gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent), l_pAdjustement);

		g_object_get(l_pOldVAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value", &value, nullptr);
		l_pAdjustement = reinterpret_cast<GtkAdjustment*>(gtk_adjustment_new(value, lower, upper, step, page, pagesize));
		if (l_f64YCenter - m_i32ViewOffsetY < upper / 2)
		{
			y = int(round(l_f64YCenter - 2 * ySize) + m_i32ViewOffsetY);
		}
		else
		{
			y = int(round(l_f64YCenter + 2 * ySize - pagesize)) + m_i32ViewOffsetY;
		}
		gtk_adjustment_set_value(l_pAdjustement, y);
		gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(m_pNotebookPageContent), l_pAdjustement);
		ret_val = true;
	}
	return ret_val;
}

void CInterfacedScenario::setScale(const double scale)
{
	m_f64CurrentScale = std::max(scale, 0.1);

	PangoContext* l_pPangoContext = gtk_widget_get_pango_context(GTK_WIDGET(m_pScenarioDrawingArea));
	PangoFontDescription* l_pPangoFontDescription = pango_context_get_font_description(l_pPangoContext);
	if (m_ui32NormalFontSize == 0)
	{
		// not done in constructor because the font size is changed elsewhere after that withour our knowledge
		m_ui32NormalFontSize = pango_font_description_get_size(l_pPangoFontDescription);
	}
	pango_font_description_set_size(l_pPangoFontDescription, gint(round(m_ui32NormalFontSize * m_f64CurrentScale)));

	//m_bScenarioModified = true;
	redraw();
}

double CInterfacedScenario::getScale() { return m_f64CurrentScale; }
