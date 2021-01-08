#include <iostream>
#include <cstdlib>
#include <cstring>
#include <json/json.h>

#include "ovdCVisualizationTree.h"
#include "ovdCVisualizationWidget.h"
#include "../ovdAssert.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Plugins;
using namespace /*OpenViBE::*/VisualizationToolkit;
using namespace std;

namespace {
template <class T>
struct STestTrue
{
	bool operator()(typename map<CIdentifier, T*>::const_iterator /*it*/) const { return true; }
};

struct STestEqVisualizationWidgetType
{
	STestEqVisualizationWidgetType(const EVisualizationWidget widgetType) : type(widgetType) { }

	bool operator()(const map<CIdentifier, IVisualizationWidget*>::const_iterator& it) const { return it->second->getType() == type; }

	EVisualizationWidget type;
};

template <class T, class TTest>
bool getNextID(const map<CIdentifier, T*>& ids, CIdentifier& id, const TTest& test)
{
	typename map<CIdentifier, T*>::const_iterator it;

	if (id == CIdentifier::undefined()) { it = ids.begin(); }
	else
	{
		it = ids.find(id);
		if (it == ids.end())
		{
			id = CIdentifier::undefined();
			return false;
		}
		++it;
	}

	while (it != ids.end())
	{
		if (test(it))
		{
			id = it->first;
			return true;
		}
		++it;
	}

	return false;
}
}  // namespace

CVisualizationTree::~CVisualizationTree()
{
	for (auto& widget : m_widgets) { delete widget.second; }
	g_object_unref(m_treeStore);
}

bool CVisualizationTree::init(const IScenario* scenario)
{
	m_scenario = scenario;
	//create tree store
	m_treeStore = gtk_tree_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_ULONG, G_TYPE_STRING, G_TYPE_POINTER);
	return true;
}

bool CVisualizationTree::getNextVisualizationWidgetIdentifier(CIdentifier& id) const
{
	return getNextID<IVisualizationWidget, STestTrue<IVisualizationWidget>>(m_widgets, id, STestTrue<IVisualizationWidget>());
}

bool CVisualizationTree::getNextVisualizationWidgetIdentifier(CIdentifier& id, const EVisualizationWidget type) const
{
	return getNextID<IVisualizationWidget, STestEqVisualizationWidgetType>(m_widgets, id, STestEqVisualizationWidgetType(type));
}

bool CVisualizationTree::isVisualizationWidget(const CIdentifier& id) const { return m_widgets.find(id) != m_widgets.end(); }

IVisualizationWidget* CVisualizationTree::getVisualizationWidget(const CIdentifier& id) const
{
	const auto it = m_widgets.find(id);
	if (it == m_widgets.end()) { return nullptr; }
	return it->second;
}

IVisualizationWidget* CVisualizationTree::getVisualizationWidgetFromBoxIdentifier(const CIdentifier& boxID) const
{
	for (auto& widget : m_widgets) { if (widget.second->getBoxIdentifier() == boxID) { return widget.second; } }
	return nullptr;
}

bool CVisualizationTree::addVisualizationWidget(CIdentifier& id, const CString& name, const EVisualizationWidget type, const CIdentifier& parentID,
												const size_t parentIdx, const CIdentifier& boxID, const size_t nChild, const CIdentifier& suggestedID)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "Adding new visualization widget\n";

	//create new widget
	IVisualizationWidget* widget = new CVisualizationWidget(m_kernelCtx);
	id                           = getUnusedIdentifier(suggestedID);

	if (!widget->initialize(id, name, type, parentID, boxID, nChild))
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to add new visualization widget (couldn't initialize it)\n";
		delete widget;
		return false;
	}

	// assign a parent to it
	if (parentID != CIdentifier::undefined())
	{
		m_kernelCtx.getLogManager() << LogLevel_Debug << "Parenting visualization widget\n";
		IVisualizationWidget* parentWidget = getVisualizationWidget(parentID);

		if (parentWidget != nullptr)
		{
			if (parentWidget->getType() == EVisualizationWidget::Window)
			{
				//extend number of children of parent window if necessary
				if (parentWidget->getNbChildren() <= parentIdx)
				{
					for (size_t i = parentWidget->getNbChildren(); i <= parentIdx; ++i) { parentWidget->addChild(CIdentifier::undefined()); }
				}
			}

			if (!parentWidget->setChildIdentifier(parentIdx, id))
			{
				m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to add new visualization widget (couldn't set child identifier in parent window)\n";
				return false;
			}
		}
		else
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to add new visualization widget (couldn't find parent)\n";
			return false;
		}
	}

	//add it to widgets map
	m_widgets[id] = widget;
	return true;
}

