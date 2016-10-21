#include "ovkCVisualisationTree.h"
#include "ovkCVisualisationManager.h"

#if defined TARGET_OS_Windows
#include <gdk/gdkwin32.h>
#elif defined TARGET_OS_Linux
#include <gdk/gdkx.h>
#elif defined TARGET_OS_MacOS
#define Cursor XCursor
#include <gdk/gdkx.h>
#undef Cursor
#else
#endif


using namespace std;
using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEVisualizationToolkit;
//using namespace OpenViBE::Tools;

CVisualisationManager::CVisualisationManager(const IKernelContext& kernelContext)
	: m_KernelContext(kernelContext)
{
}

CVisualisationManager::~CVisualisationManager()
{
}
	
bool CVisualisationManager::createVisualizationTree(CIdentifier& visualisationTreeIdentifier)
{
	IVisualisationTree* newVisualisationTree = new CVisualisationTree(m_KernelContext);

	visualisationTreeIdentifier = getUnusedIdentifier();

	m_VisualizationTrees[visualisationTreeIdentifier] = newVisualisationTree;

	return true;
}

bool CVisualisationManager::releaseVisualizationTree(const CIdentifier& visualisationTreeIdentifier)
{
	auto it = m_VisualizationTrees.find(visualisationTreeIdentifier);
	if (it != m_VisualizationTrees.end())
	{
		delete it->second;
		m_VisualizationTrees.erase(it);
		return true;
	}

	return false;
}

IVisualisationTree& CVisualisationManager::getVisualizationTree(const CIdentifier& visualisationTreeIdentifier)
{
	const auto it = m_VisualizationTrees.find(visualisationTreeIdentifier);
	if (it == m_VisualizationTrees.end())
	{
		m_KernelContext.getLogManager() << LogLevel_Fatal << "Visualisation Tree " << visualisationTreeIdentifier << " does not exist !\n";
	}
	return *it->second;
}

bool CVisualisationManager::setToolbar(const CIdentifier& visualisationTreeIdentifier, const CIdentifier& boxIdentifier, ::GtkWidget* toolbar)
{
	IVisualisationTree& l_rVisualisationTree = getVisualizationTree(visualisationTreeIdentifier);

	l_rVisualisationTree.setToolbar(boxIdentifier, toolbar);

	return true;
}

bool CVisualisationManager::setWidget(const CIdentifier& rVisualisationTreeIdentifier, const CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget)
{
	IVisualisationTree& visualisationTree = getVisualizationTree(rVisualisationTreeIdentifier);

	visualisationTree.setWidget(boxIdentifier, topmostWidget);

	return true;
}

CIdentifier CVisualisationManager::getUnusedIdentifier(void) const
{
	uint64 possibleIdentifier = (((uint64)rand())<<32)+((uint64)rand());
	CIdentifier finalIdentifier;
	map<CIdentifier, IVisualisationTree*>::const_iterator it;
	do
	{
		finalIdentifier = CIdentifier(possibleIdentifier++);
		it = m_VisualizationTrees.find(finalIdentifier);
	}
	while (it != m_VisualizationTrees.end() || finalIdentifier == OV_UndefinedIdentifier);
	return finalIdentifier;
}

