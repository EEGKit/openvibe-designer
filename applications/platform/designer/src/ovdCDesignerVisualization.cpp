#include "ovd_base.h"
#include "ovdTAttributeHandler.h"
#include "ovdCApplication.h"
#include "ovdCDesignerVisualization.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCInputDialog.h"

#include <gdk/gdkkeysyms.h>
#include <cstring>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEDesigner;
using namespace OpenViBEVisualizationToolkit;

static ::GtkTargetEntry targets [] =
{
	{ (gchar*)"STRING", 0, 0 },
	{ (gchar*)"text/plain", 0, 0 },
};

namespace OpenViBEDesigner
{
	/**
	 * \brief Display an error dialog
	 * \param[in] pText text to display in the dialog
	 * \param[in] pSecondaryText additional text to display in the dialog
	 */
	void displayErrorDialog(const char* pText, const char* pSecondaryText)
	{
		::GtkWidget* l_pErrorDialog = gtk_message_dialog_new(
						NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_OK,
						"%s",
						pText);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pErrorDialog), "%s", pSecondaryText);

		gtk_window_set_position(GTK_WINDOW(l_pErrorDialog), GTK_WIN_POS_MOUSE);

		gtk_dialog_run(GTK_DIALOG(l_pErrorDialog));

		gtk_widget_destroy(l_pErrorDialog);
	}

	/**
	 * \brief Display a yes/no question dialog
	 * \param[in] pText text to display in the dialog
	 * \param[in] pSecondaryText additional text to display in the dialog
	 * \return identifier of the button pressed
	 */
	/*
	gint displayQuestionDialog(const char* pText, const char* pSecondaryText)
	{
		::GtkWidget* l_pQuestionDialog = gtk_message_dialog_new(
						NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						pText);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pQuestionDialog),	pSecondaryText);

		gtk_window_set_position(GTK_WINDOW(l_pQuestionDialog), GTK_WIN_POS_MOUSE);

		gint ret = gtk_dialog_run(GTK_DIALOG(l_pQuestionDialog));

		gtk_widget_destroy(l_pQuestionDialog);

		return ret;
	}*/

	/**
	 * \brief Helper function retrieving a child in a table from its attach indices
	 * \param pTable Table parent to the child to be retrieved
	 * \param left_attach Left attach index
	 * \param right_attach Right attach index
	 * \param top_attach Top attach index
	 * \param bottom_attach Bottom attach index
	 * \return Pointer to table child if one was found, NULL otherwise
	 */
	::GtkTableChild* getTableChild(::GtkTable* pTable, int leftAttach, int rightAttach, int topAttach, int bottomAttach)
	{
		GList* pList = pTable->children;
		::GtkTableChild* pTC;

		do
		{
			pTC = (::GtkTableChild*)pList->data;
			if(pTC->left_attach == leftAttach && pTC->right_attach == rightAttach &&
			   pTC->top_attach == topAttach && pTC->bottom_attach == bottomAttach)
			{
				return pTC;
			}
			pList = pList->next;
		}while(pList);

		return NULL;
	}
};

//Menus
//-----

static ::GtkItemFactoryEntry unaffected_menu_items[] = {
	{ (gchar*)"/New window", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::ask_new_visualization_window_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DND_MULTIPLE }
};

static ::GtkItemFactoryEntry visualization_window_menu_items[] ={
	{ (gchar*)"/New tab", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::ask_new_visualization_panel_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DND },
	{ (gchar*)"/Rename", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::ask_rename_visualization_window_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_BOLD },
	{ (gchar*)"/Remove", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::remove_visualization_window_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DELETE }
};

static ::GtkItemFactoryEntry visualization_panel_menu_items[] = {
	{ (gchar*)"/Rename", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::ask_rename_visualization_panel_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_BOLD },
	{ (gchar*)"/Remove", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::remove_visualization_panel_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DELETE }
};

static ::GtkItemFactoryEntry visualization_box_menu_items[] = {
	{ (gchar*)"/Remove", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::remove_visualization_widget_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DELETE }
};

static ::GtkItemFactoryEntry undefined_widget_menu_items[] = {
	{ (gchar*)"/Remove", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::remove_visualization_widget_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DELETE }
};

static ::GtkItemFactoryEntry split_widget_menu_items[] = {
	{ (gchar*)"/Remove", (gchar*)"", (::GtkItemFactoryCallback)CDesignerVisualization::remove_visualization_widget_cb, 1, (gchar*)"<StockItem>", GTK_STOCK_DELETE }
};

static gint num_unaffected_menu_items           = sizeof (unaffected_menu_items) / sizeof (unaffected_menu_items[0]);
static gint num_visualization_window_menu_items = sizeof (visualization_window_menu_items) / sizeof (visualization_window_menu_items[0]);
static gint num_visualization_panel_menu_items  = sizeof (visualization_panel_menu_items) / sizeof (visualization_panel_menu_items[0]);
static gint num_visualization_box_menu_items    = sizeof (visualization_box_menu_items) / sizeof (visualization_box_menu_items[0]);
static gint num_undefined_widget_menu_items     = sizeof (undefined_widget_menu_items) / sizeof (undefined_widget_menu_items[0]);
static gint num_split_widget_menu_items         = sizeof (split_widget_menu_items) / sizeof (split_widget_menu_items[0]);

CDesignerVisualization::CDesignerVisualization(const IKernelContext& rKernelContext, IVisualizationTree& rVisualizationTree, CInterfacedScenario& rInterfacedScenario) :
	m_rKernelContext(rKernelContext),
	m_rVisualizationTree(rVisualizationTree),
	m_rInterfacedScenario(rInterfacedScenario),
	m_fpDeleteEventCB(NULL),
	m_pDeleteEventUserData(NULL),
	m_pTreeView(NULL),
	m_pDialog(NULL),
	m_pPane(NULL),
	m_pHighlightedWidget(NULL),
	m_bPreviewWindowVisible(false),
	m_ui32PreviewWindowWidth(0),
	m_ui32PreviewWindowHeight(0),
	m_pUnaffectedItemFactory(NULL),
	m_pVisualizationWindowItemFactory(NULL),
	m_pVisualizationPanelItemFactory(NULL),
	m_pVisualizationBoxItemFactory(NULL),
	m_pUndefinedItemFactory(NULL),
	m_pSplitItemFactory(NULL)
{
}

CDesignerVisualization::~CDesignerVisualization()
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_pDialog), G_CALLBACK2(configure_event_cb), this);
#ifdef HANDLE_MIN_MAX_EVENTS
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_pDialog), G_CALLBACK2(window_state_event_cb), this);
#endif
	gtk_widget_destroy(m_pDialog);

	m_rVisualizationTree.setTreeViewCB(NULL);
}

void CDesignerVisualization::init(std::string guiFile)
{
	m_sGuiFile = guiFile;

	//create tree view
	//----------------

	//register towards tree store
	m_rVisualizationTree.setTreeViewCB(this);

	m_pTreeView = m_rVisualizationTree.createTreeViewWithModel();

	::GtkTreeViewColumn* l_pTreeViewColumnName=gtk_tree_view_column_new();
	::GtkCellRenderer* l_pCellRendererIcon=gtk_cell_renderer_pixbuf_new();
	::GtkCellRenderer* l_pCellRendererName=gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Windows for current scenario");
	gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", EVisualizationTreeColumn_StringStockIcon, NULL);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", EVisualizationTreeColumn_StringName, NULL);
	//gtk_tree_view_column_set_sizing(l_pTreeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnName, TRUE/*FALSE*/);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnName, TRUE);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnName, 64);
	gtk_tree_view_append_column(m_pTreeView, l_pTreeViewColumnName);

	::GtkTreeViewColumn* l_pTreeViewColumnDesc=gtk_tree_view_column_new();
	gtk_tree_view_append_column(m_pTreeView, l_pTreeViewColumnDesc);

	gtk_tree_view_column_set_visible(l_pTreeViewColumnDesc, 0);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(m_pTreeView));

	//allow tree items to be dragged
	gtk_drag_source_set(GTK_WIDGET(m_pTreeView), GDK_BUTTON1_MASK, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);

	//require notifications upon tree item dragging, mouse button release, active item change
	g_signal_connect(G_OBJECT(m_pTreeView), "drag_data_get", G_CALLBACK(drag_data_get_from_tree_cb), this);
	g_signal_connect(G_OBJECT(m_pTreeView), "button-release-event", G_CALLBACK(button_release_cb), this);
	g_signal_connect(G_OBJECT(m_pTreeView), "cursor-changed", G_CALLBACK(cursor_changed_cb), this);

	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(m_pTreeView), GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(m_pTreeView), "key-press-event", G_CALLBACK(visualization_widget_key_press_event_cb), this);

	//create main dialog
	//------------------
	m_pDialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	//retrieve default window size
	uint32 l_ui32TreeViewWidth = 200;
	m_ui32PreviewWindowWidth = (uint32)m_rKernelContext.getConfigurationManager().expandAsUInteger("${Designer_UnaffectedVisualizationWindowWidth}", 400);
	m_ui32PreviewWindowHeight = (uint32)m_rKernelContext.getConfigurationManager().expandAsUInteger("${Designer_UnaffectedVisualizationWindowHeight}", 400);
	CIdentifier l_oVisualizationWindowIdentifier;
	//if at least one window was created, retrieve its dimensions
	if(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
	{
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);
		m_ui32PreviewWindowWidth = l_pVisualizationWindow->getWidth();
		m_ui32PreviewWindowHeight = l_pVisualizationWindow->getHeight();
		/* Change the way window sizes are stored in the widget
		TAttributeHandler l_oAttributeHandler(*l_pVisualizationWindow);
		m_ui32PreviewWindowWidth = l_oAttributeHandler.getAttributeValue<int>(OVD_AttributeId_VisualizationWindow_Width);
		m_ui32PreviewWindowHeight = l_oAttributeHandler.getAttributeValue<int>(OVD_AttributeId_VisualizationWindow_Height);
		*/
	}
	gtk_window_set_default_size(GTK_WINDOW(m_pDialog), (gint)(l_ui32TreeViewWidth + m_ui32PreviewWindowWidth), (gint)m_ui32PreviewWindowHeight);
	//set window title
	gtk_window_set_title(GTK_WINDOW(m_pDialog), " Window Manager");
	// gtk_window_set_transient_for(GTK_WINDOW(m_pDialog), GTK_WINDOW(m_rInterfacedScenario.m_rApplication.m_pMainWindow));
	gtk_signal_connect(GTK_OBJECT(m_pDialog), "configure_event", G_CALLBACK(configure_event_cb), this);
