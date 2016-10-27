#include <iostream>
#include <cstdlib>
#include <cstring>
#include <json/json.h>

#include "ovdCVisualizationTree.h"
#include "ovdCVisualizationWidget.h"
#include "../ovdAssert.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEVisualizationToolkit;
using namespace std;

namespace
{
	template <class T>
	struct TTestTrue
	{
		bool operator()(typename map<CIdentifier, T*>::const_iterator it) const { return true; }
	};

	struct TTestEqVisualizationWidgetType
	{
		TTestEqVisualizationWidgetType(EVisualizationWidgetType oType) : m_oType(oType) { }
		bool operator()(map<CIdentifier, IVisualizationWidget*>::const_iterator it) const
		{
			return it->second->getType() == m_oType;
		}
		EVisualizationWidgetType m_oType;
	};

	template <class T, class TTest>
	bool getNextTIdentifier(
		const map<CIdentifier, T*>& vMap,
		CIdentifier& rIdentifier,
		const TTest& rTest)
	{
		typename map<CIdentifier, T*>::const_iterator it;

		if(rIdentifier==OV_UndefinedIdentifier)
		{
			it=vMap.begin();
		}
		else
		{
			it=vMap.find(rIdentifier);
			if(it==vMap.end())
			{
				rIdentifier = OV_UndefinedIdentifier;
				return false;
			}
			it++;
		}

		while(it!=vMap.end())
		{
			if(rTest(it))
			{
				rIdentifier = it->first;
				return true;
			}
			it++;
		}

		return false;
	}
};

CVisualizationTree::CVisualizationTree(const IKernelContext& kernelContext) :
	m_KernelContext(kernelContext),
	m_Scenario(nullptr),
	m_TreeStore(nullptr),
	m_TreeViewCB(nullptr)
{
}

CVisualizationTree::~CVisualizationTree()
{
	for (auto& widget : m_VisualizationWidgets)
	{
		delete widget.second;
	}

	g_object_unref(m_TreeStore);
}

bool CVisualizationTree::init(const IScenario* scenario)
{
	m_Scenario = scenario;

	//create tree store
	m_TreeStore = gtk_tree_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_ULONG, G_TYPE_STRING, G_TYPE_POINTER);

	return true;
}

bool CVisualizationTree::getNextVisualizationWidgetIdentifier(CIdentifier& identifier) const
{
	return getNextTIdentifier<IVisualizationWidget, TTestTrue<IVisualizationWidget> >
		(m_VisualizationWidgets, identifier, TTestTrue<IVisualizationWidget>());
}

bool CVisualizationTree::getNextVisualizationWidgetIdentifier(CIdentifier& rIdentifier, EVisualizationWidgetType type) const
{
	return getNextTIdentifier<IVisualizationWidget, TTestEqVisualizationWidgetType >
		(m_VisualizationWidgets, rIdentifier, TTestEqVisualizationWidgetType(type));
}

bool CVisualizationTree::isVisualizationWidget(const CIdentifier& identifier) const
{
	return m_VisualizationWidgets.find(identifier) != m_VisualizationWidgets.end();
}

IVisualizationWidget* CVisualizationTree::getVisualizationWidget(const CIdentifier& identifier) const
{
	auto it = m_VisualizationWidgets.find(identifier);

	if (it == m_VisualizationWidgets.end())
	{
		return nullptr;
	}
	return it->second;
}

IVisualizationWidget* CVisualizationTree::getVisualizationWidgetFromBoxIdentifier(const CIdentifier& boxIdentifier) const
{
	for (auto& widget : m_VisualizationWidgets)
	{
		if (widget.second->getBoxIdentifier() == boxIdentifier)
		{
			return widget.second;
		}
	}
	return nullptr;
}

