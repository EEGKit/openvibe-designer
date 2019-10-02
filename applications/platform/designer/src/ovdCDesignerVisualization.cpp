#include "ovd_base.h"
#include "ovdTAttributeHandler.h"
#include "ovdCApplication.h"
#include "ovdCDesignerVisualization.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCInputDialog.h"

#include <gdk/gdkkeysyms.h>
#include <cstring>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace OpenViBEVisualizationToolkit;

static GtkTargetEntry targets[] =
{
	{ static_cast<gchar*>("STRING"), 0, 0 },
	{ static_cast<gchar*>("text/plain"), 0, 0 },
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
		GtkWidget* l_pErrorDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", pText);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pErrorDialog), "%s", pSecondaryText);

		gtk_window_set_position(GTK_WINDOW(l_pErrorDialog), GTK_WIN_POS_MOUSE);

		gtk_dialog_run(GTK_DIALOG(l_pErrorDialog));

		gtk_widget_destroy(l_pErrorDialog);
	}

	/**
	 * \brief Helper function retrieving a child in a table from its attach indices
	 * \param pTable Table parent to the child to be retrieved
	 * \param leftAttach Left attach index
	 * \param rightAttach Right attach index
	 * \param topAttach Top attach index
	 * \param bottomAttach Bottom attach index
	 * \return Pointer to table child if one was found, nullptr otherwise
	 */
	GtkTableChild* getTableChild(GtkTable* pTable, const int leftAttach, const int rightAttach, const int topAttach, const int bottomAttach)
	{
		GList* pList = pTable->children;

		do
		{
			GtkTableChild* pTC = static_cast<GtkTableChild*>(pList->data);
			if (pTC->left_attach == leftAttach && pTC->right_attach == rightAttach &&
				pTC->top_attach == topAttach && pTC->bottom_attach == bottomAttach) { return pTC; }
			pList = pList->next;
		} while (pList);

		return nullptr;
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
		::GtkWidget* l_pQuestionDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, pText);
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pQuestionDialog),	pSecondaryText);
		gtk_window_set_position(GTK_WINDOW(l_pQuestionDialog), GTK_WIN_POS_MOUSE);
		gint ret = gtk_dialog_run(GTK_DIALOG(l_pQuestionDialog));
		gtk_widget_destroy(l_pQuestionDialog);
		return ret;
	}*/
} // namespace OpenViBEDesigner

//Menus
//-----

static GtkItemFactoryEntry unaffected_menu_items[] = {
	{
		static_cast<gchar*>("/New window"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::ask_new_visualization_window_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DND_MULTIPLE
	}
};

static GtkItemFactoryEntry visualization_window_menu_items[] = {
	{
		static_cast<gchar*>("/New tab"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::ask_new_visualization_panel_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DND
	},
	{
		static_cast<gchar*>("/Rename"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::ask_rename_visualization_window_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_BOLD
	},
	{
		static_cast<gchar*>("/Remove"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::remove_visualization_window_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DELETE
	}
};

static GtkItemFactoryEntry visualization_panel_menu_items[] = {
	{
		static_cast<gchar*>("/Rename"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::ask_rename_visualization_panel_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_BOLD
	},
	{
		static_cast<gchar*>("/Remove"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::remove_visualization_panel_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DELETE
	}
};

static GtkItemFactoryEntry visualization_box_menu_items[] = {
	{
		static_cast<gchar*>("/Remove"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::remove_visualization_widget_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DELETE
	}
};

static GtkItemFactoryEntry undefined_widget_menu_items[] = {
	{
		static_cast<gchar*>("/Remove"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::remove_visualization_widget_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DELETE
	}
};

static GtkItemFactoryEntry split_widget_menu_items[] = {
	{
		static_cast<gchar*>("/Remove"), static_cast<gchar*>(""), GtkItemFactoryCallback(CDesignerVisualization::remove_visualization_widget_cb), 1,
		static_cast<gchar*>("<StockItem>"), GTK_STOCK_DELETE
	}
};

static gint num_unaffected_menu_items           = sizeof(unaffected_menu_items) / sizeof(unaffected_menu_items[0]);
static gint num_visualization_window_menu_items = sizeof(visualization_window_menu_items) / sizeof(visualization_window_menu_items[0]);
static gint num_visualization_panel_menu_items  = sizeof(visualization_panel_menu_items) / sizeof(visualization_panel_menu_items[0]);
static gint num_visualization_box_menu_items    = sizeof(visualization_box_menu_items) / sizeof(visualization_box_menu_items[0]);
static gint num_undefined_widget_menu_items     = sizeof(undefined_widget_menu_items) / sizeof(undefined_widget_menu_items[0]);
static gint num_split_widget_menu_items         = sizeof(split_widget_menu_items) / sizeof(split_widget_menu_items[0]);

CDesignerVisualization::CDesignerVisualization(const IKernelContext& ctx, IVisualizationTree& rVisualizationTree,
											   CInterfacedScenario& rInterfacedScenario)
	: m_kernelContext(ctx), m_rVisualizationTree(rVisualizationTree), m_rInterfacedScenario(rInterfacedScenario),
	  m_fpDeleteEventCB(nullptr), m_pDeleteEventUserData(nullptr) { }

CDesignerVisualization::~CDesignerVisualization()
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_pDialog), G_CALLBACK2(configure_event_cb), this);
#ifdef HANDLE_MIN_MAX_EVENTS
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_pDialog), G_CALLBACK2(window_state_event_cb), this);
#endif
	gtk_widget_destroy(m_pDialog);

	m_rVisualizationTree.setTreeViewCB(nullptr);
}

void CDesignerVisualization::init(const std::string& guiFile)
{
	m_sGuiFile = guiFile;

	//create tree view
	//----------------

	//register towards tree store
	m_rVisualizationTree.setTreeViewCB(this);

	m_pTreeView = m_rVisualizationTree.createTreeViewWithModel();

	GtkTreeViewColumn* l_pTreeViewColumnName = gtk_tree_view_column_new();
	GtkCellRenderer* l_pCellRendererIcon     = gtk_cell_renderer_pixbuf_new();
	GtkCellRenderer* l_pCellRendererName     = gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Windows for current scenario");
	gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", EVisualizationTreeColumn_StringStockIcon, nullptr);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", EVisualizationTreeColumn_StringName, nullptr);
	//gtk_tree_view_column_set_sizing(l_pTreeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnName, TRUE/*FALSE*/);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnName, TRUE);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnName, 64);
	gtk_tree_view_append_column(m_pTreeView, l_pTreeViewColumnName);

	GtkTreeViewColumn* l_pTreeViewColumnDesc = gtk_tree_view_column_new();
	gtk_tree_view_append_column(m_pTreeView, l_pTreeViewColumnDesc);

	gtk_tree_view_column_set_visible(l_pTreeViewColumnDesc, 0);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(m_pTreeView));

	//allow tree items to be dragged
	gtk_drag_source_set(GTK_WIDGET(m_pTreeView), GDK_BUTTON1_MASK, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);

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
	const uint32_t treeViewWidth = 200;
	m_previewWindowW             = uint32_t(m_kernelContext.getConfigurationManager().expandAsUInteger("${Designer_UnaffectedVisualizationWindowWidth}", 400));
	m_previewWindowH             = uint32_t(m_kernelContext.getConfigurationManager().expandAsUInteger("${Designer_UnaffectedVisualizationWindowHeight}", 400));
	CIdentifier l_oVisualizationWindowID;
	//if at least one window was created, retrieve its dimensions
	if (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
	{
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);
		m_previewWindowW                             = l_pVisualizationWindow->getWidth();
		m_previewWindowH                             = l_pVisualizationWindow->getHeight();
		/* Change the way window sizes are stored in the widget
		TAttributeHandler l_oAttributeHandler(*l_pVisualizationWindow);
		m_previewWindowW = l_oAttributeHandler.getAttributeValue<int>(OVD_AttributeId_VisualizationWindow_Width);
		m_previewWindowH = l_oAttributeHandler.getAttributeValue<int>(OVD_AttributeId_VisualizationWindow_Height);
		*/
	}
	gtk_window_set_default_size(GTK_WINDOW(m_pDialog), gint(treeViewWidth + m_previewWindowW), gint(m_previewWindowH));
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

	GtkWidget* l_pScrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l_pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(l_pScrolledWindow), GTK_WIDGET(m_pTreeView));

	//add tree view to pane
	gtk_paned_add1(GTK_PANED(m_pPane), GTK_WIDGET(l_pScrolledWindow));

	//set initial divider position
	gtk_paned_set_position(GTK_PANED(m_pPane), gint(treeViewWidth));

	//create popup menus
	//------------------
	m_pUnaffectedItemFactory = gtk_item_factory_new(GTK_TYPE_MENU, "<unaffected_main>", nullptr);
	gtk_item_factory_create_items(m_pUnaffectedItemFactory, num_unaffected_menu_items, unaffected_menu_items, this);

	m_pVisualizationWindowItemFactory = gtk_item_factory_new(GTK_TYPE_MENU, "<visualization_window_main>", nullptr);
	gtk_item_factory_create_items(m_pVisualizationWindowItemFactory, num_visualization_window_menu_items, visualization_window_menu_items, this);

	m_pVisualizationPanelItemFactory = gtk_item_factory_new(GTK_TYPE_MENU, "<visualization_panel_main>", nullptr);
	gtk_item_factory_create_items(m_pVisualizationPanelItemFactory, num_visualization_panel_menu_items, visualization_panel_menu_items, this);

	m_pVisualizationBoxItemFactory = gtk_item_factory_new(GTK_TYPE_MENU, "<visualization_box_main>", nullptr);
	gtk_item_factory_create_items(m_pVisualizationBoxItemFactory, num_visualization_box_menu_items, visualization_box_menu_items, this);

	m_pUndefinedItemFactory = gtk_item_factory_new(GTK_TYPE_MENU, "<undefined_widget_main>", nullptr);
	gtk_item_factory_create_items(m_pUndefinedItemFactory, num_undefined_widget_menu_items, undefined_widget_menu_items, this);

	m_pSplitItemFactory = gtk_item_factory_new(GTK_TYPE_MENU, "<split_widget_main>", nullptr);
	gtk_item_factory_create_items(m_pSplitItemFactory, num_split_widget_menu_items, split_widget_menu_items, this);
}