#ifdef HANDLE_MIN_MAX_EVENTS
	gtk_signal_connect(GTK_OBJECT(m_pDialog), "window_state_event", G_CALLBACK(window_state_event_cb), this);
#endif
	g_signal_connect(G_OBJECT(m_pDialog), "delete-event", G_CALLBACK(delete_event_cb), this);

	//main pane : tree view to the left, widgets table to the right
	m_pPane = gtk_hpaned_new();
	gtk_container_add(GTK_CONTAINER(m_pDialog), GTK_WIDGET(m_pPane));

	// Add a scrollview to above the treeview

	GtkWidget* l_pScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l_pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(l_pScrolledWindow), GTK_WIDGET(m_pTreeView));

	//add tree view to pane
	gtk_paned_add1(GTK_PANED(m_pPane), GTK_WIDGET(l_pScrolledWindow));

	//set initial divider position
	gtk_paned_set_position(GTK_PANED(m_pPane), (gint)l_ui32TreeViewWidth);

	//create popup menus
	//------------------
	m_pUnaffectedItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<unaffected_main>", NULL);
	gtk_item_factory_create_items(m_pUnaffectedItemFactory, num_unaffected_menu_items, unaffected_menu_items, this);

	m_pVisualizationWindowItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<visualization_window_main>", NULL);
	gtk_item_factory_create_items(m_pVisualizationWindowItemFactory, num_visualization_window_menu_items, visualization_window_menu_items, this);

	m_pVisualizationPanelItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<visualization_panel_main>", NULL);
	gtk_item_factory_create_items(m_pVisualizationPanelItemFactory, num_visualization_panel_menu_items, visualization_panel_menu_items, this);

	m_pVisualizationBoxItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<visualization_box_main>", NULL);
	gtk_item_factory_create_items(m_pVisualizationBoxItemFactory, num_visualization_box_menu_items, visualization_box_menu_items, this);

	m_pUndefinedItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<undefined_widget_main>", NULL);
	gtk_item_factory_create_items(m_pUndefinedItemFactory, num_undefined_widget_menu_items, undefined_widget_menu_items, this);

	m_pSplitItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<split_widget_main>", NULL);
	gtk_item_factory_create_items(m_pSplitItemFactory, num_split_widget_menu_items, split_widget_menu_items, this);
}

void CDesignerVisualization::load(void)
{
	m_rVisualizationTree.setTreeViewCB(this);

	m_rVisualizationTree.reloadTree();

	gtk_tree_view_expand_all(m_pTreeView);

	setActiveVisualization(m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName);
}

void CDesignerVisualization::show()
{
	// since gtk is asynchronous for the expose event,
	// the m_bPreviewWindowVisible flag is turned on in the
	// corresponding callback
	//m_bPreviewWindowVisible = true;
	gtk_widget_show_all((::GtkWidget*)m_pDialog);
}

void CDesignerVisualization::hide()
{
	m_bPreviewWindowVisible = false;
	gtk_widget_hide_all((::GtkWidget*)m_pDialog);
}

void CDesignerVisualization::setDeleteEventCB(fpDesignerVisualizationDeleteEventCB fpDeleteEventCB, gpointer user_data)
{
	m_fpDeleteEventCB = fpDeleteEventCB;
	m_pDeleteEventUserData = user_data;
}

void CDesignerVisualization::onVisualizationBoxAdded(const IBox* pBox)
{
	CIdentifier l_oVisualizationWidgetIdentifier;
	m_rVisualizationTree.addVisualizationWidget(
		l_oVisualizationWidgetIdentifier,
		pBox->getName(),
		EVisualizationWidget_VisualizationBox,
		OV_UndefinedIdentifier,
		0,
		pBox->getIdentifier(),
		0,
		OV_UndefinedIdentifier);

	m_rVisualizationTree.reloadTree();

	//refresh view
	::GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oVisualizationWidgetIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));
}

void CDesignerVisualization::onVisualizationBoxRemoved(const CIdentifier& rBoxIdentifier)
{
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidgetFromBoxIdentifier(rBoxIdentifier);
	if(l_pVisualizationWidget != NULL)
	{
		//unaffected widget : delete it
		if(l_pVisualizationWidget->getParentIdentifier() == OV_UndefinedIdentifier)
		{
			m_rVisualizationTree.destroyHierarchy(l_pVisualizationWidget->getIdentifier());
		}
		else //simplify tree
		{
			destroyVisualizationWidget(l_pVisualizationWidget->getIdentifier());
		}

		m_rVisualizationTree.reloadTree();

		//refresh view
		refreshActiveVisualization(NULL);
	}
}

void CDesignerVisualization::onVisualizationBoxRenamed(const CIdentifier& rBoxIdentifier)
{
	//retrieve visualization widget
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidgetFromBoxIdentifier(rBoxIdentifier);
	if(l_pVisualizationWidget != NULL)
	{
		//retrieve box name
		const IBox* l_pBox = m_rInterfacedScenario.m_rScenario.getBoxDetails(rBoxIdentifier);
		if(l_pBox != NULL)
		{
			//set new visualization widget name
			l_pVisualizationWidget->setName(l_pBox->getName());

			//reload tree
			m_rVisualizationTree.reloadTree();

			//refresh view
			refreshActiveVisualization(NULL);
		}
	}
}

void CDesignerVisualization::createTreeWidget(IVisualizationWidget* pWidget)
{
	if(pWidget->getType() == EVisualizationWidget_HorizontalSplit || pWidget->getType() == EVisualizationWidget_VerticalSplit)
	{
		/* TODO_JL: Find a way to store divider position and max divider position
		TAttributeHandler l_oAttributeHandler(*pWidget);
		l_oAttributeHandler.addAttribute(OVD_AttributeId_VisualizationWidget_DividerPosition, 1);
		l_oAttributeHandler.addAttribute(OVD_AttributeId_VisualizationWidget_MaxDividerPosition, 2);
		*/
	}
}

//need width request of 0 to avoid graphical bugs (label/icon overlapping other widgets) when shrinking buttons
static gint s_labelWidthRequest = 0;
static gint s_iconWidthRequest = 0;
//need expand and fill flags to TRUE to see 0-size-requesting widgets
static gboolean s_labelExpand = TRUE;
static gboolean s_labelFill = TRUE;
static gboolean s_iconExpand = TRUE;
static gboolean s_iconFill = TRUE;

