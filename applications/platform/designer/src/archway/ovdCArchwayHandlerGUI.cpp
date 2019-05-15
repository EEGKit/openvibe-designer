#ifdef MENSIA_DISTRIBUTION
#include "ovdCArchwayHandlerGUI.h"
#include <openvibe/ov_directories.h>
#include <fs/Files.h>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <sstream>

#include "../ovdCApplication.h"
#include "ovProcessUtilities.hpp"

#if defined TARGET_OS_Windows
#define pclose _pclose
#endif

using namespace Mensia;
using namespace OpenViBEDesigner;

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
		Column_SettingValue = 3
	};

	void on_button_configure_acquisition_clicked(GtkMenuItem* menuItem, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);

		std::string configurationToolLaunchCmd = std::string(OpenViBE::Directories::getBinDir().toASCIIString()) + "/mensia-device-configuration";
		//std::string configurationFilePath = std::string(getenv("USERPROFILE")) + "\\lib-mensia-engine.conf";

		std::string escapedURL = gui->m_Controller.m_DeviceURL;

		size_t startPosition = 0;
		while ((startPosition = escapedURL.find("\"", startPosition)) != std::string::npos)
		{
			escapedURL.replace(startPosition, 1, "\\\"");
			startPosition += 2; // Handles case where 'to' is a substring of 'from'
		}