bool CVisualizationTree::getVisualizationWidgetIndex(const CIdentifier& id, size_t& index) const
{
	IVisualizationWidget* visualizationWidget = getVisualizationWidget(id);
	if (!visualizationWidget)
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to get widget.\n";
		return false;
	}

	const CIdentifier& parentIdentifier = visualizationWidget->getParentIdentifier();
	if (parentIdentifier == CIdentifier::undefined())
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to get parent identifier widget\n";
		return false;
	}

	IVisualizationWidget* parentVisualizationWidget = getVisualizationWidget(parentIdentifier);
	if (!parentVisualizationWidget)
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to unparent visualization widget (couldn't find parent)\n";
		return false;
	}

	parentVisualizationWidget->getChildIndex(id, index);

	return true;
}

bool CVisualizationTree::destroyHierarchy(const CIdentifier& id, const bool destroyVisualizationBoxes)
{
	bool res = true;

	IVisualizationWidget* visualizationWidget = getVisualizationWidget(id);

	//is hierarchy top item a window?
	if (visualizationWidget->getType() == EVisualizationWidget::Window)
	{
		CIdentifier childIdentifier;
		for (size_t i = 0; i < visualizationWidget->getNbChildren(); ++i)
		{
			visualizationWidget->getChildIdentifier(i, childIdentifier);
			res &= destroyHierarchyR(childIdentifier, destroyVisualizationBoxes);
		}

		//delete this window in kernel factory and erase its slot in map
		delete visualizationWidget;
		const auto it = m_widgets.find(id);
		m_widgets.erase(it);
	}
	else //top item is a widget
	{
		size_t index;
		unparentVisualizationWidget(id, index);
		destroyHierarchyR(id, destroyVisualizationBoxes);
	}

	return res;
}

bool CVisualizationTree::destroyHierarchyR(const CIdentifier& id, const bool destroy)
{
	IVisualizationWidget* widget = getVisualizationWidget(id);
	if (!widget) { return false; }

	//remove children
	CIdentifier childID;
	const size_t nChildren = widget->getNbChildren();
	for (size_t i = 0; i < nChildren; ++i)
	{
		widget->getChildIdentifier(i, childID);
		destroyHierarchyR(childID, destroy);
	}

	//if parent widget is a window, remove this widget from it
	if (widget->getType() == EVisualizationWidget::Panel)
	{
		IVisualizationWidget* window = getVisualizationWidget(widget->getParentIdentifier());
		if (window != nullptr) { window->removeChild(id); }
	}

	//if this widget is a visualization box and they are to be unaffected
	if (widget->getType() == EVisualizationWidget::Box && !destroy)
	{
		size_t index;
		unparentVisualizationWidget(id, index);
	}
	else
	{
		m_kernelCtx.getLogManager() << LogLevel_Debug << "Deleting visualization widget\n";
		delete widget;
		const auto it = m_widgets.find(id);
		m_widgets.erase(it);
	}

	return true;
}

