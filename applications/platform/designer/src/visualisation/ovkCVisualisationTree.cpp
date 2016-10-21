#include <iostream>
#include <cstdlib>
#include <cstring>
#include <json/json.h>

#include "ovkCVisualisationTree.h"
#include "ovkCVisualisationWidget.h"
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

	struct TTestEqVisualisationWidgetType
	{
		TTestEqVisualisationWidgetType(EVisualisationWidgetType oType) : m_oType(oType) { }
		bool operator()(map<CIdentifier, IVisualisationWidget*>::const_iterator it) const
		{
			return it->second->getType() == m_oType;
		}
		EVisualisationWidgetType m_oType;
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

CVisualisationTree::CVisualisationTree(const IKernelContext& kernelContext) :
	m_KernelContext(kernelContext),
	m_Scenario(nullptr),
	m_TreeStore(nullptr),
	m_TreeViewCB(nullptr)
{
}

CVisualisationTree::~CVisualisationTree()
{
	//delete display panels
	//TODO!

	for (auto& widget : m_VisualisationWidgets)
	{
		delete widget.second;
	}

	g_object_unref(m_TreeStore);
}

bool CVisualisationTree::init(const IScenario* scenario)
{
	m_Scenario = scenario;

	//create tree store
	m_TreeStore = gtk_tree_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_ULONG, G_TYPE_STRING, G_TYPE_POINTER);

	return true;
}

bool CVisualisationTree::getNextVisualisationWidgetIdentifier(CIdentifier& identifier) const
{
	return getNextTIdentifier<IVisualisationWidget, TTestTrue<IVisualisationWidget> >
		(m_VisualisationWidgets, identifier, TTestTrue<IVisualisationWidget>());
}

bool CVisualisationTree::getNextVisualisationWidgetIdentifier(CIdentifier& rIdentifier, EVisualisationWidgetType type) const
{
	return getNextTIdentifier<IVisualisationWidget, TTestEqVisualisationWidgetType >
		(m_VisualisationWidgets, rIdentifier, TTestEqVisualisationWidgetType(type));
}

bool CVisualisationTree::isVisualisationWidget(const CIdentifier& identifier) const
{
	return m_VisualisationWidgets.find(identifier) != m_VisualisationWidgets.end();
}

IVisualisationWidget* CVisualisationTree::getVisualisationWidget(const CIdentifier& identifier) const
{
	auto it = m_VisualisationWidgets.find(identifier);

	if (it == m_VisualisationWidgets.end())
	{
		return nullptr;
	}
	return it->second;
}

IVisualisationWidget* CVisualisationTree::getVisualisationWidgetFromBoxIdentifier(const CIdentifier& boxIdentifier) const
{
	for (auto& widget : m_VisualisationWidgets)
	{
		if (widget.second->getBoxIdentifier() == boxIdentifier)
		{
			return widget.second;
		}
	}
	return nullptr;
}

