#if defined TARGET_HAS_LibArchway

#include "ovdCArchwayHandlerGUI.h"
#include <mensia/base/m_directories.h>
#include <openvibe/ov_directories.h>
#include <fs/Files.h>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <sstream>

#if defined TARGET_OS_Windows
#define pclose _pclose
#endif

using namespace Mensia;

namespace
{
	enum EnginePipelinesModelColumn {
		Column_PipelineId = 0,
		Column_PipelineHexId,
		Column_PipelineDescription,
		Column_PipelineIsConfigured
	};

	enum PipelineConfigurationModelColumn {
		Column_SettingPipelineId = 0,
		Column_SettingName = 1,
		Column_SettingDefaultValue = 2,
		Column_SettingValue =3
	};

	void on_button_configure_acquisition_clicked(GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);

		std::string configurationToolLaunchCmd = Mensia::Directories::getBinDir() + "/mensia-device-configuration";
		//std::string configurationFilePath = std::string(getenv("USERPROFILE")) + "\\lib-mensia-engine.conf";

		std::string l_sEscapedURL = l_pGUI->m_rController.m_sDeviceURL;

		size_t l_uiStartPosition = 0;
	    while((l_uiStartPosition = l_sEscapedURL.find("\"", l_uiStartPosition)) != std::string::npos) {
	        l_sEscapedURL.replace(l_uiStartPosition, 1, "\\\"");
	        l_uiStartPosition += 2; // Handles case where 'to' is a substring of 'from'
	    }


#if defined TARGET_OS_Windows
		std::string command = "\"\"" + configurationToolLaunchCmd + "\" --no-file \"" + l_sEscapedURL + "\"\"";
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		std::string command = configurationToolLaunchCmd + " --no-file \"" + l_sEscapedURL + "\"";
#endif
		FILE* l_pCommandPipe = FS::Files::popen(command.c_str(), "r");

		char l_sBuffer[512];
		std::stringstream l_oURLStream;
		while (fgets(l_sBuffer, sizeof(l_sBuffer), l_pCommandPipe) != NULL)
		{
			l_oURLStream << l_sBuffer;
		}
		pclose(l_pCommandPipe);

		auto l_sDeviceURL = l_oURLStream.str();
		l_sDeviceURL = l_sDeviceURL.substr(0, l_sDeviceURL.length() - 1);