bool CVisualizationTree::unparentVisualizationWidget(const CIdentifier& id, size_t& index)
{
	//retrieve widget to be unparented
	IVisualizationWidget* widget = getVisualizationWidget(id);
	if (widget == nullptr) { return false; }

	//get its parent identifier
	const CIdentifier& parentID = widget->getParentIdentifier();
	if (parentID == CIdentifier::undefined()) { return true; }

	//unparent widget
	widget->setParentIdentifier(CIdentifier::undefined());

	//retrieve parent and remove widget from its children list
	IVisualizationWidget* parentWidget = getVisualizationWidget(parentID);
	if (parentWidget != nullptr)
	{
		parentWidget->getChildIndex(id, index);
		parentWidget->removeChild(id);
	}

	return true;
}

bool CVisualizationTree::parentVisualizationWidget(const CIdentifier& id, const CIdentifier& parentID, const size_t index)
{
	if (parentID == CIdentifier::undefined()) { return false; }

	//retrieve widget to be parented
	IVisualizationWidget* widget = getVisualizationWidget(id);
	if (!widget) { return false; }

	widget->setParentIdentifier(parentID);

	//retrieve its parent
	IVisualizationWidget* parentWidget = getVisualizationWidget(parentID);
	if (!parentWidget)
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to parent visualization widget (couldn't find parent)\n";
		return false;
	}

	parentWidget->setChildIdentifier(index, id);

	return true;
}

CIdentifier CVisualizationTree::getUnusedIdentifier(const CIdentifier& suggestedID) const
{
	uint64_t id = (uint64_t(rand()) << 32) + uint64_t(rand());
	if (suggestedID != CIdentifier::undefined()) { id = suggestedID.id() - 1; }

	CIdentifier result;
	map<CIdentifier, IVisualizationWidget*>::const_iterator i;
	do
	{
		id++;
		result = CIdentifier(id);
		i      = m_widgets.find(result);
	} while (i != m_widgets.end() || result == CIdentifier::undefined());
	return result;
}

GtkTreeView* CVisualizationTree::createTreeViewWithModel() { return GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_treeStore))); }

bool CVisualizationTree::setTreeViewCB(ITreeViewCB* callback)
{
	m_treeViewCB = callback;
	return true;
}

bool CVisualizationTree::reloadTree()
{
	if (!m_treeViewCB) { return false; }

	//clear current tree
	GtkTreeIter iter;
	while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_treeStore), &iter, nullptr, 0) != FALSE) { gtk_tree_store_remove(m_treeStore, &iter); }
	//create 'unaffected display plugins' node
	gtk_tree_store_append(m_treeStore, &iter, nullptr);
	gtk_tree_store_set(m_treeStore, &iter,
					   EVisualizationTreeColumn::StringName, "Unaffected display plugins",
					   EVisualizationTreeColumn::StringStockIcon, m_treeViewCB->getTreeWidgetIcon(EVisualizationTreeNode::Unaffected),
					   EVisualizationTreeColumn::ULongNodeType, static_cast<unsigned long>(EVisualizationTreeNode::Unaffected),
					   EVisualizationTreeColumn::StringIdentifier, CIdentifier::undefined().str().c_str(), -1);

	//reload unaffected visualization boxes
	CIdentifier id = CIdentifier::undefined();
	while (getNextVisualizationWidgetIdentifier(id, EVisualizationWidget::Box))
	{
		IVisualizationWidget* widget = getVisualizationWidget(id);
		//load widget if it doesn't have a parent (== is unaffected)
		if (widget->getParentIdentifier() == CIdentifier::undefined()) { loadVisualizationWidget(widget, &iter); }
	}

	//reload visualization windows
	id = CIdentifier::undefined();
	while (getNextVisualizationWidgetIdentifier(id, EVisualizationWidget::Window)) { loadVisualizationWidget(getVisualizationWidget(id), nullptr); }

	return true;
}

//Tree helper functions
//---------------------

bool CVisualizationTree::getTreeSelection(GtkTreeView* preeView, GtkTreeIter* iter)
{
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(preeView);
	GtkTreeModel* treeModel         = GTK_TREE_MODEL(m_treeStore);
	return gtk_tree_selection_get_selected(treeSelection, &treeModel, iter) != 0;
}