#if defined TARGET_OS_Windows
		std::string command = "\"\"" + configurationToolLaunchCmd + "\" --no-file \"" + escapedURL + "\"\"";
		for (std::string stringToEscape : {"^", "<", ">", "&", "|", "%"}) // ^ must be escaped first!
		{
			size_t startPosition = 0;
			while ((startPosition = command.find(stringToEscape, startPosition)) != std::string::npos)
			{
				command.replace(startPosition, stringToEscape.size(), "^" + stringToEscape);
				startPosition += 1 + stringToEscape.size();
			}
		}
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		std::string command = configurationToolLaunchCmd + " --no-file \"" + escapedURL + "\"";
#endif
		FILE* commandPipe = FS::Files::popen(command.c_str(), "r");

		char buffer[512];
		std::stringstream urlStream;
		while (fgets(buffer, sizeof(buffer), commandPipe) != nullptr)
		{
			urlStream << buffer;
		}
		pclose(commandPipe);

		auto deviceURL = urlStream.str();
		deviceURL = deviceURL.substr(0, deviceURL.length() - 1);

		gui->m_Controller.m_DeviceURL = deviceURL;
		gui->m_Controller.writeArchwayConfigurationFile();
	}

	void on_button_reinitialize_archway_clicked(::GtkMenuItem*, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		auto engineTypeCombobox = GTK_COMBO_BOX(gtk_builder_get_object(gui->m_Builder, "combobox-engine-connection-type"));
		gui->m_Controller.m_EngineType = (gtk_combo_box_get_active(engineTypeCombobox) == 0 ? EngineType::Local : EngineType::LAN);

		gui->m_Controller.reinitializeArchway();
		gui->refreshEnginePipelines();
	}

	void on_combobox_engine_type_changed(::GtkComboBox*, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		if (gtk_combo_box_get_active(gui->m_ComboBoxEngineType) == 0)
		{
			gtk_widget_set_sensitive(gui->m_ButtonLaunchEngine, FALSE);
		}
		else
		{
			gtk_widget_set_sensitive(gui->m_ButtonLaunchEngine, TRUE);
		}
		on_button_reinitialize_archway_clicked(nullptr, userData);
	}

	gboolean on_dialog_engine_configuration_delete_event(::GtkDialog * dialog, ::GdkEvent*, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gui->m_ButtonOpenEngineConfigurationDialog), false);
		gtk_widget_hide(GTK_WIDGET(dialog));
		return TRUE;
	}

	gboolean on_dialog_pipeline_configuration_delete_event(GtkDialog * pDialog, GdkEvent*, gpointer)
	{
		gtk_dialog_response(pDialog, GTK_RESPONSE_CANCEL);
		gtk_widget_hide(GTK_WIDGET(pDialog));
		return TRUE;
	}

	void view_pipeline_config(CArchwayHandlerGUI * gui)
	{
		auto treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(gui->m_TreeViewEnginePipelines));
		guint64 pipelineId;
		gtk_tree_model_get(treeModel, &gui->m_SelectedPipelineIter, Column_PipelineId, &pipelineId, -1);

		gui->displayPipelineConfigurationDialog(static_cast<unsigned int>(pipelineId));

		auto enginePipelines = gui->m_Controller.getEnginePipelines();
		for (const auto& pipeline : enginePipelines)
		{
			if (pipeline.id != pipelineId)
			{
				continue;
			}
			gtk_list_store_set(GTK_LIST_STORE(treeModel), &gui->m_SelectedPipelineIter, Column_PipelineIsConfigured, pipeline.isConfigured, -1);
		}
	}

	void view_popup_menu_onOpenScenario(GtkWidget * menuitem, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		auto treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(gui->m_TreeViewEnginePipelines));
		guint64 pipelineId;
		gtk_tree_model_get(treeModel, &gui->m_SelectedPipelineIter, Column_PipelineId, &pipelineId, -1);

		if (gui->m_Controller.m_EngineType == EngineType::Local)
		{
			std::string path = gui->m_Controller.getPipelineScenarioPath(uint64_t(pipelineId));
			gui->m_Application->openScenario(path.c_str());
		}
	}

	void view_popup_menu_onConfItem(GtkWidget * menuitem, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		view_pipeline_config(gui);
	}

	void on_treeview_engine_pipelines_popup_menu(CArchwayHandlerGUI * gui, GdkEventButton * event)
	{
		GtkWidget* menu = gtk_menu_new();
		GtkWidget* openConfItem = gtk_menu_item_new_with_label("Open configuration file");
		g_signal_connect(openConfItem, "activate", (GCallback)view_popup_menu_onConfItem, gui);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), openConfItem);

		if (gui->m_Controller.m_EngineType == EngineType::Local)
		{
			GtkWidget* openScenarioItem = gtk_menu_item_new_with_label("Open scenario");
			g_signal_connect(openScenarioItem, "activate", (GCallback)view_popup_menu_onOpenScenario, gui);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), openScenarioItem);
		}

		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), nullptr, nullptr, nullptr, nullptr, event->button, gdk_event_get_time((GdkEvent*)event));
	}

	gboolean on_treeview_engine_pipelines_button_pressed(GtkWidget * treeView, GdkEventButton * event, gpointer userData)
	{
		// Only acknowledge, if it's a right click, or double click
		if (!(event->type == GDK_BUTTON_PRESS && event->button == 3) && !(event->type == GDK_2BUTTON_PRESS && event->button == 1)) { return FALSE; }

		auto gui = static_cast<CArchwayHandlerGUI*>(userData);

		GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gui->m_TreeViewEnginePipelines));
		if (gtk_tree_selection_count_selected_rows(selection) == 1)
		{
			gtk_tree_selection_get_selected(selection, &gui->m_TreeModelEnginePipelines, &gui->m_SelectedPipelineIter);

			// single click with the right mouse button
			if (event->type == GDK_BUTTON_PRESS && event->button == 3)
			{
				on_treeview_engine_pipelines_popup_menu(gui, event);
				return TRUE;
			}
			else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
			{
				view_pipeline_config(gui);
				return TRUE;
			}
		}
		return FALSE;
	}

	void startEngine(GtkWidget * widget, gpointer userData, bool isFastForward)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);

		// Get the currently selected pipeline in the list
		auto selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gui->m_TreeViewEnginePipelines));

		if (gtk_tree_selection_count_selected_rows(selection) == 1)
		{
			gtk_tree_selection_get_selected(selection, &gui->m_TreeModelEnginePipelines, &gui->m_SelectedPipelineIter);

			unsigned long long pipelineId = 0;
			gtk_tree_model_get(gui->m_TreeModelEnginePipelines,
				&gui->m_SelectedPipelineIter,
				Column_PipelineId,
				&pipelineId,
				-1);

			bool shouldAcquireImpedance = (gtk_toggle_tool_button_get_active(gui->m_ToggleAcquireImpedance) == gboolean(true));
			if (gui->m_Controller.startEngineWithPipeline(static_cast<unsigned int>(pipelineId), isFastForward, shouldAcquireImpedance))
			{
				gtk_widget_set_sensitive(GTK_WIDGET(gui->m_ToggleAcquireImpedance), false);
				gtk_widget_set_sensitive(GTK_WIDGET(gui->m_ComboBoxEngineType), false);
				gtk_widget_set_sensitive(gui->m_ButtonConfigureAcquisition, false);
				gtk_widget_set_sensitive(gui->m_ButtonLaunchEngine, false);
				gtk_widget_set_sensitive(gui->m_ButtonStartEngine, false);
				gtk_widget_set_sensitive(gui->m_ButtonStartEngineFastFoward, false);
				gtk_widget_set_sensitive(gui->m_ButtonStopEngine, true);
				gtk_widget_set_sensitive(gui->m_TreeViewEnginePipelines, false);
				gtk_spinner_start(gui->m_SpinnerEngineActivity);
				gtk_widget_show(GTK_WIDGET(gui->m_SpinnerEngineActivity));
			}
		}
		else
		{
			auto alertDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Please select a pipeline to run.");
			gtk_dialog_run(GTK_DIALOG(alertDialog));
			gtk_widget_destroy(alertDialog);
		}
	}

	void on_button_start_engine_clicked(GtkWidget * widget, gpointer userData)
	{
		startEngine(widget, userData, false);
	}

	void on_button_start_engine_fast_forward_clicked(GtkWidget * widget, gpointer userData)
	{
		startEngine(widget, userData, true);
	}

	void on_button_stop_engine_clicked(GtkWidget * widget, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);

		if (gui->m_Controller.stopEngine())
		{
			gtk_widget_set_sensitive(GTK_WIDGET(gui->m_ToggleAcquireImpedance), true);
			gtk_widget_set_sensitive(GTK_WIDGET(gui->m_ComboBoxEngineType), true);
			gtk_widget_set_sensitive(gui->m_ButtonConfigureAcquisition, true);
			gtk_widget_set_sensitive(gui->m_ButtonLaunchEngine, true);
			gtk_widget_set_sensitive(gui->m_ButtonStartEngine, true);
			gtk_widget_set_sensitive(gui->m_ButtonStartEngineFastFoward, true);
			gtk_widget_set_sensitive(gui->m_ButtonStopEngine, false);
			gtk_widget_set_sensitive(gui->m_TreeViewEnginePipelines, true);
			gtk_spinner_stop(gui->m_SpinnerEngineActivity);
			gtk_widget_hide(GTK_WIDGET(gui->m_SpinnerEngineActivity));
		}
	}

	void on_button_pipeline_configuration_apply_clicked(GtkWidget * widget, gpointer)
	{
		GtkWidget* dialog = gtk_widget_get_parent(gtk_widget_get_parent((gtk_widget_get_parent(widget))));
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_APPLY);
		gtk_widget_hide(dialog);
	}

	void on_button_pipeline_configuration_cancel_clicked(GtkWidget * widget, gpointer)
	{
		GtkWidget* dialog = gtk_widget_get_parent(gtk_widget_get_parent((gtk_widget_get_parent(widget))));
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
		gtk_widget_hide(dialog);
	}

	void on_pipeline_configuration_cellrenderer_parameter_value_edited(GtkCellRendererText * cell, gchar * path, gchar * newText, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		gui->setPipelineParameterValueAtPath(path, newText);
	}

	gboolean on_pipeline_configuration_cellrenderer_parameter_value_entry_focus_out(GtkWidget * widget, GdkEvent * event, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);
		auto entry = GTK_ENTRY(widget);
		gui->setPipelineParameterValueAtPath(gui->m_CurrentlyEditedCellPath.c_str(), gtk_entry_get_text(entry));
		return FALSE;
	}

	void on_pipeline_configuration_cellrenderer_parameter_value_editing_started(GtkCellRenderer * pCell, GtkCellEditable * pEditable, gchar * sPath, gpointer userData)
	{
		auto gui = static_cast<CArchwayHandlerGUI*>(userData);

		// As we are sending data to a callback created within a callback, we really want to avoid
		// allocating memory.
		gui->m_CurrentlyEditedCellPath = sPath;

		auto entry = GTK_ENTRY(pEditable);
		g_signal_connect(G_OBJECT(entry),
			"focus-out-event",
			G_CALLBACK(on_pipeline_configuration_cellrenderer_parameter_value_entry_focus_out), userData);
	}


	void on_button_launch_engine_clicked(GtkWidget * widget, gpointer userData)
	{
		//		auto gui = static_cast<CArchwayHandlerGUI*>(userData);

		if (!OpenViBE::ProcessUtilities::doesProcessExist("mensia-engine-server")) {
#if defined TARGET_OS_Windows
			if (FS::Files::fileExists(OpenViBE::Directories::getBinDir() + "/mensia-engine-server.exe"))
			{
				OpenViBE::ProcessUtilities::launchCommand(std::string(OpenViBE::Directories::getBinDir() + "/mensia-engine-server.exe").c_str());
			}
			else if (FS::Files::fileExists("C:/Program Files (x86)/NeuroRT/NeuroRT Engine Server/bin/mensia-engine-server.exe"))
			{
				OpenViBE::ProcessUtilities::launchCommand(std::string("C:/Program Files (x86)/NeuroRT/NeuroRT Engine Server/neurort-engine-server.cmd").c_str());
			}
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			if (FS::Files::fileExists(OpenViBE::Directories::getBinDir() + "/mensia-engine-server"))
			{
				OpenViBE::ProcessUtilities::launchCommand(std::string(OpenViBE::Directories::getBinDir() + "/mensia-engine-server").c_str());
			}
#endif
		}
	}
}

