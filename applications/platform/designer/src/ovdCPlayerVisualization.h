#pragma once

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
		CPlayerVisualization(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBEVisualizationToolkit::IVisualizationTree& rVisualizationTree, CInterfacedScenario& rInterfacedScenario);

		virtual ~CPlayerVisualization();

		void init();

		/** \name ITreeViewCB interface implementation */
		//@{
		GtkWidget* loadTreeWidget(OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget);
		void endLoadTreeWidget(OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget);
		bool setToolbar(const OpenViBE::CIdentifier& boxIdentifier, GtkWidget* pToolbarWidget);
		bool setWidget(const OpenViBE::CIdentifier& boxIdentifier, GtkWidget* widget);
		//@}

		void showTopLevelWindows();
		void hideTopLevelWindows();
		CInterfacedScenario& getInterfacedScenario() { return m_rInterfacedScenario; }

	protected:
		bool parentWidgetBox(OpenViBEVisualizationToolkit::IVisualizationWidget* widget, GtkBox* widgetBox);

		static gboolean configure_event_cb(GtkWidget* widget, GdkEventConfigure* event, gpointer data);
		static gboolean widget_expose_event_cb(GtkWidget* widget, GdkEventExpose* event, gpointer data);
		void resizeCB(GtkContainer* container);

		//callbacks for DND
		static void drag_data_get_from_widget_cb(GtkWidget* pSrcWidget, GdkDragContext* pDC, GtkSelectionData* pSelectionData, guint uiInfo, guint uiTime, gpointer data);
		static void drag_data_received_in_widget_cb(GtkWidget* pDstWidget, GdkDragContext*, gint, gint, GtkSelectionData* pSelectionData, guint, guint, gpointer data);

		//callback for toolbar
		static void toolbar_button_toggled_cb(GtkToggleButton* button, gpointer data);
		bool toggleToolbarCB(GtkToggleButton* button);
		static gboolean toolbar_delete_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data);
		bool deleteToolbarCB(GtkWidget* widget);

	private:

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBEVisualizationToolkit::IVisualizationTree& m_rVisualizationTree;
		CInterfacedScenario& m_rInterfacedScenario;

		/**
		 * \brief Vector of top level windows
		 */
		std::vector<GtkWindow*> m_vWindows;

		/**
		 * \brief Map of split (paned) widgets associated to their identifiers
		 * This map is used to retrieve size properties of split widgets upon window resizing,
		 * so as to keep the relative sizes of a hierarchy of widgets
		 */
		std::map<GtkPaned*, OpenViBE::CIdentifier> m_mSplitWidgets;

		/**
		 * \brief Map associating toolbar buttons to toolbar windows
		 */
		std::map<GtkToggleButton*, GtkWidget*> m_mToolbars;

		/**
		 * \brief Pointer to active toolbar button
		 */
		GtkToggleButton* m_pActiveToolbarButton = nullptr;

		class CPluginWidgets
		{
		public:
			CPluginWidgets() {}
			GtkWidget* m_widget = nullptr;
			GtkToggleButton* m_pToolbarButton = nullptr;
			GtkWidget* m_pToolbar = nullptr;
		};

		/**
		 * \brief Map of visualization plugins
		 */
		std::map<OpenViBE::CIdentifier, CPluginWidgets> m_mPlugins;
	};
};
