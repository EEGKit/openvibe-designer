#pragma once

#if defined TARGET_HAS_LibArchway

#include "ovdCArchwayHandler.h"
#include <gtk/gtk.h>

namespace Mensia {

	// Replace this line when we stop supporting gcc 4.6.3 (Ubuntu 12.04)
	//class CArchwayHandlerGUI final {
	class CArchwayHandlerGUI {
	public:
		CArchwayHandlerGUI(CArchwayHandler& rController);
		~CArchwayHandlerGUI();

		void refreshEnginePipelines();
		void toggleNeuroRTEngineConfigurationDialog(bool bShouldDisplay);
		void displayPipelineConfigurationDialog(unsigned int uiPipelineId);

		bool setPipelineParameterValueAtPath(gchar const* sPath, gchar const* sNewValue);

	public:
		GtkBuilder* m_pBuilder;
		GtkBuilder* m_pApplicationBuilder;
		CArchwayHandler& m_rController;

		// This variable is used to store the path of an edited cell
		// while editing the PipelineParameters.
		std::string m_sCurrentlyEditedCellPath;

		GtkWidget* m_pButtonOpenEngineConfigurationDialog;
		GtkWidget* m_pButtonReinitializeArchway;

		GtkWidget* m_pButtonStartEngine;
		GtkWidget* m_pButtonStopEngine;

		GtkWidget* m_pTreeViewEnginePipelines;

		GtkSpinner* m_pSpinnerEngineActivity;

		GtkTreeModel* m_pTreeModelEnginePipelines;

	private:
	};
}

#endif
