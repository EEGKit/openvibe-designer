#pragma once

#include "ovvtkIVisualizationTree.h"

#include <openvibe/ov_all.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBEVisualizationToolkit
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
	class IVisualizationManager
	{
	public:

		/**
			 * \brief An interface used to iteratively notify its creator of the existence of IVisualisationTree objects
			 */
		class IVisualizationTreeEnum
		{
		public:
			virtual ~IVisualizationTreeEnum(void) { }
			/**
				 * \brief Callback method called iteratively as the IVisualisationManager goes through a set of IVisualisationTree objects
				 * \param visualisationTreeIdentifier identifier of an IVisualisationTree object
				 * \param visualisationTree corresponding IVisualisationTree object
				 * \return True if IVisualisationTree object was successfully registered, false otherwise
				 */
			virtual bool callback(const OpenViBE::CIdentifier& visualisationTreeIdentifier, const IVisualisationTree& visualisationTree) = 0;
		};

		/**
			 * \brief Creates an IVisualisationTree object.
			 * \param visualisationTreeIdentifier [out] identifier of the IVisualisationTree object created by this method
			 * \return True if object was successfully created, false otherwise
			 */
		virtual bool createVisualizationTree(OpenViBE::CIdentifier& visualisationTreeIdentifier) = 0;
		/**
			 * \brief Releases an IVisualisationTree object.
			 * \param visualisationTreeIdentifier identifier of the IVisualisationTree object to be released
			 * \return True if object was successfully released, false otherwise
			 */
		virtual bool releaseVisualizationTree(const OpenViBE::CIdentifier& visualisationTreeIdentifier) = 0;
		/**
			 * \brief Looks for an IVisualisationTree object.
			 * \param visualisationTreeIdentifier identifier of the IVisualisationTree object to be returned
			 * \return Reference on IVisualisationTree looked for, OV_Undefined otherwise
			 */
		virtual IVisualisationTree& getVisualizationTree(const OpenViBE::CIdentifier& visualisationTreeIdentifier) = 0;

		/**
			 * \brief Set the toolbar of a visualisation plugin.
			 * This method is to be called by visualisation plugins as they are being initialized. It lets them send
			 * a pointer to their toolbar (if they have one) to the scenario's IVisualisationTree.
			 * \param visualisationTreeIdentifier identifier of IVisualisationTree to which the toolbar pointer is to be forwarded
			 * \param rBoxIdentifier Identifier of IBox whose toolbar pointer is being set
			 * \param pToolbarWidget pointer to the toolbar of the widget
			 * \return True if pointer was successfully forwarded to IVisualisationTree, false otherwise
			 */
		virtual bool setToolbar(const OpenViBE::CIdentifier& visualisationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* toolbar) = 0;

		/**
		 * \brief Set the topmost widget of a visualisation plugin.
		 * This method is to be called by visualisation plugins as they are being initialized. It lets them send
		 * a pointer to their topmost widget to the scenario's IVisualisationTree.
		 * \param visualisationTreeIdentifier identifier of IVisualisationTree to which the toolbar pointer is to be forwarded
		 * \param rBoxIdentifier Identifier of IBox whose topmost widget pointer is being set
		 * \param pWidget pointer to the main window of the widget
		 * \return True if pointer was successfully forwarded to IVisualisationTree, false otherwise
		 */
		virtual bool setWidget(const OpenViBE::CIdentifier& visualizationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget) = 0;

	};
};

