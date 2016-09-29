#pragma once

#include <openvibe/ov_all.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \class IVisualisationContext
		 * \author Vincent Delannoy (INRIA/IRISA)
		 * \date 2007-11
		 * \brief Visualisation manager interface for plugin objects
		 */
		class IVisualisationContext
		{
		public:

			/**
			 * \brief Set toolbar widget used by a plugin
			 * \param pToolbarWidget Pointer to toolbar widget
			 * \return True if widget was successfully set, false otherwise
			 */
			virtual OpenViBE::boolean setToolbar(
				::GtkWidget* pToolbarWidget)=0;

			/**
			 * \brief Set topmost widget used by a plugin
			 * \param pTopmostWidget Pointer to topmost widget
			 * \return True if widget was successfully set, false otherwise
			 */
			virtual OpenViBE::boolean setWidget(
				::GtkWidget* pTopmostWidget)=0;

		};
	};
};