bool CVisualisationTree::addVisualisationWidget(CIdentifier& identifier, const CString& name, EVisualisationWidgetType type,
	const CIdentifier& parentIdentifier, uint32 parentIndex, const CIdentifier& boxIdentifier, uint32 childCount, const OpenViBE::CIdentifier& suggestedIdentifier)
{
	m_KernelContext.getLogManager() << LogLevel_Trace << "Adding new visualisation widget\n";

	//create new widget
	IVisualisationWidget* visualisationWidget = new CVisualisationWidget(m_KernelContext);
	identifier = getUnusedIdentifier(suggestedIdentifier);

	if(visualisationWidget->initialize(identifier, name, type, parentIdentifier, boxIdentifier, childCount) == false)
	{
		m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to add new visualisation widget (couldn't initialize it)\n";
		delete visualisationWidget;
		return false;
	}

	// assign a parent to it
	if(parentIdentifier != OV_UndefinedIdentifier)
	{
		m_KernelContext.getLogManager() << LogLevel_Trace << "Parenting visualisation widget\n";
		IVisualisationWidget* parentVisualisationWidget = getVisualisationWidget(parentIdentifier);

		if (parentVisualisationWidget != NULL)
		{
			if(parentVisualisationWidget->getType() == EVisualisationWidget_VisualisationWindow)
			{
				//extend number of children of parent window if necessary
				if(parentVisualisationWidget->getNbChildren() <= parentIndex)
				{
					for(unsigned int i = parentVisualisationWidget->getNbChildren(); i<=parentIndex; i++)
					{
						parentVisualisationWidget->addChild(OV_UndefinedIdentifier);
					}
				}
			}

			if(parentVisualisationWidget->setChildIdentifier(parentIndex, identifier) == false)
			{
				m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to add new visualisation widget (couldn't set child identifier in parent window)\n";
				return false;
			}
		}
		else
		{
			m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to add new visualisation widget (couldn't find parent)\n";
			return false;
		}
	}

	//add it to widgets map
	m_VisualisationWidgets[identifier]=visualisationWidget;
	return true;
}

bool CVisualisationTree::getVisualisationWidgetIndex(const CIdentifier& identifier, uint32& index) const
{
	IVisualisationWidget* visualisationWidget = getVisualisationWidget(identifier);
	if (!visualisationWidget)
	{
		return false;
	}

	const CIdentifier& parentIdentifier = visualisationWidget->getParentIdentifier();
	if (parentIdentifier == OV_UndefinedIdentifier)
	{
		return false;
	}

	IVisualisationWidget* parentVisualisationWidget = getVisualisationWidget(parentIdentifier);
	if (!parentVisualisationWidget)
	{
		m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to unparent visualisation widget (couldn't find parent)\n";
		return false;
	}

	parentVisualisationWidget->getChildIndex(identifier, index);

	return true;
}

bool CVisualisationTree::destroyHierarchy(const CIdentifier& identifier, bool destroyVisualisationBoxes)
{
	bool res = true;

	IVisualisationWidget* visualisationWidget = getVisualisationWidget(identifier);

	//is hierarchy top item a window?
	if (visualisationWidget->getType() == EVisualisationWidget_VisualisationWindow)
	{
		CIdentifier childIdentifier;
		for (uint32 i = 0; i < visualisationWidget->getNbChildren(); i++)
		{
			visualisationWidget->getChildIdentifier(i, childIdentifier);
			res &=_destroyHierarchy(childIdentifier, destroyVisualisationBoxes);
		}

		//delete this window in kernel factory and erase its slot in map
		delete visualisationWidget;
		auto it = m_VisualisationWidgets.find(identifier);
		m_VisualisationWidgets.erase(it);
	}
	else //top item is a widget
	{
		uint32 index;
		unparentVisualisationWidget(identifier, index);
		_destroyHierarchy(identifier, destroyVisualisationBoxes);
	}

	return res;
}

bool CVisualisationTree::_destroyHierarchy(const CIdentifier& identifier, bool destroyVisualisationBoxes)
{
	IVisualisationWidget* visualisationWidget = getVisualisationWidget(identifier);
	if (!visualisationWidget)
	{
		return false;
	}

	//remove children
	CIdentifier l_oChildIdentifier;
	uint32 nbChildren = visualisationWidget->getNbChildren();
	for (uint32 i = 0; i < nbChildren; i++)
	{
		visualisationWidget->getChildIdentifier(i, l_oChildIdentifier);
		_destroyHierarchy(l_oChildIdentifier, destroyVisualisationBoxes);
	}

	//if parent widget is a window, remove this widget from it
	if (visualisationWidget->getType() == EVisualisationWidget_VisualisationPanel)
	{
		IVisualisationWidget* l_pVisualisationWindow = getVisualisationWidget(visualisationWidget->getParentIdentifier());
		if(l_pVisualisationWindow != NULL)
		{
			l_pVisualisationWindow->removeChild(identifier);
		}
	}

	//if this widget is a visualisation box and they are to be unaffected
	if (visualisationWidget->getType() == EVisualisationWidget_VisualisationBox && destroyVisualisationBoxes == false)
	{
		uint32 index;
		unparentVisualisationWidget(identifier, index);
	}
	else
	{
		m_KernelContext.getLogManager() << LogLevel_Trace << "Deleting visualisation widget\n";
		delete visualisationWidget;
		map<CIdentifier, IVisualisationWidget*>::iterator it = m_VisualisationWidgets.find(identifier);
		m_VisualisationWidgets.erase(it);
	}

	return true;
}

bool CVisualisationTree::unparentVisualisationWidget(const CIdentifier& identifier, uint32& index)
{
	//retrieve widget to be unparented
	IVisualisationWidget* visualisationWidget = getVisualisationWidget(identifier);
	if (visualisationWidget == NULL)
	{
		return false;
	}

	//get its parent identifier
	const CIdentifier& parentIdentifier = visualisationWidget->getParentIdentifier();
	if (parentIdentifier == OV_UndefinedIdentifier)
	{
		return true;
	}

	//unparent widget
	visualisationWidget->setParentIdentifier(OV_UndefinedIdentifier);

	//retrieve parent and remove widget from its children list
	IVisualisationWidget* parentVisualisationWidget = getVisualisationWidget(parentIdentifier);
	if (parentVisualisationWidget != NULL)
	{
		parentVisualisationWidget->getChildIndex(identifier, index);
		parentVisualisationWidget->removeChild(identifier);
	}

	return true;
}

bool CVisualisationTree::parentVisualisationWidget(const CIdentifier& identifier, const CIdentifier& rParentIdentifier, uint32 index)
{
	if (rParentIdentifier == OV_UndefinedIdentifier)
	{
		return false;
	}

	//retrieve widget to be parented
	IVisualisationWidget* l_pVisualisationWidget = getVisualisationWidget(identifier);
	if (!l_pVisualisationWidget)
	{
		return false;
	}

	l_pVisualisationWidget->setParentIdentifier(rParentIdentifier);

	//retrieve its parent
	IVisualisationWidget* parentVisualisationWidget = getVisualisationWidget(rParentIdentifier);
	if (!parentVisualisationWidget)
	{
		m_KernelContext.getLogManager() << LogLevel_Warning << "Failed to parent visualisation widget (couldn't find parent)\n";
		return false;
	}

	parentVisualisationWidget->setChildIdentifier(index, identifier);

	return true;
}

CIdentifier CVisualisationTree::getUnusedIdentifier(const CIdentifier& suggestedIdentifier) const
{
	uint64 proposedIdentifier=(((uint64)rand())<<32)+((uint64)rand());
	if(suggestedIdentifier != OV_UndefinedIdentifier)
	{
		proposedIdentifier = suggestedIdentifier.toUInteger()-1;
	}

	CIdentifier result;
	map<CIdentifier, IVisualisationWidget*>::const_iterator i;
	do
	{
		proposedIdentifier++;
		result = CIdentifier(proposedIdentifier);
		i = m_VisualisationWidgets.find(result);
	}
	while (i != m_VisualisationWidgets.end() || result == OV_UndefinedIdentifier);
	return result;
}

::GtkTreeView* CVisualisationTree::createTreeViewWithModel()
{
	return GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_TreeStore)));
}

bool CVisualisationTree::setTreeViewCB(ITreeViewCB* treeViewCB)
{
	m_TreeViewCB = treeViewCB;
	return true;
}

bool CVisualisationTree::reloadTree()
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
		EVisualisationTreeColumn_StringName, "Unaffected display plugins",
		EVisualisationTreeColumn_StringStockIcon, m_TreeViewCB->getTreeWidgetIcon(EVisualisationTreeNode_Unaffected),
		EVisualisationTreeColumn_ULongNodeType, (unsigned long)EVisualisationTreeNode_Unaffected,
		EVisualisationTreeColumn_StringIdentifier, (const char*)OV_UndefinedIdentifier.toString(),
		-1);

	//reload unaffected visualisation boxes
	CIdentifier visualisationWidgetIdentifier = OV_UndefinedIdentifier;
	while(getNextVisualisationWidgetIdentifier(visualisationWidgetIdentifier, EVisualisationWidget_VisualisationBox) == true)
	{
		IVisualisationWidget* visualisationWidget = getVisualisationWidget(visualisationWidgetIdentifier);
		//load widget if it doesn't have a parent (== is unaffected)
		if(visualisationWidget->getParentIdentifier() == OV_UndefinedIdentifier)
		{
			loadVisualisationWidget(visualisationWidget, &iter);
		}
	}

	//reload visualisation windows
	CIdentifier visualisationWindowIdentifier = OV_UndefinedIdentifier;
	while(getNextVisualisationWidgetIdentifier(visualisationWindowIdentifier, EVisualisationWidget_VisualisationWindow) == true)
	{
		loadVisualisationWidget(getVisualisationWidget(visualisationWindowIdentifier), NULL);
	}

	return true;
}