GtkTreePath* CVisualizationTree::getTreePath(GtkTreeIter* iter) const
{
	return (iter == nullptr) ? nullptr : gtk_tree_model_get_path(GTK_TREE_MODEL(m_treeStore), iter);
}

size_t CVisualizationTree::getULongValueFromTreeIter(GtkTreeIter* iter, const EVisualizationTreeColumn colType) const
{
	size_t value = 0;
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, colType, &value, -1);
	return value;
}

bool CVisualizationTree::getStringValueFromTreeIter(GtkTreeIter* iter, char*& string, const EVisualizationTreeColumn colType) const
{
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, colType, &string, -1);
	return true;
}

bool CVisualizationTree::getPointerValueFromTreeIter(GtkTreeIter* iter, void*& pointer, const EVisualizationTreeColumn colType) const
{
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, colType, &pointer, -1);
	return true;
}

bool CVisualizationTree::getIdentifierFromTreeIter(GtkTreeIter* iter, CIdentifier& id, const EVisualizationTreeColumn colType) const
{
	char* str = nullptr;
	getStringValueFromTreeIter(iter, str, colType);
	id.fromString(CString(str));
	return true;
}

//looks for a tree node named 'label' of class 'type' from tree root
bool CVisualizationTree::findChildNodeFromRoot(GtkTreeIter* iter, const char* label, const EVisualizationTreeNode type)
{
	if (!label) { return false; }

	//if tree is empty return false
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_treeStore), iter) == 0) { return false; }

	//look for node in the whole tree
	do
	{
		//look for node in current subtree
		if (findChildNodeFromParent(iter, label, type)) { return true; }

		//proceed with next top-level node
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(m_treeStore), iter) != 0);

	//node wasn't found
	return false;
}

//looks for a tree node named 'label' of class 'type' from parent passed as parameter
bool CVisualizationTree::findChildNodeFromParent(GtkTreeIter* iter, const char* label, const EVisualizationTreeNode type)
{
	if (findChildNodeFromParentR(iter, label, type))
	{
		*iter = m_internalTreeNode;
		return true;
	}
	return false;
}

//looks for a tree node named 'label' of class 'type' from parent passed as parameter
bool CVisualizationTree::findChildNodeFromParentR(GtkTreeIter* iter, const char* label, const EVisualizationTreeNode type)
{
	gchar* name;
	unsigned long typeAsInt;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, EVisualizationTreeColumn::StringName, &name, EVisualizationTreeColumn::ULongNodeType, &typeAsInt, -1);

	if (!name)
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Can not get values from the model" << "\n";
		return false;
	}

	if (strcmp(label, name) == 0 && type == EVisualizationTreeNode(typeAsInt))
	{
		m_internalTreeNode = *iter;
		return true;
	}

	//look among current node's children
	const int nChild = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_treeStore), iter);
	GtkTreeIter childIter;
	for (int i = 0; i < nChild; ++i)
	{
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_treeStore), &childIter, iter, i);

		if (findChildNodeFromParentR(&childIter, label, type)) { return true; }
	}

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromRoot(GtkTreeIter* iter, void* widget)
{
	// if tree is empty
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_treeStore), iter) == 0) { return false; }

	//look for node in the whole tree
	do
	{
		//look for node in current subtree
		if (findChildNodeFromParent(iter, widget)) { return true; }

		//proceed with next top-level node
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(m_treeStore), iter) != 0);

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromParent(GtkTreeIter* iter, void* widget)
{
	if (findChildNodeFromParentR(iter, widget))
	{
		*iter = m_internalTreeNode;
		return true;
	}
	return false;
}