CArchwayHandlerGUI::CArchwayHandlerGUI(CArchwayHandler& controller, OpenViBEDesigner::CApplication * application)
	: m_Controller(controller),
	m_Application(application)
{
	m_Builder = gtk_builder_new();
	GError* gtkError = nullptr;
	gtk_builder_add_from_file(m_Builder, OpenViBE::Directories::getDataDir() + "/applications/designer/interface-archway.ui", &gtkError);

	assert(gtkError == nullptr);
	//	gtk_builder_connect_signals(m_pBuilder, nullptr);


	m_ButtonConfigureAcquisition = GTK_WIDGET(gtk_builder_get_object(m_Builder, "button-configure-acquisition"));
	g_signal_connect(G_OBJECT(m_ButtonConfigureAcquisition), "clicked", G_CALLBACK(on_button_configure_acquisition_clicked), this);

	m_ButtonReinitializeArchway = GTK_WIDGET(gtk_builder_get_object(m_Builder, "button-reinitialize-archway"));
	g_signal_connect(G_OBJECT(m_ButtonReinitializeArchway), "clicked", G_CALLBACK(on_button_reinitialize_archway_clicked), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "button-pipeline-configuration-apply")),
		"clicked", G_CALLBACK(on_button_pipeline_configuration_apply_clicked), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "button-pipeline-configuration-cancel")),
		"clicked", G_CALLBACK(on_button_pipeline_configuration_cancel_clicked), this);

	m_SpinnerEngineActivity = GTK_SPINNER(gtk_builder_get_object(m_Builder, "spinner-engine-activity"));
	gtk_widget_hide(GTK_WIDGET(m_SpinnerEngineActivity));

	m_ToggleAcquireImpedance = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "toggle-acquire-impedance"));

	m_ButtonStartEngine = GTK_WIDGET(gtk_builder_get_object(m_Builder, "button-start-engine"));
	m_ButtonStartEngineFastFoward = GTK_WIDGET(gtk_builder_get_object(m_Builder, "button-start-engine-fast"));
	m_ButtonStopEngine = GTK_WIDGET(gtk_builder_get_object(m_Builder, "button-stop-engine"));
	gtk_widget_set_sensitive(m_ButtonStopEngine, false);

	g_signal_connect(G_OBJECT(m_ButtonStartEngine), "clicked", G_CALLBACK(on_button_start_engine_clicked), this);
	g_signal_connect(G_OBJECT(m_ButtonStartEngineFastFoward), "clicked", G_CALLBACK(on_button_start_engine_fast_forward_clicked), this);
	g_signal_connect(G_OBJECT(m_ButtonStopEngine), "clicked", G_CALLBACK(on_button_stop_engine_clicked), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "dialog-engine-configuration")),
		"delete-event", G_CALLBACK(on_dialog_engine_configuration_delete_event), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "dialog-pipeline-configuration")),
		"delete-event", G_CALLBACK(on_dialog_pipeline_configuration_delete_event), this);

	m_TreeViewEnginePipelines = GTK_WIDGET(gtk_builder_get_object(m_Builder, "treeview-engine-pipelines"));
	g_signal_connect(m_TreeViewEnginePipelines, "button-press-event", (GCallback)on_treeview_engine_pipelines_button_pressed, this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "pipeline-configuration-cellrenderer-parameter-value")),
		"edited",
		G_CALLBACK(on_pipeline_configuration_cellrenderer_parameter_value_edited), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "pipeline-configuration-cellrenderer-parameter-value")),
		"editing-started",
		G_CALLBACK(on_pipeline_configuration_cellrenderer_parameter_value_editing_started), this);

	m_TreeModelEnginePipelines = GTK_TREE_MODEL(gtk_builder_get_object(m_Builder, "liststore-pipelines"));

	m_Controller.m_GUIBridge.refreshStoppedEngine = [this]() {
		gtk_widget_set_sensitive(GTK_WIDGET(m_ToggleAcquireImpedance), true);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ComboBoxEngineType), true);
		gtk_widget_set_sensitive(m_ButtonConfigureAcquisition, true);
		gtk_widget_set_sensitive(m_ButtonLaunchEngine, true);
		gtk_widget_set_sensitive(m_ButtonStartEngine, true);
		gtk_widget_set_sensitive(m_ButtonStartEngineFastFoward, true);
		gtk_widget_set_sensitive(m_ButtonStopEngine, false);
		gtk_widget_set_sensitive(m_TreeViewEnginePipelines, true);
		gtk_spinner_stop(m_SpinnerEngineActivity);
		gtk_widget_hide(GTK_WIDGET(m_SpinnerEngineActivity));
	};

	m_ComboBoxEngineType = GTK_COMBO_BOX(gtk_builder_get_object(m_Builder, "combobox-engine-connection-type"));
	g_signal_connect(G_OBJECT(m_ComboBoxEngineType), "changed", G_CALLBACK(on_combobox_engine_type_changed), this);

	m_ButtonLaunchEngine = GTK_WIDGET(gtk_builder_get_object(m_Builder, "button-launch-engine-server"));
	g_signal_connect(G_OBJECT(m_ButtonLaunchEngine), "clicked", G_CALLBACK(on_button_launch_engine_clicked), this);

}