//Tree helper functions
//---------------------

bool CVisualisationTree::getTreeSelection(::GtkTreeView* preeView, ::GtkTreeIter* iter)
{
	::GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(preeView);
	::GtkTreeModel* treeModel = GTK_TREE_MODEL(m_TreeStore);
	return gtk_tree_selection_get_selected(treeSelection, &treeModel, iter) != 0;
}

::GtkTreePath* CVisualisationTree::getTreePath(::GtkTreeIter* pTreeIter) const
{
	return (pTreeIter == NULL) ? NULL : gtk_tree_model_get_path(GTK_TREE_MODEL(m_TreeStore), pTreeIter);
}

unsigned long	CVisualisationTree::getULongValueFromTreeIter(::GtkTreeIter* treeIter, EVisualisationTreeColumn visualisationTreeColumn) const
{
	unsigned long value = 0;
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), treeIter, visualisationTreeColumn, &value, -1);
	return value;
}

bool CVisualisationTree::getStringValueFromTreeIter(::GtkTreeIter* treeIter, char*& string, EVisualisationTreeColumn col) const
{
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), treeIter, col, &string, -1);
	return true;
}

bool CVisualisationTree::getPointerValueFromTreeIter(::GtkTreeIter* treeIter, void*& pointer, EVisualisationTreeColumn col) const
{
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), treeIter, col, &pointer, -1);
	return true;
}

