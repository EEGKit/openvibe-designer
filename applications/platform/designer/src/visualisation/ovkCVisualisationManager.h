#pragma once

#include <openvibe/ov_all.h>
#include <map>

#include <visualization-toolkit/ovvtkIVisualizationManager.h>
#include <visualization-toolkit/ovvtkIVisualizationTree.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBEDesigner
{
	class CVisualisationManager final : public OpenViBEVisualizationToolkit::IVisualizationManager
	{
	public:

		CVisualisationManager(const OpenViBE::Kernel::IKernelContext& kernelContext);

		~CVisualisationManager();

		bool createVisualizationTree(OpenViBE::CIdentifier& visualisationTreeIdentifier);
		bool releaseVisualizationTree(const OpenViBE::CIdentifier& visualisationTreeIdentifier);
		OpenViBEVisualizationToolkit::IVisualisationTree& getVisualizationTree(const OpenViBE::CIdentifier& visualisationTreeIdentifier);

		bool setToolbar(const OpenViBE::CIdentifier& visualisationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* toolbar);
		bool setWidget(const OpenViBE::CIdentifier& visualisationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget);

	private:

		OpenViBE::CIdentifier getUnusedIdentifier(void) const;

		/// Map of visualisation trees (one per scenario, storing visualisation widgets arrangement in space)
		std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualisationTree*> m_VisualizationTrees;
		const OpenViBE::Kernel::IKernelContext& m_KernelContext;
	};
}