bool CVisualizationTree::findChildNodeFromParentR(GtkTreeIter* iter, void* widget)
{
	void* currentWidget;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, EVisualizationTreeColumn::PointerWidget, &currentWidget, -1);
	if (widget == currentWidget)
	{
		m_internalTreeNode = *iter;
		return true;
	}

	//look among current node's children
	const int nChild = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_treeStore), iter);
	GtkTreeIter childIter;
	for (int i = 0; i < nChild; ++i)
	{
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_treeStore), &childIter, iter, i);

		if (findChildNodeFromParentR(&childIter, widget)) { return true; }
	}

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromRoot(GtkTreeIter* iter, const CIdentifier id)
{
	//if tree is empty return false
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_treeStore), iter) == 0) { return false; }

	//look for node in the whole tree
	do
	{
		//look for node in current subtree
		if (findChildNodeFromParent(iter, id)) { return true; }
		//proceed with next top-level node
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(m_treeStore), iter) != 0);

	//node wasn't found
	return false;
}

bool CVisualizationTree::findChildNodeFromParent(GtkTreeIter* iter, const CIdentifier id)
{
	if (findChildNodeFromParentR(iter, id))
	{
		*iter = m_internalTreeNode;
		return true;
	}
	return false;
}

bool CVisualizationTree::findChildNodeFromParentR(GtkTreeIter* iter, const CIdentifier& id)
{
	gchar* str;
	CIdentifier currentID;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, EVisualizationTreeColumn::StringIdentifier, &str, -1);
	currentID.fromString(CString(str));
	if (id == currentID)
	{
		m_internalTreeNode = *iter;
		return true;
	}

	//look among current node's children
	const int nChild = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_treeStore), iter);
	GtkTreeIter childIter;
	for (int i = 0; i < nChild; ++i)
	{
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_treeStore), &childIter, iter, i);
		if (findChildNodeFromParentR(&childIter, id)) { return true; }
	}

	//node wasn't found
	return false;
}

bool CVisualizationTree::findParentNode(GtkTreeIter* iter, const EVisualizationTreeNode type)
{
	unsigned long typeAsInt;
	GtkTreeIter currentIter;

	//is current node the one looked for?
	gtk_tree_model_get(GTK_TREE_MODEL(m_treeStore), iter, EVisualizationTreeColumn::ULongNodeType, &typeAsInt, -1);
	if (type == EVisualizationTreeNode(typeAsInt)) { return true; }
	//look one level higher
	if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(m_treeStore), &currentIter, iter) != 0)
	{
		*iter = currentIter;
		return findParentNode(iter, type);
	}
	//couldn't find desired parent node
	return false;
}

bool CVisualizationTree::dragDataReceivedOutsideWidgetCB(const CIdentifier& srcWidgetID, GtkWidget* dstWidget, const EDragLocation location)
{
	//retrieve source widget parent
	//-----------------------------
	IVisualizationWidget* srcWidget = getVisualizationWidget(srcWidgetID);
	if (!srcWidget) { return false; }

	//retrieve dest widget and dest widget parent identifiers
	//-------------------------------------------------------
	GtkTreeIter dstIt;
	if (!findChildNodeFromRoot(&dstIt, m_treeViewCB->getTreeWidget(dstWidget))) { return false; }
	CIdentifier dstWidgetID;
	getIdentifierFromTreeIter(&dstIt, dstWidgetID, EVisualizationTreeColumn::StringIdentifier);
	IVisualizationWidget* dstVisualizationWidget = getVisualizationWidget(dstWidgetID);
	if (dstVisualizationWidget == nullptr) { return false; }
	// dst widget is the widget already present in
	const CIdentifier dstParentID = dstVisualizationWidget->getParentIdentifier();

	//unparent source widget
	size_t srcIdx = 0;
	unparentVisualizationWidget(srcWidgetID, srcIdx);

	//unparent dest widget
	size_t dstIdx = 0;
	unparentVisualizationWidget(dstWidgetID, dstIdx);

	//create paned widget
	const EVisualizationWidget panedType = (location == EDragLocation::Top || location == EDragLocation::Bottom) ? EVisualizationWidget::VerticalSplit
											   : EVisualizationWidget::HorizontalSplit;
	CIdentifier panedID;
	addVisualizationWidget(panedID, CString(panedType == EVisualizationWidget::VerticalSplit ? "Vertical split" : "Horizontal split"), panedType,
						   dstParentID,				//parent paned to dest widget parent
						   dstIdx,					//put it at the index occupied by dest widget
						   CIdentifier::undefined(),	//no box algorithm for a paned
						   2,						//2 children
						   CIdentifier::undefined());	//no prefered visualization identifier
	IVisualizationWidget* panedWidget = getVisualizationWidget(panedID);

	//add attributes
	if (m_treeViewCB != nullptr) { m_treeViewCB->createTreeWidget(panedWidget); }

	//reparent widgets
	const size_t newSrcIdx = (location == EDragLocation::Top || location == EDragLocation::Left) ? 0 : 1;
	parentVisualizationWidget(srcWidgetID, panedID, newSrcIdx);
	parentVisualizationWidget(dstWidgetID, panedID, 1 - newSrcIdx);

	//update Gtk tree
	reloadTree();

	return true;
}