::GtkWidget* CDesignerVisualization::loadTreeWidget(IVisualizationWidget* pVisualizationWidget)
{
	::GtkWidget* l_pTreeWidget = NULL;

	//create widget
	//-------------
	if(pVisualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		//retrieve panel index
		IVisualizationWidget* l_pWindow = m_rVisualizationTree.getVisualizationWidget(pVisualizationWidget->getParentIdentifier());
		if(l_pWindow != NULL)
		{
			uint32 l_ui32PanelIndex;
			l_pWindow->getChildIndex(pVisualizationWidget->getIdentifier(), l_ui32PanelIndex);

			//create notebook if this is the first panel
			if(l_ui32PanelIndex == 0)
			{
				l_pTreeWidget = gtk_notebook_new();
			}
			else //otherwise retrieve it from first panel
			{
				CIdentifier l_oFirstPanelIdentifier;
				l_pWindow->getChildIdentifier(0, l_oFirstPanelIdentifier);
				GtkTreeIter l_oFirstPanelIter;
				m_rVisualizationTree.findChildNodeFromRoot(&l_oFirstPanelIter, l_oFirstPanelIdentifier);
				void* l_pNotebookWidget=NULL;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oFirstPanelIter, l_pNotebookWidget, EVisualizationTreeColumn_PointerWidget);
				l_pTreeWidget = (GtkWidget*)l_pNotebookWidget;
			}
		}
	}
	else if(pVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit ||
	        pVisualizationWidget->getType() == EVisualizationWidget_Undefined || pVisualizationWidget->getType() == EVisualizationWidget_VisualizationBox)
	{
		//tree widget = table containing event boxes + visualization widget in the center
		l_pTreeWidget = GTK_WIDGET(newWidgetsTable());
		::GtkWidget* l_pCurrentVisualizationWidget = getVisualizationWidget(l_pTreeWidget);
		if(l_pCurrentVisualizationWidget != NULL)
		{
			gtk_container_remove(GTK_CONTAINER(l_pTreeWidget), l_pCurrentVisualizationWidget);
		}

		if(pVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit)
		{
			if(gtk_widget_get_parent(l_pTreeWidget) != NULL)
			{
				gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pTreeWidget)), l_pTreeWidget);
			}

			//create a paned and insert it in table
			::GtkWidget* l_pPaned = (pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit) ? gtk_hpaned_new() : gtk_vpaned_new();
			gtk_table_attach(GTK_TABLE(l_pTreeWidget), l_pPaned, 1, 2, 1, 2,
				::GtkAttachOptions(GTK_EXPAND|GTK_SHRINK|GTK_FILL),
				::GtkAttachOptions(GTK_EXPAND|GTK_SHRINK|GTK_FILL), 0, 0);
		}
		else //undefined or visualization box : visualization widget is a GtkButton (left : icon, right : label)
		{
			if(gtk_widget_get_parent(l_pTreeWidget) != NULL)
			{
				gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pTreeWidget)), l_pTreeWidget);
			}

			//create a button and insert it in table
			::GtkWidget* l_pButton = gtk_button_new();
			gtk_widget_set_size_request(l_pButton, 0, 0);
			gtk_table_attach(GTK_TABLE(l_pTreeWidget), l_pButton, 1, 2, 1, 2,
				::GtkAttachOptions(GTK_EXPAND|GTK_SHRINK|GTK_FILL),
				::GtkAttachOptions(GTK_EXPAND|GTK_SHRINK|GTK_FILL), 0, 0);

			//box inserted in button
			::GtkBox* l_pBox = GTK_BOX(gtk_vbox_new(FALSE, 0));
			gtk_widget_set_size_request(GTK_WIDGET(l_pBox), 0, 0);

			//icon - actual icon will be loaded in endLoadTreeWidget
			::GtkWidget* l_pIcon = gtk_image_new_from_stock(getTreeWidgetIcon(EVisualizationTreeNode_Undefined), GTK_ICON_SIZE_BUTTON);
			if(s_iconWidthRequest == 0)
			{
				gtk_widget_set_size_request(l_pIcon, 0, 0);
			}
			gtk_box_pack_start(l_pBox, l_pIcon, s_iconExpand, s_iconFill, 0);

			//label
			::GtkWidget* l_pLabel = gtk_label_new((const char*)pVisualizationWidget->getName());
			if(s_labelWidthRequest == 0)
			{
				gtk_widget_set_size_request(l_pLabel, 0, 0);
			}
			gtk_box_pack_start(l_pBox, l_pLabel, s_labelExpand, s_labelFill, 0);

			//add box to button
			gtk_container_add(GTK_CONTAINER(l_pButton), GTK_WIDGET(l_pBox));

			//set up button as drag destination
			gtk_drag_dest_set(l_pButton, GTK_DEST_DEFAULT_ALL, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
			g_signal_connect(G_OBJECT(l_pButton), "drag_data_received", G_CALLBACK(drag_data_received_in_widget_cb), this);

			//set up button as drag source as well
			gtk_drag_source_set(l_pButton, GDK_BUTTON1_MASK, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
			g_signal_connect(G_OBJECT(l_pButton), "drag_data_get", G_CALLBACK(drag_data_get_from_widget_cb), this);

			//ask for notification of some events
			if(pVisualizationWidget->getType() == EVisualizationWidget_VisualizationBox)
			{
				GTK_WIDGET_SET_FLAGS(l_pButton, GDK_KEY_PRESS_MASK|GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK);
				g_signal_connect(G_OBJECT(l_pButton), "key-press-event", G_CALLBACK(visualization_widget_key_press_event_cb), this);
				g_signal_connect(G_OBJECT(l_pButton), "enter-notify-event", G_CALLBACK(visualization_widget_enter_notify_event_cb), this);
				g_signal_connect(G_OBJECT(l_pButton), "leave-notify-event", G_CALLBACK(visualization_widget_leave_notify_event_cb), this);
			}
		}

		//parent widget to its parent, if any
		//-----------------------------------
		IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(pVisualizationWidget->getParentIdentifier());
		if(l_pParentVisualizationWidget != NULL) //visualization boxes may be unparented
		{
			GtkTreeIter l_oParentIter;
			m_rVisualizationTree.findChildNodeFromRoot(&l_oParentIter, l_pParentVisualizationWidget->getIdentifier());

			if(l_pParentVisualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
			{
				//parent widget to notebook as a new page
				void* l_pNotebook = NULL;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oParentIter, l_pNotebook, EVisualizationTreeColumn_PointerWidget);
				char* l_pVisualizationPanelName = NULL;
				m_rVisualizationTree.getStringValueFromTreeIter(&l_oParentIter, l_pVisualizationPanelName, EVisualizationTreeColumn_StringName);
				gtk_notebook_append_page(GTK_NOTEBOOK(l_pNotebook), l_pTreeWidget, gtk_label_new(l_pVisualizationPanelName));
			}
			else if(l_pParentVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || l_pParentVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit)
			{
				//insert widget in parent paned
				void* l_pParentTreeWidget = NULL;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oParentIter, l_pParentTreeWidget, EVisualizationTreeColumn_PointerWidget);

				if(l_pParentTreeWidget != NULL && GTK_IS_WIDGET(l_pParentTreeWidget))
				{
					GtkWidget* l_pParentWidget = getVisualizationWidget(GTK_WIDGET(l_pParentTreeWidget));
					if(l_pParentWidget != NULL && GTK_IS_PANED(l_pParentWidget))
					{
						uint32 l_ui32ChildIndex;
						if(l_pParentVisualizationWidget->getChildIndex(pVisualizationWidget->getIdentifier(), l_ui32ChildIndex))
						{
							if(l_ui32ChildIndex == 0)
							{
								gtk_paned_pack1(GTK_PANED(l_pParentWidget), l_pTreeWidget, TRUE, TRUE);
							}
							else
							{
								gtk_paned_pack2(GTK_PANED(l_pParentWidget), l_pTreeWidget, TRUE, TRUE);
							}
						}
					}
				}
			}
		}
	}

	//resize widgets once they are allocated : this is the case when they are shown on an expose event
	//FIXME : perform resizing only once (when it is done as many times as there are widgets in the tree here)
	if(l_pTreeWidget != NULL)
	{
		gtk_signal_connect(GTK_OBJECT(getVisualizationWidget(l_pTreeWidget)), "expose-event", G_CALLBACK(widget_expose_event_cb), this);
	}

	return l_pTreeWidget;
}

void CDesignerVisualization::endLoadTreeWidget(OpenViBEVisualizationToolkit::IVisualizationWidget* pVisualizationWidget)
{
	//retrieve tree widget
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, pVisualizationWidget->getIdentifier());
	void* l_pTreeWidget = NULL;
	m_rVisualizationTree.getPointerValueFromTreeIter(&l_oIter, l_pTreeWidget, EVisualizationTreeColumn_PointerWidget);

	//get actual visualization widget
	::GtkWidget* l_pWidget = getVisualizationWidget((GtkWidget*)l_pTreeWidget);

	if(pVisualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		//reposition paned widget handles
		resizeCB(NULL);
	}
	else if(pVisualizationWidget->getType() == EVisualizationWidget_Undefined || pVisualizationWidget->getType() == EVisualizationWidget_VisualizationBox)
	{
		if(GTK_IS_BUTTON(l_pWidget) != FALSE)
		{
			//replace dummy icon with correct one
			//-----------------------------------
			//retrieve icon name from tree
			char* l_pIconString = NULL;
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oIter, l_pIconString, EVisualizationTreeColumn_StringStockIcon);
			//retrieve hbox
			GList* l_pButtonChildren = gtk_container_get_children(GTK_CONTAINER(l_pWidget));
			::GtkContainer* l_pBox = GTK_CONTAINER(l_pButtonChildren->data);
			//remove first widget
			GList* l_pBoxChildren = gtk_container_get_children(l_pBox);
			gtk_container_remove(l_pBox, GTK_WIDGET(l_pBoxChildren->data));
			//create new icon
			::GtkWidget* l_pIcon = gtk_image_new_from_stock(l_pIconString, GTK_ICON_SIZE_BUTTON);
			if(s_iconWidthRequest == 0)
			{
				gtk_widget_set_size_request(l_pIcon, 0, 0);
			}
			gtk_box_pack_start(GTK_BOX(l_pBox), l_pIcon, s_iconExpand, s_iconFill, 0);
			//insert it in first position
			gtk_box_reorder_child(GTK_BOX(l_pBox), l_pIcon, 0);
		}
	}
}

::GtkWidget* CDesignerVisualization::getTreeWidget(::GtkWidget* visualizationWidget)
{
	return gtk_widget_get_parent(visualizationWidget);
}

::GtkWidget* CDesignerVisualization::getVisualizationWidget(::GtkWidget* pWidget)
{
	if(GTK_IS_TABLE(pWidget))
	{
		return getTableChild(GTK_TABLE(pWidget), 1, 2, 1, 2)->widget;
	}
	else
	{
		return pWidget;
	}
}

const char* CDesignerVisualization::getTreeWidgetIcon(EVisualizationTreeNode type)
{
	switch(type)
	{
		case EVisualizationTreeNode_Unaffected:
			return GTK_STOCK_DIALOG_QUESTION;
		case EVisualizationTreeNode_Undefined:
			return GTK_STOCK_CANCEL;
		case EVisualizationTreeNode_VisualizationWindow:
			return GTK_STOCK_DND_MULTIPLE;
		case EVisualizationTreeNode_VisualizationPanel:
			return GTK_STOCK_DND;
		case EVisualizationTreeNode_VisualizationBox:
			return GTK_STOCK_EXECUTE; //default (actual icon name may be retrieved from box descriptor)
		case EVisualizationTreeNode_HorizontalSplit:
		case EVisualizationTreeNode_VerticalSplit:
			return GTK_STOCK_ADD;
		default:
			return "";
	}
}

gboolean CDesignerVisualization::delete_event_cb(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	return static_cast<CDesignerVisualization*>(user_data)->deleteEventCB() == true ? TRUE : FALSE;
}

OpenViBE::boolean CDesignerVisualization::deleteEventCB()
{
	if(m_fpDeleteEventCB != NULL)
	{
		m_fpDeleteEventCB(m_pDeleteEventUserData);
		return true;
	}

	return false;
}