bool CVisualizationTree::addVisualizationWidget(CIdentifier& identifier, const CString& name, EVisualizationWidgetType type,
	const CIdentifier& parentIdentifier, uint32 parentIndex, const CIdentifier& boxIdentifier, uint32 childCount, const OpenViBE::CIdentifier& suggestedIdentifier)
{
	m_KernelContext.getLogManager() << LogLevel_Trace << "Adding new visualization widget\n";

	//create new widget
	IVisualizationWidget* visualizationWidget = new CVisualizationWidget(m_KernelContext);
	identifier = getUnusedIdentifier(suggestedIdentifier);

	if(visualizationWidget->initialize(identifier, name, type, parentIdentifier, boxIdentifier, childCount) == false)
	{
		m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to add new visualization widget (couldn't initialize it)\n";
		delete visualizationWidget;
		return false;
	}

	// assign a parent to it
	if(parentIdentifier != OV_UndefinedIdentifier)
	{
		m_KernelContext.getLogManager() << LogLevel_Trace << "Parenting visualization widget\n";
		IVisualizationWidget* parentVisualizationWidget = getVisualizationWidget(parentIdentifier);

		if (parentVisualizationWidget != NULL)
		{
			if(parentVisualizationWidget->getType() == EVisualizationWidget_VisualizationWindow)
			{
				//extend number of children of parent window if necessary
				if(parentVisualizationWidget->getNbChildren() <= parentIndex)
				{
					for(unsigned int i = parentVisualizationWidget->getNbChildren(); i<=parentIndex; i++)
					{
						parentVisualizationWidget->addChild(OV_UndefinedIdentifier);
					}
				}
			}

			if(parentVisualizationWidget->setChildIdentifier(parentIndex, identifier) == false)
			{
				m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to add new visualization widget (couldn't set child identifier in parent window)\n";
				return false;
			}
		}
		else
		{
			m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to add new visualization widget (couldn't find parent)\n";
			return false;
		}
	}

	//add it to widgets map
	m_VisualizationWidgets[identifier]=visualizationWidget;
	return true;
}

bool CVisualizationTree::getVisualizationWidgetIndex(const CIdentifier& identifier, uint32& index) const
{
	IVisualizationWidget* visualizationWidget = getVisualizationWidget(identifier);
	if (!visualizationWidget)
	{
		return false;
	}

	const CIdentifier& parentIdentifier = visualizationWidget->getParentIdentifier();
	if (parentIdentifier == OV_UndefinedIdentifier)
	{
		return false;
	}

	IVisualizationWidget* parentVisualizationWidget = getVisualizationWidget(parentIdentifier);
	if (!parentVisualizationWidget)
	{
		m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to unparent visualization widget (couldn't find parent)\n";
		return false;
	}

	parentVisualizationWidget->getChildIndex(identifier, index);

	return true;
}

bool CVisualizationTree::destroyHierarchy(const CIdentifier& identifier, bool destroyVisualizationBoxes)
{
	bool res = true;

	IVisualizationWidget* visualizationWidget = getVisualizationWidget(identifier);

	//is hierarchy top item a window?
	if (visualizationWidget->getType() == EVisualizationWidget_VisualizationWindow)
	{
		CIdentifier childIdentifier;
		for (uint32 i = 0; i < visualizationWidget->getNbChildren(); i++)
		{
			visualizationWidget->getChildIdentifier(i, childIdentifier);
			res &=_destroyHierarchy(childIdentifier, destroyVisualizationBoxes);
		}

		//delete this window in kernel factory and erase its slot in map
		delete visualizationWidget;
		auto it = m_VisualizationWidgets.find(identifier);
		m_VisualizationWidgets.erase(it);
	}
	else //top item is a widget
	{
		uint32 index;
		unparentVisualizationWidget(identifier, index);
		_destroyHierarchy(identifier, destroyVisualizationBoxes);
	}

	return res;
}

bool CVisualizationTree::_destroyHierarchy(const CIdentifier& identifier, bool destroyVisualizationBoxes)
{
	IVisualizationWidget* visualizationWidget = getVisualizationWidget(identifier);
	if (!visualizationWidget)
	{
		return false;
	}

	//remove children
	CIdentifier l_oChildIdentifier;
	uint32 nbChildren = visualizationWidget->getNbChildren();
	for (uint32 i = 0; i < nbChildren; i++)
	{
		visualizationWidget->getChildIdentifier(i, l_oChildIdentifier);
		_destroyHierarchy(l_oChildIdentifier, destroyVisualizationBoxes);
	}

	//if parent widget is a window, remove this widget from it
	if (visualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		IVisualizationWidget* l_pVisualizationWindow = getVisualizationWidget(visualizationWidget->getParentIdentifier());
		if(l_pVisualizationWindow != NULL)
		{
			l_pVisualizationWindow->removeChild(identifier);
		}
	}

	//if this widget is a visualization box and they are to be unaffected
	if (visualizationWidget->getType() == EVisualizationWidget_VisualizationBox && destroyVisualizationBoxes == false)
	{
		uint32 index;
		unparentVisualizationWidget(identifier, index);
	}
	else
	{
		m_KernelContext.getLogManager() << LogLevel_Trace << "Deleting visualization widget\n";
		delete visualizationWidget;
		map<CIdentifier, IVisualizationWidget*>::iterator it = m_VisualizationWidgets.find(identifier);
		m_VisualizationWidgets.erase(it);
	}

	return true;
}

bool CVisualizationTree::unparentVisualizationWidget(const CIdentifier& identifier, uint32& index)
{
	//retrieve widget to be unparented
	IVisualizationWidget* visualizationWidget = getVisualizationWidget(identifier);
	if (visualizationWidget == NULL)
	{
		return false;
	}

	//get its parent identifier
	const CIdentifier& parentIdentifier = visualizationWidget->getParentIdentifier();
	if (parentIdentifier == OV_UndefinedIdentifier)
	{
		return true;
	}

	//unparent widget
	visualizationWidget->setParentIdentifier(OV_UndefinedIdentifier);

	//retrieve parent and remove widget from its children list
	IVisualizationWidget* parentVisualizationWidget = getVisualizationWidget(parentIdentifier);
	if (parentVisualizationWidget != NULL)
	{
		parentVisualizationWidget->getChildIndex(identifier, index);
		parentVisualizationWidget->removeChild(identifier);
	}

	return true;
}

bool CVisualizationTree::parentVisualizationWidget(const CIdentifier& identifier, const CIdentifier& rParentIdentifier, uint32 index)
{
	if (rParentIdentifier == OV_UndefinedIdentifier)
	{
		return false;
	}

	//retrieve widget to be parented
	IVisualizationWidget* l_pVisualizationWidget = getVisualizationWidget(identifier);
	if (!l_pVisualizationWidget)
	{
		return false;
	}

	l_pVisualizationWidget->setParentIdentifier(rParentIdentifier);

	//retrieve its parent
	IVisualizationWidget* parentVisualizationWidget = getVisualizationWidget(rParentIdentifier);
	if (!parentVisualizationWidget)
	{
		m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to parent visualization widget (couldn't find parent)\n";
		return false;
	}

	parentVisualizationWidget->setChildIdentifier(index, identifier);

	return true;
}

CIdentifier CVisualizationTree::getUnusedIdentifier(const CIdentifier& suggestedIdentifier) const
{
	uint64 proposedIdentifier=(((uint64)rand())<<32)+((uint64)rand());
	if(suggestedIdentifier != OV_UndefinedIdentifier)
	{
		proposedIdentifier = suggestedIdentifier.toUInteger()-1;
	}

	CIdentifier result;
	map<CIdentifier, IVisualizationWidget*>::const_iterator i;
	do
	{
		proposedIdentifier++;
		result = CIdentifier(proposedIdentifier);
		i = m_VisualizationWidgets.find(result);
	}
	while (i != m_VisualizationWidgets.end() || result == OV_UndefinedIdentifier);
	return result;
}

::GtkTreeView* CVisualizationTree::createTreeViewWithModel()
{
	return GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_TreeStore)));
}

bool CVisualizationTree::setTreeViewCB(ITreeViewCB* treeViewCB)
{
	m_TreeViewCB = treeViewCB;
	return true;
}

bool CVisualizationTree::reloadTree()
{
	if (!m_TreeViewCB)
	{
		return false;
	}

	//clear current tree
	::GtkTreeIter iter;
	while(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_TreeStore), &iter, NULL, 0) != FALSE)
	{
		gtk_tree_store_remove(m_TreeStore, &iter);
	}
	//create 'unaffected display plugins' node
	gtk_tree_store_append(m_TreeStore, &iter, NULL);
	gtk_tree_store_set(m_TreeStore, &iter,
		EVisualizationTreeColumn_StringName, "Unaffected display plugins",
		EVisualizationTreeColumn_StringStockIcon, m_TreeViewCB->getTreeWidgetIcon(EVisualizationTreeNode_Unaffected),
		EVisualizationTreeColumn_ULongNodeType, (unsigned long)EVisualizationTreeNode_Unaffected,
		EVisualizationTreeColumn_StringIdentifier, (const char*)OV_UndefinedIdentifier.toString(),
		-1);

	//reload unaffected visualization boxes
	CIdentifier visualizationWidgetIdentifier = OV_UndefinedIdentifier;
	while(getNextVisualizationWidgetIdentifier(visualizationWidgetIdentifier, EVisualizationWidget_VisualizationBox) == true)
	{
		IVisualizationWidget* visualizationWidget = getVisualizationWidget(visualizationWidgetIdentifier);
		//load widget if it doesn't have a parent (== is unaffected)
		if(visualizationWidget->getParentIdentifier() == OV_UndefinedIdentifier)
		{
			loadVisualizationWidget(visualizationWidget, &iter);
		}
	}

	//reload visualization windows
	CIdentifier visualizationWindowIdentifier = OV_UndefinedIdentifier;
	while(getNextVisualizationWidgetIdentifier(visualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
	{
		loadVisualizationWidget(getVisualizationWidget(visualizationWindowIdentifier), NULL);
	}

	return true;
}

//Tree helper functions
//---------------------

bool CVisualizationTree::getTreeSelection(::GtkTreeView* preeView, ::GtkTreeIter* iter)
{
	::GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(preeView);
	::GtkTreeModel* treeModel = GTK_TREE_MODEL(m_TreeStore);
	return gtk_tree_selection_get_selected(treeSelection, &treeModel, iter) != 0;
}

::GtkTreePath* CVisualizationTree::getTreePath(::GtkTreeIter* pTreeIter) const
{
	return (pTreeIter == NULL) ? NULL : gtk_tree_model_get_path(GTK_TREE_MODEL(m_TreeStore), pTreeIter);
}

unsigned long	CVisualizationTree::getULongValueFromTreeIter(::GtkTreeIter* treeIter, EVisualizationTreeColumn visualizationTreeColumn) const
{
	unsigned long value = 0;
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), treeIter, visualizationTreeColumn, &value, -1);
	return value;
}