bool CVisualisationTree::getIdentifierFromTreeIter(::GtkTreeIter* iter, CIdentifier& identifier, EVisualisationTreeColumn col) const
{
	char* stringIdentifier = NULL;
	getStringValueFromTreeIter(iter, stringIdentifier, col);
	identifier.fromString(CString(stringIdentifier));
	return true;
}

//looks for a tree node named 'label' of class 'type' from tree root
bool CVisualisationTree::findChildNodeFromRoot(::GtkTreeIter* iter, const char* label, EVisualisationTreeNode type)
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
bool CVisualisationTree::findChildNodeFromParent(::GtkTreeIter* iter, const char* label, EVisualisationTreeNode type)
{
	if (_findChildNodeFromParent(iter, label, type) == true)
	{
		*iter = m_InternalTreeNode;
		return true;
	}
	return false;
}

//looks for a tree node named 'label' of class 'type' from parent passed as parameter
bool CVisualisationTree::_findChildNodeFromParent(::GtkTreeIter* iter, const char* label, EVisualisationTreeNode type)
{
	gchar* name;
	unsigned long typeAsInt;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), iter,
		EVisualisationTreeColumn_StringName, &name,
		EVisualisationTreeColumn_ULongNodeType, &typeAsInt,
		-1);

	if (!name)
	{
		m_KernelContext.getLogManager() << LogLevel_Error << "Can not get values from the model" << "\n";
		return false;
	}

	if (strcmp(label, name) == 0 && type == (EVisualisationTreeNode)typeAsInt)
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

bool CVisualisationTree::findChildNodeFromRoot(::GtkTreeIter* iter, void* widget)
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

bool CVisualisationTree::findChildNodeFromParent(::GtkTreeIter* iter, void* widget)
{
	if (_findChildNodeFromParent(iter, widget))
	{
		*iter = m_InternalTreeNode;
		return true;
	}
	return false;
}

bool CVisualisationTree::_findChildNodeFromParent(::GtkTreeIter* iter, void* widget)
{
	void* currentWidget;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), iter, EVisualisationTreeColumn_PointerWidget, &currentWidget, -1);
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

bool CVisualisationTree::findChildNodeFromRoot(::GtkTreeIter* iter, CIdentifier identifier)
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