bool CVisualizationTree::dragDataReceivedInWidgetCB(const CIdentifier& srcWidgetID, GtkWidget* dstWidget)
{
	//retrieve source widget parent
	IVisualizationWidget* srcWidget = getVisualizationWidget(srcWidgetID);
	OV_EXCEPTION_UNLESS_D(srcWidget, "Source visualization identifier does not exist in the tree", ErrorType::ResourceNotFound);

	const CIdentifier srcParentID = srcWidget->getParentIdentifier();

	//retrieve dest widget and dest widget parent identifiers
	GtkTreeIter dstIt;
	if (!findChildNodeFromRoot(&dstIt, m_treeViewCB->getTreeWidget(dstWidget))) { return false; }

	CIdentifier dstWidgetID;
	getIdentifierFromTreeIter(&dstIt, dstWidgetID, EVisualizationTreeColumn::StringIdentifier);
	IVisualizationWidget* dstVisualizationWidget = getVisualizationWidget(dstWidgetID);
	if (!dstVisualizationWidget) { return false; }

	const CIdentifier dstParentID = dstVisualizationWidget->getParentIdentifier();

	//unparent source widget
	size_t srcIdx;
	unparentVisualizationWidget(srcWidgetID, srcIdx);

	//destroy, unparent or reparent dest widget
	size_t dstIdx;

	//if source widget was unaffected
	if (srcParentID == CIdentifier::undefined())
	{
		//if dest widget was dummy, destroy it
		if (dstVisualizationWidget->getType() == EVisualizationWidget::Undefined)
		{
			getVisualizationWidgetIndex(dstWidgetID, dstIdx);
			destroyHierarchy(dstWidgetID, true);
		}
		else //dest widget becomes unaffected
		{
			unparentVisualizationWidget(dstWidgetID, dstIdx);
		}
	}
	else //source widget was affected
	{
		//unparent dest widget
		unparentVisualizationWidget(dstWidgetID, dstIdx);

		//reparent it to source widget parent
		parentVisualizationWidget(dstWidgetID, srcParentID, srcIdx);
	}

	//reparent source widget
	parentVisualizationWidget(srcWidgetID, dstParentID, dstIdx);

	//update Gtk tree
	reloadTree();

	return true;
}

