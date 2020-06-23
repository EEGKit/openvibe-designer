#include "ovvizCVisualizationContext.hpp"

using namespace OpenViBE;
using namespace /*OpenViBE::*/VisualizationToolkit;

bool CVisualizationContext::setWidget(Toolkit::TBoxAlgorithm<Plugins::IBoxAlgorithm>& box, GtkWidget* widget)
{
	const CIdentifier boxID = box.getStaticBoxContext().getIdentifier();

	CIdentifier treeID = CIdentifier::undefined();

	// When a scenario is created in the designer, the designer creates a visualization tree for it. The box will then need to add
	// visualization widgets into this tree. The designer can not pass this information directly to a box in a way that can be read
	// here (for example an Attribute could be written in the designer but then it is not possible to read it here).
	//
	// What we do is that we write the visualization tree identifier into the local scenario configuration manager. This manager
	// can be accessed as long as we have the boxAlgorithm.
	if (!treeID.fromString(box.getConfigurationManager().lookUpConfigurationTokenValue("VisualizationContext_VisualizationTreeId").toASCIIString())) { return false; }

	return m_VisualizationManager->setWidget(treeID, boxID, widget);
}

bool CVisualizationContext::setToolbar(Toolkit::TBoxAlgorithm<Plugins::IBoxAlgorithm>& box, GtkWidget* toolbarWidget)
{
	const CIdentifier boxID = box.getStaticBoxContext().getIdentifier();

	CIdentifier treeID = CIdentifier::undefined();

	if (!treeID.fromString(box.getConfigurationManager().lookUpConfigurationTokenValue("VisualizationContext_VisualizationTreeId").toASCIIString())) { return false; }

	return m_VisualizationManager->setToolbar(treeID, boxID, toolbarWidget);
}
