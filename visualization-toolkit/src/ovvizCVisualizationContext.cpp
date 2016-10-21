#include <cstdio>
#include "ovvizCVisualizationContext.hpp"

using namespace OpenViBE;
using namespace OpenViBEVisualizationToolkit;

bool CVisualizationContext::setWidget(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* widget)
{
	CIdentifier boxIdentifier = box.getStaticBoxContext().getIdentifier();

	CIdentifier treeIdentifier = OV_UndefinedIdentifier;

	// When a scenario is created in the designer, the designer creates a visualization tree for it. The box will then need to add
	// visualization widgets into this tree. The designer can not pass this information directly to a box in a way that can be read
	// here (for example an Attribute could be written in the designer but then it is not possible to read it here).
	//
	// What we do is that we write the visualization tree identifier into the local scenario configuration manager. This manager
	// can be accessed as long as we have the boxAlgorithm.
	if (!treeIdentifier.fromString(box.getConfigurationManager().lookUpConfigurationTokenValue("VisualizationContext_VisualizationTreeId")))
	{
		return false;
	}

	return m_VisualizationManager->setWidget(treeIdentifier, boxIdentifier, widget);
}

bool CVisualizationContext::setToolbar(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* toolbarWidget)
{
	CIdentifier boxIdentifier = box.getStaticBoxContext().getIdentifier();

	CIdentifier treeIdentifier = OV_UndefinedIdentifier;

	if (!treeIdentifier.fromString(box.getConfigurationManager().lookUpConfigurationTokenValue("VisualizationContext_VisualizationTreeId")))
	{
		return false;
	}

	return m_VisualizationManager->setToolbar(treeIdentifier, boxIdentifier, toolbarWidget);
}