void CDesignerVisualization::load()

{
	m_rVisualizationTree.setTreeViewCB(this);

	m_rVisualizationTree.reloadTree();

	//if at least one window was created, retrieve its dimensions
	CIdentifier l_oVisualizationWindowID;
	if (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
	{
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);
		m_previewWindowW                             = l_pVisualizationWindow->getWidth();
		m_previewWindowH                             = l_pVisualizationWindow->getHeight();
	}
	const uint32_t treeViewWidth = gtk_paned_get_position(GTK_PANED(m_pPane));
	gtk_widget_set_size_request(GTK_WIDGET(m_pDialog), gint(treeViewWidth + m_previewWindowW), gint(m_previewWindowH));

	gtk_tree_view_expand_all(m_pTreeView);

	setActiveVisualization(m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName);
}

void CDesignerVisualization::show() const
{
	// since gtk is asynchronous for the expose event,
	// the m_bPreviewWindowVisible flag is turned on in the
	// corresponding callback
	//m_bPreviewWindowVisible = true;
	gtk_widget_show_all(static_cast<GtkWidget*>(m_pDialog));
}

void CDesignerVisualization::hide()
{
	m_bPreviewWindowVisible = false;
	gtk_widget_hide_all(static_cast<GtkWidget*>(m_pDialog));
}

void CDesignerVisualization::setDeleteEventCB(fpDesignerVisualizationDeleteEventCB fpDeleteEventCB, gpointer data)
{
	m_fpDeleteEventCB      = fpDeleteEventCB;
	m_pDeleteEventUserData = data;
}

void CDesignerVisualization::onVisualizationBoxAdded(const IBox* pBox)
{
	CIdentifier l_oVisualizationWidgetID;
	m_rVisualizationTree.addVisualizationWidget(l_oVisualizationWidgetID, pBox->getName(), EVisualizationWidget_VisualizationBox,
												OV_UndefinedIdentifier, 0, pBox->getIdentifier(), 0, OV_UndefinedIdentifier);

	m_rVisualizationTree.reloadTree();

	//refresh view
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oVisualizationWidgetID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));
}

void CDesignerVisualization::onVisualizationBoxRemoved(const CIdentifier& boxID)
{
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidgetFromBoxIdentifier(boxID);
	if (l_pVisualizationWidget != nullptr)
	{
		//unaffected widget : delete it
		if (l_pVisualizationWidget->getParentIdentifier() == OV_UndefinedIdentifier)
		{
			m_rVisualizationTree.destroyHierarchy(l_pVisualizationWidget->getIdentifier());
		}
		else //simplify tree
		{
			destroyVisualizationWidget(l_pVisualizationWidget->getIdentifier());
		}

		m_rVisualizationTree.reloadTree();

		//refresh view
		refreshActiveVisualization(nullptr);
	}
}

void CDesignerVisualization::onVisualizationBoxRenamed(const CIdentifier& boxID)
{
	//retrieve visualization widget
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidgetFromBoxIdentifier(boxID);
	if (l_pVisualizationWidget != nullptr)
	{
		//retrieve box name
		const IBox* l_pBox = m_rInterfacedScenario.m_rScenario.getBoxDetails(boxID);
		if (l_pBox != nullptr)
		{
			//set new visualization widget name
			l_pVisualizationWidget->setName(l_pBox->getName());

			//reload tree
			m_rVisualizationTree.reloadTree();

			//refresh view
			refreshActiveVisualization(nullptr);
		}
	}
}

void CDesignerVisualization::createTreeWidget(IVisualizationWidget* widget)
{
	if (widget->getType() == EVisualizationWidget_HorizontalSplit || widget->getType() == EVisualizationWidget_VerticalSplit)
	{
		/* TODO_JL: Find a way to store divider position and max divider position
		TAttributeHandler l_oAttributeHandler(*widget);
		l_oAttributeHandler.addAttribute(OVD_AttributeId_VisualizationWidget_DividerPosition, 1);
		l_oAttributeHandler.addAttribute(OVD_AttributeId_VisualizationWidget_MaxDividerPosition, 2);
		*/
	}
}

//need width request of 0 to avoid graphical bugs (label/icon overlapping other widgets) when shrinking buttons
static gint s_labelWidthRequest = 0;
static gint s_iconWidthRequest  = 0;
//need expand and fill flags to TRUE to see 0-size-requesting widgets
static gboolean s_labelExpand = TRUE;
static gboolean s_labelFill   = TRUE;
static gboolean s_iconExpand  = TRUE;
static gboolean s_iconFill    = TRUE;