bool CVisualisationTree::findChildNodeFromParent(::GtkTreeIter* iter, CIdentifier identifier)
{
	if(_findChildNodeFromParent(iter, identifier) == true)
	{
		*iter = m_InternalTreeNode;
		return true;
	}
	return false;
}

bool CVisualisationTree::_findChildNodeFromParent(::GtkTreeIter* pIter, CIdentifier identifier)
{
	gchar* identifierAsString;
	CIdentifier currentIdentifier;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), pIter, EVisualisationTreeColumn_StringIdentifier, &identifierAsString, -1);
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

bool CVisualisationTree::findParentNode(::GtkTreeIter* iter, EVisualisationTreeNode type)
{
	unsigned long typeAsInt;
	::GtkTreeIter currentIter;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_TreeStore), iter, EVisualisationTreeColumn_ULongNodeType, &typeAsInt, -1);
	if (type == (EVisualisationTreeNode)typeAsInt)
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

bool CVisualisationTree::dragDataReceivedOutsideWidgetCB(const CIdentifier& sourceWidgetIdentifierr, ::GtkWidget* destinationWidget, EDragDataLocation location)
{
	//retrieve source widget parent
	//-----------------------------
	IVisualisationWidget* sourceVisualizationWidget = getVisualisationWidget(sourceWidgetIdentifierr);
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
	getIdentifierFromTreeIter(&destinationIterator, destinationWidgetIdentifier, EVisualisationTreeColumn_StringIdentifier);
	IVisualisationWidget* destinationVisualizationWidget = getVisualisationWidget(destinationWidgetIdentifier);
	if (destinationVisualizationWidget == NULL)
	{
		return false;
	}
	// dst widget is the widget already present in
	CIdentifier destinationParentIdentifier = destinationVisualizationWidget->getParentIdentifier();

	//unparent source widget
	uint32 sourceIndex = 0;
	unparentVisualisationWidget(sourceWidgetIdentifierr, sourceIndex);

	//unparent dest widget
	uint32 destinationIndex = 0;
	unparentVisualisationWidget(destinationWidgetIdentifier, destinationIndex);

	//create paned widget
	EVisualisationWidgetType panedType = (location == EDragData_Top || location == EDragData_Bottom) ? EVisualisationWidget_VerticalSplit : EVisualisationWidget_HorizontalSplit;
	CIdentifier panedIdentifier;
	addVisualisationWidget(
		panedIdentifier,
		CString(panedType == EVisualisationWidget_VerticalSplit ? "Vertical split" : "Horizontal split"),
		panedType,
		destinationParentIdentifier, //parent paned to dest widget parent
		destinationIndex, //put it at the index occupied by dest widget
		OV_UndefinedIdentifier, //no box algorithm for a paned
		2, //2 children
		OV_UndefinedIdentifier); //no prefered visualization identifier
	IVisualisationWidget* panedVisualisationWidget = getVisualisationWidget(panedIdentifier);

	//add attributes
	if (m_TreeViewCB != NULL)
	{
		m_TreeViewCB->createTreeWidget(panedVisualisationWidget);
	}

	//reparent widgets
	uint32 newSourceIndex = (location == EDragData_Top || location == EDragData_Left) ? 0 : 1;
	parentVisualisationWidget(sourceWidgetIdentifierr, panedIdentifier, newSourceIndex);
	parentVisualisationWidget(destinationWidgetIdentifier, panedIdentifier, 1-newSourceIndex);

	//update Gtk tree
	reloadTree();

	return true;
}