#ifdef HANDLE_MIN_MAX_EVENTS
gboolean CDesignerVisualization::window_state_event_cb(::GtkWidget* widget,GdkEventWindowState* event, gpointer user_data)
{
	//refresh widgets if window was maximized or minimized
	if(event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED ||
		event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
	{
		//widgets haven't been reallocated yet, perform resizing only when this happens
		//gtk_signal_connect(GTK_OBJECT(gtk_paned_get_child2(GTK_PANED(m_pPane))), "size-allocate", G_CALLBACK(widget_size_allocate_cb), this);
		gtk_signal_connect(GTK_OBJECT(gtk_paned_get_child2(GTK_PANED(m_pPane))), "expose-event", G_CALLBACK(widget_expose_cb), this);
	}

	return FALSE;
}
#endif

//event generated whenever window size changes, including when it is first created
gboolean CDesignerVisualization::configure_event_cb(::GtkWidget* widget, GdkEventConfigure* event, gpointer user_data)
{
	static_cast<CDesignerVisualization*>(user_data)->m_bPreviewWindowVisible = true;
	/*
	//upon first show, resize window so that the preview widget has the desired size
	if(m_bFirstShow == true)
	{
		//set preview widget size
		::GtkWidget* l_pNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
		//gtk_window_resize(m_iInitialWidth, m_iInitialHeight);

		m_bFirstShow == false;
	}*/

	static_cast<CDesignerVisualization*>(user_data)->resizeCB(NULL);

	return FALSE;
}

gboolean CDesignerVisualization::widget_expose_event_cb(::GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	static_cast<CDesignerVisualization*>(user_data)->m_bPreviewWindowVisible = true;

	g_signal_handlers_disconnect_by_func(G_OBJECT(widget), G_CALLBACK2(CDesignerVisualization::widget_expose_event_cb), user_data);

	static_cast<CDesignerVisualization*>(user_data)->resizeCB(NULL);

	return FALSE;
}

void CDesignerVisualization::resizeCB(IVisualizationWidget* pVisualizationWidget)
{
	if(pVisualizationWidget == NULL)
	{
		//assign current window size to each window
		::GtkWidget* l_pNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
		if(l_pNotebook != NULL)
		{
			CIdentifier l_oVisualizationWindowIdentifier = OV_UndefinedIdentifier;
			//retrieve current preview window size, if window is visible
			if(m_bPreviewWindowVisible == true)
			{
				::GtkWidget* l_pNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
				if(l_pNotebook != NULL)
				{
					//update preview window dims
					m_ui32PreviewWindowWidth = l_pNotebook->allocation.width;
					m_ui32PreviewWindowHeight = l_pNotebook->allocation.height;
				}
			}

			while(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
			{
				IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);

				//store new dimensions
				l_pVisualizationWindow->setWidth(m_ui32PreviewWindowWidth);
				l_pVisualizationWindow->setHeight(m_ui32PreviewWindowHeight);
			}
		}
		else
		{
			//return; //?
		}

		//retrieve active visualization panel
		::GtkTreeIter l_oWindowIter;
		if(m_rVisualizationTree.findChildNodeFromRoot(&l_oWindowIter, m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow) == false)
		{
			return;
		}
		::GtkTreeIter l_oPanelIter = l_oWindowIter;
		if(m_rVisualizationTree.findChildNodeFromParent(&l_oPanelIter, m_oActiveVisualizationPanelName, EVisualizationTreeNode_VisualizationPanel) == false)
		{
			return;
		}
		CIdentifier l_oVisualizationPanelIdentifier;
		if(m_rVisualizationTree.getIdentifierFromTreeIter(&l_oPanelIter, l_oVisualizationPanelIdentifier, EVisualizationTreeColumn_StringIdentifier) == false)
		{
			return;
		}
		IVisualizationWidget* l_pVisualizationPanel = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationPanelIdentifier);

		//resize visualization panel hierarchy
		if(l_pVisualizationPanel != NULL)
		{
			CIdentifier l_oChildIdentifier;
			l_pVisualizationPanel->getChildIdentifier(0, l_oChildIdentifier);
			IVisualizationWidget* l_pChildVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oChildIdentifier);
			if(l_pChildVisualizationWidget != NULL)
			{
				resizeCB(l_pChildVisualizationWidget);
			}
		}
	}
	else if(pVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit)
	{
		GtkTreeIter l_oIter;
		if(m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, pVisualizationWidget->getIdentifier()) == TRUE)
		{
			//retrieve paned widget
			void* l_pTreeWidget = NULL;
			m_rVisualizationTree.getPointerValueFromTreeIter(&l_oIter, l_pTreeWidget, EVisualizationTreeColumn_PointerWidget);
			::GtkWidget* l_pPaned = getVisualizationWidget(GTK_WIDGET(l_pTreeWidget));
			enablePanedSignals(l_pPaned, false);

			//retrieve paned attributes
			int l_i32HandlePos = pVisualizationWidget->getDividerPosition();
			int l_i32MaxHandlePos = pVisualizationWidget->getMaxDividerPosition();

			if(l_i32MaxHandlePos > 0)
			{
				//retrieve current maximum handle position
				int l_i32CurrentMaxHandlePos = GTK_IS_VPANED(l_pPaned) ?
					GTK_PANED(l_pPaned)->container.widget.allocation.height : GTK_PANED(l_pPaned)->container.widget.allocation.width;

				//set new paned handle position
				gtk_paned_set_position(GTK_PANED(l_pPaned), l_i32HandlePos * l_i32CurrentMaxHandlePos / l_i32MaxHandlePos);
			}

			enablePanedSignals(l_pPaned, true);

			//go down child 1
			CIdentifier l_oChildIdentifier;
			pVisualizationWidget->getChildIdentifier(0, l_oChildIdentifier);
			IVisualizationWidget* l_pChildVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oChildIdentifier);
			if(l_pChildVisualizationWidget != NULL)
			{
				resizeCB(l_pChildVisualizationWidget);
			}

			//go down child 2
			pVisualizationWidget->getChildIdentifier(1, l_oChildIdentifier);
			l_pChildVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oChildIdentifier);
			if(l_pChildVisualizationWidget != NULL)
			{
				resizeCB(l_pChildVisualizationWidget);
			}
		}
	}
}

void CDesignerVisualization::notebook_page_switch_cb(::GtkNotebook* notebook, ::GtkNotebookPage* page, guint pagenum, gpointer user_data)
{
	static_cast<CDesignerVisualization*>(user_data)->notebookPageSelectedCB(notebook, pagenum);
}

gboolean CDesignerVisualization::notify_position_paned_cb(::GtkWidget *widget, GParamSpec* spec, gpointer user_data)
{
	static_cast<CDesignerVisualization*>(user_data)->notifyPositionPanedCB(widget);
	return TRUE;
}

//--------------------------
//Event box table management
//--------------------------

void CDesignerVisualization::setupNewEventBoxTable(GtkBuilder* xml)
{
	//set up event boxes as drag targets
	gtk_drag_dest_set(GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox2")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	gtk_drag_dest_set(GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox4")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	gtk_drag_dest_set(GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox6")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	gtk_drag_dest_set(GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox8")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);

	//set up event boxes callbacks for drag data received events
	char buf[256];
	sprintf(buf, "%p %s", this, "top");
	m_sTopEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox2")), "drag_data_received", G_CALLBACK(drag_data_received_in_event_box_cb),
		gpointer(m_sTopEventBoxData.c_str()));
	sprintf(buf, "%p %s", this, "left");
	m_sLeftEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox4")), "drag_data_received", G_CALLBACK(drag_data_received_in_event_box_cb),
		gpointer(m_sLeftEventBoxData.c_str()));
	sprintf(buf, "%p %s", this, "right");
	m_sRightEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox6")), "drag_data_received", G_CALLBACK(drag_data_received_in_event_box_cb),
		gpointer(m_sRightEventBoxData.c_str()));
	sprintf(buf, "%p %s", this, "bottom");
	m_sBottomEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox8")), "drag_data_received", G_CALLBACK(drag_data_received_in_event_box_cb),
		gpointer(m_sBottomEventBoxData.c_str()));
}

void CDesignerVisualization::refreshActiveVisualization(::GtkTreePath* pSelectedItemPath)
{
	//show tree
	gtk_tree_view_expand_all(m_pTreeView);

	//select item
	if(pSelectedItemPath != NULL)
	{
		gtk_tree_view_set_cursor(m_pTreeView, pSelectedItemPath, NULL, false);
	}
	else //select previous visualization tab again (or another tab if it doesn't exist anymore)
	{
		setActiveVisualization(m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName);
	}
}