GtkWidget* CDesignerVisualization::loadTreeWidget(IVisualizationWidget* widget)
{
	GtkWidget* l_pTreeWidget = nullptr;

	//create widget
	//-------------
	if (widget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		//retrieve panel index
		IVisualizationWidget* l_pWindow = m_rVisualizationTree.getVisualizationWidget(widget->getParentIdentifier());
		if (l_pWindow != nullptr)
		{
			uint32_t l_ui32PanelIdx;
			l_pWindow->getChildIndex(widget->getIdentifier(), l_ui32PanelIdx);

			//create notebook if this is the first panel
			if (l_ui32PanelIdx == 0) { l_pTreeWidget = gtk_notebook_new(); }
			else //otherwise retrieve it from first panel
			{
				CIdentifier l_oFirstPanelID;
				l_pWindow->getChildIdentifier(0, l_oFirstPanelID);
				GtkTreeIter l_oFirstPanelIter;
				m_rVisualizationTree.findChildNodeFromRoot(&l_oFirstPanelIter, l_oFirstPanelID);
				void* l_pNotebookWidget = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oFirstPanelIter, l_pNotebookWidget, EVisualizationTreeColumn_PointerWidget);
				l_pTreeWidget = static_cast<GtkWidget*>(l_pNotebookWidget);
			}
		}
	}
	else if (widget->getType() == EVisualizationWidget_VerticalSplit || widget->getType() == EVisualizationWidget_HorizontalSplit ||
			 widget->getType() == EVisualizationWidget_Undefined || widget->getType() == EVisualizationWidget_VisualizationBox)
	{
		//tree widget = table containing event boxes + visualization widget in the center
		l_pTreeWidget                            = GTK_WIDGET(newWidgetsTable());
		GtkWidget* l_pCurrentVisualizationWidget = getVisualizationWidget(l_pTreeWidget);
		if (l_pCurrentVisualizationWidget != nullptr) { gtk_container_remove(GTK_CONTAINER(l_pTreeWidget), l_pCurrentVisualizationWidget); }

		if (widget->getType() == EVisualizationWidget_VerticalSplit || widget->getType() == EVisualizationWidget_HorizontalSplit)
		{
			if (gtk_widget_get_parent(l_pTreeWidget) != nullptr) { gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pTreeWidget)), l_pTreeWidget); }

			//create a paned and insert it in table
			GtkWidget* l_pPaned = (widget->getType() == EVisualizationWidget_HorizontalSplit) ? gtk_hpaned_new() : gtk_vpaned_new();
			gtk_table_attach(GTK_TABLE(l_pTreeWidget), l_pPaned, 1, 2, 1, 2,
							 GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
							 GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
		}
		else //undefined or visualization box : visualization widget is a GtkButton (left : icon, right : label)
		{
			if (gtk_widget_get_parent(l_pTreeWidget) != nullptr) { gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pTreeWidget)), l_pTreeWidget); }

			//create a button and insert it in table
			GtkWidget* l_pButton = gtk_button_new();
			gtk_widget_set_size_request(l_pButton, 0, 0);
			gtk_table_attach(GTK_TABLE(l_pTreeWidget), l_pButton, 1, 2, 1, 2,
							 GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
							 GtkAttachOptions(GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

			//box inserted in button
			GtkBox* l_pBox = GTK_BOX(gtk_vbox_new(FALSE, 0));
			gtk_widget_set_size_request(GTK_WIDGET(l_pBox), 0, 0);

			//icon - actual icon will be loaded in endLoadTreeWidget
			GtkWidget* icon = gtk_image_new_from_stock(getTreeWidgetIcon(EVisualizationTreeNode_Undefined), GTK_ICON_SIZE_BUTTON);
			if (s_iconWidthRequest == 0) { gtk_widget_set_size_request(icon, 0, 0); }
			gtk_box_pack_start(l_pBox, icon, s_iconExpand, s_iconFill, 0);

			//label
			GtkWidget* l_pLabel = gtk_label_new(static_cast<const char*>(widget->getName()));
			if (s_labelWidthRequest == 0) { gtk_widget_set_size_request(l_pLabel, 0, 0); }
			gtk_box_pack_start(l_pBox, l_pLabel, s_labelExpand, s_labelFill, 0);

			//add box to button
			gtk_container_add(GTK_CONTAINER(l_pButton), GTK_WIDGET(l_pBox));

			//set up button as drag destination
			gtk_drag_dest_set(l_pButton, GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
			g_signal_connect(G_OBJECT(l_pButton), "drag_data_received", G_CALLBACK(drag_data_received_in_widget_cb), this);

			//set up button as drag source as well
			gtk_drag_source_set(l_pButton, GDK_BUTTON1_MASK, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
			g_signal_connect(G_OBJECT(l_pButton), "drag_data_get", G_CALLBACK(drag_data_get_from_widget_cb), this);

			//ask for notification of some events
			if (widget->getType() == EVisualizationWidget_VisualizationBox)
			{
				GTK_WIDGET_SET_FLAGS(l_pButton, GDK_KEY_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
				g_signal_connect(G_OBJECT(l_pButton), "key-press-event", G_CALLBACK(visualization_widget_key_press_event_cb), this);
				g_signal_connect(G_OBJECT(l_pButton), "enter-notify-event", G_CALLBACK(visualization_widget_enter_notify_event_cb), this);
				g_signal_connect(G_OBJECT(l_pButton), "leave-notify-event", G_CALLBACK(visualization_widget_leave_notify_event_cb), this);
			}
		}

		//parent widget to its parent, if any
		//-----------------------------------
		IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(widget->getParentIdentifier());
		if (l_pParentVisualizationWidget != nullptr) //visualization boxes may be unparented
		{
			GtkTreeIter l_oParentIter;
			m_rVisualizationTree.findChildNodeFromRoot(&l_oParentIter, l_pParentVisualizationWidget->getIdentifier());

			if (l_pParentVisualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
			{
				//parent widget to notebook as a new page
				void* l_pNotebook = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oParentIter, l_pNotebook, EVisualizationTreeColumn_PointerWidget);
				char* l_pVisualizationPanelName = nullptr;
				m_rVisualizationTree.getStringValueFromTreeIter(&l_oParentIter, l_pVisualizationPanelName, EVisualizationTreeColumn_StringName);
				gtk_notebook_append_page(GTK_NOTEBOOK(l_pNotebook), l_pTreeWidget, gtk_label_new(l_pVisualizationPanelName));
			}
			else if (l_pParentVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || l_pParentVisualizationWidget->getType() ==
					 EVisualizationWidget_HorizontalSplit)
			{
				//insert widget in parent paned
				void* l_pParentTreeWidget = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oParentIter, l_pParentTreeWidget, EVisualizationTreeColumn_PointerWidget);


				if (l_pParentTreeWidget != nullptr && GTK_IS_WIDGET(l_pParentTreeWidget))
				{
					GtkWidget* l_pParentWidget = getVisualizationWidget(GTK_WIDGET(l_pParentTreeWidget));
					if (l_pParentWidget != nullptr && GTK_IS_PANED(l_pParentWidget))
					{
						uint32_t l_ui32ChildIdx;
						if (l_pParentVisualizationWidget->getChildIndex(widget->getIdentifier(), l_ui32ChildIdx))
						{
							if (l_ui32ChildIdx == 0) { gtk_paned_pack1(GTK_PANED(l_pParentWidget), l_pTreeWidget, TRUE, TRUE); }
							else { gtk_paned_pack2(GTK_PANED(l_pParentWidget), l_pTreeWidget, TRUE, TRUE); }
						}
					}
				}
			}
		}
	}

	//resize widgets once they are allocated : this is the case when they are shown on an expose event
	//FIXME : perform resizing only once (when it is done as many times as there are widgets in the tree here)
	if (l_pTreeWidget != nullptr)
	{
		gtk_signal_connect(GTK_OBJECT(getVisualizationWidget(l_pTreeWidget)), "expose-event", G_CALLBACK(widget_expose_event_cb), this);
	}

	return l_pTreeWidget;
}

void CDesignerVisualization::endLoadTreeWidget(IVisualizationWidget* widget)
{
	//retrieve tree widget
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, widget->getIdentifier());
	void* l_pTreeWidget = nullptr;
	m_rVisualizationTree.getPointerValueFromTreeIter(&l_oIter, l_pTreeWidget, EVisualizationTreeColumn_PointerWidget);

	//get actual visualization widget
	GtkWidget* l_widget = getVisualizationWidget(static_cast<GtkWidget*>(l_pTreeWidget));

	if (widget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		//reposition paned widget handles
		resizeCB(nullptr);
	}
	else if (widget->getType() == EVisualizationWidget_Undefined || widget->getType() == EVisualizationWidget_VisualizationBox)
	{
		if (GTK_IS_BUTTON(l_widget) != FALSE)
		{
			//replace dummy icon with correct one
			//-----------------------------------
			//retrieve icon name from tree
			char* iconString = nullptr;
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oIter, iconString, EVisualizationTreeColumn_StringStockIcon);
			//retrieve hbox
			GList* l_pButtonChildren = gtk_container_get_children(GTK_CONTAINER(l_widget));
			GtkContainer* l_pBox     = GTK_CONTAINER(l_pButtonChildren->data);
			//remove first widget
			GList* l_pBoxChildren = gtk_container_get_children(l_pBox);
			gtk_container_remove(l_pBox, GTK_WIDGET(l_pBoxChildren->data));
			//create new icon
			GtkWidget* icon = gtk_image_new_from_stock(iconString, GTK_ICON_SIZE_BUTTON);
			if (s_iconWidthRequest == 0) { gtk_widget_set_size_request(icon, 0, 0); }
			gtk_box_pack_start(GTK_BOX(l_pBox), icon, s_iconExpand, s_iconFill, 0);
			//insert it in first position
			gtk_box_reorder_child(GTK_BOX(l_pBox), icon, 0);
		}
	}
}

GtkWidget* CDesignerVisualization::getTreeWidget(GtkWidget* widget) { return gtk_widget_get_parent(widget); }

GtkWidget* CDesignerVisualization::getVisualizationWidget(GtkWidget* widget)
{
	if (GTK_IS_TABLE(widget)) { return getTableChild(GTK_TABLE(widget), 1, 2, 1, 2)->widget; }
	return widget;
}

