#pragma once

#include <visualization-toolkit/ovvizIVisualizationTree.h>

#include <map>
#include <gtk/gtk.h>

namespace json
{
	class Object;
}

namespace OpenViBEDesigner
{
	class CVisualizationTree final : public OpenViBEVisualizationToolkit::IVisualizationTree
	{
	public:
		CVisualizationTree(const OpenViBE::Kernel::IKernelContext& ctx) : m_kernelCtx(ctx) {}
		virtual ~CVisualizationTree();

		bool init(const OpenViBE::Kernel::IScenario* scenario);

		bool getNextVisualizationWidgetIdentifier(OpenViBE::CIdentifier& id) const;
		bool getNextVisualizationWidgetIdentifier(OpenViBE::CIdentifier& id, OpenViBEVisualizationToolkit::EVisualizationWidgetType type) const;
		bool isVisualizationWidget(const OpenViBE::CIdentifier& id) const;
		OpenViBEVisualizationToolkit::IVisualizationWidget* getVisualizationWidget(const OpenViBE::CIdentifier& id) const;
		OpenViBEVisualizationToolkit::IVisualizationWidget* getVisualizationWidgetFromBoxIdentifier(const OpenViBE::CIdentifier& boxID) const;
		bool addVisualizationWidget(OpenViBE::CIdentifier& id, const OpenViBE::CString& name, OpenViBEVisualizationToolkit::EVisualizationWidgetType type,
									const OpenViBE::CIdentifier& parentID, size_t parentIdx, const OpenViBE::CIdentifier& boxID, size_t nChild,
									const OpenViBE::CIdentifier& suggestedID);
		bool getVisualizationWidgetIndex(const OpenViBE::CIdentifier& id, size_t& index) const;
		bool unparentVisualizationWidget(const OpenViBE::CIdentifier& id, size_t& index);
		bool parentVisualizationWidget(const OpenViBE::CIdentifier& id, const OpenViBE::CIdentifier& parentID, const size_t index);
		bool destroyHierarchy(const OpenViBE::CIdentifier& id, bool destroyVisualizationBoxes);

		GtkTreeView* createTreeViewWithModel();
		bool setTreeViewCB(OpenViBEVisualizationToolkit::ITreeViewCB* callback);

		bool reloadTree();

		bool getTreeSelection(GtkTreeView* preeView, GtkTreeIter* iter);
		GtkTreePath* getTreePath(GtkTreeIter* iter) const;
		size_t getULongValueFromTreeIter(GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const;
		bool getStringValueFromTreeIter(GtkTreeIter* iter, char*& string, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const;
		bool getPointerValueFromTreeIter(GtkTreeIter* iter, void*& pointer, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const;
		bool getIdentifierFromTreeIter(GtkTreeIter* iter, OpenViBE::CIdentifier& id, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const;

		bool findChildNodeFromRoot(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type);
		bool findChildNodeFromParent(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type);
		bool findChildNodeFromRoot(GtkTreeIter* iter, void* widget);
		bool findChildNodeFromParent(GtkTreeIter* iter, void* widget);
		bool findChildNodeFromRoot(GtkTreeIter* iter, OpenViBE::CIdentifier id);
		bool findChildNodeFromParent(GtkTreeIter* iter, OpenViBE::CIdentifier id);
		bool findParentNode(GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualizationTreeNode type);

		bool dragDataReceivedInWidgetCB(const OpenViBE::CIdentifier& srcWidgetID, GtkWidget* dstWidget);
		bool dragDataReceivedOutsideWidgetCB(const OpenViBE::CIdentifier& srcWidgetID, GtkWidget* dstWidget,
											 OpenViBEVisualizationToolkit::EDragDataLocation location);

		bool setToolbar(const OpenViBE::CIdentifier& boxID, GtkWidget* toolbar);
		bool setWidget(const OpenViBE::CIdentifier& boxID, GtkWidget* widget);

		OpenViBE::CString serialize() const;
		bool deserialize(const OpenViBE::CString& tree);

	private:
		json::Object serializeWidget(OpenViBEVisualizationToolkit::IVisualizationWidget& widget) const;

		bool _destroyHierarchy(const OpenViBE::CIdentifier& id, bool destroy);
		OpenViBE::CIdentifier getUnusedIdentifier(const OpenViBE::CIdentifier& suggestedID) const;

		bool _findChildNodeFromParent(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type);
		bool _findChildNodeFromParent(GtkTreeIter* iter, void* widget);
		bool _findChildNodeFromParent(GtkTreeIter* iter, const OpenViBE::CIdentifier& id);

		bool loadVisualizationWidget(OpenViBEVisualizationToolkit::IVisualizationWidget* widget, GtkTreeIter* parentIter);

		std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualizationWidget*> m_widgets;
		OpenViBE::CIdentifier m_scenarioID = OV_UndefinedIdentifier;
		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		const OpenViBE::Kernel::IScenario* m_scenario = nullptr;
		GtkTreeStore* m_treeStore                     = nullptr;
		GtkTreeIter m_internalTreeNode;
		OpenViBEVisualizationToolkit::ITreeViewCB* m_treeViewCB = nullptr;
	};
}
