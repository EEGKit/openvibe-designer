#pragma once

#ifdef MENSIA_DISTRIBUTION

#include "ovdCArchwayHandler.h"
#include <gtk/gtk.h>

namespace OpenViBE
{
	namespace Designer
	{
	class CApplication;
	}
}

namespace Mensia
{
	class CArchwayHandlerGUI final
	{
	public:
		CArchwayHandlerGUI(CArchwayHandler& controller, OpenViBE::Designer::CApplication* application);
		~CArchwayHandlerGUI();

		void refreshEnginePipelines();
		void toggleNeuroRTEngineConfigurationDialog(const bool shouldDisplay);
		void displayPipelineConfigurationDialog(const size_t pipelineID);

		bool setPipelineParameterValueAtPath(gchar const* path, gchar const* value);

		GtkBuilder* m_Builder            = nullptr;
		GtkBuilder* m_ApplicationBuilder = nullptr;
		CArchwayHandler& m_Controller;
		OpenViBE::Designer::CApplication* m_Application = nullptr;

		// This variable is used to store the path of an edited cell
		// while editing the PipelineParameters.
		std::string m_CurrentlyEditedCellPath;

		GtkWidget* m_ButtonOpenEngineConfigurationDialog = nullptr;

		GtkWidget* m_ButtonConfigureAcquisition       = nullptr;
		GtkToggleToolButton* m_ToggleAcquireImpedance = nullptr;

		GtkComboBox* m_ComboBoxEngineType      = nullptr;
		GtkWidget* m_ButtonReinitializeArchway = nullptr;
		GtkWidget* m_ButtonLaunchEngine        = nullptr;

		GtkWidget* m_ButtonStartEngine           = nullptr;
		GtkWidget* m_ButtonStartEngineFastFoward = nullptr;
		GtkWidget* m_ButtonStopEngine            = nullptr;

		GtkWidget* m_TreeViewEnginePipelines = nullptr;

		GtkSpinner* m_SpinnerEngineActivity = nullptr;

		GtkTreeModel* m_TreeModelEnginePipelines = nullptr;
		GtkTreeIter m_SelectedPipelineIter;
	};
}

#endif