void CDesignerVisualization::setActiveVisualization(const char* _activeWindow, const char* _activePanel)
{
	//ensures correct behavior when _active[Window/Panel] point to m_oActiveVisualization[Window/Panel]Name.m_pSecretImplementation
	CString activeWindow = _activeWindow;
	CString activePanel = _activePanel;

	//clear active window/panel names
	m_oActiveVisualizationWindowName = "";
	m_oActiveVisualizationPanelName = "";

	//retrieve active window
	::GtkTreeIter l_oWindowIter;

	if(m_rVisualizationTree.findChildNodeFromRoot(&l_oWindowIter, activeWindow, EVisualizationTreeNode_VisualizationWindow) == true)
	{
		m_oActiveVisualizationWindowName = activeWindow;
	}
	else
	{
		//pick first window if previously active window doesn't exist anymore
		CIdentifier l_oIdentifier = OV_UndefinedIdentifier;

		if(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oIdentifier, EVisualizationWidget_VisualizationWindow) == true)
		{
			m_oActiveVisualizationWindowName = m_rVisualizationTree.getVisualizationWidget(l_oIdentifier)->getName();
			m_rVisualizationTree.findChildNodeFromRoot(&l_oWindowIter, (const char*)m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow);
		}
		else //no windows left
		{
			if(gtk_paned_get_child2(GTK_PANED(m_pPane)) != NULL)
			{
				gtk_container_remove(GTK_CONTAINER(m_pPane), gtk_paned_get_child2(GTK_PANED(m_pPane)));
			}
			return;
		}
	}

	//retrieve active panel
	::GtkTreeIter l_oPanelIter = l_oWindowIter;
	if(m_rVisualizationTree.findChildNodeFromParent(&l_oPanelIter, activePanel, EVisualizationTreeNode_VisualizationPanel) == true)
	{
		m_oActiveVisualizationPanelName = activePanel;
	}
	else //couldn't find panel : select first one
	{
		CIdentifier l_oWindowIdentifier;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oWindowIter, l_oWindowIdentifier, EVisualizationTreeColumn_StringIdentifier);
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oWindowIdentifier);
		CIdentifier l_oPanelIdentifier;
		if(l_pVisualizationWindow->getChildIdentifier(0, l_oPanelIdentifier))
		{
			l_oPanelIter = l_oWindowIter;
			m_rVisualizationTree.findChildNodeFromParent(&l_oPanelIter, l_oPanelIdentifier);
			char* l_pString = NULL;
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oPanelIter, l_pString, EVisualizationTreeColumn_StringName);
			m_oActiveVisualizationPanelName = l_pString;
		}
		else //no panel in window
		{
			::GtkWidget* l_pCurrentNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
			if(l_pCurrentNotebook != NULL)
			{
				gtk_object_ref(GTK_OBJECT(l_pCurrentNotebook));
				gtk_container_remove(GTK_CONTAINER(m_pPane), l_pCurrentNotebook);
			}
			return;
		}
	}

	//retrieve notebook	and set it visible
	void* l_pNotebook = NULL;
	m_rVisualizationTree.getPointerValueFromTreeIter(&l_oPanelIter, l_pNotebook, EVisualizationTreeColumn_PointerWidget);
	::GtkWidget* l_pCurrentNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
	if(l_pCurrentNotebook != GTK_WIDGET(l_pNotebook))
	{
		if(l_pCurrentNotebook != NULL)
		{
			//FIXME : don't ref previous notebook if parent window doesn't exist anymore
			gtk_object_ref(GTK_OBJECT(l_pCurrentNotebook));
			gtk_container_remove(GTK_CONTAINER(m_pPane), l_pCurrentNotebook);
		}
		gtk_paned_add2(GTK_PANED(m_pPane), GTK_WIDGET(l_pNotebook));
		//gtk_object_unref(l_pCurrentNotebook);
	}

	//disable switch page notifications
	enableNotebookSignals(GTK_WIDGET(l_pNotebook), false);

	//set active panel visible
	int i;
	for(i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(l_pNotebook)); i++)
	{
		if(strcmp(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(l_pNotebook),
			gtk_notebook_get_nth_page(GTK_NOTEBOOK(l_pNotebook), i)), m_oActiveVisualizationPanelName) == 0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(l_pNotebook), i);
			break;
		}
	}

	//if active page couldn't be found
	if(i == gtk_notebook_get_n_pages(GTK_NOTEBOOK(l_pNotebook)))
	{
		//error!
		//pick first page if it exists
		if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(l_pNotebook)) > 0)
		{
			m_oActiveVisualizationPanelName = gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(l_pNotebook),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK(l_pNotebook), 0));
			gtk_notebook_set_current_page(GTK_NOTEBOOK(l_pNotebook), 0);
		}
		else //error : no pages in notebook, clear panel name
		{
			m_oActiveVisualizationPanelName = "";
		}
	}

	//enable switch page notifications
	enableNotebookSignals(GTK_WIDGET(l_pNotebook), true);

	//refresh display
	gtk_widget_show_all(m_pPane);
}

//creates a new widgets table and sets it as current
::GtkTable* CDesignerVisualization::newWidgetsTable()
{
	//@FIXME is the memory ever freed? Valgrind is suspicious about this. It seems that a builder is allocated, but only a member of builder is returned as GtkTable*.
	GtkBuilder* pGtkBuilderTable = gtk_builder_new(); // glade_xml_new(m_sGuiFile.c_str(), "window_manager_eventbox-table", NULL);
	gtk_builder_add_from_file(pGtkBuilderTable, m_sGuiFile.c_str(), NULL);
	gtk_builder_connect_signals(pGtkBuilderTable, NULL);

	//set up event boxes
	setupNewEventBoxTable(pGtkBuilderTable);

	::GtkTable* pTable = GTK_TABLE(gtk_builder_get_object(pGtkBuilderTable, "window_manager_eventbox-table"));

	//clear central button label
	::GtkTableChild* pTC = getTableChild(pTable, 1, 2, 1, 2);
	::GtkButton* pButton = GTK_BUTTON(pTC->widget);
	gtk_button_set_label(pButton, "");

	//set it up as drag destination
	gtk_drag_dest_set(GTK_WIDGET(pButton), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(pButton), "drag_data_received", G_CALLBACK(drag_data_received_in_widget_cb), this);

	//set it up as drag source as well
	gtk_drag_source_set(GTK_WIDGET(pButton), GDK_BUTTON1_MASK, targets, sizeof(targets)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(pButton), "drag_data_get", G_CALLBACK(drag_data_get_from_widget_cb), this);

	return pTable;
}

void CDesignerVisualization::askNewVisualizationWindow()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::new_visualization_window_cb, this, "New window", "Please enter name of new window : ");

	id.run();
}

boolean CDesignerVisualization::newVisualizationWindow(const char* label)
{
	//ensure name is unique
	IVisualizationWidget* l_pVisualizationWindow;
	CIdentifier l_oVisualizationWindowIdentifier = OV_UndefinedIdentifier;

	while(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
	{
		l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);

		if(strcmp((const char*)l_pVisualizationWindow->getName(), label) == 0)
		{
			displayErrorDialog("Window creation failed !", "An existing window already uses this name. Please choose another name.");
			return false;
		}
	}

	//proceed with window creation
	//m_rVisualizationTree.addVisualizationWindow(l_oVisualizationWindowIdentifier, CString(label));
	m_rVisualizationTree.addVisualizationWidget(
		l_oVisualizationWindowIdentifier,
		CString(label),
		EVisualizationWidget_VisualizationWindow,
		OV_UndefinedIdentifier,
		0,
		OV_UndefinedIdentifier,
		0,
		OV_UndefinedIdentifier);

	l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);

	//add attributes
	l_pVisualizationWindow->setWidth(1);
	l_pVisualizationWindow->setHeight(1);

	//create default visualization panel as well
	CIdentifier l_oChildIdentifier;
	CString l_oChildName = "Default tab";

	m_rVisualizationTree.addVisualizationWidget(
		l_oChildIdentifier,
		l_oChildName,
		EVisualizationWidget_VisualizationPanel,
		l_oVisualizationWindowIdentifier,
		0,
		OV_UndefinedIdentifier,
		1,
		OV_UndefinedIdentifier);

	m_rVisualizationTree.reloadTree();

	//refresh view
	::GtkTreeIter l_oChildIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oChildIter, l_oChildIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oChildIter));

	return true;
}

void CDesignerVisualization::askRenameVisualizationWindow()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::rename_visualization_window_cb, this, "Rename window", "Please enter new name of window : ");

	id.run();
}

boolean CDesignerVisualization::renameVisualizationWindow(const char* pNewVisualizationWindowName)
{
	//retrieve visualization window
	::GtkTreeIter l_oIter;
	if(m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow) == false)
	{
		displayErrorDialog("Window renaming failed !", "Couldn't retrieve window.");
		return false;
	}

	CIdentifier l_oWindowIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oWindowIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oWindowIdentifier);
	if(l_pVisualizationWindow == NULL)
	{
		displayErrorDialog("Window renaming failed !", "Couldn't retrieve window.");
		return false;
	}

	//if trying to set identical name, return
	CString l_oNewWindowName = pNewVisualizationWindowName;
	if(l_pVisualizationWindow->getName() == l_oNewWindowName)
	{
		return true;
	}

	//ensure name is unique
	CIdentifier l_oVisualizationWindowIdentifier = OV_UndefinedIdentifier;
	while(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
	{
		//name already in use : warn user
		if(m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier)->getName() == l_oNewWindowName)
		{
			displayErrorDialog("Window renaming failed !", "An existing window already uses this name. Please choose another name.");
			return false;
		}
	}

	//change its name
	l_pVisualizationWindow->setName(l_oNewWindowName);

	m_rVisualizationTree.reloadTree();

	//refresh view
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oWindowIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));

	return true;
}

boolean CDesignerVisualization::removeVisualizationWindow()
{
	//retrieve visualization window
	CIdentifier l_oVisualizationWindowIdentifier = OV_UndefinedIdentifier;
	while(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
	{
		if(m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier)->getName() == m_oActiveVisualizationWindowName)
		{
			break;
		}
	}

	//return if window was not found
	if(l_oVisualizationWindowIdentifier == OV_UndefinedIdentifier)
	{
		displayErrorDialog("Window removal failed !", "Couldn't retrieve window.");
		return false;
	}

	//destroy hierarchy but only unaffect visualization boxes
	m_rVisualizationTree.destroyHierarchy(l_oVisualizationWindowIdentifier, false);

	m_rVisualizationTree.reloadTree();

	//refresh view
	refreshActiveVisualization(NULL);

	return true;
}

void CDesignerVisualization::askNewVisualizationPanel()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::new_visualization_panel_cb, this, "New tab", "Please enter name of new tab : ");

	id.run();
}

boolean CDesignerVisualization::newVisualizationPanel(const char* label)
{
	//retrieve visualization window
	IVisualizationWidget* l_pVisualizationWindow=NULL;
	CIdentifier l_oVisualizationWindowIdentifier = OV_UndefinedIdentifier;

	while(m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowIdentifier, EVisualizationWidget_VisualizationWindow) == true)
	{
		l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);
		if(l_pVisualizationWindow->getName() == m_oActiveVisualizationWindowName)
		{
			break;
		}
	}

	//return if parent window was not found
	if(l_oVisualizationWindowIdentifier == OV_UndefinedIdentifier || l_pVisualizationWindow==NULL)
	{
		displayErrorDialog("Tab creation failed !", "Couldn't retrieve parent window.");
		return false;
	}

	CIdentifier l_oChildIdentifier;
	CString l_oNewChildName = label;

	//ensure visualization panel name is unique in this window
	for(uint32 i=0; i<l_pVisualizationWindow->getNbChildren(); i++)
	{
		l_pVisualizationWindow->getChildIdentifier(i, l_oChildIdentifier);
		if(m_rVisualizationTree.getVisualizationWidget(l_oChildIdentifier)->getName() == l_oNewChildName)
		{
			displayErrorDialog("Tab creation failed !", "An existing tab already uses this name. Please choose another name.");
			return false;
		}
	}

	//proceed with panel creation
	m_rVisualizationTree.addVisualizationWidget(
		l_oChildIdentifier,
		l_oNewChildName,
		EVisualizationWidget_VisualizationPanel,
		l_oVisualizationWindowIdentifier,
		l_pVisualizationWindow->getNbChildren(),
		OV_UndefinedIdentifier,
		1,
		OV_UndefinedIdentifier);

	m_rVisualizationTree.reloadTree();

	//refresh view
	::GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oChildIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));

	return true;
}

