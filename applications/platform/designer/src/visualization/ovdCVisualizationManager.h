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

		CVisualizationManager(const OpenViBE::Kernel::IKernelContext& kernelContext);

		~CVisualizationManager();

		bool createVisualizationTree(OpenViBE::CIdentifier& visualizationTreeIdentifier);
		bool releaseVisualizationTree(const OpenViBE::CIdentifier& visualizationTreeIdentifier);
		OpenViBEVisualizationToolkit::IVisualizationTree& getVisualizationTree(const OpenViBE::CIdentifier& visualizationTreeIdentifier);

		bool setToolbar(const OpenViBE::CIdentifier& visualizationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, GtkWidget* toolbar);
		bool setWidget(const OpenViBE::CIdentifier& visualizationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, GtkWidget* topmostWidget);

	private:

		OpenViBE::CIdentifier getUnusedIdentifier() const;

		/// Map of visualization trees (one per scenario, storing visualization widgets arrangement in space)
		std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualizationTree*> m_VisualizationTrees;
		const OpenViBE::Kernel::IKernelContext& m_KernelContext;
	};
}
