#pragma once

#include <visualization-toolkit/ovvtkIVisualizationTree.h>

#include <map>
#include <gtk/gtk.h>

namespace json
{
	class Object;
}
namespace OpenViBEDesigner
{
	class CVisualisationTree final : public OpenViBEVisualizationToolkit::IVisualisationTree
	{
	public:
		CVisualisationTree(const OpenViBE::Kernel::IKernelContext& kernelContext);
		virtual ~CVisualisationTree(void);

		bool init(const OpenViBE::Kernel::IScenario* scenario);

		bool getNextVisualisationWidgetIdentifier(OpenViBE::CIdentifier& identifier) const;
		bool getNextVisualisationWidgetIdentifier(OpenViBE::CIdentifier& identifier, OpenViBEVisualizationToolkit::EVisualisationWidgetType type) const;
		bool isVisualisationWidget(const OpenViBE::CIdentifier& identifier) const;
		OpenViBEVisualizationToolkit::IVisualisationWidget* getVisualisationWidget(const OpenViBE::CIdentifier& identifier) const;
		OpenViBEVisualizationToolkit::IVisualisationWidget* getVisualisationWidgetFromBoxIdentifier(const OpenViBE::CIdentifier& boxIdentifier) const;
		bool addVisualisationWidget(
		        OpenViBE::CIdentifier& identifier,
		        const OpenViBE::CString& name,
		        OpenViBEVisualizationToolkit::EVisualisationWidgetType type,
		        const OpenViBE::CIdentifier& parentIdentifier,
		        OpenViBE::uint32 parentIndex,
		        const OpenViBE::CIdentifier& boxIdentifier,
		        OpenViBE::uint32 childCount,
		        const OpenViBE::CIdentifier& suggestedIdentifier);
		bool getVisualisationWidgetIndex(const OpenViBE::CIdentifier& identifier, OpenViBE::uint32& index) const;
		bool unparentVisualisationWidget(const OpenViBE::CIdentifier& identifier, OpenViBE::uint32& index);
		bool parentVisualisationWidget(const OpenViBE::CIdentifier& identifier, const OpenViBE::CIdentifier& parentIdentifier, OpenViBE::uint32 index);
		bool destroyHierarchy(const OpenViBE::CIdentifier& identifier, bool destroyVisualisationBoxes);

		::GtkTreeView* createTreeViewWithModel(void);
		bool setTreeViewCB(OpenViBEVisualizationToolkit::ITreeViewCB* treeViewCB);

		bool reloadTree(void);

		bool getTreeSelection(::GtkTreeView* preeView, ::GtkTreeIter* iter);
		::GtkTreePath* getTreePath(::GtkTreeIter* treeIter) const;
		unsigned long getULongValueFromTreeIter(::GtkTreeIter* treeIter, OpenViBEVisualizationToolkit::EVisualisationTreeColumn visualisationTreeColumn) const;
		bool getStringValueFromTreeIter(::GtkTreeIter* treeIter, char*& string, OpenViBEVisualizationToolkit::EVisualisationTreeColumn visualisationTreeColumn) const;
		bool getPointerValueFromTreeIter(::GtkTreeIter* treeIter, void*& pointer, OpenViBEVisualizationToolkit::EVisualisationTreeColumn visualisationTreeColumn) const;
		bool getIdentifierFromTreeIter(::GtkTreeIter* iter, OpenViBE::CIdentifier& identifier, OpenViBEVisualizationToolkit::EVisualisationTreeColumn visualisationTreeColumn) const;

		bool findChildNodeFromRoot(::GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualisationTreeNode type);
		bool findChildNodeFromParent(::GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualisationTreeNode type);
		bool findChildNodeFromRoot(::GtkTreeIter* iter, void* widget);
		bool findChildNodeFromParent(::GtkTreeIter* iter, void* widget);
		bool findChildNodeFromRoot(::GtkTreeIter* iter, OpenViBE::CIdentifier identifier);
		bool findChildNodeFromParent(::GtkTreeIter* iter, OpenViBE::CIdentifier identifier);
		bool findParentNode(::GtkTreeIter* iter, OpenViBEVisualizationToolkit::EVisualisationTreeNode type);

		bool dragDataReceivedInWidgetCB(const OpenViBE::CIdentifier& sourceWidgetIdentifier, ::GtkWidget* destinationWidget);
		bool dragDataReceivedOutsideWidgetCB(const OpenViBE::CIdentifier& sourceWidgetIdentifier, ::GtkWidget* destinationWidget, OpenViBEVisualizationToolkit::EDragDataLocation location);

		bool setToolbar(const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* toolbarWidget);
		bool setWidget(const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget);

		OpenViBE::CString serialize() const;
		bool deserialize(const OpenViBE::CString& serializedVisualizationTree);

	private:
		json::Object serializeWidget(OpenViBEVisualizationToolkit::IVisualisationWidget& widget) const;

	private:

		bool _destroyHierarchy(const OpenViBE::CIdentifier& identifier, bool destroyVisualisationBoxes);
		OpenViBE::CIdentifier getUnusedIdentifier(const OpenViBE::CIdentifier& suggestedIdentifier) const;

		bool _findChildNodeFromParent(::GtkTreeIter* iter, const char* label, OpenViBEVisualizationToolkit::EVisualisationTreeNode type);
		bool _findChildNodeFromParent(::GtkTreeIter* iter, void* widget);
		bool _findChildNodeFromParent(::GtkTreeIter* iter, OpenViBE::CIdentifier identifier);

		bool loadVisualisationWidget(OpenViBEVisualizationToolkit::IVisualisationWidget* visualisationWidget, ::GtkTreeIter* parentIter);

	private:

		std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualisationWidget*> m_VisualisationWidgets;
		OpenViBE::CIdentifier m_ScenarioIdentifier;
		const OpenViBE::Kernel::IKernelContext& m_KernelContext;
		const OpenViBE::Kernel::IScenario* m_Scenario;
		::GtkTreeStore* m_TreeStore;
		::GtkTreeIter m_InternalTreeNode;
		OpenViBEVisualizationToolkit::ITreeViewCB* m_TreeViewCB;
	};
}