		l_pGUI->m_rController.m_sDeviceURL = l_sDeviceURL;
		l_pGUI->m_rController.writeArchwayConfigurationFile();
	}

	void on_button_reinitialize_archway_clicked(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);
		auto l_pEngineTypeCombobox =
		        GTK_COMBO_BOX(gtk_builder_get_object(l_pGUI->m_pBuilder, "combobox-engine-connection-type"));
		auto l_iSelectedEngineType = gtk_combo_box_get_active(l_pEngineTypeCombobox);

		// TODO: For now we always assume LAN Engine connection
		l_iSelectedEngineType = 1;
		l_pGUI->m_rController.reinitializeArchway(l_iSelectedEngineType == 0 ? EngineType::Local : EngineType::LAN);
		l_pGUI->refreshEnginePipelines();
	}

	gboolean on_dialog_engine_configuration_delete_event(::GtkDialog* pDialog, ::GdkEvent*, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pGUI->m_pButtonOpenEngineConfigurationDialog), false);
		gtk_widget_hide(GTK_WIDGET(pDialog));
		return TRUE;
	}

	gboolean on_dialog_pipeline_configuration_delete_event(GtkDialog* pDialog, GdkEvent*, gpointer)
	{
		gtk_dialog_response(pDialog, GTK_RESPONSE_CANCEL);
		gtk_widget_hide(GTK_WIDGET(pDialog));
		return TRUE;
	}

	void on_treeview_engine_pipelines_row_activated(GtkTreeView* pTreeView, GtkTreePath* pPath, GtkTreeViewColumn*, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);

		GtkTreeIter l_oIterator;
		auto l_pTreeModel = gtk_tree_view_get_model(pTreeView);
		if (gtk_tree_model_get_iter(l_pTreeModel, &l_oIterator, pPath))
		{
			guint64 uiPipelineId;

			gtk_tree_model_get(l_pTreeModel, &l_oIterator,
			                   Column_PipelineId, &uiPipelineId,
			                   -1);

			l_pGUI->displayPipelineConfigurationDialog(static_cast<unsigned int>(uiPipelineId));

			auto l_vEnginePipelines = l_pGUI->m_rController.getEnginePipelines();
			for (auto& pipeline : l_vEnginePipelines)
			{
				if (pipeline.id != uiPipelineId)
				{
					continue;
				}
				gtk_list_store_set(GTK_LIST_STORE(l_pTreeModel), &l_oIterator,
				                   Column_PipelineIsConfigured, pipeline.isConfigured,
				                   -1);
			}
		}
	}

	void on_button_start_engine_clicked(GtkWidget* pWidget, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);

		// Get the currently selected pipeline in the list
		auto l_pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(l_pGUI->m_pTreeViewEnginePipelines));
		if (gtk_tree_selection_count_selected_rows(l_pSelection) == 1)
		{
			GtkTreeIter l_oIterator;
			gtk_tree_selection_get_selected(l_pSelection, &l_pGUI->m_pTreeModelEnginePipelines, &l_oIterator);

			unsigned long long uiPipelineId = 0;
			gtk_tree_model_get(l_pGUI->m_pTreeModelEnginePipelines, &l_oIterator,
			                   Column_PipelineId, &uiPipelineId,
			                   -1);
			if (l_pGUI->m_rController.startEngineWithPipeline(static_cast<unsigned int>(uiPipelineId))) {
				gtk_widget_set_sensitive(l_pGUI->m_pButtonStartEngine, false);
				gtk_widget_set_sensitive(l_pGUI->m_pButtonStopEngine, true);
				gtk_widget_set_sensitive(l_pGUI->m_pTreeViewEnginePipelines, false);
				gtk_spinner_start(l_pGUI->m_pSpinnerEngineActivity);
				gtk_widget_show(GTK_WIDGET(l_pGUI->m_pSpinnerEngineActivity));
			}
		}
		else
		{
			auto m_pAlertDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Plase select a pipeline to run.");
			gtk_dialog_run(GTK_DIALOG(m_pAlertDialog));
			gtk_widget_destroy(m_pAlertDialog);
		}
	}

	void on_button_stop_engine_clicked(GtkWidget* pWidget, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);

		if (l_pGUI->m_rController.stopEngine()) {
			gtk_widget_set_sensitive(l_pGUI->m_pButtonStartEngine, true);
			gtk_widget_set_sensitive(l_pGUI->m_pButtonStopEngine, false);
			gtk_widget_set_sensitive(l_pGUI->m_pTreeViewEnginePipelines, true);
			gtk_spinner_stop(l_pGUI->m_pSpinnerEngineActivity);
			gtk_widget_hide(GTK_WIDGET(l_pGUI->m_pSpinnerEngineActivity));
		}
	}

	void on_button_pipeline_configuration_apply_clicked(GtkWidget* pWidget, gpointer)
	{
		GtkWidget* l_pDialog = gtk_widget_get_parent(gtk_widget_get_parent((gtk_widget_get_parent(pWidget))));
		gtk_dialog_response(GTK_DIALOG(l_pDialog), GTK_RESPONSE_APPLY);
		gtk_widget_hide(l_pDialog);
	}

	void on_button_pipeline_configuration_cancel_clicked(GtkWidget* pWidget, gpointer)
	{
		GtkWidget* l_pDialog = gtk_widget_get_parent(gtk_widget_get_parent((gtk_widget_get_parent(pWidget))));
		gtk_dialog_response(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CANCEL);
		gtk_widget_hide(l_pDialog);
	}

	void on_pipeline_configuration_cellrenderer_parameter_value_edited(GtkCellRendererText *pCell, gchar* sPath, gchar* sNewText, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);
		l_pGUI->setPipelineParameterValueAtPath(sPath, sNewText);
	}

	gboolean on_pipeline_configuration_cellrenderer_parameter_value_entry_focus_out(GtkWidget* pWidget, GdkEvent* pEvent, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);
		auto l_pEntry = GTK_ENTRY(pWidget);
		l_pGUI->setPipelineParameterValueAtPath(l_pGUI->m_sCurrentlyEditedCellPath.c_str(), gtk_entry_get_text(l_pEntry));
		return FALSE;
	}

	void on_pipeline_configuration_cellrenderer_parameter_value_editing_started(GtkCellRenderer* pCell, GtkCellEditable* pEditable, gchar* sPath, gpointer pUserData)
	{
		auto l_pGUI = static_cast<CArchwayHandlerGUI*>(pUserData);

		// As we are sending data to a callback created within a callback, we really want to avoid
		// allocating memory.
		l_pGUI->m_sCurrentlyEditedCellPath = sPath;

		auto l_pEntry = GTK_ENTRY(pEditable);
		g_signal_connect(G_OBJECT(l_pEntry),
		                 "focus-out-event",
		                 G_CALLBACK(on_pipeline_configuration_cellrenderer_parameter_value_entry_focus_out), pUserData);
	}


}

