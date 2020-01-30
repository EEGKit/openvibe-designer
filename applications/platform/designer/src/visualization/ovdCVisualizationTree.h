#pragma once

#include <visualization-toolkit/ovvizIVisualizationTree.h>

#include <map>
#include <gtk/gtk.h>

namespace json
{
	class Object;
}

namespace OpenViBE
{
	namespace Designer
	{
	class CVisualizationTree final : public OpenViBEVisualizationToolkit::IVisualizationTree
	{
	public:
		CVisualizationTree(const Kernel::IKernelContext& ctx) : m_kernelCtx(ctx) {}
		~CVisualizationTree() override;

		bool init(const Kernel::IScenario* scenario) override;

		bool getNextVisualizationWidgetIdentifier(CIdentifier& id) const override;
		bool getNextVisualizationWidgetIdentifier(CIdentifier& id, OpenViBEVisualizationToolkit::EVisualizationWidgetType type) const override;
		bool isVisualizationWidget(const CIdentifier& id) const override;
		OpenViBEVisualizationToolkit::IVisualizationWidget* getVisualizationWidget(const CIdentifier& id) const override;
		OpenViBEVisualizationToolkit::IVisualizationWidget* getVisualizationWidgetFromBoxIdentifier(const CIdentifier& boxID) const override;
		bool addVisualizationWidget(CIdentifier& id, const CString& name, OpenViBEVisualizationToolkit::EVisualizationWidgetType type,
									const CIdentifier& parentID, size_t parentIdx, const CIdentifier& boxID, size_t nChild,
									const CIdentifier& suggestedID) override;
		bool getVisualizationWidgetIndex(const CIdentifier& id, size_t& index) const override;
		bool unparentVisualizationWidget(const CIdentifier& id, size_t& index) override;
		bool parentVisualizationWidget(const CIdentifier& id, const CIdentifier& parentID, const size_t index) override;
		bool destroyHierarchy(const CIdentifier& id, bool destroyVisualizationBoxes) override;

		GtkTreeView* createTreeViewWithModel() override;
		bool setTreeViewCB(OpenViBEVisualizationToolkit::ITreeViewCB* callback) override;

		bool reloadTree() override;

		bool getTreeSelection(GtkTreeView* preeView, GtkTreeIter* iter) override;
		GtkTreePath* getTreePath(GtkTreeIter* iter) const override;
		size_t getULongValueFromTreeIter(GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;
		bool getStringValueFromTreeIter(GtkTreeIter* iter, char*& string, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;
		bool getPointerValueFromTreeIter(GtkTreeIter* iter, void*& pointer, OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;
		bool getIdentifierFromTreeIter(GtkTreeIter* iter, CIdentifier& id,
									   OpenViBEVisualizationToolkit::EVisualizationTreeColumn colType) const override;

		bool findChildNodeFromRoot(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type) override;
		bool findChildNodeFromParent(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type) override;
		bool findChildNodeFromRoot(GtkTreeIter* iter, void* widget) override;
		bool findChildNodeFromParent(GtkTreeIter* iter, void* widget) override;
		bool findChildNodeFromRoot(GtkTreeIter* iter, CIdentifier id) override;
		bool findChildNodeFromParent(GtkTreeIter* iter, CIdentifier id) override;
		bool findParentNode(GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualizationTreeNode type) override;

		bool dragDataReceivedInWidgetCB(const CIdentifier& srcWidgetID, GtkWidget* dstWidget) override;
		bool dragDataReceivedOutsideWidgetCB(const CIdentifier& srcWidgetID, GtkWidget* dstWidget,
											 OpenViBEVisualizationToolkit::EDragDataLocation location) override;

		bool setToolbar(const CIdentifier& boxID, GtkWidget* toolbar) override;
		bool setWidget(const CIdentifier& boxID, GtkWidget* widget) override;

		CString serialize() const override;
		bool deserialize(const CString& tree) override;

	private:
		json::Object serializeWidget(OpenViBEVisualizationToolkit::IVisualizationWidget& widget) const;

		bool destroyHierarchyR(const CIdentifier& id, bool destroy);
		CIdentifier getUnusedIdentifier(const CIdentifier& suggestedID) const;

		bool findChildNodeFromParentR(GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualizationTreeNode type);
		bool findChildNodeFromParentR(GtkTreeIter* iter, void* widget);
		bool findChildNodeFromParentR(GtkTreeIter* iter, const CIdentifier& id);

		bool loadVisualizationWidget(OpenViBEVisualizationToolkit::IVisualizationWidget* widget, GtkTreeIter* parentIter);

		std::map<CIdentifier, OpenViBEVisualizationToolkit::IVisualizationWidget*> m_widgets;
		CIdentifier m_scenarioID = OV_UndefinedIdentifier;
		const Kernel::IKernelContext& m_kernelCtx;
		const Kernel::IScenario* m_scenario = nullptr;
		GtkTreeStore* m_treeStore                     = nullptr;
		GtkTreeIter m_internalTreeNode;
		OpenViBEVisualizationToolkit::ITreeViewCB* m_treeViewCB = nullptr;
	};
	}  // namespace Designer
}  // namespace OpenViBE