bool CVisualizationTree::getStringValueFromTreeIter(::GtkTreeIter* treeIter, char*& string, EVisualizationTreeColumn col) const
{
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), treeIter, col, &string, -1);
	return true;
}

bool CVisualizationTree::getPointerValueFromTreeIter(::GtkTreeIter* treeIter, void*& pointer, EVisualizationTreeColumn col) const
{
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), treeIter, col, &pointer, -1);
	return true;
}

bool CVisualizationTree::getIdentifierFromTreeIter(::GtkTreeIter* iter, CIdentifier& identifier, EVisualizationTreeColumn col) const
{
	char* stringIdentifier = NULL;
	getStringValueFromTreeIter(iter, stringIdentifier, col);
	identifier.fromString(CString(stringIdentifier));
	return true;
}

//looks for a tree node named 'label' of class 'type' from tree root
bool CVisualizationTree::findChildNodeFromRoot(::GtkTreeIter* iter, const char* label, EVisualizationTreeNode type)
{
	if (!label)
	{
		return false;
	}

	//if tree is empty return false
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_TreeStore), iter) == false)
	{
		return false;
	}

	//look for node in the whole tree
	do
	{
		//look for node in current subtree
		if (findChildNodeFromParent(iter, label, type) == true)
		{
			return true;
		}

		//proceed with next top-level node
	} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_TreeStore), iter) != 0);

	//node wasn't found
	return false;
}