CArchwayHandlerGUI::CArchwayHandlerGUI(CArchwayHandler& rController)
    : m_rController(rController)
{
	m_pBuilder = gtk_builder_new();
	GError* l_pError = nullptr;
	gtk_builder_add_from_file(m_pBuilder, OpenViBE::Directories::getDataDir() + "/applications/designer/interface-archway.ui", &l_pError);

	assert(l_pError == nullptr);
//	gtk_builder_connect_signals(m_pBuilder, NULL);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "button-configure-acquisition")),
	                 "clicked", G_CALLBACK(on_button_configure_acquisition_clicked), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "button-reinitialize-archway")),
	                 "clicked", G_CALLBACK(on_button_reinitialize_archway_clicked), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "button-pipeline-configuration-apply")),
	                 "clicked", G_CALLBACK(on_button_pipeline_configuration_apply_clicked), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "button-pipeline-configuration-cancel")),
	                 "clicked", G_CALLBACK(on_button_pipeline_configuration_cancel_clicked), this);

	m_pSpinnerEngineActivity = GTK_SPINNER(gtk_builder_get_object(m_pBuilder, "spinner-engine-activity"));
	gtk_widget_hide(GTK_WIDGET(m_pSpinnerEngineActivity));

	m_pButtonStartEngine = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "button-start-engine"));
	m_pButtonStopEngine = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "button-stop-engine"));
	gtk_widget_set_sensitive(m_pButtonStopEngine, false);

	g_signal_connect(G_OBJECT(m_pButtonStartEngine), "clicked", G_CALLBACK(on_button_start_engine_clicked), this);
	g_signal_connect(G_OBJECT(m_pButtonStopEngine), "clicked", G_CALLBACK(on_button_stop_engine_clicked), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "dialog-engine-configuration")),
	                 "delete-event", G_CALLBACK(on_dialog_engine_configuration_delete_event), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "dialog-pipeline-configuration")),
	                 "delete-event", G_CALLBACK(on_dialog_pipeline_configuration_delete_event), this);

	m_pTreeViewEnginePipelines = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "treeview-engine-pipelines"));
	g_signal_connect(G_OBJECT(m_pTreeViewEnginePipelines), "row-activated", G_CALLBACK(on_treeview_engine_pipelines_row_activated), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "pipeline-configuration-cellrenderer-parameter-value")),
	                 "edited",
	                 G_CALLBACK(on_pipeline_configuration_cellrenderer_parameter_value_edited), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilder, "pipeline-configuration-cellrenderer-parameter-value")),
	                 "editing-started",
	                 G_CALLBACK(on_pipeline_configuration_cellrenderer_parameter_value_editing_started), this);

	m_pTreeModelEnginePipelines = GTK_TREE_MODEL(gtk_builder_get_object(m_pBuilder, "liststore-pipelines"));
}

CArchwayHandlerGUI::~CArchwayHandlerGUI()
{
	if(m_pBuilder)
	{
		g_object_unref(G_OBJECT(m_pBuilder));
	}
}

