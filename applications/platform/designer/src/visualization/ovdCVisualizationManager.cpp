#include "ovdCVisualizationTree.h"
#include "ovdCVisualizationManager.h"

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
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEVisualizationToolkit;
//using namespace OpenViBE::Tools;

CVisualizationManager::CVisualizationManager(const IKernelContext& kernelContext)
	: m_KernelContext(kernelContext) { }

CVisualizationManager::~CVisualizationManager() = default;

bool CVisualizationManager::createVisualizationTree(CIdentifier& visualizationTreeIdentifier)
{
	IVisualizationTree* newVisualizationTree = new CVisualizationTree(m_KernelContext);

	visualizationTreeIdentifier = getUnusedIdentifier();

	m_VisualizationTrees[visualizationTreeIdentifier] = newVisualizationTree;

	return true;
}

bool CVisualizationManager::releaseVisualizationTree(const CIdentifier& visualizationTreeIdentifier)
{
	auto it = m_VisualizationTrees.find(visualizationTreeIdentifier);
	if (it != m_VisualizationTrees.end())
	{
		delete it->second;
		m_VisualizationTrees.erase(it);
		return true;
	}

	return false;
}

IVisualizationTree& CVisualizationManager::getVisualizationTree(const CIdentifier& visualizationTreeIdentifier)
{
	const auto it = m_VisualizationTrees.find(visualizationTreeIdentifier);
	if (it == m_VisualizationTrees.end())
	{
		m_KernelContext.getLogManager() << LogLevel_Fatal << "Visualization Tree " << visualizationTreeIdentifier << " does not exist !\n";
	}
	return *it->second;
}

bool CVisualizationManager::setToolbar(const CIdentifier& visualizationTreeIdentifier, const CIdentifier& boxIdentifier, GtkWidget* toolbar)
{
	IVisualizationTree& l_rVisualizationTree = getVisualizationTree(visualizationTreeIdentifier);

	l_rVisualizationTree.setToolbar(boxIdentifier, toolbar);

	return true;
}

bool CVisualizationManager::setWidget(const CIdentifier& rVisualizationTreeIdentifier, const CIdentifier& boxIdentifier, GtkWidget* topmostWidget)
{
	IVisualizationTree& visualizationTree = getVisualizationTree(rVisualizationTreeIdentifier);

	visualizationTree.setWidget(boxIdentifier, topmostWidget);

	return true;
}

CIdentifier CVisualizationManager::getUnusedIdentifier() const
{
	uint64_t possibleIdentifier = CIdentifier::random().toUInteger();
	CIdentifier finalIdentifier;
	map<CIdentifier, IVisualizationTree*>::const_iterator it;
	do
	{
		finalIdentifier = CIdentifier(possibleIdentifier++);
		it = m_VisualizationTrees.find(finalIdentifier);
	} while (it != m_VisualizationTrees.end() || finalIdentifier == OV_UndefinedIdentifier);
	return finalIdentifier;
}
