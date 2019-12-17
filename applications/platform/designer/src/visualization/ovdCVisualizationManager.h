#pragma once

#include <openvibe/ov_all.h>
#include <map>

#include <visualization-toolkit/ovvizIVisualizationManager.h>
#include <visualization-toolkit/ovvizIVisualizationTree.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBEDesigner
{
	class CVisualizationManager final : public OpenViBEVisualizationToolkit::IVisualizationManager
	{
	public:

		explicit CVisualizationManager(const OpenViBE::Kernel::IKernelContext& ctx) : m_kernelCtx(ctx) {}
		~CVisualizationManager() override = default;

		bool createVisualizationTree(OpenViBE::CIdentifier& treeID) override;
		bool releaseVisualizationTree(const OpenViBE::CIdentifier& treeID) override;
		OpenViBEVisualizationToolkit::IVisualizationTree& getVisualizationTree(const OpenViBE::CIdentifier& id) override;

		bool setToolbar(const OpenViBE::CIdentifier& treeID, const OpenViBE::CIdentifier& boxID, GtkWidget* toolbar) override;
		bool setWidget(const OpenViBE::CIdentifier& treeID, const OpenViBE::CIdentifier& boxID, GtkWidget* topmostWidget) override;

	private:

		OpenViBE::CIdentifier getUnusedIdentifier() const;

		/// Map of visualization trees (one per scenario, storing visualization widgets arrangement in space)
		std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualizationTree*> m_trees;
		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
	};
}