//looks for a tree node named 'label' of class 'type' from parent passed as parameter
bool CVisualizationTree::findChildNodeFromParent(::GtkTreeIter* iter, const char* label, EVisualizationTreeNode type)
{
	if (_findChildNodeFromParent(iter, label, type) == true)
	{
		*iter = m_InternalTreeNode;
		return true;
	}
	return false;
}

//looks for a tree node named 'label' of class 'type' from parent passed as parameter
bool CVisualizationTree::_findChildNodeFromParent(::GtkTreeIter* iter, const char* label, EVisualizationTreeNode type)
{
	gchar* name;
	unsigned long typeAsInt;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), iter,
		EVisualizationTreeColumn_StringName, &name,
		EVisualizationTreeColumn_ULongNodeType, &typeAsInt,
		-1);

	if (!name)
	{
		m_KernelContext.getLogManager() << LogLevel_Error << "Can not get values from the model" << "\n";
		return false;
	}

	if (strcmp(label, name) == 0 && type == (EVisualizationTreeNode)typeAsInt)
	{
		m_InternalTreeNode = *iter;
		return true;
	}

	//look among current node's children
	int childCount = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_TreeStore), iter);
	::GtkTreeIter childIter;
	for (int i = 0; i < childCount; i++)
	{
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_TreeStore), &childIter, iter, i);

		if (_findChildNodeFromParent(&childIter, label, type) == true)
		{
			return true;
		}
	}

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromRoot(::GtkTreeIter* iter, void* widget)
{
	// if tree is empty
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_TreeStore), iter) == false)
	{
		return false;
	}

	//look for node in the whole tree
	do
	{
		//look for node in current subtree
		if (findChildNodeFromParent(iter, widget) == true)
		{
			return true;
		}

		//proceed with next top-level node
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(m_TreeStore), iter) != 0);

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromParent(::GtkTreeIter* iter, void* widget)
{
	if (_findChildNodeFromParent(iter, widget))
	{
		*iter = m_InternalTreeNode;
		return true;
	}
	return false;
}