CArchwayHandlerGUI::~CArchwayHandlerGUI()
{
	if (m_Builder)
	{
		g_object_unref(G_OBJECT(m_Builder));
	}
}

void CArchwayHandlerGUI::refreshEnginePipelines()
{
	auto listStorePipelines = GTK_LIST_STORE(m_TreeModelEnginePipelines);

	gtk_list_store_clear(listStorePipelines);
	auto enginePipelines = m_Controller.getEnginePipelines();
	GtkTreeIter iter;
	for (auto& pipeline : enginePipelines)
	{
		std::stringstream pipelineIdInHex;
		pipelineIdInHex << std::hex << pipeline.id;

		gtk_list_store_append(listStorePipelines, &iter);
		gtk_list_store_set(listStorePipelines, &iter,
			Column_PipelineId, pipeline.id,
			Column_PipelineHexId, pipelineIdInHex.str().c_str(),
			Column_PipelineDescription, pipeline.description.c_str(),
			Column_PipelineIsConfigured, pipeline.isConfigured,
			-1);
	}

}

void CArchwayHandlerGUI::toggleNeuroRTEngineConfigurationDialog(bool shouldDisplay)
{
	auto engineConfigurationWidget = GTK_WIDGET(gtk_builder_get_object(m_Builder, "dialog-engine-configuration"));

	if (shouldDisplay)
	{
		gtk_combo_box_set_active(m_ComboBoxEngineType, m_Controller.m_EngineType == EngineType::Local ? 0 : 1);
		this->refreshEnginePipelines();
		gtk_widget_show(engineConfigurationWidget);
	}
	else
	{
		gtk_widget_hide(engineConfigurationWidget);
	}
}

