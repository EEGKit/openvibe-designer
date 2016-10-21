#pragma once

#include <openvibe/plugins/ovIPluginObject.h>
#include <toolkit/ovtk_all.h>
#include <gtk/gtk.h>

#include "ovvizIVisualizationManager.h"

#define OV_ClassId_VisualizationContext OpenViBE::CIdentifier(0xE06B92EF, 0xB6B68081)

namespace OpenViBEVisualizationToolkit
{
	class IVisualizationContext : public OpenViBE::Plugins::IPluginObject
	{
	public:
		/**
		 * @brief Assing a visualization manager to the visualization context singleton
		 * @param visualizationManager The visualization manager that handles visualization widgets for the application
		 * @retval true In case of success.
		 * @retval false In case of error.
		 */
		virtual bool setManager(OpenViBEVisualizationToolkit::IVisualizationManager* visualizationManager) = 0;

		/**
		 * @brief Forward a GtkWidget which displays content to the visualization manager
		 * @param box The current box.
		 * @param widget The Gtk widget used to display the content.
		 * @retval true In case of success.
		 * @retval false In case of error.
		 */
		virtual bool setWidget(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* widget) = 0;

		/**
		 * @brief Forward a GtkWidget which displays the box toolbar to the visualization manager
		 * @param box The current box.
		 * @param widget The Gtk widget used to display the controls of the visualization.
		 * @retval true In case of success.
		 * @retval false In case of error.
		 */
		virtual bool setToolbar(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* toolbarWidget) = 0;

		_IsDerivedFromClass_(OpenViBE::Plugins::IPluginObject, OV_ClassId_VisualizationContext)
	};
}