bool CVisualizationTree::_findChildNodeFromParent(::GtkTreeIter* iter, void* widget)
{
	void* currentWidget;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), iter, EVisualizationTreeColumn_PointerWidget, &currentWidget, -1);
	if (widget == currentWidget)
	{
		m_InternalTreeNode = *iter;
		return true;
	}

	//look among current node's children
	int childCount = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_TreeStore), iter);
	::GtkTreeIter childIter;
	for (int i = 0; i < childCount; i++)
	{
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_TreeStore), &childIter, iter, i);

		if (_findChildNodeFromParent(&childIter, widget) == true)
		{
			return true;
		}
	}

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromRoot(::GtkTreeIter* iter, CIdentifier identifier)
{
	//if tree is empty return false
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_TreeStore), iter) == false)
	{
		return false;
	}

	//look for node in the whole tree
	do
	{
		//look for node in current subtree
		if(findChildNodeFromParent(iter, identifier) == true)
		{
			return true;
		}

		//proceed with next top-level node
	} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_TreeStore), iter) != 0);

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromParent(::GtkTreeIter* iter, CIdentifier identifier)
{
	if(_findChildNodeFromParent(iter, identifier) == true)
	{
		*iter = m_InternalTreeNode;
		return true;
	}
	return false;
}

bool CVisualizationTree::_findChildNodeFromParent(::GtkTreeIter* pIter, CIdentifier identifier)
{
	gchar* identifierAsString;
	CIdentifier currentIdentifier;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), pIter, EVisualizationTreeColumn_StringIdentifier, &identifierAsString, -1);
	currentIdentifier.fromString(CString(identifierAsString));
	if (identifier == currentIdentifier)
	{
		m_InternalTreeNode = *pIter;
		return true;
	}

	//look among current node's children
	int childCount = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_TreeStore), pIter);
	::GtkTreeIter childIter;
	for (int i = 0; i < childCount; i++)
	{
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_TreeStore), &childIter, pIter, i);

		if (_findChildNodeFromParent(&childIter, identifier))
		{
			return true;
		}
	}

	//node wasn't found
	return false;
}

bool CVisualizationTree::findParentNode(::GtkTreeIter* iter, EVisualizationTreeNode type)
{
	unsigned long typeAsInt;
	::GtkTreeIter currentIter;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), iter, EVisualizationTreeColumn_ULongNodeType, &typeAsInt, -1);
	if (type == (EVisualizationTreeNode)typeAsInt)
	{
		return true;
	}
	//look one level higher
	else if(gtk_tree_model_iter_parent(GTK_TREE_MODEL(m_TreeStore), &currentIter, iter) != 0)
	{
		*iter = currentIter;
		return findParentNode(iter, type);
	}
	else //couldn't find desired parent node
	{
		return false;
	}
}