bool CVisualizationTree::loadVisualizationWidget(IVisualizationWidget* widget, GtkTreeIter* parentIter)
{
	//create visualization widget
	//---------------------------
	GtkWidget* tmp = m_treeViewCB->loadTreeWidget(widget);

	//add visualization widget node to tree store
	GtkTreeIter iter;
	gtk_tree_store_append(m_treeStore, &iter, parentIter);

	//retrieve values of tree node fields
	EVisualizationTreeNode childType;
	switch (widget->getType())
	{
		case EVisualizationWidget::Window: childType = EVisualizationTreeNode::VisualizationWindow;
			break;
		case EVisualizationWidget::Panel: childType = EVisualizationTreeNode::VisualizationPanel;
			break;
		case EVisualizationWidget::Box: childType = EVisualizationTreeNode::VisualizationBox;
			break;
		case EVisualizationWidget::HorizontalSplit: childType = EVisualizationTreeNode::HorizontalSplit;
			break;
		case EVisualizationWidget::VerticalSplit: childType = EVisualizationTreeNode::VerticalSplit;
			break;
		case EVisualizationWidget::Undefined: childType = EVisualizationTreeNode::Undefined;
			break;
		default: childType = EVisualizationTreeNode::Undefined;
			break;
	}

	CString stockIconString = m_treeViewCB->getTreeWidgetIcon(childType);

	if (widget->getType() == EVisualizationWidget::Box)
	{
		const IBox* box = m_scenario->getBoxDetails(widget->getBoxIdentifier());
		if (!box)
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Box with identifier " << widget->getBoxIdentifier() <<
					" not found in the scenario" << "\n";
			return false;
		}
		const IBoxAlgorithmDesc* boxDesc = dynamic_cast<const IBoxAlgorithmDesc*>(m_kernelCtx
																				  .getPluginManager().getPluginObjectDescCreating(
																					  box->getAlgorithmClassIdentifier()));
		if (boxDesc) { stockIconString = boxDesc->getStockItemName(); }
	}

	//set tree node fields
	gtk_tree_store_set(m_treeStore, &iter,
					   EVisualizationTreeColumn::StringName, widget->getName().toASCIIString(),
					   EVisualizationTreeColumn::StringStockIcon, stockIconString.toASCIIString(),
					   EVisualizationTreeColumn::ULongNodeType, static_cast<unsigned long>(childType),
					   EVisualizationTreeColumn::StringIdentifier, widget->getIdentifier().str().c_str(),
					   EVisualizationTreeColumn::PointerWidget, tmp, -1);

	//load visualization widget hierarchy
	//-----------------------------------
	//create a dummy child for visualization panels if none exists
	if (widget->getType() == EVisualizationWidget::Panel)
	{
		CIdentifier id;
		widget->getChildIdentifier(0, id);
		if (id == CIdentifier::undefined())
		{
			addVisualizationWidget(id, "Empty", EVisualizationWidget::Undefined, widget->getIdentifier(), 0, CIdentifier::undefined(), 0, CIdentifier::undefined());
		}
	}

	for (size_t i = 0; i < widget->getNbChildren(); ++i)
	{
		CIdentifier id;
		widget->getChildIdentifier(i, id);

		loadVisualizationWidget(getVisualizationWidget(id), &iter);
	}

	//complete visualization widget loading now that its hierarchy is loaded
	m_treeViewCB->endLoadTreeWidget(widget);

	return true;
}

bool CVisualizationTree::setToolbar(const CIdentifier& boxID, GtkWidget* toolbar)
{
	if (m_treeViewCB != nullptr) { return m_treeViewCB->setToolbar(boxID, toolbar); }
	return false;
}

bool CVisualizationTree::setWidget(const CIdentifier& boxID, GtkWidget* widget)
{
	if (m_treeViewCB != nullptr) { return m_treeViewCB->setWidget(boxID, widget); }
	return false;
}


json::Object CVisualizationTree::serializeWidget(IVisualizationWidget& widget) const
{
	json::Object representation;

	representation["identifier"] = widget.getIdentifier().str().c_str();

	// visualization box name can be retrieved from corresponding IBox, so we can skip it for these
	if (widget.getType() != EVisualizationWidget::Box) { representation["name"] = widget.getName().toASCIIString(); }

	representation["type"]             = int(widget.getType());
	representation["parentIdentifier"] = widget.getParentIdentifier().str().c_str();

	// visualization widget index
	IVisualizationWidget* parentVisualizationWidget = this->getVisualizationWidget(widget.getParentIdentifier());
	if (parentVisualizationWidget)
	{
		size_t childIndex = 0;
		parentVisualizationWidget->getChildIndex(widget.getIdentifier(), childIndex);
		representation["index"] = int(childIndex);
	}

	representation["boxIdentifier"] = widget.getBoxIdentifier().str().c_str();
	representation["childCount"]    = int(widget.getNbChildren());

	if (widget.getType() == EVisualizationWidget::Window)
	{
		representation["width"]  = int(widget.getWidth());
		representation["height"] = int(widget.getHeight());
	}
	if (widget.getType() == EVisualizationWidget::HorizontalSplit || widget.getType() == EVisualizationWidget::VerticalSplit)
	{
		representation["dividerPosition"]    = widget.getDividerPosition();
		representation["maxDividerPosition"] = widget.getMaxDividerPosition();
	}

	return representation;
}

