#pragma once

#include "ovkIVisualisationTree.h"

#include <openvibe/ov_all.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \class IVisualisationManager
		 * \author Vincent Delannoy (INRIA/IRISA)
		 * \date 2007-11
		 * \brief The VisualisationManager handles IVisualisationTree objects
		 * It maintains a list of IVisualisationTree objects, each of which is associated to a scenario. This
		 * manager is used both at scenario creation time (to load or create IVisualisationTree objects), and when the
		 * scenario is being run (to forward IVisualisationWidget pointers to the related IVisualisationTree).
		 */
		class IVisualisationManager
		{
		public:

			/**
			 * \brief An interface used to iteratively notify its creator of the existence of IVisualisationTree objects
			 */
			class IVisualisationTreeEnum
			{
			public:
				virtual ~IVisualisationTreeEnum(void) { }
				/**
				 * \brief Callback method called iteratively as the IVisualisationManager goes through a set of IVisualisationTree objects
				 * \param rVisualisationTreeIdentifier [in] identifier of an IVisualisationTree object
				 * \param rVisualisationTree [in] corresponding IVisualisationTree object
				 * \return True if IVisualisationTree object was successfully registered, false otherwise
				 */
				virtual OpenViBE::boolean callback(
					const OpenViBE::CIdentifier& rVisualisationTreeIdentifier,
					const OpenViBE::Kernel::IVisualisationTree& rVisualisationTree)=0;
			};

			/**
			 * \brief Creates an IVisualisationTree object.
			 * \param rVisualisationTreeIdentifier [out] identifier of the IVisualisationTree object created by this method
			 * \return True if object was successfully created, false otherwise
			 */
			virtual OpenViBE::boolean createVisualisationTree(
				OpenViBE::CIdentifier& rVisualisationTreeIdentifier)=0;
			/**
			 * \brief Releases an IVisualisationTree object.
			 * \param rVisualisationTreeIdentifier [in] identifier of the IVisualisationTree object to be released
			 * \return True if object was successfully released, false otherwise
			 */
			virtual OpenViBE::boolean releaseVisualisationTree(
				const OpenViBE::CIdentifier& rVisualisationTreeIdentifier)=0;
			/**
			 * \brief Looks for an IVisualisationTree object.
			 * \param rVisualisationTreeIdentifier [in] identifier of the IVisualisationTree object to be returned
			 * \return Reference on IVisualisationTree looked for, OV_Undefined otherwise
			 */
			virtual OpenViBE::Kernel::IVisualisationTree& getVisualisationTree(
				const OpenViBE::CIdentifier& rVisualisationTreeIdentifier)=0;
			/**
			 * \brief Enumerates IVisualisationTree objects registered in this manager.
			 * \param rCallback [in] IVisualisationTreeEnum object to be notified for each IVisualisationTree registered in this manager.
			 * \return True if IVisualisationTree objects were successfully enumerated, false otherwise
			 */
			virtual OpenViBE::boolean enumerateVisualisationTrees(
				OpenViBE::Kernel::IVisualisationManager::IVisualisationTreeEnum& rCallBack) const=0;

			/**
			 * \brief Set the toolbar of a visualisation plugin.
			 * This method is to be called by visualisation plugins as they are being initialized. It lets them send
			 * a pointer to their toolbar (if they have one) to the scenario's IVisualisationTree.
			 * \param rVisualisationTreeIdentifier [in] identifier of IVisualisationTree to which the toolbar pointer is to be forwarded
			 * \param rBoxIdentifier [in] Identifier of IBox whose toolbar pointer is being set
			 * \param pToolbarWidget [in] pointer to the toolbar of the widget
			 * \return True if pointer was successfully forwarded to IVisualisationTree, false otherwise
			 */
			virtual OpenViBE::boolean setToolbar(
				const CIdentifier& rVisualisationTreeIdentifier,
				const CIdentifier& rBoxIdentifier,
				::GtkWidget* pToolbar)=0;

			/**
			 * \brief Set the topmost widget of a visualisation plugin.
			 * This method is to be called by visualisation plugins as they are being initialized. It lets them send
			 * a pointer to their topmost widget to the scenario's IVisualisationTree.
			 * \param rVisualisationTreeIdentifier [in] identifier of IVisualisationTree to which the toolbar pointer is to be forwarded
			 * \param rBoxIdentifier [in] Identifier of IBox whose topmost widget pointer is being set
			 * \param pWidget [in] pointer to the main window of the widget
			 * \return True if pointer was successfully forwarded to IVisualisationTree, false otherwise
			 */
			virtual OpenViBE::boolean setWidget(
				const CIdentifier& rVisualisationTreeIdentifier,
				const CIdentifier& rBoxIdentifier,
				::GtkWidget* pTopmostWidget)=0;

		};
	};
};