const char* CDesignerVisualization::getTreeWidgetIcon(const EVisualizationTreeNode type)
{
	switch (type)
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

gboolean CDesignerVisualization::delete_event_cb(GtkWidget* /*widget*/, GdkEvent* /*event*/, gpointer data)
{
	return static_cast<CDesignerVisualization*>(data)->deleteEventCB() ? TRUE : FALSE;
}

bool CDesignerVisualization::deleteEventCB() const
{
	if (m_fpDeleteEventCB != nullptr)
	{
		m_fpDeleteEventCB(m_pDeleteEventUserData);
		return true;
	}

	return false;
}

#ifdef HANDLE_MIN_MAX_EVENTS
gboolean CDesignerVisualization::window_state_event_cb(::GtkWidget * widget, GdkEventWindowState * event, gpointer data)
{
	//refresh widgets if window was maximized or minimized
	if (event->changed_mask& GDK_WINDOW_STATE_MAXIMIZED ||
		event->changed_mask& GDK_WINDOW_STATE_ICONIFIED)
	{
		//widgets haven't been reallocated yet, perform resizing only when this happens
		//gtk_signal_connect(GTK_OBJECT(gtk_paned_get_child2(GTK_PANED(m_pPane))), "size-allocate", G_CALLBACK(widget_size_allocate_cb), this);
		gtk_signal_connect(GTK_OBJECT(gtk_paned_get_child2(GTK_PANED(m_pPane))), "expose-event", G_CALLBACK(widget_expose_cb), this);
	}

	return FALSE;
}
#endif

//event generated whenever window size changes, including when it is first created
gboolean CDesignerVisualization::configure_event_cb(GtkWidget* /*widget*/, GdkEventConfigure* /*event*/, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->m_bPreviewWindowVisible = true;
	/*
	//upon first show, resize window so that the preview widget has the desired size
	if(m_bFirstShow == true)
	{
		//set preview widget size
		::GtkWidget* l_pNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
		//gtk_window_resize(m_iInitialWidth, m_iInitialHeight);

		m_bFirstShow == false;
	}*/

	static_cast<CDesignerVisualization*>(data)->resizeCB(nullptr);

	return FALSE;
}

gboolean CDesignerVisualization::widget_expose_event_cb(GtkWidget* widget, GdkEventExpose* /*event*/, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->m_bPreviewWindowVisible = true;

	g_signal_handlers_disconnect_by_func(G_OBJECT(widget), G_CALLBACK2(CDesignerVisualization::widget_expose_event_cb), data);

	static_cast<CDesignerVisualization*>(data)->resizeCB(nullptr);

	return FALSE;
}

void CDesignerVisualization::resizeCB(IVisualizationWidget* pVisualizationWidget)
{
	if (pVisualizationWidget == nullptr)
	{
		//assign current window size to each window
		GtkWidget* l_pNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
		if (l_pNotebook != nullptr)
		{
			CIdentifier l_oVisualizationWindowID = OV_UndefinedIdentifier;
			//retrieve current preview window size, if window is visible
			if (m_bPreviewWindowVisible)
			{
				l_pNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
				if (l_pNotebook != nullptr)
				{
					//update preview window dims
					m_previewWindowW = l_pNotebook->allocation.width;
					m_previewWindowH = l_pNotebook->allocation.height;
				}
			}

			while (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
			{
				IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);

				//store new dimensions
				l_pVisualizationWindow->setWidth(m_previewWindowW);
				l_pVisualizationWindow->setHeight(m_previewWindowH);
			}
		}
		else
		{
			//return; //?
		}

		//retrieve active visualization panel
		GtkTreeIter l_oWindowIter;
		if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oWindowIter, m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow))
		{
			return;
		}
		GtkTreeIter l_oPanelIter = l_oWindowIter;
		if (!m_rVisualizationTree.findChildNodeFromParent(&l_oPanelIter, m_oActiveVisualizationPanelName, EVisualizationTreeNode_VisualizationPanel))
		{
			return;
		}
		CIdentifier l_oVisualizationPanelID;
		if (!m_rVisualizationTree.getIdentifierFromTreeIter(&l_oPanelIter, l_oVisualizationPanelID, EVisualizationTreeColumn_StringIdentifier))
		{
			return;
		}
		IVisualizationWidget* l_pVisualizationPanel = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationPanelID);

		//resize visualization panel hierarchy
		if (l_pVisualizationPanel != nullptr)
		{
			CIdentifier l_oChildID;
			l_pVisualizationPanel->getChildIdentifier(0, l_oChildID);
			IVisualizationWidget* l_pChildVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oChildID);
			if (l_pChildVisualizationWidget != nullptr) { resizeCB(l_pChildVisualizationWidget); }
		}
	}
	else if (pVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit)
	{
		GtkTreeIter l_oIter;
		if (m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, pVisualizationWidget->getIdentifier()) == TRUE)
		{
			//retrieve paned widget
			void* treeWidget = nullptr;
			m_rVisualizationTree.getPointerValueFromTreeIter(&l_oIter, treeWidget, EVisualizationTreeColumn_PointerWidget);
			GtkWidget* paned = getVisualizationWidget(GTK_WIDGET(treeWidget));
			enablePanedSignals(paned, false);

			//retrieve paned attributes
			const int handlePos    = pVisualizationWidget->getDividerPosition();
			const int maxHandlePos = pVisualizationWidget->getMaxDividerPosition();

			if (handlePos == std::numeric_limits<int>::min() || maxHandlePos == std::numeric_limits<int>::min())
			{
				// these variables hadn't been initialized meaningfully before. @fixme what is the correct place to init them?
				notifyPositionPanedCB(paned); // for now, this inits them as a side effect
			}
			if (maxHandlePos > 0)
			{
				//retrieve current maximum handle position
				const int l_i32CurrentMaxHandlePos = GTK_IS_VPANED(paned) ? GTK_PANED(paned)->container.widget.allocation.height
														 : GTK_PANED(paned)->container.widget.allocation.width;

				//set new paned handle position
				gtk_paned_set_position(GTK_PANED(paned), handlePos * l_i32CurrentMaxHandlePos / maxHandlePos);
			}

			enablePanedSignals(paned, true);

			//go down child 1
			CIdentifier l_oChildID;
			pVisualizationWidget->getChildIdentifier(0, l_oChildID);
			IVisualizationWidget* l_pChildVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oChildID);
			if (l_pChildVisualizationWidget != nullptr) { resizeCB(l_pChildVisualizationWidget); }

			//go down child 2
			pVisualizationWidget->getChildIdentifier(1, l_oChildID);
			l_pChildVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oChildID);
			if (l_pChildVisualizationWidget != nullptr) { resizeCB(l_pChildVisualizationWidget); }
		}
	}
}

void CDesignerVisualization::notebook_page_switch_cb(GtkNotebook* notebook, GtkNotebookPage* /*page*/, const guint pagenum, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->notebookPageSelectedCB(notebook, pagenum);
}

gboolean CDesignerVisualization::notify_position_paned_cb(GtkWidget* widget, GParamSpec* /*spec*/, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->notifyPositionPanedCB(widget);
	return TRUE;
}

//--------------------------
//Event box table management
//--------------------------

void CDesignerVisualization::setupNewEventBoxTable(GtkBuilder* xml)
{
	//set up event boxes as drag targets
	gtk_drag_dest_set(
		GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox2")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry),
		GDK_ACTION_COPY);
	gtk_drag_dest_set(
		GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox4")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry),
		GDK_ACTION_COPY);
	gtk_drag_dest_set(
		GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox6")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry),
		GDK_ACTION_COPY);
	gtk_drag_dest_set(
		GTK_WIDGET(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox8")), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry),
		GDK_ACTION_COPY);

	//set up event boxes callbacks for drag data received events
	char buf[256];
	sprintf(buf, "%p %s", this, "top");
	m_sTopEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox2")), "drag_data_received",
					 G_CALLBACK(drag_data_received_in_event_box_cb), gpointer(m_sTopEventBoxData.c_str()));
	sprintf(buf, "%p %s", this, "left");
	m_sLeftEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox4")), "drag_data_received",
					 G_CALLBACK(drag_data_received_in_event_box_cb), gpointer(m_sLeftEventBoxData.c_str()));
	sprintf(buf, "%p %s", this, "right");
	m_sRightEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox6")), "drag_data_received",
					 G_CALLBACK(drag_data_received_in_event_box_cb), gpointer(m_sRightEventBoxData.c_str()));
	sprintf(buf, "%p %s", this, "bottom");
	m_sBottomEventBoxData = buf;
	g_signal_connect(G_OBJECT(gtk_builder_get_object(xml, "window_manager_eventbox-eventbox8")), "drag_data_received",
					 G_CALLBACK(drag_data_received_in_event_box_cb), gpointer(m_sBottomEventBoxData.c_str()));
}

void CDesignerVisualization::refreshActiveVisualization(GtkTreePath* pSelectedItemPath)
{
	//show tree
	gtk_tree_view_expand_all(m_pTreeView);

	//select item
	if (pSelectedItemPath != nullptr) { gtk_tree_view_set_cursor(m_pTreeView, pSelectedItemPath, nullptr, false); }
	else //select previous visualization tab again (or another tab if it doesn't exist anymore)
	{
		setActiveVisualization(m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName);
	}
}

void CDesignerVisualization::setActiveVisualization(const char* activeWindow, const char* activePanel)
{
	//ensures correct behavior when _active[Window/Panel] point to m_oActiveVisualization[Window/Panel]Name.m_pSecretImplementation
	const CString window = activeWindow;
	const CString panel  = activePanel;

	//clear active window/panel names
	m_oActiveVisualizationWindowName = "";
	m_oActiveVisualizationPanelName  = "";

	//retrieve active window
	GtkTreeIter l_oWindowIter;

	if (m_rVisualizationTree.findChildNodeFromRoot(&l_oWindowIter, window, EVisualizationTreeNode_VisualizationWindow))
	{
		m_oActiveVisualizationWindowName = window;
	}
	else
	{
		//pick first window if previously active window doesn't exist anymore
		CIdentifier l_oID = OV_UndefinedIdentifier;

		if (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oID, EVisualizationWidget_VisualizationWindow))
		{
			m_oActiveVisualizationWindowName = m_rVisualizationTree.getVisualizationWidget(l_oID)->getName();
			m_rVisualizationTree.findChildNodeFromRoot(&l_oWindowIter, static_cast<const char*>(m_oActiveVisualizationWindowName),
													   EVisualizationTreeNode_VisualizationWindow);
		}
		else //no windows left
		{
			if (gtk_paned_get_child2(GTK_PANED(m_pPane)) != nullptr) { gtk_container_remove(GTK_CONTAINER(m_pPane), gtk_paned_get_child2(GTK_PANED(m_pPane))); }
			return;
		}
	}

	//retrieve active panel
	GtkTreeIter l_oPanelIter = l_oWindowIter;
	if (m_rVisualizationTree.findChildNodeFromParent(&l_oPanelIter, panel, EVisualizationTreeNode_VisualizationPanel))
	{
		m_oActiveVisualizationPanelName = panel;
	}
	else //couldn't find panel : select first one
	{
		CIdentifier l_oWindowID;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oWindowIter, l_oWindowID, EVisualizationTreeColumn_StringIdentifier);
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oWindowID);
		CIdentifier l_oPanelID;
		if (l_pVisualizationWindow->getChildIdentifier(0, l_oPanelID))
		{
			l_oPanelIter = l_oWindowIter;
			m_rVisualizationTree.findChildNodeFromParent(&l_oPanelIter, l_oPanelID);
			char* l_pString = nullptr;
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oPanelIter, l_pString, EVisualizationTreeColumn_StringName);
			m_oActiveVisualizationPanelName = l_pString;
		}
		else //no panel in window
		{
			GtkWidget* l_pCurrentNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
			if (l_pCurrentNotebook != nullptr)
			{
				gtk_object_ref(GTK_OBJECT(l_pCurrentNotebook));
				gtk_container_remove(GTK_CONTAINER(m_pPane), l_pCurrentNotebook);
			}
			return;
		}
	}

	//retrieve notebook	and set it visible
	void* l_pNotebook = nullptr;
	m_rVisualizationTree.getPointerValueFromTreeIter(&l_oPanelIter, l_pNotebook, EVisualizationTreeColumn_PointerWidget);
	GtkWidget* l_pCurrentNotebook = gtk_paned_get_child2(GTK_PANED(m_pPane));
	if (l_pCurrentNotebook != GTK_WIDGET(l_pNotebook))
	{
		if (l_pCurrentNotebook != nullptr)
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
	for (i = 0; i < gtk_notebook_get_n_pages(GTK_NOTEBOOK(l_pNotebook)); ++i)
	{
		if (strcmp(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(l_pNotebook),
												   gtk_notebook_get_nth_page(GTK_NOTEBOOK(l_pNotebook), i)), m_oActiveVisualizationPanelName) == 0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(l_pNotebook), i);
			break;
		}
	}

	//if active page couldn't be found
	if (i == gtk_notebook_get_n_pages(GTK_NOTEBOOK(l_pNotebook)))
	{
		//error!
		//pick first page if it exists
		if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(l_pNotebook)) > 0)
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
GtkTable* CDesignerVisualization::newWidgetsTable()
{
	//@FIXME is the memory ever freed? Valgrind is suspicious about this. It seems that a builder is allocated, but only a member of builder is returned as GtkTable*.
	GtkBuilder* pGtkBuilderTable = gtk_builder_new(); // glade_xml_new(m_sGuiFile.c_str(), "window_manager_eventbox-table", nullptr);
	gtk_builder_add_from_file(pGtkBuilderTable, m_sGuiFile.c_str(), nullptr);
	gtk_builder_connect_signals(pGtkBuilderTable, nullptr);

	//set up event boxes
	setupNewEventBoxTable(pGtkBuilderTable);

	GtkTable* pTable = GTK_TABLE(gtk_builder_get_object(pGtkBuilderTable, "window_manager_eventbox-table"));

	//clear central button label
	GtkTableChild* pTC = getTableChild(pTable, 1, 2, 1, 2);
	GtkButton* button  = GTK_BUTTON(pTC->widget);
	gtk_button_set_label(button, "");

	//set it up as drag destination
	gtk_drag_dest_set(GTK_WIDGET(button), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(button), "drag_data_received", G_CALLBACK(drag_data_received_in_widget_cb), this);

	//set it up as drag source as well
	gtk_drag_source_set(GTK_WIDGET(button), GDK_BUTTON1_MASK, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(button), "drag_data_get", G_CALLBACK(drag_data_get_from_widget_cb), this);

	return pTable;
}

