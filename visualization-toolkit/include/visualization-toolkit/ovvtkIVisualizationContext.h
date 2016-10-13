#pragma once

#include <openvibe/plugins/ovIPluginObject.h>
#include <toolkit/ovtk_all.h>
#include <gtk/gtk.h>

#include "ovvtkIVisualizationManager.h"

#define OV_ClassId_VisualizationContext OpenViBE::CIdentifier(0xE06B92EF, 0xB6B68081)

namespace OpenViBEVisualizationToolkit
{
	class IVisualizationContext : public OpenViBE::Plugins::IPluginObject
	{
	public:
		virtual bool setManager(OpenViBEVisualizationToolkit::IVisualisationManager* visualizationManager) = 0;
		virtual bool setWidget(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* widget) = 0;
		virtual bool setToolbar(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* toolbarWidget) = 0;

		_IsDerivedFromClass_(OpenViBE::Plugins::IPluginObject, OV_ClassId_VisualizationContext)
	};
}
