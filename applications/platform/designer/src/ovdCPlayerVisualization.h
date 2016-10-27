#ifndef __OpenViBEDesigner_CPlayerVisualization_H__
#define __OpenViBEDesigner_CPlayerVisualization_H__

#include "ovd_base.h"

#include <string>
#include <vector>
#include <map>

namespace OpenViBEDesigner
{
	class CInterfacedScenario;
	class CPlayerVisualization : public OpenViBEVisualizationToolkit::ITreeViewCB
	{
	public:
		CPlayerVisualization(
			const OpenViBE::Kernel::IKernelContext& rKernelContext,
			OpenViBEVisualizationToolkit::IVisualizationTree& rVisualizationTree,
			CInterfacedScenario& rInterfacedScenario);

		virtual ~CPlayerVisualization();

		void init();

		/** \name ITreeViewCB interface implementation */
		//@{
		::GtkWidget* loadTreeWidget(
			OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget);
		void endLoadTreeWidget(
			OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget);
		OpenViBE::boolean setToolbar(
			const OpenViBE::CIdentifier& rBoxIdentifier,
			::GtkWidget* pToolbarWidget);
		OpenViBE::boolean setWidget(
			const OpenViBE::CIdentifier& rBoxIdentifier,
			::GtkWidget* pWidget);
		//@}

		void showTopLevelWindows();
		void hideTopLevelWindows();
		OpenViBEDesigner::CInterfacedScenario& getInterfacedScenario() { return m_rInterfacedScenario; }

	protected:
		OpenViBE::boolean parentWidgetBox(
			OpenViBEVisualizationToolkit::IVisualizationWidget* pWidget,
			::GtkBox* pWidgetBox);

		static gboolean configure_event_cb(
			::GtkWidget* widget,
			::GdkEventConfigure* event,
			gpointer user_data);
		static gboolean widget_expose_event_cb(
			::GtkWidget* widget,
			::GdkEventExpose* event,
			gpointer user_data);
		void resizeCB(
			::GtkContainer* container);

		//callbacks for DND
		static void	drag_data_get_from_widget_cb(
			::GtkWidget* pSrcWidget,
			::GdkDragContext* pDC,
			::GtkSelectionData* pSelectionData,
			guint uiInfo,
			guint uiTime,
			gpointer pData);
		static void	drag_data_received_in_widget_cb(
			::GtkWidget* pDstWidget,
			::GdkDragContext*,
			gint,
			gint,
			::GtkSelectionData* pSelectionData,
			guint,
			guint,
			gpointer pData);

		//callback for toolbar
		static void toolbar_button_toggled_cb(
			::GtkToggleButton* pButton,
			gpointer user_data);
		OpenViBE::boolean toggleToolbarCB(::GtkToggleButton* pButton);
		static gboolean toolbar_delete_event_cb(
			::GtkWidget *widget,
			::GdkEvent  *event,
      gpointer   user_data);
		OpenViBE::boolean deleteToolbarCB(::GtkWidget* pWidget);

	private:

		const OpenViBE::Kernel::IKernelContext&	m_rKernelContext;
		OpenViBEVisualizationToolkit::IVisualizationTree& m_rVisualizationTree;
		OpenViBEDesigner::CInterfacedScenario& m_rInterfacedScenario;

		/**
		 * \brief Vector of top level windows
		 */
		std::vector < ::GtkWindow* > m_vWindows;

		/**
		 * \brief Map of split (paned) widgets associated to their identifiers
		 * This map is used to retrieve size properties of split widgets upon window resizing,
		 * so as to keep the relative sizes of a hierarchy of widgets
		 */
		std::map < ::GtkPaned*, OpenViBE::CIdentifier > m_mSplitWidgets;

		/**
		 * \brief Map associating toolbar buttons to toolbar windows
		 */
		std::map < ::GtkToggleButton*, ::GtkWidget* > m_mToolbars;

		/**
		 * \brief Pointer to active toolbar button
		 */
		::GtkToggleButton* m_pActiveToolbarButton;

		class CPluginWidgets
		{
		public:
			CPluginWidgets() :
			m_pWidget(NULL), m_pToolbarButton(NULL),	m_pToolbar(NULL)
			{}
			::GtkWidget* m_pWidget;
			::GtkToggleButton* m_pToolbarButton;
			::GtkWidget* m_pToolbar;
		};

		/**
		 * \brief Map of visualization plugins
		 */
		std::map < OpenViBE::CIdentifier, CPlayerVisualization::CPluginWidgets > m_mPlugins;
	};
};

#endif // __OpenViBEDesigner_CPlayerVisualization_H__