void CDesignerVisualization::askRenameVisualizationPanel()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::rename_visualization_panel_cb, this, "Rename tab", "Please enter new name of tab : ");

	id.run();
}

boolean CDesignerVisualization::renameVisualizationPanel(const char* pNewVisualizationPanelName)
{
	//retrieve visualization window
	::GtkTreeIter l_oIter;
	if(m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, (const char*)m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow) == false)
	{
		displayErrorDialog("Tab renaming failed !", "Couldn't retrieve parent window.");
		return false;
	}
	CIdentifier l_oVisualizationWindowIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oVisualizationWindowIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);
	if(l_pVisualizationWindow == NULL)
	{
		displayErrorDialog("Tab renaming failed !", "Couldn't retrieve parent window.");
		return false;
	}

	//retrieve visualization panel
	if(m_rVisualizationTree.findChildNodeFromParent(&l_oIter, (const char*)m_oActiveVisualizationPanelName, EVisualizationTreeNode_VisualizationPanel) == false)
	{
		displayErrorDialog("Tab renaming failed !", "Couldn't retrieve tab.");
		return false;
	}

	CIdentifier l_oVisualizationPanelIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oVisualizationPanelIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationPanelIdentifier);
	if(l_pVisualizationWidget == NULL)
	{
		displayErrorDialog("tab renaming failed !", "Couldn't retrieve tab.");
		return false;
	}

	//if trying to set identical name, return
	CString l_oNewPanelName = pNewVisualizationPanelName;
	if(l_pVisualizationWidget->getName() == l_oNewPanelName)
	{
		return true;
	}

	//ensure visualization panel name is unique in this window
	CIdentifier l_oChildIdentifier;
	for(uint32 i=0; i<l_pVisualizationWindow->getNbChildren(); i++)
	{
		l_pVisualizationWindow->getChildIdentifier(i, l_oChildIdentifier);
		if(m_rVisualizationTree.getVisualizationWidget(l_oChildIdentifier)->getName() == l_oNewPanelName)
		{
			displayErrorDialog("Tab renaming failed !", "An existing tab already uses this name. Please choose another name.");
			return false;
		}
	}

	l_pVisualizationWidget->setName(l_oNewPanelName);

	m_rVisualizationTree.reloadTree();

	//refresh view
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oVisualizationPanelIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));

	return true;
}

boolean CDesignerVisualization::removeVisualizationPanel()
{
	//retrieve visualization window
	::GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, (const char*)m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow);

	//retrieve visualization panel
	m_rVisualizationTree.findChildNodeFromParent(&l_oIter, (const char*)m_oActiveVisualizationPanelName, EVisualizationTreeNode_VisualizationPanel);
	CIdentifier l_oVisualizationPanelIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oVisualizationPanelIdentifier, EVisualizationTreeColumn_StringIdentifier);

	//destroy hierarchy but only unaffect visualization boxes (as opposed to destroying them)
	if(m_rVisualizationTree.destroyHierarchy(m_rVisualizationTree.getVisualizationWidget(l_oVisualizationPanelIdentifier)->getIdentifier(), false) == false)
	{
		displayErrorDialog("Tab removal failed !", "An error occurred while destroying widget hierarchy.");
		return false;
	}

	m_rVisualizationTree.reloadTree();

	//refresh view
	refreshActiveVisualization(NULL);

	return true;
}

boolean CDesignerVisualization::removeVisualizationWidget()
{
	//retrieve widget
	::GtkTreeIter l_oIter;
	if(m_rVisualizationTree.getTreeSelection(m_pTreeView, &l_oIter) == false)
	{
		return false;
	}
	CIdentifier l_oIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oIdentifier, EVisualizationTreeColumn_StringIdentifier);
	return removeVisualizationWidget(l_oIdentifier);
}

//TODO : move this to CVisualizationTree?
boolean CDesignerVisualization::removeVisualizationWidget(const CIdentifier& rIdentifier)
{
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(rIdentifier);
	if(l_pVisualizationWidget == NULL)
	{
		return false;
	}

	IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_pVisualizationWidget->getParentIdentifier());

	//unparent or destroy widget
	uint32 l_ui32ChildIndex;
	m_rVisualizationTree.unparentVisualizationWidget(rIdentifier, l_ui32ChildIndex);
	if(l_pVisualizationWidget->getType() != EVisualizationWidget_VisualizationBox)
	{
		m_rVisualizationTree.destroyHierarchy(rIdentifier, false);
	}

	//reparent other child widget, if any
	if(l_pParentVisualizationWidget->getType() != EVisualizationWidget_VisualizationPanel)
	{
		//retrieve parent's other widget
		CIdentifier l_oOtherVisualizationWidgetIdentifier;
		l_pParentVisualizationWidget->getChildIdentifier(1-l_ui32ChildIndex, l_oOtherVisualizationWidgetIdentifier);

		//unparent parent
		uint32 l_ui32ParentIndex;
		CIdentifier l_oParentParentIdentifier = l_pParentVisualizationWidget->getParentIdentifier();
		m_rVisualizationTree.unparentVisualizationWidget(l_pParentVisualizationWidget->getIdentifier(), l_ui32ParentIndex);

		//reparent other widget to its grandparent
		m_rVisualizationTree.unparentVisualizationWidget(l_oOtherVisualizationWidgetIdentifier, l_ui32ChildIndex);
		m_rVisualizationTree.parentVisualizationWidget(l_oOtherVisualizationWidgetIdentifier, l_oParentParentIdentifier, l_ui32ParentIndex);

		//destroy parent
		m_rVisualizationTree.destroyHierarchy(l_pParentVisualizationWidget->getIdentifier(), false);
	}

	m_rVisualizationTree.reloadTree();

	//refresh view
	refreshActiveVisualization(NULL);

	return true;
}

boolean CDesignerVisualization::destroyVisualizationWidget(const CIdentifier& rIdentifier)
{
	boolean b = removeVisualizationWidget(rIdentifier);
	m_rVisualizationTree.destroyHierarchy(rIdentifier, true);
	return b;
}

//CALLBACKS
//---------

void CDesignerVisualization::notebookPageSelectedCB(::GtkNotebook* pNotebook, guint pagenum)
{
	::GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, (void*)pNotebook);
	CIdentifier l_oIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oIdentifier);
	if(l_pVisualizationWidget != NULL)
	{
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_pVisualizationWidget->getParentIdentifier());
		if(l_pVisualizationWindow != NULL)
		{
			l_pVisualizationWindow->getChildIdentifier(pagenum, l_oIdentifier);
			if(m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oIdentifier) == true)
			{
				refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));
			}
		}
	}
}

void CDesignerVisualization::enableNotebookSignals(::GtkWidget* pNotebook, boolean b)
{
	if(b)
	{
		g_signal_connect(G_OBJECT(pNotebook), "switch-page", G_CALLBACK(notebook_page_switch_cb), this);
	}
	else
	{
		g_signal_handlers_disconnect_by_func(G_OBJECT(pNotebook), G_CALLBACK2(notebook_page_switch_cb), this);
	}
}

void CDesignerVisualization::notifyPositionPanedCB(::GtkWidget* pWidget)
{
	::GtkPaned* l_pPaned = GTK_PANED(pWidget);

	//return if handle pos was changed because parent window was resized
	int l_iPos = gtk_paned_get_position(l_pPaned);
	int l_iMaxPos = GTK_IS_VPANED(l_pPaned) ? l_pPaned->container.widget.allocation.height : l_pPaned->container.widget.allocation.width;
	int l_iHandleThickness = GTK_IS_VPANED(l_pPaned) ? l_pPaned->handle_pos.height : l_pPaned->handle_pos.width;

	if(l_iPos + l_iHandleThickness == l_iMaxPos)
	{
		return;
	}

	//look for widget in tree
	::GtkWidget* l_pTreeWidget = getTreeWidget(pWidget);
	::GtkTreeIter l_oIter;
	if(m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_pTreeWidget) == true)
	{
		CIdentifier l_oIdentifier;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oIdentifier, EVisualizationTreeColumn_StringIdentifier);

		//store new position and max position
		auto* visualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oIdentifier);
		visualizationWidget->setDividerPosition(l_iPos);
		visualizationWidget->setMaxDividerPosition(l_iMaxPos);
	}
}

void CDesignerVisualization::enablePanedSignals(::GtkWidget* pPaned, boolean b)
{
	if(b)
	{
		g_signal_connect(G_OBJECT(pPaned), "notify::position", G_CALLBACK(notify_position_paned_cb), this);
	}
	else
	{
		g_signal_handlers_disconnect_by_func(G_OBJECT(pPaned), G_CALLBACK2(notify_position_paned_cb), this);
	}
}

void CDesignerVisualization::ask_new_visualization_window_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->askNewVisualizationWindow();
}

void CDesignerVisualization::new_visualization_window_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	CInputDialog* l_pInputDialog = static_cast<CInputDialog*>(pUserData);

	if(l_pInputDialog->getUserData() != NULL)
	{
		static_cast<CDesignerVisualization*>(l_pInputDialog->getUserData())->newVisualizationWindow(l_pInputDialog->getEntry());
	}
}

void CDesignerVisualization::ask_rename_visualization_window_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->askRenameVisualizationWindow();
}

void CDesignerVisualization::rename_visualization_window_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	CInputDialog* l_pInputDialog = static_cast<CInputDialog*>(pUserData);

	if(l_pInputDialog->getUserData() != NULL)
	{
		static_cast<CDesignerVisualization*>(l_pInputDialog->getUserData())->renameVisualizationWindow(l_pInputDialog->getEntry());
	}
}

void CDesignerVisualization::remove_visualization_window_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->removeVisualizationWindow();
}

void CDesignerVisualization::ask_new_visualization_panel_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->askNewVisualizationPanel();
}

