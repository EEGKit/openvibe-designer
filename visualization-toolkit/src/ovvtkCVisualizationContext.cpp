#include <cstdio>
#include "ovvtkCVisualizationContext.hpp"

using namespace OpenViBE;
using namespace OpenViBEToolkit;

bool CVisualizationContext::setWidget(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* widget)
{
	CIdentifier boxIdentifier = box.getStaticBoxContext().getIdentifier();

	CIdentifier treeIdentifier = OV_UndefinedIdentifier;

	if (!treeIdentifier.fromString(box.getConfigurationManager().lookUpConfigurationTokenValue("VisualizationContext_GroupId")))
	{
		return false;
	}

	return m_visualizationManager->setWidget(treeIdentifier, boxIdentifier, widget);
}

bool CVisualizationContext::setToolbar(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* toolbarWidget)
{
	CIdentifier boxIdentifier = box.getStaticBoxContext().getIdentifier();

	CIdentifier treeIdentifier = OV_UndefinedIdentifier;

	if (!treeIdentifier.fromString(box.getConfigurationManager().lookUpConfigurationTokenValue("VisualizationContext_GroupId")))
	{
		return false;
	}

	return m_visualizationManager->setToolbar(treeIdentifier, boxIdentifier, toolbarWidget);
}