void CArchwayHandlerGUI::refreshEnginePipelines()
{
	auto l_pListStorePipelines = GTK_LIST_STORE(m_pTreeModelEnginePipelines);

	gtk_list_store_clear(l_pListStorePipelines);
	auto l_vEnginePipelines = m_rController.getEnginePipelines();
	GtkTreeIter iter;
	for (auto& pipeline : l_vEnginePipelines)
	{
		std::stringstream l_sPipelineIdInHex;
		l_sPipelineIdInHex << std::hex << pipeline.id;

		gtk_list_store_append(l_pListStorePipelines, &iter);
		gtk_list_store_set(l_pListStorePipelines, &iter,
		                   Column_PipelineId, pipeline.id,
		                   Column_PipelineHexId, l_sPipelineIdInHex.str().c_str(),
		                   Column_PipelineDescription, pipeline.description.c_str(),
		                   Column_PipelineIsConfigured, pipeline.isConfigured,
		                   -1);
	}

}

void CArchwayHandlerGUI::toggleNeuroRTEngineConfigurationDialog(bool bShouldDisplay)
{
	auto l_pEngineConfigurationWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "dialog-engine-configuration"));

	if (bShouldDisplay)
	{
		this->refreshEnginePipelines();
		gtk_widget_show(l_pEngineConfigurationWidget);
	}
	else
	{
		gtk_widget_hide(l_pEngineConfigurationWidget);
	}
}

void CArchwayHandlerGUI::displayPipelineConfigurationDialog(unsigned int uiPipelineId)
{
	auto l_pPipelineConfigurationWidget =
	        GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "dialog-pipeline-configuration"));
	auto l_pPipelineConfigurationListStore =
	        GTK_LIST_STORE(gtk_builder_get_object(m_pBuilder, "liststore-pipeline-configuration"));

	gtk_list_store_clear(l_pPipelineConfigurationListStore);

	auto l_vPipelineParameters = m_rController.getPipelineParameters(uiPipelineId);

	GtkTreeIter iter;
	for (auto& parameter : l_vPipelineParameters)
	{
		gtk_list_store_append(l_pPipelineConfigurationListStore, &iter);
		unsigned long long l_ui64PipelineId = uiPipelineId;
		gtk_list_store_set(l_pPipelineConfigurationListStore, &iter,
						   Column_SettingPipelineId, l_ui64PipelineId,
		                   Column_SettingName, parameter.name.c_str(),
		                   Column_SettingDefaultValue, parameter.defaultValue.c_str(),
		                   Column_SettingValue, parameter.value.c_str(),
		                   -1);
	}

	auto l_vSavedSettings = m_rController.getPipelineSettings(uiPipelineId);
	auto l_iResponse = gtk_dialog_run(GTK_DIALOG(l_pPipelineConfigurationWidget));
	if (l_iResponse == GTK_RESPONSE_CANCEL)
	{
		m_rController.getPipelineSettings(uiPipelineId) = l_vSavedSettings;
	}
}

bool CArchwayHandlerGUI::setPipelineParameterValueAtPath(gchar const* sPath, gchar const* sNewValue)
{
	auto l_pPipelineConfigurationListStore =
	        GTK_TREE_MODEL(gtk_builder_get_object(this->m_pBuilder, "liststore-pipeline-configuration"));

	GtkTreeIter l_oIterator;
	gboolean l_bIsIteratorValid = gtk_tree_model_get_iter_from_string(l_pPipelineConfigurationListStore, &l_oIterator, sPath);
	assert(l_bIsIteratorValid);

	unsigned long long uiPipelineId = 0;
	gchar* sParameterName;
	
	gtk_tree_model_get(l_pPipelineConfigurationListStore, &l_oIterator,
	                   Column_SettingName, &sParameterName,
					   -1);
					   
	gtk_tree_model_get(l_pPipelineConfigurationListStore, &l_oIterator,
					   Column_SettingPipelineId, &uiPipelineId,
					   -1);
	
	gtk_list_store_set(GTK_LIST_STORE(l_pPipelineConfigurationListStore), &l_oIterator,
					   Column_SettingValue, sNewValue,
					   -1);

	this->m_rController.setPipelineParameterValue(static_cast<unsigned int>(uiPipelineId), sParameterName, sNewValue);
	g_free(sParameterName);

	return true;
}

#endif