void CDesignerVisualization::new_visualization_panel_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	CInputDialog* l_pInputDialog = static_cast<CInputDialog*>(pUserData);

	if(l_pInputDialog->getUserData() != NULL)
	{
		static_cast<CDesignerVisualization*>(l_pInputDialog->getUserData())->newVisualizationPanel(l_pInputDialog->getEntry());
	}
}

void CDesignerVisualization::ask_rename_visualization_panel_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->askRenameVisualizationPanel();
}

void CDesignerVisualization::rename_visualization_panel_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	CInputDialog* l_pInputDialog = static_cast<CInputDialog*>(pUserData);

	if(l_pInputDialog->getUserData() != NULL)
	{
		static_cast<CDesignerVisualization*>(l_pInputDialog->getUserData())->renameVisualizationPanel(l_pInputDialog->getEntry());
	}
}

void CDesignerVisualization::remove_visualization_panel_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->removeVisualizationPanel();
}

void CDesignerVisualization::remove_visualization_widget_cb(gpointer pUserData, guint callback_action, ::GtkWidget* pWidget)
{
	static_cast<CDesignerVisualization*>(pUserData)->removeVisualizationWidget();
}

void CDesignerVisualization::visualization_widget_key_press_event_cb(::GtkWidget* pWidget, GdkEventKey* pEvent, gpointer pUserData)
{
	static_cast<CDesignerVisualization*>(pUserData)->visualizationWidgetKeyPressEventCB(pWidget, pEvent);
}

void CDesignerVisualization::visualizationWidgetKeyPressEventCB(::GtkWidget*, GdkEventKey* pEventKey)
{
	//remove widget
	if(pEventKey->keyval==GDK_Delete || pEventKey->keyval==GDK_KP_Delete)
	{
		if(m_pHighlightedWidget != NULL)
		{
			::GtkTreeIter l_oIter;
			if(m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, getTreeWidget(m_pHighlightedWidget)) == true)
			{
				CIdentifier l_oIdentifier;
				m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oIdentifier, EVisualizationTreeColumn_StringIdentifier);
				removeVisualizationWidget(l_oIdentifier);
			}
		}
	}
}

gboolean CDesignerVisualization::visualization_widget_enter_notify_event_cb(::GtkWidget* pWidget, GdkEventCrossing* pEventCrossing, gpointer pUserData)
{
	static_cast<CDesignerVisualization*>(pUserData)->visualizationWidgetEnterNotifyEventCB(pWidget, pEventCrossing);
	return FALSE;
}

void CDesignerVisualization::visualizationWidgetEnterNotifyEventCB(::GtkWidget* pWidget, GdkEventCrossing* pEventCrossing)
{
	m_pHighlightedWidget = pWidget;
}

gboolean CDesignerVisualization::visualization_widget_leave_notify_event_cb(::GtkWidget* pWidget, GdkEventCrossing* pEventCrossing, gpointer pUserData)
{
	static_cast<CDesignerVisualization*>(pUserData)->visualizationWidgetLeaveNotifyEventCB(pWidget, pEventCrossing);
	return FALSE;
}

void CDesignerVisualization::visualizationWidgetLeaveNotifyEventCB(::GtkWidget* pWidget, GdkEventCrossing* pEventCrossing)
{
	m_pHighlightedWidget = NULL;
}

gboolean CDesignerVisualization::button_release_cb(::GtkWidget* pWidget, GdkEventButton *pEvent, gpointer pUserData)
{
	static_cast<CDesignerVisualization*>(pUserData)->buttonReleaseCB(pWidget, pEvent);

	return FALSE;
}

void CDesignerVisualization::buttonReleaseCB(::GtkWidget* pWidget, GdkEventButton* pEvent)
{
	if(GTK_IS_TREE_VIEW(pWidget))
	{
		if(pEvent->button == 3) //right button
		{
			if(pEvent->type != GDK_BUTTON_PRESS)
			{
				::GtkTreeIter l_oIter;

				if(m_rVisualizationTree.getTreeSelection(m_pTreeView, &l_oIter) == false)
				{
					return;
				}

				unsigned long l_ulType = m_rVisualizationTree.getULongValueFromTreeIter(&l_oIter, EVisualizationTreeColumn_ULongNodeType);

				if(l_ulType == EVisualizationTreeNode_Unaffected)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pUnaffectedItemFactory, "<unaffected_main>")),NULL,NULL,NULL,NULL,pEvent->button,pEvent->time);
				}
				else if(l_ulType == EVisualizationTreeNode_VisualizationWindow)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pVisualizationWindowItemFactory, "<visualization_window_main>")),NULL,NULL,NULL,NULL,pEvent->button,pEvent->time);
				}
				else if(l_ulType == EVisualizationTreeNode_VisualizationPanel)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pVisualizationPanelItemFactory, "<visualization_panel_main>")),NULL,NULL,NULL,NULL,pEvent->button,pEvent->time);
				}
				else if(l_ulType == EVisualizationTreeNode_HorizontalSplit || l_ulType == EVisualizationTreeNode_VerticalSplit)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pSplitItemFactory, "<split_widget_main>")),NULL,NULL,NULL,NULL,pEvent->button,pEvent->time);
				}
				else if(l_ulType == EVisualizationTreeNode_VisualizationBox)
				{
					//ensure visualization box is parented to a tab
					if(m_rVisualizationTree.findParentNode(&l_oIter, EVisualizationTreeNode_VisualizationPanel) == true)
					{
						gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pVisualizationBoxItemFactory, "<visualization_box_main>")),NULL,NULL,NULL,NULL,pEvent->button,pEvent->time);
					}
				}
				else if(l_ulType == EVisualizationTreeNode_Undefined)
				{
					//ensure empty plugin is not parented to a panel (because an empty widget is always present in an empty panel)
					CIdentifier l_oIdentifier;
					m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oIdentifier, EVisualizationTreeColumn_StringIdentifier);
					IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oIdentifier);
					if(l_pVisualizationWidget != NULL)
					{
						IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_pVisualizationWidget->getParentIdentifier());
						if(l_pParentVisualizationWidget != NULL)
						{
							if(l_pParentVisualizationWidget->getType() != EVisualizationWidget_VisualizationPanel)
							{
								gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pUndefinedItemFactory, "<undefined_widget_main>")),NULL,NULL,NULL,NULL,pEvent->button,pEvent->time);
							}
						}
					}
				}
			}
		}
	}
}

void CDesignerVisualization::cursor_changed_cb(::GtkTreeView* pTreeView, gpointer pUserData)
{
	static_cast<CDesignerVisualization*>(pUserData)->cursorChangedCB(pTreeView);
}

void CDesignerVisualization::cursorChangedCB(::GtkTreeView* pTreeView)
{
	//retrieve selection
	::GtkTreeIter l_oSelectionIter;
	if(m_rVisualizationTree.getTreeSelection(pTreeView, &l_oSelectionIter) == false)
	{
		return;
	}

	//save active item
	if(m_rVisualizationTree.getULongValueFromTreeIter(&l_oSelectionIter, EVisualizationTreeColumn_ULongNodeType) == EVisualizationTreeNode_VisualizationBox)
	{
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oSelectionIter, m_oActiveVisualizationBoxIdentifier, EVisualizationTreeColumn_StringIdentifier);
	}

	::GtkTreeIter l_oVisualizationPanelIter = l_oSelectionIter;

	//if selection lies in a visualization panel subtree, display this subtree
	if(m_rVisualizationTree.findParentNode(&l_oVisualizationPanelIter, EVisualizationTreeNode_VisualizationPanel) == true)
	{
		//get visualization panel name
		char* l_pVisualizationPanelName = NULL;
		m_rVisualizationTree.getStringValueFromTreeIter(&l_oVisualizationPanelIter, l_pVisualizationPanelName, EVisualizationTreeColumn_StringName);

		//retrieve visualization window that contains selection
		::GtkTreeIter l_oVisualizationWindowIter = l_oVisualizationPanelIter;
		if(m_rVisualizationTree.findParentNode(&l_oVisualizationWindowIter, EVisualizationTreeNode_VisualizationWindow) == true)
		{
			//get its name
			char* l_pVisualizationWindowName = NULL;
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oVisualizationWindowIter, l_pVisualizationWindowName, EVisualizationTreeColumn_StringName);

			//set active visualization
			setActiveVisualization(l_pVisualizationWindowName, l_pVisualizationPanelName);
		}
	}
	else
	{
		::GtkTreeIter l_oVisualizationWindowIter = l_oSelectionIter;

		//if selection is a visualization window, display it
		if(m_rVisualizationTree.findParentNode(&l_oVisualizationWindowIter, EVisualizationTreeNode_VisualizationWindow) == true)
		{
			//retrieve visualization window
			CIdentifier l_oVisualizationWindowIdentifier;
			m_rVisualizationTree.getIdentifierFromTreeIter(&l_oVisualizationWindowIter, l_oVisualizationWindowIdentifier, EVisualizationTreeColumn_StringIdentifier);
			IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowIdentifier);

			//if window has at least one panel
			if(l_pVisualizationWindow->getNbChildren() > 0)
			{
				//retrieve first panel
				CIdentifier l_oVisualizationPanelIdentifier;
				l_pVisualizationWindow->getChildIdentifier(0, l_oVisualizationPanelIdentifier);
				m_rVisualizationTree.findChildNodeFromParent(&l_oVisualizationPanelIter, l_oVisualizationPanelIdentifier);

				//retrieve notebook
				void* l_pNotebook = NULL;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oVisualizationPanelIter, l_pNotebook, EVisualizationTreeColumn_PointerWidget);

				//get label of its active tab
				::GtkWidget* l_pCurrentPageLabel =
					gtk_notebook_get_tab_label(GTK_NOTEBOOK(l_pNotebook),
						gtk_notebook_get_nth_page(GTK_NOTEBOOK(l_pNotebook),
							gtk_notebook_get_current_page(GTK_NOTEBOOK(l_pNotebook))));

				//set active visualization
				if(l_pCurrentPageLabel != NULL)
				{
					setActiveVisualization((const char*)l_pVisualizationWindow->getName(), gtk_label_get_text(GTK_LABEL(l_pCurrentPageLabel)));
				}
				else
				{
					setActiveVisualization((const char*)l_pVisualizationWindow->getName(), NULL);
				}
			}
			else //window has no panels
			{
				setActiveVisualization((const char*)l_pVisualizationWindow->getName(), NULL);
			}
		}
		else
		{
			//refresh active visualization (::GtkWidgets may have changed if tree was reloaded)
			setActiveVisualization(m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName);
		}
	}
}

