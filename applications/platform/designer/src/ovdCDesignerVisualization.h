#ifndef __OpenViBEDesigner_CDesignerVisualization_H__
#define __OpenViBEDesigner_CDesignerVisualization_H__

#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <visualization-toolkit/ovvizIVisualizationTree.h>

namespace OpenViBEDesigner
{
	typedef void (*fpDesignerVisualizationDeleteEventCB)(gpointer user_data);

	class CInterfacedScenario;

	class CDesignerVisualization : public OpenViBEVisualizationToolkit::ITreeViewCB
	{
	public:
		CDesignerVisualization(
			const OpenViBE::Kernel::IKernelContext& rKernelContext,
			OpenViBEVisualizationToolkit::IVisualizationTree& rVisualizationTree,
			CInterfacedScenario& rInterfacedScenario);

		virtual ~CDesignerVisualization();

		void init(
			std::string guiFile);
		void load();
		void show();
		void hide();

		void setDeleteEventCB(fpDesignerVisualizationDeleteEventCB fpDeleteEventCB, gpointer user_data);

		void onVisualizationBoxAdded(
			const OpenViBE::Kernel::IBox* pBox);
		void onVisualizationBoxRemoved(
			const OpenViBE::CIdentifier& rBoxIdentifier);
		void onVisualizationBoxRenamed(
			const OpenViBE::CIdentifier& rBoxIdentifier);

		//ITreeViewCB callbacks overloading
		void createTreeWidget(
			OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget);
		GtkWidget* loadTreeWidget(
			OpenViBEVisualizationToolkit::IVisualizationWidget*);
		void endLoadTreeWidget(
			OpenViBEVisualizationToolkit::IVisualizationWidget*);
		GtkWidget* getTreeWidget(
			GtkWidget* visualizationWidget);
		GtkWidget* getVisualizationWidget(
			GtkWidget* treeWidget);
		const char* getTreeWidgetIcon(
			OpenViBEVisualizationToolkit::EVisualizationTreeNode);

		//callbacks for dialog
#ifdef HANDLE_MIN_MAX_EVENTS
		static gboolean window_state_event_cb(
			GtkWidget* widget,
			GdkEventWindowState* event,
      gpointer user_data);
#endif
		static gboolean configure_event_cb(
			GtkWidget* widget,
			GdkEventConfigure* event,
			gpointer user_data);
		static gboolean widget_expose_event_cb(
			GtkWidget* widget,
			GdkEventExpose* event,
			gpointer user_data);
		void resizeCB(
			OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget);

		static void notebook_page_switch_cb(
			GtkNotebook* notebook,
			GtkNotebookPage* page,
			guint pagenum,
			gpointer user_data);

		//callback for paned handle position changes
		static gboolean notify_position_paned_cb(
			GtkWidget* widget,
			GParamSpec* spec,
			gpointer user_data);

