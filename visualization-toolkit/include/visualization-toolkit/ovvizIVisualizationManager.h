#pragma once

#include "ovvizIVisualizationTree.h"

#include <openvibe/ov_all.h>

typedef struct _GtkWidget GtkWidget;

namespace OpenViBEVisualizationToolkit
{
	/**
		 * \class IVisualizationManager
		 * \author Vincent Delannoy (INRIA/IRISA)
		 * \date 2007-11
		 * \brief The VisualizationManager handles IVisualizationTree objects
		 * It maintains a list of IVisualizationTree objects, each of which is associated to a scenario. This
		 * manager is used both at scenario creation time (to load or create IVisualizationTree objects), and when the
		 * scenario is being run (to forward IVisualizationWidget pointers to the related IVisualizationTree).
		 */
	class IVisualizationManager
	{
	public:

		/**
			 * \brief An interface used to iteratively notify its creator of the existence of IVisualizationTree objects
			 */
		class IVisualizationTreeEnum
		{
		public:
			virtual ~IVisualizationTreeEnum(void) { }
			/**
				 * \brief Callback method called iteratively as the IVisualizationManager goes through a set of IVisualizationTree objects
				 * \param visualizationTreeIdentifier identifier of an IVisualizationTree object
				 * \param visualizationTree corresponding IVisualizationTree object
				 * \return True if IVisualizationTree object was successfully registered, false otherwise
				 */
			virtual bool callback(const OpenViBE::CIdentifier& visualizationTreeIdentifier, const IVisualizationTree& visualizationTree) = 0;
		};

		/**
			 * \brief Creates an IVisualizationTree object.
			 * \param visualizationTreeIdentifier [out] identifier of the IVisualizationTree object created by this method
			 * \return True if object was successfully created, false otherwise
			 */
		virtual bool createVisualizationTree(OpenViBE::CIdentifier& visualizationTreeIdentifier) = 0;
		/**
			 * \brief Releases an IVisualizationTree object.
			 * \param visualizationTreeIdentifier identifier of the IVisualizationTree object to be released
			 * \return True if object was successfully released, false otherwise
			 */
		virtual bool releaseVisualizationTree(const OpenViBE::CIdentifier& visualizationTreeIdentifier) = 0;
		/**
			 * \brief Looks for an IVisualizationTree object.
			 * \param visualizationTreeIdentifier identifier of the IVisualizationTree object to be returned
			 * \return Reference on IVisualizationTree looked for, OV_Undefined otherwise
			 */
		virtual IVisualizationTree& getVisualizationTree(const OpenViBE::CIdentifier& visualizationTreeIdentifier) = 0;

		/**
			 * \brief Set the toolbar of a visualization plugin.
			 * This method is to be called by visualization plugins as they are being initialized. It lets them send
			 * a pointer to their toolbar (if they have one) to the scenario's IVisualizationTree.
			 * \param visualizationTreeIdentifier identifier of IVisualizationTree to which the toolbar pointer is to be forwarded
			 * \param rBoxIdentifier Identifier of IBox whose toolbar pointer is being set
			 * \param pToolbarWidget pointer to the toolbar of the widget
			 * \return True if pointer was successfully forwarded to IVisualizationTree, false otherwise
			 */
		virtual bool setToolbar(const OpenViBE::CIdentifier& visualizationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* toolbar) = 0;

		/**
		 * \brief Set the topmost widget of a visualization plugin.
		 * This method is to be called by visualization plugins as they are being initialized. It lets them send
		 * a pointer to their topmost widget to the scenario's IVisualizationTree.
		 * \param visualizationTreeIdentifier identifier of IVisualizationTree to which the toolbar pointer is to be forwarded
		 * \param rBoxIdentifier Identifier of IBox whose topmost widget pointer is being set
		 * \param pWidget pointer to the main window of the widget
		 * \return True if pointer was successfully forwarded to IVisualizationTree, false otherwise
		 */
		virtual bool setWidget(const OpenViBE::CIdentifier& visualizationTreeIdentifier, const OpenViBE::CIdentifier& boxIdentifier, ::GtkWidget* topmostWidget) = 0;

	};
};

