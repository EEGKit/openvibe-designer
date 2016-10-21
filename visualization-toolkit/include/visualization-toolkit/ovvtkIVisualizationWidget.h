#pragma once

#include <openvibe/ov_all.h>

namespace OpenViBEVisualizationToolkit
{
	/**
	 * \brief This enum lists the different types of IVisualisationWidget supported by the platform
	 */
	enum EVisualisationWidgetType
	{
		EVisualisationWidget_Undefined, /**< Undefined widget (empty slot in an IVisualisationTree) */
		EVisualisationWidget_VisualisationWindow, /**< Top-level IVisualisationWidget container */
		EVisualisationWidget_VisualisationPanel, /**< Notebook tab containing IVisualisationWidget objects */
		EVisualisationWidget_VisualisationBox, /**< Visualisation plugin */
		EVisualisationWidget_VerticalSplit, /**< Split widget that divides its client area vertically in two */
		EVisualisationWidget_HorizontalSplit /**< Split widget that divides its client area horizontally in two */
	};

	/**
	 * \class IVisualisationWidget
	 * \author Vincent Delannoy (INRIA/IRISA)
	 * \date 2007-11
	 * \brief Interface of visualisation widgets that are handled by an IVisualisationTree
	 * These objects are stored in an IVisualisationTree object as they are being created and modified
	 * to suit the graphical needs of a scenario.
	 */
	class IVisualisationWidget
	{
	public:
		virtual ~IVisualisationWidget() {}
		/**
		 * \brief Initializes the widget
		 * \param identifier identifier of the widget
		 * \param name name of the widget (optional)
		 * \param type type of the widget
		 * \param parentIdentifier parent widget identifier (OV_Undefined for top-level widgets)
		 * \param boxIdentifier if widget type is EVisualisationWidget_VisualisationBox, identifier of corresponding IBox
		 * \param childCount number of children of this widget (none for a visualisation box, 1 for a visualisation panel, 2 for split widgets, variable number for windows)
		 * \return True if widget was successfully initialized, false otherwise
		 */
		virtual OpenViBE::boolean initialize(
		        const OpenViBE::CIdentifier& identifier,
		        const OpenViBE::CString& name,
		        EVisualisationWidgetType type,
		        const OpenViBE::CIdentifier& parentIdentifier,
		        const OpenViBE::CIdentifier& boxIdentifier,
		        OpenViBE::uint32 childCount) = 0;

		/**
		 * \brief Returns the identifier of the widget
		 * \return Widget identifier
		 */
		virtual OpenViBE::CIdentifier getIdentifier(void) const = 0;

		/**
		 * \brief Returns the name of the widget
		 * \return Widget name
		 */
		virtual const OpenViBE::CString& getName(void) const = 0;

		/**
		 * \brief Sets the name of the widget
		 * \param name name to give to the widget
		 */
		virtual void setName(const OpenViBE::CString& name) = 0;

		/**
		 * \brief Returns the type of the widget
		 * \return Widget type
		 */
		virtual EVisualisationWidgetType getType(void) const = 0;

		/**
		 * \brief Returns the identifier of the widget's parent (if any)
		 * \return Widget's parent identifier if any, OV_Undefined otherwise
		 */
		virtual OpenViBE::CIdentifier getParentIdentifier(void) const = 0;
		/**
		 * \brief Sets the identifier of the widget's parent
		 * \param parentIdentifier identifier of the widget's parent
		 */
		virtual void setParentIdentifier(const OpenViBE::CIdentifier& parentIdentifier) = 0;

		/**
		 * \brief Returns the identifier of the IBox associated to this widget.
		 *
		 * This only applies to widgets of type EVisualisationWidget_VisualisationBox.
		 * \return Identifier of IBox associated to this widget
		 */
		virtual OpenViBE::CIdentifier getBoxIdentifier(void) const = 0;

		/**
		 * \brief Returns the number of children of this widget
		 * \return Number of child widgets
		 */
		virtual OpenViBE::uint32 getNbChildren(void) const = 0;

		/**
		 * \brief Returns the index of a given child
		 * \param identifier identifier of a child widget
		 * \param index [out] index at which the child widget is stored
		 * \return True if the child was found, false otherwise
		 */
		virtual OpenViBE::boolean getChildIndex(const OpenViBE::CIdentifier& identifier, OpenViBE::uint32& index) const = 0;

		/**
		 * \brief Adds a child to a widget
		 *
		 * Only useful for top-level widgets (EVisualisationWidget_VisualisationWindow) since the number
		 * of tabs their notebook may contain is unknown a priori. The child is added after existing children.
		 * \param childIdentifier identifier of child to be added to widget
		 * \return True if child was successfully added
		 */
		virtual OpenViBE::boolean addChild(const OpenViBE::CIdentifier& childIdentifier) = 0;

		/**
		 * \brief Removes a child from a widget
		 * \param childIdentifier identifier of child to be removed to the widget
		 * \return True if the child was successfully removed
		 */
		virtual OpenViBE::boolean removeChild(const OpenViBE::CIdentifier& childIdentifier) = 0;

		/**
		 * \brief Returns the identifier of a given child
		 * \param childIndex index of child whose identifier is to be retrieved
		 * \param childIdentifier [out] identifier of child
		 * \return True if child identifier was successfully returned, false otherwise
		 */
		virtual OpenViBE::boolean getChildIdentifier(OpenViBE::uint32 childIndex, OpenViBE::CIdentifier& childIdentifier) const = 0;

		/**
		 * \brief Sets the identifier of a child
		 * \param childIndex index of child whose identifier is to be set
		 * \param childIdentifier identifier of the child to be added to the widget
		 * \return True if the child was successfully set
		 */
		virtual OpenViBE::boolean setChildIdentifier(OpenViBE::uint32 childIndex, const OpenViBE::CIdentifier& childIdentifier) = 0;


		virtual void setWidth(unsigned int width) = 0;
		virtual void setHeight(unsigned int height) = 0;
		virtual unsigned int getWidth() = 0;
		virtual unsigned int getHeight() = 0;

		virtual void setDividerPosition(int dividerPosition) = 0;
		virtual void setMaxDividerPosition(int maxDividerPosition) = 0;
		virtual int getDividerPosition() = 0;
		virtual int getMaxDividerPosition() = 0;
	};
}