void CArchwayHandlerGUI::displayPipelineConfigurationDialog(unsigned int pipelineId)
{
	auto pipelineConfigurationWidget = GTK_WIDGET(gtk_builder_get_object(m_Builder, "dialog-pipeline-configuration"));
	auto pipelineConfigurationListStore = GTK_LIST_STORE(gtk_builder_get_object(m_Builder, "liststore-pipeline-configuration"));

	gtk_list_store_clear(pipelineConfigurationListStore);

	auto pipelineParameters = m_Controller.getPipelineParameters(pipelineId);

	GtkTreeIter iter;
	for (auto& parameter : pipelineParameters)
	{
		gtk_list_store_append(pipelineConfigurationListStore, &iter);
		gtk_list_store_set(pipelineConfigurationListStore, &iter,
			Column_SettingPipelineId, static_cast<unsigned long long>(pipelineId),
			Column_SettingName, parameter.name.c_str(),
			Column_SettingDefaultValue, parameter.defaultValue.c_str(),
			Column_SettingValue, parameter.value.c_str(),
			-1);
	}

	auto savedSettings = m_Controller.getPipelineSettings(pipelineId);
	auto response = gtk_dialog_run(GTK_DIALOG(pipelineConfigurationWidget));
	if (response == GTK_RESPONSE_CANCEL)
	{
		m_Controller.getPipelineSettings(pipelineId) = savedSettings;
	}
}

bool CArchwayHandlerGUI::setPipelineParameterValueAtPath(gchar const* path, gchar const* newValue)
{
	auto pipelineConfigurationListStore =
		GTK_TREE_MODEL(gtk_builder_get_object(this->m_Builder, "liststore-pipeline-configuration"));

	GtkTreeIter parameterIter;
	gboolean isIteratorValid = gtk_tree_model_get_iter_from_string(pipelineConfigurationListStore, &parameterIter, path);
	assert(isIteratorValid);

	unsigned long long pipelineId = 0;
	gchar* parameterName;

	gtk_tree_model_get(pipelineConfigurationListStore, &parameterIter,
		Column_SettingName, &parameterName,
		-1);

	gtk_tree_model_get(pipelineConfigurationListStore, &parameterIter,
		Column_SettingPipelineId, &pipelineId,
		-1);

	gtk_list_store_set(GTK_LIST_STORE(pipelineConfigurationListStore), &parameterIter,
		Column_SettingValue, newValue,
		-1);

	this->m_Controller.setPipelineParameterValue(static_cast<unsigned int>(pipelineId), parameterName, newValue);
	g_free(parameterName);

	return true;
}

#endif