void CDesignerVisualization::askNewVisualizationWindow()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::new_visualization_window_cb, this, "New window", "Please enter name of new window : ");

	id.run();
}

bool CDesignerVisualization::newVisualizationWindow(const char* label)
{
	//ensure name is unique
	IVisualizationWidget* l_pVisualizationWindow;
	CIdentifier l_oVisualizationWindowID = OV_UndefinedIdentifier;

	while (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
	{
		l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);

		if (strcmp(static_cast<const char*>(l_pVisualizationWindow->getName()), label) == 0)
		{
			displayErrorDialog("Window creation failed !", "An existing window already uses this name. Please choose another name.");
			return false;
		}
	}

	//proceed with window creation
	//m_rVisualizationTree.addVisualizationWindow(l_oVisualizationWindowID, CString(label));
	m_rVisualizationTree.addVisualizationWidget(l_oVisualizationWindowID, CString(label), EVisualizationWidget_VisualizationWindow,
												OV_UndefinedIdentifier, 0, OV_UndefinedIdentifier, 0, OV_UndefinedIdentifier);

	l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);

	//add attributes
	l_pVisualizationWindow->setWidth(1);
	l_pVisualizationWindow->setHeight(1);

	//create default visualization panel as well
	CIdentifier l_oChildID;
	const CString l_oChildName = "Default tab";

	m_rVisualizationTree.addVisualizationWidget(l_oChildID, l_oChildName, EVisualizationWidget_VisualizationPanel,
												l_oVisualizationWindowID, 0, OV_UndefinedIdentifier, 1, OV_UndefinedIdentifier);

	m_rVisualizationTree.reloadTree();

	//refresh view
	GtkTreeIter l_oChildIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oChildIter, l_oChildID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oChildIter));

	return true;
}

void CDesignerVisualization::askRenameVisualizationWindow()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::rename_visualization_window_cb, this, "Rename window", "Please enter new name of window : ");

	id.run();
}

bool CDesignerVisualization::renameVisualizationWindow(const char* label)
{
	//retrieve visualization window
	GtkTreeIter l_oIter;
	if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, m_oActiveVisualizationWindowName, EVisualizationTreeNode_VisualizationWindow))
	{
		displayErrorDialog("Window renaming failed !", "Couldn't retrieve window.");
		return false;
	}

	CIdentifier l_oWindowID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oWindowID, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oWindowID);
	if (l_pVisualizationWindow == nullptr)
	{
		displayErrorDialog("Window renaming failed !", "Couldn't retrieve window.");
		return false;
	}

	//if trying to set identical name, return
	const CString l_oNewWindowName = label;
	if (l_pVisualizationWindow->getName() == l_oNewWindowName) { return true; }

	//ensure name is unique
	CIdentifier l_oVisualizationWindowID = OV_UndefinedIdentifier;
	while (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
	{
		//name already in use : warn user
		if (m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID)->getName() == l_oNewWindowName)
		{
			displayErrorDialog("Window renaming failed !", "An existing window already uses this name. Please choose another name.");
			return false;
		}
	}

	//change its name
	l_pVisualizationWindow->setName(l_oNewWindowName);

	m_rVisualizationTree.reloadTree();

	//refresh view
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oWindowID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));

	return true;
}

bool CDesignerVisualization::removeVisualizationWindow()
{
	//retrieve visualization window
	CIdentifier l_oVisualizationWindowID = OV_UndefinedIdentifier;
	while (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
	{
		if (m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID)->getName() == m_oActiveVisualizationWindowName) { break; }
	}

	//return if window was not found
	if (l_oVisualizationWindowID == OV_UndefinedIdentifier)
	{
		displayErrorDialog("Window removal failed !", "Couldn't retrieve window.");
		return false;
	}

	//destroy hierarchy but only unaffect visualization boxes
	m_rVisualizationTree.destroyHierarchy(l_oVisualizationWindowID, false);

	m_rVisualizationTree.reloadTree();

	//refresh view
	refreshActiveVisualization(nullptr);

	return true;
}

void CDesignerVisualization::askNewVisualizationPanel()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::new_visualization_panel_cb, this, "New tab", "Please enter name of new tab : ");

	id.run();
}

bool CDesignerVisualization::newVisualizationPanel(const char* label)
{
	//retrieve visualization window
	IVisualizationWidget* l_pVisualizationWindow = nullptr;
	CIdentifier l_oVisualizationWindowID = OV_UndefinedIdentifier;

	while (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(l_oVisualizationWindowID, EVisualizationWidget_VisualizationWindow))
	{
		l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);
		if (l_pVisualizationWindow->getName() == m_oActiveVisualizationWindowName) { break; }
	}

	//return if parent window was not found
	if (l_oVisualizationWindowID == OV_UndefinedIdentifier || l_pVisualizationWindow == nullptr)
	{
		displayErrorDialog("Tab creation failed !", "Couldn't retrieve parent window.");
		return false;
	}

	CIdentifier l_oChildID;
	const CString l_oNewChildName = label;

	//ensure visualization panel name is unique in this window
	for (uint32_t i = 0; i < l_pVisualizationWindow->getNbChildren(); ++i)
	{
		l_pVisualizationWindow->getChildIdentifier(i, l_oChildID);
		if (m_rVisualizationTree.getVisualizationWidget(l_oChildID)->getName() == l_oNewChildName)
		{
			displayErrorDialog("Tab creation failed !", "An existing tab already uses this name. Please choose another name.");
			return false;
		}
	}

	//proceed with panel creation
	m_rVisualizationTree.addVisualizationWidget(l_oChildID, l_oNewChildName, EVisualizationWidget_VisualizationPanel,
												l_oVisualizationWindowID, l_pVisualizationWindow->getNbChildren(), OV_UndefinedIdentifier, 1,
												OV_UndefinedIdentifier);

	m_rVisualizationTree.reloadTree();

	//refresh view
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oChildID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));

	return true;
}

void CDesignerVisualization::askRenameVisualizationPanel()
{
	//show dialog
	CInputDialog id(m_sGuiFile.c_str(), &CDesignerVisualization::rename_visualization_panel_cb, this, "Rename tab", "Please enter new name of tab : ");

	id.run();
}

bool CDesignerVisualization::renameVisualizationPanel(const char* label)
{
	//retrieve visualization window
	GtkTreeIter l_oIter;
	if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, static_cast<const char*>(m_oActiveVisualizationWindowName),
													EVisualizationTreeNode_VisualizationWindow))
	{
		displayErrorDialog("Tab renaming failed !", "Couldn't retrieve parent window.");
		return false;
	}
	CIdentifier l_oVisualizationWindowID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oVisualizationWindowID, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);
	if (l_pVisualizationWindow == nullptr)
	{
		displayErrorDialog("Tab renaming failed !", "Couldn't retrieve parent window.");
		return false;
	}

	//retrieve visualization panel
	if (!m_rVisualizationTree.findChildNodeFromParent(&l_oIter, static_cast<const char*>(m_oActiveVisualizationPanelName),
													  EVisualizationTreeNode_VisualizationPanel))
	{
		displayErrorDialog("Tab renaming failed !", "Couldn't retrieve tab.");
		return false;
	}

	CIdentifier l_oVisualizationPanelID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oVisualizationPanelID, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationPanelID);
	if (l_pVisualizationWidget == nullptr)
	{
		displayErrorDialog("tab renaming failed !", "Couldn't retrieve tab.");
		return false;
	}

	//if trying to set identical name, return
	const CString l_oNewPanelName = label;
	if (l_pVisualizationWidget->getName() == l_oNewPanelName) { return true; }

	//ensure visualization panel name is unique in this window
	CIdentifier l_oChildID;
	for (uint32_t i = 0; i < l_pVisualizationWindow->getNbChildren(); ++i)
	{
		l_pVisualizationWindow->getChildIdentifier(i, l_oChildID);
		if (m_rVisualizationTree.getVisualizationWidget(l_oChildID)->getName() == l_oNewPanelName)
		{
			displayErrorDialog("Tab renaming failed !", "An existing tab already uses this name. Please choose another name.");
			return false;
		}
	}

	l_pVisualizationWidget->setName(l_oNewPanelName);

	m_rVisualizationTree.reloadTree();

	//refresh view
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oVisualizationPanelID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter));

	return true;
}

