#pragma once

#include <openvibe/ov_all.h>
#include <map>

#include <visualization-toolkit/ovvtkIVisualizationManager.h>
#include <visualization-toolkit/ovvtkIVisualizationTree.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBE
{
	namespace Kernel
	{
		class CVisualisationManager : public OpenViBEVisualizationToolkit::IVisualisationManager
		{
		public:

			CVisualisationManager(
				const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual ~CVisualisationManager();

			virtual bool createVisualisationTree(
				OpenViBE::CIdentifier& rVisualisationTreeIdentifier);
			virtual bool releaseVisualisationTree(
				const OpenViBE::CIdentifier& rVisualisationTreeIdentifier);
			virtual OpenViBEVisualizationToolkit::IVisualisationTree& getVisualisationTree(
				const OpenViBE::CIdentifier& rVisualisationTreeIdentifier);
			virtual bool enumerateVisualisationTrees(
				OpenViBEVisualizationToolkit::IVisualisationManager::IVisualisationTreeEnum& rCallBack) const;

			virtual bool setToolbar(
				const CIdentifier& rVisualisationTreeIdentifier,
				const CIdentifier& rBoxIdentifier,
				::GtkWidget* pToolbar);
			virtual bool setWidget(
				const CIdentifier& rVisualisationTreeIdentifier,
				const CIdentifier& rBoxIdentifier,
				::GtkWidget* pTopmostWidget);

		protected:

			virtual OpenViBE::CIdentifier getUnusedIdentifier(void) const;

		protected:

			/// Map of visualisation trees (one per scenario, storing visualisation widgets arrangement in space)
			std::map<OpenViBE::CIdentifier, OpenViBEVisualizationToolkit::IVisualisationTree*> m_vVisualisationTree;
			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		};
	}
}