CString CVisualizationTree::serialize() const
{
	json::Array representation;

	std::vector<CIdentifier> widgetsToExport;

	CIdentifier widgetID;
	while (this->getNextVisualizationWidgetIdentifier(widgetID))
	{
		IVisualizationWidget* widget = this->getVisualizationWidget(widgetID);
		if (widget->getType() == EVisualizationWidget::Window || widget->getParentIdentifier() == CIdentifier::undefined())
		{
			widgetsToExport.push_back(widgetID);
		}
	}

	for (size_t i = 0; i < widgetsToExport.size(); ++i)
	{
		IVisualizationWidget* widget = this->getVisualizationWidget(widgetsToExport[i]);

		representation.push_back(this->serializeWidget(*widget));

		for (size_t j = 0; j < widget->getNbChildren(); ++j) { if (widget->getChildIdentifier(j, widgetID)) { widgetsToExport.push_back(widgetID); } }
	}

	return CString(Serialize(representation).c_str());
}

bool CVisualizationTree::deserialize(const CString& tree)
{
	// Empty this visualization tree
	auto widgetID = CIdentifier::undefined();
	while (this->getNextVisualizationWidgetIdentifier(widgetID) && widgetID != CIdentifier::undefined())
	{
		this->destroyHierarchy(widgetID, true);
		widgetID = CIdentifier::undefined();
	}


	json::Array representation = json::Deserialize(tree.toASCIIString());

	for (auto it = representation.begin(); it != representation.end(); ++it)
	{
		json::Value& jsonWidget = *it;

		widgetID.fromString(jsonWidget["identifier"].ToString().c_str());

		CIdentifier boxID;
		boxID.fromString(jsonWidget["boxIdentifier"].ToString().c_str());

		const EVisualizationWidget widgetType = EVisualizationWidget(jsonWidget["type"].ToInt());

		CString name;
		if (widgetType == EVisualizationWidget::Box)
		{
			const IBox* box = m_scenario->getBoxDetails(boxID);
			if (!box)
			{
				m_kernelCtx.getLogManager() << LogLevel_Error << "The box identifier [" << boxID << "] used in Window manager was not found in the scenario.\n";
				return false;
			}
			name = box->getName();
		}
		else { name = jsonWidget["name"].ToString().c_str(); }

		CIdentifier id, parentID;
		parentID.fromString(jsonWidget["parentIdentifier"].ToString().c_str());

		size_t index = 0;
		if (this->getVisualizationWidget(parentID)) { index = size_t(jsonWidget["index"].ToInt()); }
		const size_t nChild = size_t(jsonWidget["childCount"].ToInt());

		this->addVisualizationWidget(id, name, widgetType, parentID, index, boxID, nChild, widgetID);

		if (widgetID != id)
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Visualization widget [" << widgetID << "] for box [" << boxID << "] could not be imported.\n";
			return false;
		}

		IVisualizationWidget* widget = this->getVisualizationWidget(widgetID);

		if (widget)
		{
			if (widget->getType() == EVisualizationWidget::Window)
			{
				widget->setWidth(size_t(jsonWidget["width"].ToInt()));
				widget->setHeight(size_t(jsonWidget["height"].ToInt()));
			}
			if (widget->getType() == EVisualizationWidget::HorizontalSplit || widget->getType() == EVisualizationWidget::VerticalSplit)
			{
				widget->setDividerPosition(jsonWidget["dividerPosition"].ToInt());
				widget->setMaxDividerPosition(jsonWidget["maxDividerPosition"].ToInt());
			}
		}
	}

	return true;
}