void CDesignerVisualization::drag_data_get_from_tree_cb(::GtkWidget* pSrcWidget, ::GdkDragContext* pDragContex, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT, gpointer pData)
{
	static_cast<CDesignerVisualization*>(pData)->dragDataGetFromTreeCB(pSrcWidget, pSelectionData);
}

void CDesignerVisualization::dragDataGetFromTreeCB(::GtkWidget* pSrcWidget, ::GtkSelectionData* pSelectionData)
{
	char l_sString[1024];
	sprintf(l_sString, "%p", pSrcWidget);
	gtk_selection_data_set_text(pSelectionData, l_sString, strlen(l_sString));
}

void CDesignerVisualization::drag_data_get_from_widget_cb(::GtkWidget* pSrcWidget, GdkDragContext* pDragContext, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiTime, gpointer pData)
{
	static_cast<CDesignerVisualization*>(pData)->dragDataGetFromWidgetCB(pSrcWidget, pSelectionData);
}

void CDesignerVisualization::dragDataGetFromWidgetCB(::GtkWidget* pSrcWidget, ::GtkSelectionData* pSelectionData)
{
	char l_sString[1024];
	sprintf(l_sString, "%p", pSrcWidget);
	gtk_selection_data_set_text(pSelectionData, l_sString, strlen(l_sString));
}

void CDesignerVisualization::drag_data_received_in_widget_cb(::GtkWidget* dstWidget, GdkDragContext* pDragContext,gint iX,gint iY,::GtkSelectionData* pSelectionData,guint uiInfo,guint uiTime,gpointer pData)
{
	static_cast<CDesignerVisualization*>(pData)->dragDataReceivedInWidgetCB(dstWidget, pSelectionData);
}

void CDesignerVisualization::dragDataReceivedInWidgetCB(::GtkWidget* pDstWidget, ::GtkSelectionData* pSelectionData)
{
	void* l_pSrcWidget = NULL;
	sscanf((const char*)gtk_selection_data_get_text(pSelectionData), "%p", &l_pSrcWidget);
	::GtkTreeIter l_oSrcIter;

	//retrieve source widget iterator
	if(GTK_IS_TREE_VIEW(l_pSrcWidget))
	{
		//ensure dragged widget is a visualization box
		if(m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, m_oActiveVisualizationBoxIdentifier) == false)
		{
			m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of active visualization box!\n";
			return;
		}
	}
	else if(GTK_IS_BUTTON(l_pSrcWidget))
	{
		if(l_pSrcWidget == pDstWidget)
		{
			return;
		}
		if(m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, getTreeWidget(GTK_WIDGET(l_pSrcWidget))) == false)
		{
			m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of dragged button!\n";
			return;
		}
	}
	else
	{
		return;
	}

	//retrieve src widget identifier and src visualization widget
	CIdentifier l_oSrcIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oSrcIter, l_oSrcIdentifier, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pSrcVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oSrcIdentifier);
	if(l_pSrcVisualizationWidget == NULL)
	{
		m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve source visualization widget!\n";
		return;
	}

	//retrieve dest widget type
	::GtkTreeIter l_oDstIter;
	if(m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, getTreeWidget(pDstWidget)) == false)
	{
		m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of destination widget!\n";
		return;
	}

	//if src widget is unaffected or if dest widget is a visualization box, perform the drop operation directly
	if(l_pSrcVisualizationWidget->getParentIdentifier() == OV_UndefinedIdentifier ||
		m_rVisualizationTree.getULongValueFromTreeIter(&l_oDstIter, EVisualizationTreeColumn_ULongNodeType) == EVisualizationTreeNode_VisualizationBox)
	{
		m_rVisualizationTree.dragDataReceivedInWidgetCB(l_oSrcIdentifier, pDstWidget);
	}
	else //dest widget is a dummy : unaffect src widget and simplify the tree before performing the drop operation
	{
		//save dest widget identifier
		CIdentifier l_oDstIdentifier;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oDstIter, l_oDstIdentifier, EVisualizationTreeColumn_StringIdentifier);

		//unaffect src widget, so that tree is simplified
		if(removeVisualizationWidget(l_oSrcIdentifier) == false)
		{
			m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't remove source widget from its parent!\n";
			return;
		}

		//then drop it
		if(m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, l_oDstIdentifier) == false)
		{
			m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of dummy destination widget to delete!\n";
			return;
		}
		void* l_pNewDstTreeWidget = NULL;
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oDstIter, l_pNewDstTreeWidget, EVisualizationTreeColumn_PointerWidget);
		m_rVisualizationTree.dragDataReceivedInWidgetCB(l_oSrcIdentifier, getVisualizationWidget(GTK_WIDGET(l_pNewDstTreeWidget)));
	}

	//refresh view
	::GtkTreeIter l_oDraggedIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oDraggedIter, l_oSrcIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oDraggedIter));
}

void CDesignerVisualization::drag_data_received_in_event_box_cb(::GtkWidget* pDstWidget,GdkDragContext*,gint,gint,::GtkSelectionData* pSelectionData,guint,guint,gpointer pData)
{
	char buf[1024];
	void* pDesignerVisualization = NULL;
	sscanf((const char*)pData, "%p %s", &pDesignerVisualization, buf);

	EDragDataLocation l_oLocation;
	if(strcmp(buf, "left") == 0)
	{
		l_oLocation = EDragData_Left;
	}
	else if(strcmp(buf, "right")==0)
	{
		l_oLocation = EDragData_Right;
	}
	else if(strcmp(buf, "top")==0)
	{
		l_oLocation = EDragData_Top;
	}
	else
	{
		l_oLocation = EDragData_Bottom;
	}

	((CDesignerVisualization*)pDesignerVisualization)->dragDataReceivedInEventBoxCB(pDstWidget, pSelectionData, l_oLocation);
}

void CDesignerVisualization::dragDataReceivedInEventBoxCB(::GtkWidget* pDstWidget, ::GtkSelectionData* pSelectionData, EDragDataLocation l_oLocation)
{
	void* l_pSrcWidget = NULL;
	sscanf((const char*)gtk_selection_data_get_text(pSelectionData), "%p", &l_pSrcWidget);
	::GtkTreeIter l_oSrcIter;

	//get iterator to src widget
	if(GTK_IS_TREE_VIEW(l_pSrcWidget))
	{
		if(m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, m_oActiveVisualizationBoxIdentifier) == false)
		{
			return;
		}
		//get actual src widget (item being dropped) and ensure it isn't being dropped in its own table
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oSrcIter, l_pSrcWidget, EVisualizationTreeColumn_PointerWidget);
		if(l_pSrcWidget == gtk_widget_get_parent(pDstWidget))
		{
			return;
		}
	}
	else if(GTK_IS_BUTTON(l_pSrcWidget))
	{
		//ensure src widget isn't being dropped in its own table
		if(gtk_widget_get_parent(GTK_WIDGET(l_pSrcWidget)) == gtk_widget_get_parent(pDstWidget))
		{
			return;
		}
		m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, getTreeWidget(GTK_WIDGET(l_pSrcWidget)));
	}
	else
	{
		return;
	}

	//ensure src widget is a visualization box
	if(m_rVisualizationTree.getULongValueFromTreeIter(&l_oSrcIter, EVisualizationTreeColumn_ULongNodeType) != EVisualizationTreeNode_VisualizationBox)
	{
		return;
	}

	//retrieve src widget identifier
	CIdentifier l_oSrcIdentifier;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oSrcIter, l_oSrcIdentifier, EVisualizationTreeColumn_StringIdentifier);

	//if widget is unaffected, just drag n drop it
	::GtkTreeIter l_oUnaffectedIter = l_oSrcIter;
	if(m_rVisualizationTree.findParentNode(&l_oUnaffectedIter, EVisualizationTreeNode_Unaffected) == true)
	{
		m_rVisualizationTree.dragDataReceivedOutsideWidgetCB(l_oSrcIdentifier, pDstWidget, l_oLocation);
	}
	else
	{
		//save dest widget identifier
		::GtkTreeIter l_oDstIter;
		m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, getTreeWidget(pDstWidget));
		CIdentifier l_oDstIdentifier;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oDstIter, l_oDstIdentifier, EVisualizationTreeColumn_StringIdentifier);

		//if dest widget is src widget's parent (paned widget), drop src widget in corresponding event box of parent's other child
		//(otherwise, DND will fail due to parent's removal during tree simplification process)
		IVisualizationWidget* l_pSrcVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oSrcIdentifier);
		if(l_pSrcVisualizationWidget->getParentIdentifier() == l_oDstIdentifier)
		{
			IVisualizationWidget* l_pSrcParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_pSrcVisualizationWidget->getParentIdentifier());
			l_pSrcParentVisualizationWidget->getChildIdentifier(0, l_oDstIdentifier);
			if(l_oSrcIdentifier == l_oDstIdentifier)
			{
				l_pSrcParentVisualizationWidget->getChildIdentifier(1, l_oDstIdentifier);
			}
		}

		//unaffect src widget, so that tree is simplified
		removeVisualizationWidget(l_oSrcIdentifier);

		//then drop it
		m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, l_oDstIdentifier);
		void* l_pNewDstTreeWidget = NULL;
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oDstIter, l_pNewDstTreeWidget, EVisualizationTreeColumn_PointerWidget);
		m_rVisualizationTree.dragDataReceivedOutsideWidgetCB(l_oSrcIdentifier, getVisualizationWidget(GTK_WIDGET(l_pNewDstTreeWidget)), l_oLocation);
	}

	//refresh view
	::GtkTreeIter l_oDraggedIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oDraggedIter, l_oSrcIdentifier);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oDraggedIter));
}