bool CVisualisationTree::dragDataReceivedInWidgetCB(const CIdentifier& sourceWidgetIdentifier, ::GtkWidget* destinationWidget)
{
	//retrieve source widget parent
	IVisualisationWidget* sourceVisualisationWidget = getVisualisationWidget(sourceWidgetIdentifier);
	OV_EXCEPTION_UNLESS_D(
	            sourceVisualisationWidget,
	            "Source visualization identifier does not exist in the tree", ErrorType::ResourceNotFound);

	CIdentifier sourceParentIdentifier = sourceVisualisationWidget->getParentIdentifier();

	//retrieve dest widget and dest widget parent identifiers
	::GtkTreeIter destinationIterator;
	if (!findChildNodeFromRoot(&destinationIterator, m_TreeViewCB->getTreeWidget(destinationWidget)))
	{
		return false;
	}

	CIdentifier destinationWidgetIdentifier;
	getIdentifierFromTreeIter(&destinationIterator, destinationWidgetIdentifier, EVisualisationTreeColumn_StringIdentifier);
	IVisualisationWidget* destinationVisualisationWidget = getVisualisationWidget(destinationWidgetIdentifier);
	if (!destinationVisualisationWidget)
	{
		return false;
	}

	CIdentifier destinationParentIdentifier = destinationVisualisationWidget->getParentIdentifier();

	//unparent source widget
	uint32 sourceIndex;
	unparentVisualisationWidget(sourceWidgetIdentifier, sourceIndex);

	//destroy, unparent or reparent dest widget
	uint32 destinationIndex;

	//if source widget was unaffected
	if (sourceParentIdentifier == OV_UndefinedIdentifier)
	{
		//if dest widget was dummy, destroy it
		if (destinationVisualisationWidget->getType() == EVisualisationWidget_Undefined)
		{
			getVisualisationWidgetIndex(destinationWidgetIdentifier, destinationIndex);
			destroyHierarchy(destinationWidgetIdentifier, true);
		}
		else //dest widget becomes unaffected
		{
			unparentVisualisationWidget(destinationWidgetIdentifier, destinationIndex);
		}
	}
	else //source widget was affected
	{
		//unparent dest widget
		unparentVisualisationWidget(destinationWidgetIdentifier, destinationIndex);

		//reparent it to source widget parent
		parentVisualisationWidget(destinationWidgetIdentifier, sourceParentIdentifier, sourceIndex);
	}

	//reparent source widget
	parentVisualisationWidget(sourceWidgetIdentifier, destinationParentIdentifier, destinationIndex);

	//update Gtk tree
	reloadTree();

	return true;
}