bool CDesignerVisualization::removeVisualizationPanel()
{
	//retrieve visualization window
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, static_cast<const char*>(m_oActiveVisualizationWindowName),
											   EVisualizationTreeNode_VisualizationWindow);

	//retrieve visualization panel
	m_rVisualizationTree.findChildNodeFromParent(&l_oIter, static_cast<const char*>(m_oActiveVisualizationPanelName),
												 EVisualizationTreeNode_VisualizationPanel);
	CIdentifier l_oVisualizationPanelID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oVisualizationPanelID, EVisualizationTreeColumn_StringIdentifier);

	//destroy hierarchy but only unaffect visualization boxes (as opposed to destroying them)
	if (!m_rVisualizationTree.destroyHierarchy(m_rVisualizationTree.getVisualizationWidget(l_oVisualizationPanelID)->getIdentifier(), false))
	{
		displayErrorDialog("Tab removal failed !", "An error occurred while destroying widget hierarchy.");
		return false;
	}

	m_rVisualizationTree.reloadTree();

	//refresh view
	refreshActiveVisualization(nullptr);

	return true;
}

bool CDesignerVisualization::removeVisualizationWidget()
{
	//retrieve widget
	GtkTreeIter l_oIter;
	if (!m_rVisualizationTree.getTreeSelection(m_pTreeView, &l_oIter)) { return false; }
	CIdentifier l_oID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oID, EVisualizationTreeColumn_StringIdentifier);
	return removeVisualizationWidget(l_oID);
}

//TODO : move this to CVisualizationTree?
bool CDesignerVisualization::removeVisualizationWidget(const CIdentifier& identifier)
{
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(identifier);
	if (l_pVisualizationWidget == nullptr) { return false; }

	IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_pVisualizationWidget->getParentIdentifier());

	//unparent or destroy widget
	uint32_t l_ui32ChildIdx;
	m_rVisualizationTree.unparentVisualizationWidget(identifier, l_ui32ChildIdx);
	if (l_pVisualizationWidget->getType() != EVisualizationWidget_VisualizationBox) { m_rVisualizationTree.destroyHierarchy(identifier, false); }

	//reparent other child widget, if any
	if (l_pParentVisualizationWidget->getType() != EVisualizationWidget_VisualizationPanel)
	{
		//retrieve parent's other widget
		CIdentifier l_oOtherVisualizationWidgetID;
		l_pParentVisualizationWidget->getChildIdentifier(1 - l_ui32ChildIdx, l_oOtherVisualizationWidgetID);

		//unparent parent
		uint32_t l_ui32ParentIdx;
		const CIdentifier l_oParentParentID = l_pParentVisualizationWidget->getParentIdentifier();
		m_rVisualizationTree.unparentVisualizationWidget(l_pParentVisualizationWidget->getIdentifier(), l_ui32ParentIdx);

		//reparent other widget to its grandparent
		m_rVisualizationTree.unparentVisualizationWidget(l_oOtherVisualizationWidgetID, l_ui32ChildIdx);
		m_rVisualizationTree.parentVisualizationWidget(l_oOtherVisualizationWidgetID, l_oParentParentID, l_ui32ParentIdx);

		//destroy parent
		m_rVisualizationTree.destroyHierarchy(l_pParentVisualizationWidget->getIdentifier(), false);
	}

	m_rVisualizationTree.reloadTree();

	//refresh view
	refreshActiveVisualization(nullptr);

	return true;
}

bool CDesignerVisualization::destroyVisualizationWidget(const CIdentifier& identifier)
{
	const bool b = removeVisualizationWidget(identifier);
	m_rVisualizationTree.destroyHierarchy(identifier, true);
	return b;
}

//CALLBACKS
//---------

void CDesignerVisualization::notebookPageSelectedCB(GtkNotebook* notebook, const guint pagenum)
{
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, static_cast<void*>(notebook));
	CIdentifier l_oID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oID, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oID);
	if (l_pVisualizationWidget != nullptr)
	{
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_pVisualizationWidget->getParentIdentifier());
		if (l_pVisualizationWindow != nullptr)
		{
			l_pVisualizationWindow->getChildIdentifier(pagenum, l_oID);
			if (m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_oID)) { refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oIter)); }
		}
	}
}

void CDesignerVisualization::enableNotebookSignals(GtkWidget* notebook, const bool b)
{
	if (b) { g_signal_connect(G_OBJECT(notebook), "switch-page", G_CALLBACK(notebook_page_switch_cb), this); }
	else { g_signal_handlers_disconnect_by_func(G_OBJECT(notebook), G_CALLBACK2(notebook_page_switch_cb), this); }
}

void CDesignerVisualization::notifyPositionPanedCB(GtkWidget* widget)
{
	GtkPaned* l_pPaned = GTK_PANED(widget);

	//return if handle pos was changed because parent window was resized
	const int l_iPos             = gtk_paned_get_position(l_pPaned);
	const int l_iMaxPos          = GTK_IS_VPANED(l_pPaned) ? l_pPaned->container.widget.allocation.height : l_pPaned->container.widget.allocation.width;
	const int l_iHandleThickness = GTK_IS_VPANED(l_pPaned) ? l_pPaned->handle_pos.height : l_pPaned->handle_pos.width;

	if (l_iPos + l_iHandleThickness == l_iMaxPos) { return; }

	//look for widget in tree
	GtkWidget* l_pTreeWidget = getTreeWidget(widget);
	GtkTreeIter l_oIter;
	if (m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, l_pTreeWidget))
	{
		CIdentifier l_oID;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oID, EVisualizationTreeColumn_StringIdentifier);

		//store new position and max position
		auto* visualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oID);
		visualizationWidget->setDividerPosition(l_iPos);
		visualizationWidget->setMaxDividerPosition(l_iMaxPos);
	}
}

void CDesignerVisualization::enablePanedSignals(GtkWidget* pPaned, const bool b)
{
	if (b) { g_signal_connect(G_OBJECT(pPaned), "notify::position", G_CALLBACK(notify_position_paned_cb), this); }
	else { g_signal_handlers_disconnect_by_func(G_OBJECT(pPaned), G_CALLBACK2(notify_position_paned_cb), this); }
}

void CDesignerVisualization::ask_new_visualization_window_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->askNewVisualizationWindow();
}

void CDesignerVisualization::new_visualization_window_cb(GtkWidget* /*widget*/, gpointer data)
{
	CInputDialog* inputDialog = static_cast<CInputDialog*>(data);

	if (inputDialog->getUserData() != nullptr)
	{
		static_cast<CDesignerVisualization*>(inputDialog->getUserData())->newVisualizationWindow(inputDialog->getEntry());
	}
}

void CDesignerVisualization::ask_rename_visualization_window_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->askRenameVisualizationWindow();
}

void CDesignerVisualization::rename_visualization_window_cb(GtkWidget* /*widget*/, gpointer data)
{
	CInputDialog* inputDialog = static_cast<CInputDialog*>(data);

	if (inputDialog->getUserData() != nullptr)
	{
		static_cast<CDesignerVisualization*>(inputDialog->getUserData())->renameVisualizationWindow(inputDialog->getEntry());
	}
}

void CDesignerVisualization::remove_visualization_window_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->removeVisualizationWindow();
}

void CDesignerVisualization::ask_new_visualization_panel_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->askNewVisualizationPanel();
}

void CDesignerVisualization::new_visualization_panel_cb(GtkWidget* /*widget*/, gpointer data)
{
	auto* inputDialog = static_cast<CInputDialog*>(data);

	if (inputDialog->getUserData() != nullptr)
	{
		static_cast<CDesignerVisualization*>(inputDialog->getUserData())->newVisualizationPanel(inputDialog->getEntry());
	}
}

void CDesignerVisualization::ask_rename_visualization_panel_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->askRenameVisualizationPanel();
}

void CDesignerVisualization::rename_visualization_panel_cb(GtkWidget* /*widget*/, gpointer data)
{
	auto* inputDialog = static_cast<CInputDialog*>(data);

	if (inputDialog->getUserData() != nullptr)
	{
		static_cast<CDesignerVisualization*>(inputDialog->getUserData())->renameVisualizationPanel(inputDialog->getEntry());
	}
}

void CDesignerVisualization::remove_visualization_panel_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->removeVisualizationPanel();
}

void CDesignerVisualization::remove_visualization_widget_cb(gpointer data, guint /*callback_action*/, GtkWidget* /*widget*/)
{
	static_cast<CDesignerVisualization*>(data)->removeVisualizationWidget();
}