bool CVisualizationTree::dragDataReceivedOutsideWidgetCB(const CIdentifier& sourceWidgetIdentifierr, ::GtkWidget* destinationWidget, EDragDataLocation location)
{
	//retrieve source widget parent
	//-----------------------------
	IVisualizationWidget* sourceVisualizationWidget = getVisualizationWidget(sourceWidgetIdentifierr);
	if (!sourceVisualizationWidget)
	{
		return false;
	}

	//retrieve dest widget and dest widget parent identifiers
	//-------------------------------------------------------
	::GtkTreeIter destinationIterator;
	if (findChildNodeFromRoot(&destinationIterator, m_TreeViewCB->getTreeWidget(destinationWidget)) == false)
	{
		return false;
	}
	CIdentifier destinationWidgetIdentifier;
	getIdentifierFromTreeIter(&destinationIterator, destinationWidgetIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* destinationVisualizationWidget = getVisualizationWidget(destinationWidgetIdentifier);
	if (destinationVisualizationWidget == NULL)
	{
		return false;
	}
	// dst widget is the widget already present in
	CIdentifier destinationParentIdentifier = destinationVisualizationWidget->getParentIdentifier();

	//unparent source widget
	uint32 sourceIndex = 0;
	unparentVisualizationWidget(sourceWidgetIdentifierr, sourceIndex);

	//unparent dest widget
	uint32 destinationIndex = 0;
	unparentVisualizationWidget(destinationWidgetIdentifier, destinationIndex);

	//create paned widget
	EVisualizationWidgetType panedType = (location == EDragData_Top || location == EDragData_Bottom) ? EVisualizationWidget_VerticalSplit : EVisualizationWidget_HorizontalSplit;
	CIdentifier panedIdentifier;
	addVisualizationWidget(
		panedIdentifier,
		CString(panedType == EVisualizationWidget_VerticalSplit ? "Vertical split" : "Horizontal split"),
		panedType,
		destinationParentIdentifier, //parent paned to dest widget parent
		destinationIndex, //put it at the index occupied by dest widget
		OV_UndefinedIdentifier, //no box algorithm for a paned
		2, //2 children
		OV_UndefinedIdentifier); //no prefered visualization identifier
	IVisualizationWidget* panedVisualizationWidget = getVisualizationWidget(panedIdentifier);

	//add attributes
	if (m_TreeViewCB != NULL)
	{
		m_TreeViewCB->createTreeWidget(panedVisualizationWidget);
	}

	//reparent widgets
	uint32 newSourceIndex = (location == EDragData_Top || location == EDragData_Left) ? 0 : 1;
	parentVisualizationWidget(sourceWidgetIdentifierr, panedIdentifier, newSourceIndex);
	parentVisualizationWidget(destinationWidgetIdentifier, panedIdentifier, 1-newSourceIndex);

	//update Gtk tree
	reloadTree();

	return true;
}

bool CVisualizationTree::dragDataReceivedInWidgetCB(const CIdentifier& sourceWidgetIdentifier, ::GtkWidget* destinationWidget)
{
	//retrieve source widget parent
	IVisualizationWidget* sourceVisualizationWidget = getVisualizationWidget(sourceWidgetIdentifier);
	OV_EXCEPTION_UNLESS_D(
	            sourceVisualizationWidget,
	            "Source visualization identifier does not exist in the tree", ErrorType::ResourceNotFound);

	CIdentifier sourceParentIdentifier = sourceVisualizationWidget->getParentIdentifier();

	//retrieve dest widget and dest widget parent identifiers
	::GtkTreeIter destinationIterator;
	if (!findChildNodeFromRoot(&destinationIterator, m_TreeViewCB->getTreeWidget(destinationWidget)))
	{
		return false;
	}

	CIdentifier destinationWidgetIdentifier;
	getIdentifierFromTreeIter(&destinationIterator, destinationWidgetIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* destinationVisualizationWidget = getVisualizationWidget(destinationWidgetIdentifier);
	if (!destinationVisualizationWidget)
	{
		return false;
	}

	CIdentifier destinationParentIdentifier = destinationVisualizationWidget->getParentIdentifier();

	//unparent source widget
	uint32 sourceIndex;
	unparentVisualizationWidget(sourceWidgetIdentifier, sourceIndex);

	//destroy, unparent or reparent dest widget
	uint32 destinationIndex;

	//if source widget was unaffected
	if (sourceParentIdentifier == OV_UndefinedIdentifier)
	{
		//if dest widget was dummy, destroy it
		if (destinationVisualizationWidget->getType() == EVisualizationWidget_Undefined)
		{
			getVisualizationWidgetIndex(destinationWidgetIdentifier, destinationIndex);
			destroyHierarchy(destinationWidgetIdentifier, true);
		}
		else //dest widget becomes unaffected
		{
			unparentVisualizationWidget(destinationWidgetIdentifier, destinationIndex);
		}
	}
	else //source widget was affected
	{
		//unparent dest widget
		unparentVisualizationWidget(destinationWidgetIdentifier, destinationIndex);

		//reparent it to source widget parent
		parentVisualizationWidget(destinationWidgetIdentifier, sourceParentIdentifier, sourceIndex);
	}

	//reparent source widget
	parentVisualizationWidget(sourceWidgetIdentifier, destinationParentIdentifier, destinationIndex);

	//update Gtk tree
	reloadTree();

	return true;
}

bool CVisualizationTree::loadVisualizationWidget(IVisualizationWidget* visualizationWidget, ::GtkTreeIter* parentIter)
{
	//create visualization widget
	//---------------------------
	::GtkWidget* widget = m_TreeViewCB->loadTreeWidget(visualizationWidget);

	//add visualization widget node to tree store
	::GtkTreeIter iter;
	gtk_tree_store_append(m_TreeStore, &iter, parentIter);

	//retrieve values of tree node fields
	EVisualizationTreeNode childType;
	switch(visualizationWidget->getType())
	{
		case EVisualizationWidget_VisualizationWindow:
			childType = EVisualizationTreeNode_VisualizationWindow;
			break;
		case EVisualizationWidget_VisualizationPanel:
			childType = EVisualizationTreeNode_VisualizationPanel; break;
		case EVisualizationWidget_VisualizationBox:
			childType = EVisualizationTreeNode_VisualizationBox;
			break;
		case EVisualizationWidget_HorizontalSplit:
			childType = EVisualizationTreeNode_HorizontalSplit;
			break;
		case EVisualizationWidget_VerticalSplit:
			childType = EVisualizationTreeNode_VerticalSplit;
			break;
		case EVisualizationWidget_Undefined:
			childType=EVisualizationTreeNode_Undefined;
			break;
	}

	CString stockIconString = m_TreeViewCB->getTreeWidgetIcon(childType);

	if (visualizationWidget->getType() == EVisualizationWidget_VisualizationBox)
	{
		const IBox* box = m_Scenario->getBoxDetails(visualizationWidget->getBoxIdentifier());
		if (!box)
		{
			m_KernelContext.getLogManager() << LogLevel_Error << "Box with identifier " << visualizationWidget->getBoxIdentifier() << " not found in the scenario" << "\n";
			return false;
		}
		const IBoxAlgorithmDesc* boxDesc = dynamic_cast<const IBoxAlgorithmDesc*>(m_KernelContext.getPluginManager().getPluginObjectDescCreating(box->getAlgorithmClassIdentifier()));
		if (boxDesc)
		{
			stockIconString = boxDesc->getStockItemName();
		}
	}

	//set tree node fields
	gtk_tree_store_set(m_TreeStore, &iter,
		EVisualizationTreeColumn_StringName, (const char*)visualizationWidget->getName(),
		EVisualizationTreeColumn_StringStockIcon, (const char*)stockIconString,
		EVisualizationTreeColumn_ULongNodeType, (unsigned long)childType,
		EVisualizationTreeColumn_StringIdentifier, (const char*)visualizationWidget->getIdentifier().toString(),
		EVisualizationTreeColumn_PointerWidget, widget,
		-1);

	//load visualization widget hierarchy
	//-----------------------------------
	//create a dummy child for visualization panels if none exists
	if(visualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		CIdentifier childIdentifier;
		visualizationWidget->getChildIdentifier(0, childIdentifier);
		if (childIdentifier == OV_UndefinedIdentifier)
		{
			addVisualizationWidget(
				childIdentifier,
				"Empty",
				EVisualizationWidget_Undefined,
				visualizationWidget->getIdentifier(),
				0,
				OV_UndefinedIdentifier,
				0,
				OV_UndefinedIdentifier);
		}
	}

	for (uint32 i = 0;  i < visualizationWidget->getNbChildren(); i++)
	{
		CIdentifier childIdentifier;
		visualizationWidget->getChildIdentifier(i, childIdentifier);

		loadVisualizationWidget(getVisualizationWidget(childIdentifier), &iter);
	}

	//complete visualization widget loading now that its hierarchy is loaded
	m_TreeViewCB->endLoadTreeWidget(visualizationWidget);

	return true;
}

bool CVisualizationTree::setToolbar(const CIdentifier& boxIdentifier, ::GtkWidget* toolbarWidget)
{
	if(m_TreeViewCB != NULL)
	{
		return m_TreeViewCB->setToolbar(boxIdentifier, toolbarWidget);
	}
	else
	{
		return false;
	}
}

bool CVisualizationTree::setWidget(const CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget)
{
	if(m_TreeViewCB != NULL)
	{
		return m_TreeViewCB->setWidget(boxIdentifier, topmostWidget);
	}
	else
	{
		return false;
	}
}



json::Object CVisualizationTree::serializeWidget(IVisualizationWidget& widget) const
{
	json::Object jsonRepresentation;

	jsonRepresentation["identifier"] = widget.getIdentifier().toString().toASCIIString();

	// visualization box name can be retrieved from corresponding IBox, so we can skip it for these
	if (widget.getType() != EVisualizationWidget_VisualizationBox)
	{
		jsonRepresentation["name"] = widget.getName().toASCIIString();
	}

	jsonRepresentation["type"] = widget.getType();
	jsonRepresentation["parentIdentifier"] = widget.getParentIdentifier().toString().toASCIIString();

	// visualization widget index
	IVisualizationWidget* parentVisualizationWidget = this->getVisualizationWidget(widget.getParentIdentifier());
	if (parentVisualizationWidget)
	{
		uint32 childIndex = 0;
		parentVisualizationWidget->getChildIndex(widget.getIdentifier(), childIndex);
		jsonRepresentation["index"] = static_cast<int>(childIndex);
	}

	jsonRepresentation["boxIdentifier"] = widget.getBoxIdentifier().toString().toASCIIString();
	jsonRepresentation["childCount"] = static_cast<int>(widget.getNbChildren());
	jsonRepresentation["width"] = static_cast<int>(widget.getWidth());
	jsonRepresentation["height"] = static_cast<int>(widget.getHeight());
	jsonRepresentation["dividerPosition"] = widget.getDividerPosition();
	jsonRepresentation["maxDividerPosition"] = widget.getMaxDividerPosition();

	return jsonRepresentation;
}

OpenViBE::CString CVisualizationTree::serialize() const
{
	json::Array jsonRepresentation;

	std::vector<CIdentifier> widgetsToExport;

	CIdentifier visualizationWidgetIdentifier;
	while (this->getNextVisualizationWidgetIdentifier(visualizationWidgetIdentifier))
	{
		IVisualizationWidget* widget = this->getVisualizationWidget(visualizationWidgetIdentifier);
		if (widget->getType() == EVisualizationWidget_VisualizationWindow
		        || widget->getParentIdentifier()==OV_UndefinedIdentifier)
		{
			widgetsToExport.push_back(visualizationWidgetIdentifier);
		}
	}

	for (auto& widgetIdentifier : widgetsToExport)
	{
		IVisualizationWidget* widget = this->getVisualizationWidget(widgetIdentifier);

		jsonRepresentation.push_back(this->serializeWidget(*widget));

		for (uint32 j = 0; j < widget->getNbChildren(); ++j)
		{
			if (widget->getChildIdentifier(j, visualizationWidgetIdentifier))
			{
				widgetsToExport.push_back(visualizationWidgetIdentifier);
			}
		}
	}

	CString serializedString = json::Serialize(jsonRepresentation).c_str();
	return serializedString;
}

bool CVisualizationTree::deserialize(const CString& serializedVisualizationTree)
{
	// Empty this visualization tree
	auto widgetIdentifier = OV_UndefinedIdentifier;
	while (this->getNextVisualizationWidgetIdentifier(widgetIdentifier) && widgetIdentifier != OV_UndefinedIdentifier)
	{
		this->destroyHierarchy(widgetIdentifier, true);
		widgetIdentifier = OV_UndefinedIdentifier;
	}


	json::Array jsonRepresentation = json::Deserialize(serializedVisualizationTree.toASCIIString());

	for (auto itWidget = jsonRepresentation.begin(); itWidget != jsonRepresentation.end(); ++itWidget)
	{
		json::Value& jsonWidget = *itWidget;

		CIdentifier widgetIdentifier;
		widgetIdentifier.fromString(jsonWidget["identifier"].ToString().c_str());

		CIdentifier boxIdentifier;
		boxIdentifier.fromString(jsonWidget["boxIdentifier"].ToString().c_str());

		EVisualizationWidgetType widgetType = EVisualizationWidgetType(jsonWidget["type"].ToInt());

		CString widgetName;
		if (widgetType == EVisualizationWidget_VisualizationBox)
		{
			const IBox* box = m_Scenario->getBoxDetails(boxIdentifier);
			if(!box)
			{
				m_KernelContext.getLogManager() << LogLevel_Error << "The box identifier [" << boxIdentifier << "] used in Window manager was not found in the scenario.\n";
				return false;
			}
			widgetName = box->getName();
		}
		else
		{
			widgetName = jsonWidget["name"].ToString().c_str();
		}

		CIdentifier newVisualizationWidgetIdentifier;

		CIdentifier parentIdentifier;
		parentIdentifier.fromString(jsonWidget["parentIdentifier"].ToString().c_str());

		unsigned int widgetIndex = 0;
		if (this->getVisualizationWidget(parentIdentifier))
		{
			widgetIndex = static_cast<unsigned int>(jsonWidget["index"].ToInt());
		}
		unsigned int widgetChildCount = static_cast<unsigned int>(jsonWidget["childCount"].ToInt());

		this->addVisualizationWidget(
		            newVisualizationWidgetIdentifier,
		            widgetName,
		            widgetType,
		            parentIdentifier,
		            widgetIndex,
		            boxIdentifier,
		            widgetChildCount,
		            widgetIdentifier
		            );

		if (widgetIdentifier != newVisualizationWidgetIdentifier)
		{
			m_KernelContext.getLogManager() << LogLevel_Error << "Visualization widget [" << widgetIdentifier << "] for box [" << boxIdentifier << "] could not be imported.\n";
			return false;
		}

		IVisualizationWidget* visualizationWidget = this->getVisualizationWidget(widgetIdentifier);

		if (visualizationWidget)
		{
			visualizationWidget->setWidth(static_cast<unsigned int>(jsonWidget["width"].ToInt()));
			visualizationWidget->setHeight(static_cast<unsigned int>(jsonWidget["height"].ToInt()));
			visualizationWidget->setDividerPosition(jsonWidget["dividerPosition"].ToInt());
			visualizationWidget->setMaxDividerPosition(jsonWidget["maxDividerPosition"].ToInt());
		}

	}

	return true;
}