bool CVisualisationTree::loadVisualisationWidget(IVisualisationWidget* visualisationWidget, ::GtkTreeIter* parentIter)
{
	//create visualisation widget
	//---------------------------
	::GtkWidget* widget = m_TreeViewCB->loadTreeWidget(visualisationWidget);

	//add visualisation widget node to tree store
	::GtkTreeIter iter;
	gtk_tree_store_append(m_TreeStore, &iter, parentIter);

	//retrieve values of tree node fields
	EVisualisationTreeNode childType;
	switch(visualisationWidget->getType())
	{
		case EVisualisationWidget_VisualisationWindow:
			childType = EVisualisationTreeNode_VisualisationWindow;
			break;
		case EVisualisationWidget_VisualisationPanel:
			childType = EVisualisationTreeNode_VisualisationPanel; break;
		case EVisualisationWidget_VisualisationBox:
			childType = EVisualisationTreeNode_VisualisationBox;
			break;
		case EVisualisationWidget_HorizontalSplit:
			childType = EVisualisationTreeNode_HorizontalSplit;
			break;
		case EVisualisationWidget_VerticalSplit:
			childType = EVisualisationTreeNode_VerticalSplit;
			break;
		case EVisualisationWidget_Undefined:
			childType=EVisualisationTreeNode_Undefined;
			break;
	}

	CString stockIconString = m_TreeViewCB->getTreeWidgetIcon(childType);

	if (visualisationWidget->getType() == EVisualisationWidget_VisualisationBox)
	{
		const IBox* box = m_Scenario->getBoxDetails(visualisationWidget->getBoxIdentifier());
		if (!box)
		{
			m_KernelContext.getLogManager() << LogLevel_Error << "Box with identifier " << visualisationWidget->getBoxIdentifier() << " not found in the scenario" << "\n";
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
		EVisualisationTreeColumn_StringName, (const char*)visualisationWidget->getName(),
		EVisualisationTreeColumn_StringStockIcon, (const char*)stockIconString,
		EVisualisationTreeColumn_ULongNodeType, (unsigned long)childType,
		EVisualisationTreeColumn_StringIdentifier, (const char*)visualisationWidget->getIdentifier().toString(),
		EVisualisationTreeColumn_PointerWidget, widget,
		-1);

	//load visualisation widget hierarchy
	//-----------------------------------
	//create a dummy child for visualisation panels if none exists
	if(visualisationWidget->getType() == EVisualisationWidget_VisualisationPanel)
	{
		CIdentifier childIdentifier;
		visualisationWidget->getChildIdentifier(0, childIdentifier);
		if (childIdentifier == OV_UndefinedIdentifier)
		{
			addVisualisationWidget(
				childIdentifier,
				"Empty",
				EVisualisationWidget_Undefined,
				visualisationWidget->getIdentifier(),
				0,
				OV_UndefinedIdentifier,
				0,
				OV_UndefinedIdentifier);
		}
	}

	for (uint32 i = 0;  i < visualisationWidget->getNbChildren(); i++)
	{
		CIdentifier childIdentifier;
		visualisationWidget->getChildIdentifier(i, childIdentifier);

		loadVisualisationWidget(getVisualisationWidget(childIdentifier), &iter);
	}

	//complete visualisation widget loading now that its hierarchy is loaded
	m_TreeViewCB->endLoadTreeWidget(visualisationWidget);

	return true;
}

bool CVisualisationTree::setToolbar(const CIdentifier& boxIdentifier, ::GtkWidget* toolbarWidget)
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

bool CVisualisationTree::setWidget(const CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget)
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



json::Object CVisualisationTree::serializeWidget(IVisualisationWidget& widget) const
{
	json::Object jsonRepresentation;

	jsonRepresentation["identifier"] = widget.getIdentifier().toString().toASCIIString();

	// visualisation box name can be retrieved from corresponding IBox, so we can skip it for these
	if (widget.getType() != EVisualisationWidget_VisualisationBox)
	{
		jsonRepresentation["name"] = widget.getName().toASCIIString();
	}

	jsonRepresentation["type"] = widget.getType();
	jsonRepresentation["parentIdentifier"] = widget.getParentIdentifier().toString().toASCIIString();

	// visualisation widget index
	IVisualisationWidget* parentVisualisationWidget = this->getVisualisationWidget(widget.getParentIdentifier());
	if (parentVisualisationWidget)
	{
		uint32 childIndex = 0;
		parentVisualisationWidget->getChildIndex(widget.getIdentifier(), childIndex);
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

OpenViBE::CString CVisualisationTree::serialize() const
{
	json::Array jsonRepresentation;

	std::vector<CIdentifier> widgetsToExport;

	CIdentifier visualizationWidgetIdentifier;
	while (this->getNextVisualisationWidgetIdentifier(visualizationWidgetIdentifier))
	{
		IVisualisationWidget* widget = this->getVisualisationWidget(visualizationWidgetIdentifier);
		if (widget->getType() == EVisualisationWidget_VisualisationWindow
		        || widget->getParentIdentifier()==OV_UndefinedIdentifier)
		{
			widgetsToExport.push_back(visualizationWidgetIdentifier);
		}
	}

	for (size_t i = 0; i < widgetsToExport.size(); ++i)
	{
		IVisualisationWidget* widget = this->getVisualisationWidget(widgetsToExport[i]);

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

bool CVisualisationTree::deserialize(const CString& serializedVisualizationTree)
{
	// Empty this visualization tree
	auto widgetIdentifier = OV_UndefinedIdentifier;
	while (this->getNextVisualisationWidgetIdentifier(widgetIdentifier) && widgetIdentifier != OV_UndefinedIdentifier)
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

		EVisualisationWidgetType widgetType = EVisualisationWidgetType(jsonWidget["type"].ToInt());

		CString widgetName;
		if (widgetType == EVisualisationWidget_VisualisationBox)
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
		if (this->getVisualisationWidget(parentIdentifier))
		{
			widgetIndex = static_cast<unsigned int>(jsonWidget["index"].ToInt());
		}
		unsigned int widgetChildCount = static_cast<unsigned int>(jsonWidget["childCount"].ToInt());

		this->addVisualisationWidget(
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

		IVisualisationWidget* visualizationWidget = this->getVisualisationWidget(widgetIdentifier);

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