void CDesignerVisualization::visualization_widget_key_press_event_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->visualizationWidgetKeyPressEventCB(widget, event);
}

void CDesignerVisualization::visualizationWidgetKeyPressEventCB(GtkWidget*, GdkEventKey* eventKey)
{
	//remove widget
	if (eventKey->keyval == GDK_Delete || eventKey->keyval == GDK_KP_Delete)
	{
		if (m_pHighlightedWidget != nullptr)
		{
			GtkTreeIter l_oIter;
			if (m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, getTreeWidget(m_pHighlightedWidget)))
			{
				CIdentifier l_oID;
				m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oID, EVisualizationTreeColumn_StringIdentifier);
				removeVisualizationWidget(l_oID);
			}
		}
	}
}

gboolean CDesignerVisualization::visualization_widget_enter_notify_event_cb(GtkWidget* widget, GdkEventCrossing* eventCrossing, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->visualizationWidgetEnterNotifyEventCB(widget, eventCrossing);
	return FALSE;
}

void CDesignerVisualization::visualizationWidgetEnterNotifyEventCB(GtkWidget* widget, GdkEventCrossing* /*eventCrossing*/) { m_pHighlightedWidget = widget; }

gboolean CDesignerVisualization::visualization_widget_leave_notify_event_cb(GtkWidget* widget, GdkEventCrossing* eventCrossing, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->visualizationWidgetLeaveNotifyEventCB(widget, eventCrossing);
	return FALSE;
}

void CDesignerVisualization::visualizationWidgetLeaveNotifyEventCB(GtkWidget* /*widget*/, GdkEventCrossing* /*eventCrossing*/)
{
	m_pHighlightedWidget = nullptr;
}

gboolean CDesignerVisualization::button_release_cb(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->buttonReleaseCB(widget, event);

	return FALSE;
}

void CDesignerVisualization::buttonReleaseCB(GtkWidget* widget, GdkEventButton* event) const
{
	if (GTK_IS_TREE_VIEW(widget))
	{
		if (event->button == 3) //right button
		{
			if (event->type != GDK_BUTTON_PRESS)
			{
				GtkTreeIter l_oIter;

				if (!m_rVisualizationTree.getTreeSelection(m_pTreeView, &l_oIter)) { return; }

				const unsigned long type = m_rVisualizationTree.getULongValueFromTreeIter(&l_oIter, EVisualizationTreeColumn_ULongNodeType);

				if (type == EVisualizationTreeNode_Unaffected)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pUnaffectedItemFactory, "<unaffected_main>")), nullptr, nullptr, nullptr, nullptr,
								   event->button, event->time);
				}
				else if (type == EVisualizationTreeNode_VisualizationWindow)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pVisualizationWindowItemFactory, "<visualization_window_main>")), nullptr, nullptr,
								   nullptr, nullptr, event->button, event->time);
				}
				else if (type == EVisualizationTreeNode_VisualizationPanel)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pVisualizationPanelItemFactory, "<visualization_panel_main>")), nullptr, nullptr,
								   nullptr, nullptr, event->button, event->time);
				}
				else if (type == EVisualizationTreeNode_HorizontalSplit || type == EVisualizationTreeNode_VerticalSplit)
				{
					gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pSplitItemFactory, "<split_widget_main>")), nullptr, nullptr, nullptr, nullptr,
								   event->button, event->time);
				}
				else if (type == EVisualizationTreeNode_VisualizationBox)
				{
					//ensure visualization box is parented to a tab
					if (m_rVisualizationTree.findParentNode(&l_oIter, EVisualizationTreeNode_VisualizationPanel))
					{
						gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pVisualizationBoxItemFactory, "<visualization_box_main>")), nullptr, nullptr,
									   nullptr, nullptr, event->button, event->time);
					}
				}
				else if (type == EVisualizationTreeNode_Undefined)
				{
					//ensure empty plugin is not parented to a panel (because an empty widget is always present in an empty panel)
					CIdentifier l_oID;
					m_rVisualizationTree.getIdentifierFromTreeIter(&l_oIter, l_oID, EVisualizationTreeColumn_StringIdentifier);
					IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oID);
					if (l_pVisualizationWidget != nullptr)
					{
						IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(
							l_pVisualizationWidget->getParentIdentifier());
						if (l_pParentVisualizationWidget != nullptr)
						{
							if (l_pParentVisualizationWidget->getType() != EVisualizationWidget_VisualizationPanel)
							{
								gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(m_pUndefinedItemFactory, "<undefined_widget_main>")), nullptr, nullptr,
											   nullptr, nullptr, event->button, event->time);
							}
						}
					}
				}
			}
		}
	}
}

void CDesignerVisualization::cursor_changed_cb(GtkTreeView* pTreeView, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->cursorChangedCB(pTreeView);
}

void CDesignerVisualization::cursorChangedCB(GtkTreeView* pTreeView)
{
	//retrieve selection
	GtkTreeIter l_oSelectionIter;
	if (!m_rVisualizationTree.getTreeSelection(pTreeView, &l_oSelectionIter)) { return; }

	//save active item
	if (m_rVisualizationTree.getULongValueFromTreeIter(&l_oSelectionIter, EVisualizationTreeColumn_ULongNodeType) == EVisualizationTreeNode_VisualizationBox)
	{
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oSelectionIter, m_oActiveVisualizationBoxIdentifier, EVisualizationTreeColumn_StringIdentifier);
	}

	GtkTreeIter l_oVisualizationPanelIter = l_oSelectionIter;

	//if selection lies in a visualization panel subtree, display this subtree
	if (m_rVisualizationTree.findParentNode(&l_oVisualizationPanelIter, EVisualizationTreeNode_VisualizationPanel))
	{
		//get visualization panel name
		char* l_pVisualizationPanelName = nullptr;
		m_rVisualizationTree.getStringValueFromTreeIter(&l_oVisualizationPanelIter, l_pVisualizationPanelName, EVisualizationTreeColumn_StringName);

		//retrieve visualization window that contains selection
		GtkTreeIter l_oVisualizationWindowIter = l_oVisualizationPanelIter;
		if (m_rVisualizationTree.findParentNode(&l_oVisualizationWindowIter, EVisualizationTreeNode_VisualizationWindow))
		{
			//get its name
			char* l_pVisualizationWindowName = nullptr;
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oVisualizationWindowIter, l_pVisualizationWindowName, EVisualizationTreeColumn_StringName);

			//set active visualization
			setActiveVisualization(l_pVisualizationWindowName, l_pVisualizationPanelName);
		}
	}
	else
	{
		GtkTreeIter l_oVisualizationWindowIter = l_oSelectionIter;

		//if selection is a visualization window, display it
		if (m_rVisualizationTree.findParentNode(&l_oVisualizationWindowIter, EVisualizationTreeNode_VisualizationWindow))
		{
			//retrieve visualization window
			CIdentifier l_oVisualizationWindowID;
			m_rVisualizationTree.getIdentifierFromTreeIter(&l_oVisualizationWindowIter, l_oVisualizationWindowID,
														   EVisualizationTreeColumn_StringIdentifier);
			IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_oVisualizationWindowID);

			//if window has at least one panel
			if (l_pVisualizationWindow->getNbChildren() > 0)
			{
				//retrieve first panel
				CIdentifier l_oVisualizationPanelID;
				l_pVisualizationWindow->getChildIdentifier(0, l_oVisualizationPanelID);
				m_rVisualizationTree.findChildNodeFromParent(&l_oVisualizationPanelIter, l_oVisualizationPanelID);

				//retrieve notebook
				void* l_pNotebook = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oVisualizationPanelIter, l_pNotebook, EVisualizationTreeColumn_PointerWidget);

				//get label of its active tab
				GtkWidget* l_pCurrentPageLabel =
						gtk_notebook_get_tab_label(GTK_NOTEBOOK(l_pNotebook),
												   gtk_notebook_get_nth_page(GTK_NOTEBOOK(l_pNotebook),
																			 gtk_notebook_get_current_page(GTK_NOTEBOOK(l_pNotebook))));

				//set active visualization
				if (l_pCurrentPageLabel != nullptr)
				{
					setActiveVisualization(static_cast<const char*>(l_pVisualizationWindow->getName()), gtk_label_get_text(GTK_LABEL(l_pCurrentPageLabel)));
				}
				else { setActiveVisualization(static_cast<const char*>(l_pVisualizationWindow->getName()), nullptr); }
			}
			else //window has no panels
			{
				setActiveVisualization(static_cast<const char*>(l_pVisualizationWindow->getName()), nullptr);
			}
		}
		else
		{
			//refresh active visualization (::GtkWidgets may have changed if tree was reloaded)
			setActiveVisualization(m_oActiveVisualizationWindowName, m_oActiveVisualizationPanelName);
		}
	}
}

void CDesignerVisualization::drag_data_get_from_tree_cb(GtkWidget* pSrcWidget, GdkDragContext* /*pDragContex*/, GtkSelectionData* pSelectionData,
														guint /*uiInfo*/, guint /*uiT*/, gpointer /*data*/)
{
	dragDataGetFromTreeCB(pSrcWidget, pSelectionData);
}

void CDesignerVisualization::dragDataGetFromTreeCB(GtkWidget* pSrcWidget, GtkSelectionData* pSelectionData)
{
	char l_sString[1024];
	sprintf(l_sString, "%p", pSrcWidget);
	gtk_selection_data_set_text(pSelectionData, l_sString, gint(strlen(l_sString)));
}

