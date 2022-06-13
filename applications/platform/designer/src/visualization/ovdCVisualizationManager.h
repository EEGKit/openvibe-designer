#pragma once

#include <openvibe/ov_all.h>
#include <map>

#include <visualization-toolkit/ovvizIVisualizationManager.h>
#include <visualization-toolkit/ovvizIVisualizationTree.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBE {
namespace Designer {
class CVisualizationManager final : public VisualizationToolkit::IVisualizationManager
{
public:
	explicit CVisualizationManager(const Kernel::IKernelContext& ctx) : m_kernelCtx(ctx) {}
	~CVisualizationManager() override = default;

	bool createVisualizationTree(CIdentifier& treeID) override;
	bool releaseVisualizationTree(const CIdentifier& treeID) override;
	VisualizationToolkit::IVisualizationTree& getVisualizationTree(const CIdentifier& id) override;

	bool setToolbar(const CIdentifier& treeID, const CIdentifier& boxID, GtkWidget* toolbar) override;
	bool setWidget(const CIdentifier& treeID, const CIdentifier& boxID, GtkWidget* topmostWidget) override;

private:
	CIdentifier getUnusedIdentifier() const;

	/// Map of visualization trees (one per scenario, storing visualization widgets arrangement in space)
	std::map<CIdentifier, VisualizationToolkit::IVisualizationTree*> m_trees;
	const Kernel::IKernelContext& m_kernelCtx;
};
}  // namespace Designer
}  // namespace OpenViBE
