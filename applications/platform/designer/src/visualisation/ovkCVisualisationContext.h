#pragma once

#include "ovkIVisualisationManager.h"
#include "ovkIVisualisationContext.h"

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \class CVisualisationContext
		 * \author Vincent Delannoy (INRIA/IRISA)
		 * \date 2008-06
		 * \brief Visualisation context made available to plugins and allowing them to interact with a 3D scene
		 * This class offers a simplified, library independent 3D API to be used by plugin developers.
		 */
		class CVisualisationContext : public OpenViBE::Kernel::IVisualisationContext
		{
		public:
			/**
			 * \brief Constructor
			 */
			CVisualisationContext(const OpenViBE::Kernel::IKernelContext& rKernelContext,
			                      const OpenViBE::Kernel::IVisualisationManager& rVisualisationManager,
			                      OpenViBE::CIdentifier oVisualisationTreeIdentifier);

			/**
			 * \brief Destructor
			 */
			virtual ~CVisualisationContext(void);

			virtual bool setToolbar(
				::GtkWidget* pToolbarWidget);

			virtual bool setWidget(
				::GtkWidget* pTopmostWidget);

		protected:

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			OpenViBE::CIdentifier m_oVisualisationTreeIdentifier;
			const OpenViBE::Kernel::IVisualisationManager& m_rVisualisationManager;
		};
	};
};

