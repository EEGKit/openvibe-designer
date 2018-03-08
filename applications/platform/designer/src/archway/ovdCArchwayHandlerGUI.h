#pragma once

#ifdef MENSIA_DISTRIBUTION

#include "ovdCArchwayHandler.h"
#include <gtk/gtk.h>

namespace Mensia {

	// Replace this line when we stop supporting gcc 4.6.3 (Ubuntu 12.04)
	//class CArchwayHandlerGUI final {
	class CArchwayHandlerGUI {
	public:
		CArchwayHandlerGUI(CArchwayHandler& controller);
		~CArchwayHandlerGUI();

		void refreshEnginePipelines();
		void toggleNeuroRTEngineConfigurationDialog(bool shouldDisplay);
		void displayPipelineConfigurationDialog(unsigned int pipelineId);

		bool setPipelineParameterValueAtPath(gchar const* path, gchar const* newValue);

	public:
		GtkBuilder* m_Builder;
		GtkBuilder* m_ApplicationBuilder;
		CArchwayHandler& m_Controller;

		// This variable is used to store the path of an edited cell
		// while editing the PipelineParameters.
		std::string m_CurrentlyEditedCellPath;

		GtkWidget* m_ButtonOpenEngineConfigurationDialog;

		GtkWidget* m_ButtonConfigureAcquisition;
		GtkToggleToolButton* m_ToggleAcquireImpedance;

		GtkComboBox* m_ComboBoxEngineType;
		GtkWidget* m_ButtonReinitializeArchway;
		GtkWidget* m_ButtonLaunchEngine;

		GtkWidget* m_ButtonStartEngine;
		GtkWidget* m_ButtonStartEngineFastFoward;
		GtkWidget* m_ButtonStopEngine;

		GtkWidget* m_TreeViewEnginePipelines;

		GtkSpinner* m_SpinnerEngineActivity;

		GtkTreeModel* m_TreeModelEnginePipelines;

	};
}

#endif
