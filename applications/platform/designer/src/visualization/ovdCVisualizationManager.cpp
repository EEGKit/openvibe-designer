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
using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Plugins;
using namespace /*OpenViBE::*/VisualizationToolkit;

bool CVisualizationManager::createVisualizationTree(CIdentifier& treeID)
{
	IVisualizationTree* newTree = new CVisualizationTree(m_kernelCtx);
	treeID                      = getUnusedIdentifier();
	m_trees[treeID]             = newTree;
	return true;
}

bool CVisualizationManager::releaseVisualizationTree(const CIdentifier& treeID)
{
	auto it = m_trees.find(treeID);
	if (it != m_trees.end())
	{
		delete it->second;
		m_trees.erase(it);
		return true;
	}

	return false;
}

IVisualizationTree& CVisualizationManager::getVisualizationTree(const CIdentifier& id)
{
	const auto it = m_trees.find(id);
	if (it == m_trees.end()) { m_kernelCtx.getLogManager() << LogLevel_Fatal << "Visualization Tree " << id << " does not exist !\n"; }
	return *it->second;
}

bool CVisualizationManager::setToolbar(const CIdentifier& treeID, const CIdentifier& boxID, GtkWidget* toolbar)
{
	IVisualizationTree& tree = getVisualizationTree(treeID);
	tree.setToolbar(boxID, toolbar);
	return true;
}

bool CVisualizationManager::setWidget(const CIdentifier& treeID, const CIdentifier& boxID, GtkWidget* topmostWidget)
{
	IVisualizationTree& tree = getVisualizationTree(treeID);
	tree.setWidget(boxID, topmostWidget);
	return true;
}

CIdentifier CVisualizationManager::getUnusedIdentifier() const
{
	uint64_t id = CIdentifier::random().id();
	CIdentifier res;
	map<CIdentifier, IVisualizationTree*>::const_iterator it;
	do
	{
		res = CIdentifier(id++);
		it  = m_trees.find(res);
	} while (it != m_trees.end() || res == OV_UndefinedIdentifier);
	return res;
}