void CDesignerVisualization::drag_data_get_from_widget_cb(GtkWidget* pSrcWidget, GdkDragContext* /*pDC*/, GtkSelectionData* pSelectionData,
														  guint /*uiInfo*/, guint /*uiTime*/, gpointer /*data*/)
{
	dragDataGetFromWidgetCB(pSrcWidget, pSelectionData);
}

void CDesignerVisualization::dragDataGetFromWidgetCB(GtkWidget* pSrcWidget, GtkSelectionData* pSelectionData)
{
	char l_sString[1024];
	sprintf(l_sString, "%p", pSrcWidget);
	gtk_selection_data_set_text(pSelectionData, l_sString, gint(strlen(l_sString)));
}

void CDesignerVisualization::drag_data_received_in_widget_cb(GtkWidget* dstWidget, GdkDragContext* /*pDragContext*/, gint /*iX*/, gint /*iY*/,
															 GtkSelectionData* pSelectionData, guint /*uiInfo*/, guint /*uiTime*/, gpointer data)
{
	static_cast<CDesignerVisualization*>(data)->dragDataReceivedInWidgetCB(dstWidget, pSelectionData);
}

void CDesignerVisualization::dragDataReceivedInWidgetCB(GtkWidget* dstWidget, GtkSelectionData* pSelectionData)
{
	void* l_pSrcWidget = nullptr;
	sscanf(reinterpret_cast<const char*>(gtk_selection_data_get_text(pSelectionData)), "%p", &l_pSrcWidget);
	GtkTreeIter l_oSrcIter;

	//retrieve source widget iterator
	if (GTK_IS_TREE_VIEW(l_pSrcWidget))
	{
		//ensure dragged widget is a visualization box
		if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, m_oActiveVisualizationBoxIdentifier))
		{
			m_kernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of active visualization box!\n";
			return;
		}
	}
	else if (GTK_IS_BUTTON(l_pSrcWidget))
	{
		if (l_pSrcWidget == dstWidget) { return; }
		if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, getTreeWidget(GTK_WIDGET(l_pSrcWidget))))
		{
			m_kernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of dragged button!\n";
			return;
		}
	}
	else { return; }

	//retrieve src widget identifier and src visualization widget
	CIdentifier l_oSrcID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oSrcIter, l_oSrcID, EVisualizationTreeColumn_StringIdentifier);
	IVisualizationWidget* l_pSrcVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oSrcID);
	if (l_pSrcVisualizationWidget == nullptr)
	{
		m_kernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve source visualization widget!\n";
		return;
	}

	//retrieve dest widget type
	GtkTreeIter l_oDstIter;
	if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, getTreeWidget(dstWidget)))
	{
		m_kernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't retrieve iterator of destination widget!\n";
		return;
	}

	//if src widget is unaffected or if dest widget is a visualization box, perform the drop operation directly
	if (l_pSrcVisualizationWidget->getParentIdentifier() == OV_UndefinedIdentifier ||
		m_rVisualizationTree.getULongValueFromTreeIter(&l_oDstIter, EVisualizationTreeColumn_ULongNodeType) == EVisualizationTreeNode_VisualizationBox)
	{
		m_rVisualizationTree.dragDataReceivedInWidgetCB(l_oSrcID, dstWidget);
	}
	else //dest widget is a dummy : unaffect src widget and simplify the tree before performing the drop operation
	{
		//save dest widget identifier
		CIdentifier l_oDstID;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oDstIter, l_oDstID, EVisualizationTreeColumn_StringIdentifier);

		//unaffect src widget, so that tree is simplified
		if (!removeVisualizationWidget(l_oSrcID))
		{
			m_kernelContext.getLogManager() << LogLevel_Debug << "dragDataReceivedInWidgetCB couldn't remove source widget from its parent!\n";
			return;
		}

		//then drop it
		if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, l_oDstID))
		{
			m_kernelContext.getLogManager() << LogLevel_Debug <<
					"dragDataReceivedInWidgetCB couldn't retrieve iterator of dummy destination widget to delete!\n";
			return;
		}
		void* l_pNewDstTreeWidget = nullptr;
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oDstIter, l_pNewDstTreeWidget, EVisualizationTreeColumn_PointerWidget);
		m_rVisualizationTree.dragDataReceivedInWidgetCB(l_oSrcID, getVisualizationWidget(GTK_WIDGET(l_pNewDstTreeWidget)));
	}

	//refresh view
	GtkTreeIter l_oDraggedIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oDraggedIter, l_oSrcID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oDraggedIter));
}

void CDesignerVisualization::drag_data_received_in_event_box_cb(GtkWidget* dstWidget, GdkDragContext*, gint, gint, GtkSelectionData* pSelectionData, guint,
																guint, gpointer data)
{
	char buf[1024];
	void* pDesignerVisualization = nullptr;
	sscanf(static_cast<const char*>(data), "%p %s", &pDesignerVisualization, buf);

	EDragDataLocation l_oLocation;
	if (strcmp(buf, "left") == 0) { l_oLocation = EDragData_Left; }
	else if (strcmp(buf, "right") == 0) { l_oLocation = EDragData_Right; }
	else if (strcmp(buf, "top") == 0) { l_oLocation = EDragData_Top; }
	else { l_oLocation = EDragData_Bottom; }

	static_cast<CDesignerVisualization*>(pDesignerVisualization)->dragDataReceivedInEventBoxCB(dstWidget, pSelectionData, l_oLocation);
}

void CDesignerVisualization::dragDataReceivedInEventBoxCB(GtkWidget* dstWidget, GtkSelectionData* pSelectionData, const EDragDataLocation oLocation)
{
	void* l_pSrcWidget = nullptr;
	sscanf(reinterpret_cast<const char*>(gtk_selection_data_get_text(pSelectionData)), "%p", &l_pSrcWidget);
	GtkTreeIter l_oSrcIter;

	//get iterator to src widget
	if (GTK_IS_TREE_VIEW(l_pSrcWidget))
	{
		if (!m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, m_oActiveVisualizationBoxIdentifier)) { return; }
		//get actual src widget (item being dropped) and ensure it isn't being dropped in its own table
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oSrcIter, l_pSrcWidget, EVisualizationTreeColumn_PointerWidget);
		if (l_pSrcWidget == gtk_widget_get_parent(dstWidget)) { return; }
	}
	else if (GTK_IS_BUTTON(l_pSrcWidget))
	{
		//ensure src widget isn't being dropped in its own table
		if (gtk_widget_get_parent(GTK_WIDGET(l_pSrcWidget)) == gtk_widget_get_parent(dstWidget)) { return; }
		m_rVisualizationTree.findChildNodeFromRoot(&l_oSrcIter, getTreeWidget(GTK_WIDGET(l_pSrcWidget)));
	}
	else { return; }

	//ensure src widget is a visualization box
	if (m_rVisualizationTree.getULongValueFromTreeIter(&l_oSrcIter, EVisualizationTreeColumn_ULongNodeType) != EVisualizationTreeNode_VisualizationBox)
	{
		return;
	}

	//retrieve src widget identifier
	CIdentifier l_oSrcID;
	m_rVisualizationTree.getIdentifierFromTreeIter(&l_oSrcIter, l_oSrcID, EVisualizationTreeColumn_StringIdentifier);

	//if widget is unaffected, just drag n drop it
	GtkTreeIter l_oUnaffectedIter = l_oSrcIter;
	if (m_rVisualizationTree.findParentNode(&l_oUnaffectedIter, EVisualizationTreeNode_Unaffected))
	{
		m_rVisualizationTree.dragDataReceivedOutsideWidgetCB(l_oSrcID, dstWidget, oLocation);
	}
	else
	{
		//save dest widget identifier
		GtkTreeIter l_oDstIter;
		m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, getTreeWidget(dstWidget));
		CIdentifier l_oDstID;
		m_rVisualizationTree.getIdentifierFromTreeIter(&l_oDstIter, l_oDstID, EVisualizationTreeColumn_StringIdentifier);

		//if dest widget is src widget's parent (paned widget), drop src widget in corresponding event box of parent's other child
		//(otherwise, DND will fail due to parent's removal during tree simplification process)
		IVisualizationWidget* l_pSrcVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oSrcID);
		if (l_pSrcVisualizationWidget->getParentIdentifier() == l_oDstID)
		{
			IVisualizationWidget* l_pSrcParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(
				l_pSrcVisualizationWidget->getParentIdentifier());
			l_pSrcParentVisualizationWidget->getChildIdentifier(0, l_oDstID);
			if (l_oSrcID == l_oDstID) { l_pSrcParentVisualizationWidget->getChildIdentifier(1, l_oDstID); }
		}

		//unaffect src widget, so that tree is simplified
		removeVisualizationWidget(l_oSrcID);

		//then drop it
		m_rVisualizationTree.findChildNodeFromRoot(&l_oDstIter, l_oDstID);
		void* l_pNewDstTreeWidget = nullptr;
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oDstIter, l_pNewDstTreeWidget, EVisualizationTreeColumn_PointerWidget);
		m_rVisualizationTree.dragDataReceivedOutsideWidgetCB(l_oSrcID, getVisualizationWidget(GTK_WIDGET(l_pNewDstTreeWidget)), oLocation);
	}

	//refresh view
	GtkTreeIter l_oDraggedIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oDraggedIter, l_oSrcID);
	refreshActiveVisualization(m_rVisualizationTree.getTreePath(&l_oDraggedIter));
}
