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
		~CVisualizationTree() override;

		bool init(const OpenViBE::Kernel::IScenario* scenario) override;

		bool getNextVisualizationWidgetIdentifier(OpenViBE::CIdentifier& id) const override;
		bool getNextVisualizationWidgetIdentifier(OpenViBE::CIdentifier& id, OpenViBEVisualizationToolkit::EVisualizationWidgetType type) const override;
		bool isVisualizationWidget(const OpenViBE::CIdentifier& id) const override;
		OpenViBEVisualizationToolkit::IVisualizationWidget* getVisualizationWidget(const OpenViBE::CIdentifier& id) const override;
		OpenViBEVisualizationToolkit::IVisualizationWidget* getVisualizationWidgetFromBoxIdentifier(const OpenViBE::CIdentifier& boxID) const override;
		bool addVisualizationWidget(OpenViBE::CIdentifier& id, const OpenViBE::CString& name, OpenViBEVisualizationToolkit::EVisualizationWidgetType type,
									const OpenViBE::CIdentifier& parentID, size_t parentIdx, const OpenViBE::CIdentifier& boxID, size_t nChild,
									const OpenViBE::CIdentifier& suggestedID) override;
		bool getVisualizationWidgetIndex(const OpenViBE::CIdentifier& id, size_t& index) const override;
		bool unparentVisualizationWidget(const OpenViBE::CIdentifier& id, size_t& index) override;
		bool parentVisualizationWidget(const OpenViBE::CIdentifier& id, const OpenViBE::CIdentifier& parentID, const size_t index) override;
		bool destroyHierarchy(const OpenViBE::CIdentifier& id, bool destroyVisualizationBoxes) override;

		GtkTreeView* createTreeViewWithModel() override;
		bool setTreeViewCB(OpenViBEVisualizationToolkit::ITreeViewCB* callback) override;

		bool reloadTree() override;

		bool getTreeSelection(GtkTreeView* preeView, GtkTreeIter* iter) override;
		GtkTreePath* getTreePath(GtkTreeIter* iter) const override;
		size_t getULongValueFromTreeIter(GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;
		bool getStringValueFromTreeIter(GtkTreeIter* iter, char*& string, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;
		bool getPointerValueFromTreeIter(GtkTreeIter* iter, void*& pointer, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;
		bool getIdentifierFromTreeIter(GtkTreeIter* iter, OpenViBE::CIdentifier& id,
									   OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;

		bool findChildNodeFromRoot(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type) override;
		bool findChildNodeFromParent(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type) override;
		bool findChildNodeFromRoot(GtkTreeIter* iter, void* widget) override;
		bool findChildNodeFromParent(GtkTreeIter* iter, void* widget) override;
		bool findChildNodeFromRoot(GtkTreeIter* iter, OpenViBE::CIdentifier id) override;
		bool findChildNodeFromParent(GtkTreeIter* iter, OpenViBE::CIdentifier id) override;
		bool findParentNode(GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualizationTreeNode type) override;

		bool dragDataReceivedInWidgetCB(const OpenViBE::CIdentifier& srcWidgetID, GtkWidget* dstWidget) override;
		bool dragDataReceivedOutsideWidgetCB(const OpenViBE::CIdentifier& srcWidgetID, GtkWidget* dstWidget,
											 OpenViBEVisualizationToolkit::EDragDataLocation location) override;

		bool setToolbar(const OpenViBE::CIdentifier& boxID, GtkWidget* toolbar) override;
		bool setWidget(const OpenViBE::CIdentifier& boxID, GtkWidget* widget) override;

		OpenViBE::CString serialize() const override;
		bool deserialize(const OpenViBE::CString& tree) override;

	private:
		json::Object serializeWidget(OpenViBEVisualizationToolkit::IVisualizationWidget& widget) const;

		bool destroyHierarchyR(const OpenViBE::CIdentifier& id, bool destroy);
		OpenViBE::CIdentifier getUnusedIdentifier(const OpenViBE::CIdentifier& suggestedID) const;

		bool findChildNodeFromParentR(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type);
		bool findChildNodeFromParentR(GtkTreeIter* iter, void* widget);
		bool findChildNodeFromParentR(GtkTreeIter* iter, const OpenViBE::CIdentifier& id);

		bool loadVisualizationWidget(OpenViBEVisualizationToolkit::IVisualizationWidget* widget, GtkTreeIter* parentIter);

		std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualizationWidget*> m_widgets;
		OpenViBE::CIdentifier m_scenarioID = OV_UndefinedIdentifier;
		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		const OpenViBE::Kernel::IScenario* m_scenario = nullptr;
		GtkTreeStore* m_treeStore                     = nullptr;
		GtkTreeIter m_internalTreeNode;
		OpenViBEVisualizationToolkit::ITreeViewCB* m_treeViewCB = nullptr;
	};
}  // namespace OpenViBEDesigner