		static void ask_new_visualization_window_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);
		static void new_visualization_window_cb(
			GtkWidget* pWidget,
			gpointer pUserData);
		static void ask_rename_visualization_window_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);
		static void rename_visualization_window_cb(
			GtkWidget* pWidget,
			gpointer pUserData);
		static void remove_visualization_window_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);

		static void ask_new_visualization_panel_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);
		static void new_visualization_panel_cb(
			GtkWidget* pWidget,
			gpointer pUserData);
		static void ask_rename_visualization_panel_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);
		static void rename_visualization_panel_cb(
			GtkWidget* pWidget,
			gpointer pUserData);
		static void remove_visualization_panel_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);

		static void remove_visualization_widget_cb(
			gpointer pUserData,
			guint callback_action,
			GtkWidget* pWidget);

	private:
		static gboolean delete_event_cb(
			GtkWidget* widget,
      GdkEvent* event,
      gpointer user_data);
		OpenViBE::boolean deleteEventCB();

		void refreshActiveVisualization(
			GtkTreePath* pSelectedItemPath);

		void setActiveVisualization(
			const char* activeWindow,
			const char* activePanel);

		GtkTable* newWidgetsTable();
		void setupNewEventBoxTable(
			GtkBuilder* xml);

		//visualization windows
		void askNewVisualizationWindow(void);
	public:
		OpenViBE::boolean newVisualizationWindow(
			const char* label);
	private:
		void askRenameVisualizationWindow(void);
		OpenViBE::boolean renameVisualizationWindow(
			const char* label);
		OpenViBE::boolean	removeVisualizationWindow(void);

		//visualization panels
		void askNewVisualizationPanel(void);
		OpenViBE::boolean newVisualizationPanel(
			const char* label);
		void askRenameVisualizationPanel(void);
		OpenViBE::boolean renameVisualizationPanel(
			const char* label);
		OpenViBE::boolean removeVisualizationPanel(void);

		//visualization widgets
		OpenViBE::boolean removeVisualizationWidget(void);
		OpenViBE::boolean removeVisualizationWidget(
			const OpenViBE::CIdentifier& rIdentifier);
		OpenViBE::boolean destroyVisualizationWidget(
			const OpenViBE::CIdentifier& rIdentifier);

		void enableNotebookSignals(
			GtkWidget* pNotebook,
			OpenViBE::boolean b);
		void notebookPageSelectedCB(
			GtkNotebook* notebook,
			guint pagenum);

		virtual void enablePanedSignals(
			GtkWidget* pPaned,
			OpenViBE::boolean b);
		void notifyPositionPanedCB(
			GtkWidget* widget);

		//Mouse/Key event callbacks
		static void visualization_widget_key_press_event_cb(
			GtkWidget* pWidget,
			GdkEventKey* pEvent,
			gpointer pUserData);
		void visualizationWidgetKeyPressEventCB(
			GtkWidget* pWidget,
			GdkEventKey* pEventKey);
		static gboolean visualization_widget_enter_notify_event_cb(
			GtkWidget* pWidget,
      GdkEventCrossing* pEventCrossing,
      gpointer pUserData);
		void visualizationWidgetEnterNotifyEventCB(
			GtkWidget* pWidget,
			GdkEventCrossing* pEventCrossing);
		static gboolean visualization_widget_leave_notify_event_cb(
			GtkWidget* pWidget,
      GdkEventCrossing* pEventCrossing,
      gpointer pUserData);
		void visualizationWidgetLeaveNotifyEventCB(
			GtkWidget* pWidget,
			GdkEventCrossing* pEventCrossing);

		//Tree management callbacks
		static gboolean button_release_cb(
			GtkWidget* pWidget,
			GdkEventButton *pEvent,
			gpointer pUserData);
		static void cursor_changed_cb(
			GtkTreeView* pTreeView,
			gpointer pUserData);

		//Tree management methods
		void buttonReleaseCB(
			GtkWidget* pWidget,
			GdkEventButton *pEvent);
		void cursorChangedCB(
			GtkTreeView* pTreeView);

		//Drag methods
		static void drag_data_get_from_tree_cb(
			GtkWidget* pSrcWidget,
			GdkDragContext* pDragContex,
			GtkSelectionData* pSelectionData,
			guint uiInfo,
			guint uiT,
			gpointer pData);
		void dragDataGetFromTreeCB(
			GtkWidget* pSrcWidget,
			GtkSelectionData* pSelectionData);
		static void drag_data_get_from_widget_cb(
			GtkWidget* pSrcWidget,
			GdkDragContext* pDC,
			GtkSelectionData* pSelectionData,
			guint uiInfo,
			guint uiTime,
			gpointer pData);
		void dragDataGetFromWidgetCB(
			GtkWidget* pSrcWidget,
			GtkSelectionData* pSelectionData);

		//Drop methods
		static void drag_data_received_in_widget_cb(
			GtkWidget* pDstWidget,
			GdkDragContext*,
			gint,
			gint,
			GtkSelectionData* pSelectionData,
			guint,
			guint,
			gpointer pData);
		void dragDataReceivedInWidgetCB(
			GtkWidget* pDstWidget,
			GtkSelectionData* pSelectionData);
		static void drag_data_received_in_event_box_cb(
			GtkWidget* pDstWidget,
			GdkDragContext* pDC,
			gint iX,
			gint iY,
			GtkSelectionData* pSelectionData,
			guint uiInfo,
			guint uiTime,
			gpointer pData);
		void dragDataReceivedInEventBoxCB(
			GtkWidget* pDstWidget,
			GtkSelectionData* pSelectionData,
			OpenViBEVisualizationToolkit::EDragDataLocation oLocation);

	private:

		const OpenViBE::Kernel::IKernelContext&	m_rKernelContext;
		OpenViBEVisualizationToolkit::IVisualizationTree& m_rVisualizationTree;
		OpenViBEDesigner::CInterfacedScenario& m_rInterfacedScenario;
		fpDesignerVisualizationDeleteEventCB m_fpDeleteEventCB;
		gpointer m_pDeleteEventUserData;
		std::string m_sGuiFile;
		GtkTreeView* m_pTreeView;
		GtkWidget* m_pDialog;
		GtkWidget* m_pPane;
		//highlighted widget
		GtkWidget* m_pHighlightedWidget;
		//active items
		OpenViBE::CString m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName;
		OpenViBE::CIdentifier m_oActiveVisualizationBoxIdentifier;
		//preview window visibility flag
		OpenViBE::boolean m_bPreviewWindowVisible;
		OpenViBE::uint32 m_ui32PreviewWindowWidth;
		OpenViBE::uint32 m_ui32PreviewWindowHeight;
		//factories used to build contextual menus
		GtkItemFactory *m_pUnaffectedItemFactory, *m_pVisualizationWindowItemFactory, *m_pVisualizationPanelItemFactory;
		GtkItemFactory *m_pVisualizationBoxItemFactory, *m_pUndefinedItemFactory, *m_pSplitItemFactory;
		//strings identifying top, left, right and bottom event boxes
		std::string m_sTopEventBoxData, m_sLeftEventBoxData, m_sRightEventBoxData, m_sBottomEventBoxData;
	};
};

#endif // __OpenViBEDesigner_CDesignerVisualization_H__
