#include "ovd_base.h"
#include "ovdTAttributeHandler.h"
#include "ovdCApplication.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCPlayerVisualization.h"

#include <cstring>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;
using namespace OpenViBEVisualizationToolkit;

static GtkTargetEntry targets[] =
{
	{ static_cast<gchar*>("STRING"), 0, 0 },
	{ static_cast<gchar*>("text/plain"), 0, 0 },
};

static void delete_window_manager_window_cb(GtkWidget* widget, GdkEvent*, gpointer data)
{
	CPlayerVisualization* visualization     = reinterpret_cast<CPlayerVisualization*>(data);
	CInterfacedScenario& interfacedScenario = visualization->getInterfacedScenario();
	GtkWidget* confirmationDialog           = gtk_message_dialog_new(GTK_WINDOW(widget), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Would you like to stop the scenario execution?");
	const gint returnValue                  = gtk_dialog_run(GTK_DIALOG(confirmationDialog));

	if (returnValue == GTK_RESPONSE_YES)
	{
		if (visualization != nullptr) { interfacedScenario.m_pPlayer->stop(); }
	}
	else if (returnValue == GTK_RESPONSE_NO) {}

	gtk_widget_destroy(confirmationDialog);
}

CPlayerVisualization::CPlayerVisualization(const IKernelContext& rKernelContext, IVisualizationTree& rVisualizationTree, CInterfacedScenario& rInterfacedScenario)
	: m_kernelContext(rKernelContext), m_rVisualizationTree(rVisualizationTree), m_rInterfacedScenario(rInterfacedScenario) { }

CPlayerVisualization::~CPlayerVisualization()

{
	hideTopLevelWindows();

	m_pActiveToolbarButton = nullptr;

	for (auto& window : m_vWindows)
	{
		g_signal_handlers_disconnect_by_func(G_OBJECT(window), G_CALLBACK2(CPlayerVisualization::configure_event_cb), this);
		gtk_widget_destroy(GTK_WIDGET(window));
	}

	m_rVisualizationTree.setTreeViewCB(nullptr);
}

void CPlayerVisualization::init()

{
	//empty windows vector
	m_vWindows.clear();

	//empty split widgets map
	m_mSplitWidgets.clear();

	//empty toolbars map
	m_mToolbars.clear();

	//empty plugin widgets map
	m_mPlugins.clear();

	m_pActiveToolbarButton = nullptr;

	//register towards tree store
	m_rVisualizationTree.setTreeViewCB(this);

	//rebuild widgets
	m_rVisualizationTree.reloadTree();

	//must be called after the previous call to reload tree
	m_rInterfacedScenario.setModifiableSettingsWidgets();
}

GtkWidget* CPlayerVisualization::loadTreeWidget(IVisualizationWidget* pVisualizationWidget)
{
	GtkWidget* treeWidget = nullptr;

	if (pVisualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
	{
		//retrieve panel index
		IVisualizationWidget* l_pVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(pVisualizationWidget->getParentIdentifier());
		if (l_pVisualizationWindow != nullptr)
		{
			uint32_t panelIndex;
			l_pVisualizationWindow->getChildIndex(pVisualizationWidget->getIdentifier(), panelIndex);

			//create notebook if this is the first panel
			if (panelIndex == 0)
			{
				treeWidget = gtk_notebook_new();
			}
			else //otherwise retrieve it from first panel
			{
				CIdentifier l_oFirstPanelIdentifier;
				l_pVisualizationWindow->getChildIdentifier(0, l_oFirstPanelIdentifier);
				GtkTreeIter l_oFirstPanelIter;
				m_rVisualizationTree.findChildNodeFromRoot(&l_oFirstPanelIter, l_oFirstPanelIdentifier);
				void* l_pNotebookWidget = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oFirstPanelIter, l_pNotebookWidget, EVisualizationTreeColumn_PointerWidget);
				treeWidget = static_cast<GtkWidget*>(l_pNotebookWidget);
			}
		}
	}
	else if (pVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit ||
			 pVisualizationWidget->getType() == EVisualizationWidget_Undefined || pVisualizationWidget->getType() == EVisualizationWidget_VisualizationBox)
	{
		if (pVisualizationWidget->getType() == EVisualizationWidget_VisualizationBox)
		{
			if (pVisualizationWidget->getParentIdentifier() != OV_UndefinedIdentifier)
			{
				//dummy widget (actual one will be created at plugin initialization time)
				treeWidget = gtk_button_new();
			}
			else
			{
				//widget will be added to a top level window in setWidget()
			}
		}
		else if (pVisualizationWidget->getType() == EVisualizationWidget_Undefined)
		{
			treeWidget = gtk_button_new();
			gtk_button_set_label(GTK_BUTTON(treeWidget), static_cast<const char*>(pVisualizationWidget->getName()));
		}
		else if (pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit || pVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit)
		{
			treeWidget = (pVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit) ? gtk_hpaned_new() : gtk_vpaned_new();

			//store paned widget in paned map
			m_mSplitWidgets[GTK_PANED(treeWidget)] = pVisualizationWidget->getIdentifier();

			//retrieve its attributes
			const int handlePos = pVisualizationWidget->getDividerPosition();

			//initialize paned handle position
			gtk_paned_set_position(GTK_PANED(treeWidget), handlePos);
		}

		//parent widget to its parent
		//---------------------------
		IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(pVisualizationWidget->getParentIdentifier());

		if (l_pParentVisualizationWidget != nullptr) //unparented visualization boxes don't have a parent
		{
			GtkTreeIter parentIter;
			m_rVisualizationTree.findChildNodeFromRoot(&parentIter, l_pParentVisualizationWidget->getIdentifier());

			if (l_pParentVisualizationWidget->getType() == EVisualizationWidget_VisualizationPanel)
			{
				//parent widget to notebook as a new page
				void* notebook = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&parentIter, notebook, EVisualizationTreeColumn_PointerWidget);
				char* visualizationPanelName = nullptr;
				m_rVisualizationTree.getStringValueFromTreeIter(&parentIter, visualizationPanelName, EVisualizationTreeColumn_StringName);
				gtk_notebook_append_page(GTK_NOTEBOOK(notebook), treeWidget, gtk_label_new(visualizationPanelName));
			}
			else if (l_pParentVisualizationWidget->getType() == EVisualizationWidget_VerticalSplit || l_pParentVisualizationWidget->getType() == EVisualizationWidget_HorizontalSplit)
			{
				//insert widget in parent paned
				void* paned = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&parentIter, paned, EVisualizationTreeColumn_PointerWidget);
				if (paned != nullptr && GTK_IS_PANED(paned))
				{
					uint32_t l_ui32ChildIndex;
					if (l_pParentVisualizationWidget->getChildIndex(pVisualizationWidget->getIdentifier(), l_ui32ChildIndex))
					{
						if (l_ui32ChildIndex == 0)
						{
							gtk_paned_pack1(GTK_PANED(paned), treeWidget, TRUE, TRUE);
						}
						else
						{
							gtk_paned_pack2(GTK_PANED(paned), treeWidget, TRUE, TRUE);
						}
					}
				}
			}
		}
	}
	else if (pVisualizationWidget->getType() == EVisualizationWidget_VisualizationWindow)
	{
		//create this window only if it contains at least one visualization box
		CIdentifier identifier = OV_UndefinedIdentifier;
		bool l_bCreateWindow   = false;

		//for all visualization boxes
		while (m_rVisualizationTree.getNextVisualizationWidgetIdentifier(identifier, EVisualizationWidget_VisualizationBox))
		{
			//retrieve window containing current visualization box
			CIdentifier l_oParentIdentifier;
			IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(identifier);
			while (l_pVisualizationWidget->getParentIdentifier() != OV_UndefinedIdentifier)
			{
				l_oParentIdentifier    = l_pVisualizationWidget->getParentIdentifier();
				l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oParentIdentifier);
			}

			//if current box is parented to window passed in parameter, break and create it
			if (m_rVisualizationTree.getVisualizationWidget(l_oParentIdentifier) == pVisualizationWidget)
			{
				l_bCreateWindow = true;
				break;
			}
		}

		if (l_bCreateWindow)
		{
			//create new top level window
			treeWidget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			m_vWindows.push_back(GTK_WINDOW(treeWidget));

			//retrieve its size
			gtk_window_set_default_size(GTK_WINDOW(treeWidget), int(pVisualizationWidget->getWidth()), int(pVisualizationWidget->getHeight()));
			//set its title
			gtk_window_set_title(GTK_WINDOW(treeWidget), static_cast<const char*>(pVisualizationWidget->getName()));

			//set it transient for main window
			//gtk_window_set_transient_for(GTK_WINDOW(l_pTreeWidget), GTK_WINDOW(m_rInterfacedScenario.m_rApplication.m_pMainWindow));

			//centered on the main window
			if (m_kernelContext.getConfigurationManager().expandAsBoolean("${Designer_WindowManager_Center}", false))
			{
				gtk_window_set_position(GTK_WINDOW(treeWidget), GTK_WIN_POS_CENTER_ON_PARENT);
			}

			//FIXME wrong spelling (-)
			gtk_signal_connect(GTK_OBJECT(treeWidget), "configure_event", G_CALLBACK(configure_event_cb), this);
			//FIXME wrong spelling (-)
			g_signal_connect(treeWidget, "delete_event", G_CALLBACK(delete_window_manager_window_cb), this);
		}
	}

	//show newly created widget
	if (treeWidget != nullptr && pVisualizationWidget->getType() != EVisualizationWidget_VisualizationWindow) { gtk_widget_show(treeWidget); }

	return treeWidget;
}

void CPlayerVisualization::endLoadTreeWidget(IVisualizationWidget* pVisualizationWidget)
{
	//retrieve tree widget
	GtkTreeIter l_oIter;
	m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, pVisualizationWidget->getIdentifier());
	void* l_pTreeWidget;
	m_rVisualizationTree.getPointerValueFromTreeIter(&l_oIter, l_pTreeWidget, EVisualizationTreeColumn_PointerWidget);

	if (l_pTreeWidget != nullptr && pVisualizationWidget->getType() == EVisualizationWidget_VisualizationWindow)
	{
		//retrieve notebook
		CIdentifier l_oChildIdentifier;
		pVisualizationWidget->getChildIdentifier(0, l_oChildIdentifier);
		GtkTreeIter l_oChildIter;
		m_rVisualizationTree.findChildNodeFromRoot(&l_oChildIter, l_oChildIdentifier);
		void* l_pChildTreeWidget;
		m_rVisualizationTree.getPointerValueFromTreeIter(&l_oChildIter, l_pChildTreeWidget, EVisualizationTreeColumn_PointerWidget);

		//insert notebook in window
		if (l_pChildTreeWidget != nullptr && GTK_IS_NOTEBOOK(static_cast<GtkWidget*>(l_pChildTreeWidget)))
		{
			gtk_container_add(GTK_CONTAINER(static_cast<GtkWidget*>(l_pTreeWidget)), static_cast<GtkWidget*>(l_pChildTreeWidget));
		}
	}
}

bool CPlayerVisualization::setToolbar(const CIdentifier& boxIdentifier, GtkWidget* pToolbarWidget)
{
	//retrieve visualization widget
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidgetFromBoxIdentifier(boxIdentifier);
	if (l_pVisualizationWidget == nullptr)
	{
		m_kernelContext.getLogManager() << LogLevel_Warning << "CPlayerVisualization::setToolbar FAILED : couldn't retrieve simulated box with identifier " << boxIdentifier << "\n";
		return false;
	}

	//ensure toolbar pointer is not null
	if (pToolbarWidget == nullptr)
	{
		m_kernelContext.getLogManager() << LogLevel_Warning << "CPlayerVisualization::setToolbar FAILED : toolbar pointer is nullptr for plugin " << l_pVisualizationWidget->getName() << "\n";
		return false;
	}

	//ensure toolbar pointer is a window
	if (GTK_IS_WINDOW(pToolbarWidget) == 0)
	{
		m_kernelContext.getLogManager() << LogLevel_Warning << "CPlayerVisualization::setToolbar FAILED : toolbar pointer is not a GtkWindow for plugin " << l_pVisualizationWidget->getName() << "\n";
		return false;
	}

	//retrieve identifier
	const CIdentifier identifier = l_pVisualizationWidget->getIdentifier();

	//store toolbar
	m_mPlugins[identifier].m_pToolbar = pToolbarWidget;

	//ensure it is open at mouse position
	gtk_window_set_position(GTK_WINDOW(pToolbarWidget), GTK_WIN_POS_MOUSE);

	//if toolbar button has been created, set it sensitive (otherwise it will be set active later)
	if (m_mPlugins[identifier].m_pToolbarButton != nullptr)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(m_mPlugins[identifier].m_pToolbarButton), 1);

		//associate toolbar button to toolbar window
		m_mToolbars[m_mPlugins[identifier].m_pToolbarButton] = pToolbarWidget;
	}

	//catch delete events
	g_signal_connect(G_OBJECT(pToolbarWidget), "delete-event", G_CALLBACK(toolbar_delete_event_cb), this);

	return true;
}

bool CPlayerVisualization::setWidget(const CIdentifier& boxIdentifier, GtkWidget* widget)
{
	//retrieve visualization widget
	IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidgetFromBoxIdentifier(boxIdentifier);
	if (l_pVisualizationWidget == nullptr)
	{
		m_kernelContext.getLogManager() << LogLevel_Warning << "CPlayerVisualization::setWidget FAILED : couldn't retrieve simulated box with identifier " << boxIdentifier << "\n";
		return false;
	}

	//ensure widget pointer is not null
	if (widget == nullptr)
	{
		m_kernelContext.getLogManager() << LogLevel_Warning << "CPlayerVisualization::setWidget FAILED : widget pointer is nullptr for plugin " << l_pVisualizationWidget->getName() << "\n";
		return false;
	}

	//unparent top widget, if necessary
	GtkWidget* l_widgetParent = gtk_widget_get_parent(widget);
	if (GTK_IS_CONTAINER(l_widgetParent))
	{
		gtk_object_ref(GTK_OBJECT(widget));
		gtk_container_remove(GTK_CONTAINER(l_widgetParent), widget);
	}

	//create a box to store toolbar button and plugin widget
	GtkBox* l_pVBox = GTK_BOX(gtk_vbox_new(FALSE, 0));
	//gtk_widget_set_size_request(GTK_WIDGET(l_pVBox), 0, 0);

	//create toolbar button
	GtkToggleButton* l_pButton = GTK_TOGGLE_BUTTON(gtk_toggle_button_new());
	{
		//horizontal container : icon + label
		GtkBox* l_pHBox = GTK_BOX(gtk_hbox_new(FALSE, 0));
		//gtk_widget_set_size_request(GTK_WIDGET(l_pHBox), 0, 0);

		//retrieve icon name
		GtkTreeIter l_oIter;
		char* iconString = nullptr;
		if (m_rVisualizationTree.findChildNodeFromRoot(&l_oIter, static_cast<const char*>(l_pVisualizationWidget->getName()), EVisualizationTreeNode_VisualizationBox))
		{
			m_rVisualizationTree.getStringValueFromTreeIter(&l_oIter, iconString, EVisualizationTreeColumn_StringStockIcon);
		}

		//create icon
		GtkWidget* icon = gtk_image_new_from_stock(iconString != nullptr ? iconString : GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON);
		//gtk_widget_set_size_request(icon, 0, 0);
		gtk_box_pack_start(l_pHBox, icon, TRUE, TRUE, 0);

		//create label
		GtkWidget* l_pLabel = gtk_label_new(static_cast<const char*>(l_pVisualizationWidget->getName()));
		//gtk_widget_set_size_request(l_pLabel, 0, 0);
		gtk_box_pack_start(l_pHBox, l_pLabel, TRUE, TRUE, 0);

		//add box to button
		gtk_container_add(GTK_CONTAINER(l_pButton), GTK_WIDGET(l_pHBox));
	}

	//detect toolbar button toggle events
	g_signal_connect(G_OBJECT(l_pButton), "toggled", G_CALLBACK(toolbar_button_toggled_cb), this);

	//set up toolbar button as drag destination
	gtk_drag_dest_set(GTK_WIDGET(l_pButton), GTK_DEST_DEFAULT_ALL, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(l_pButton), "drag_data_received", G_CALLBACK(drag_data_received_in_widget_cb), this);

	//set up toolbar button as drag source as well
	gtk_drag_source_set(GTK_WIDGET(l_pButton), GDK_BUTTON1_MASK, targets, sizeof(targets) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(l_pButton), "drag_data_get", G_CALLBACK(drag_data_get_from_widget_cb), this);

	//store plugin widget and toolbar button
	const CIdentifier identifier            = l_pVisualizationWidget->getIdentifier();
	m_mPlugins[identifier].m_widget         = widget;
	m_mPlugins[identifier].m_pToolbarButton = l_pButton;

	//if a toolbar was registered for this widget, set its button sensitive
	if (m_mPlugins[identifier].m_pToolbar != nullptr)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(l_pButton), 1);

		//associate toolbar button to toolbar window
		m_mToolbars[l_pButton] = m_mPlugins[identifier].m_pToolbar;
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(l_pButton), 0);
	}

	//vertical container : button on top, visualization box below
	gtk_box_pack_start(l_pVBox, GTK_WIDGET(l_pButton), FALSE, TRUE, 0);
	gtk_box_pack_start(l_pVBox, widget, TRUE, TRUE, 0);

	//show vbox hierarchy
	gtk_widget_show_all(GTK_WIDGET(l_pVBox));

	//parent box at the appropriate location
	parentWidgetBox(l_pVisualizationWidget, l_pVBox);

	return true;
}

bool CPlayerVisualization::parentWidgetBox(IVisualizationWidget* widget, GtkBox* widgetBox)
{
	//if widget is unaffected, open it in its own window
	if (widget->getParentIdentifier() == OV_UndefinedIdentifier)
	{
		//create a top level window
		GtkWidget* l_pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		m_vWindows.push_back(GTK_WINDOW(l_pWindow));
		const uint64_t l_ui64DefaultWidth  = m_kernelContext.getConfigurationManager().expandAsUInteger("${Designer_UnaffectedVisualizationWindowWidth}", 400);
		const uint64_t l_ui64DefaultHeight = m_kernelContext.getConfigurationManager().expandAsUInteger("${Designer_UnaffectedVisualizationWindowHeight}", 400);

		gtk_window_set_default_size(GTK_WINDOW(l_pWindow), gint(l_ui64DefaultWidth), gint(l_ui64DefaultHeight));
		//set its title
		gtk_window_set_title(GTK_WINDOW(l_pWindow), static_cast<const char*>(widget->getName()));
		//set it transient for main window
		//gtk_window_set_transient_for(GTK_WINDOW(l_pWindow), GTK_WINDOW(m_rInterfacedScenario.m_rApplication.m_pMainWindow));
		//insert box in top level window
		gtk_container_add(GTK_CONTAINER(l_pWindow), reinterpret_cast<GtkWidget*>(widgetBox));
		//prevent user from closing this window
		g_signal_connect(l_pWindow, "delete_event", G_CALLBACK(delete_window_manager_window_cb), this);

		//position: centered in the main window
		if (m_kernelContext.getConfigurationManager().expandAsBoolean("${Designer_WindowManager_Center}", false))
		{
			gtk_window_set_position(GTK_WINDOW(l_pWindow), GTK_WIN_POS_CENTER_ON_PARENT);
		}

		//show window (and realize widget in doing so)
		gtk_widget_show(l_pWindow);
	}
	else //retrieve parent widget in which to insert current widget
	{
		GtkTreeIter l_oParentIter;
		if (m_rVisualizationTree.findChildNodeFromRoot(&l_oParentIter, widget->getParentIdentifier()))
		{
			void* l_pParentWidget = nullptr;
			m_rVisualizationTree.getPointerValueFromTreeIter(&l_oParentIter, l_pParentWidget, EVisualizationTreeColumn_PointerWidget);
			CIdentifier l_oParentIdentifier;
			m_rVisualizationTree.getIdentifierFromTreeIter(&l_oParentIter, l_oParentIdentifier, EVisualizationTreeColumn_StringIdentifier);

			//widget is to be parented to a paned widget
			if (GTK_IS_PANED(l_pParentWidget))
			{
				//retrieve index at which to insert child
				IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oParentIdentifier);
				uint32_t l_oVisualizationBoxIndex;
				l_pParentVisualizationWidget->getChildIndex(widget->getIdentifier(), l_oVisualizationBoxIndex);
				//insert visualization box in paned
				if (l_oVisualizationBoxIndex == 0)
				{
					gtk_container_remove(GTK_CONTAINER(l_pParentWidget), gtk_paned_get_child1(GTK_PANED(l_pParentWidget)));
					gtk_paned_pack1(GTK_PANED(l_pParentWidget), GTK_WIDGET(widgetBox), TRUE, TRUE);
				}
				else
				{
					gtk_container_remove(GTK_CONTAINER(l_pParentWidget), gtk_paned_get_child2(GTK_PANED(l_pParentWidget)));
					gtk_paned_pack2(GTK_PANED(l_pParentWidget), GTK_WIDGET(widgetBox), TRUE, TRUE);
				}
			}
			else if (GTK_IS_NOTEBOOK(l_pParentWidget)) //widget is to be added to a notebook page
			{
				//retrieve notebook page index
				IVisualizationWidget* l_pParentVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oParentIdentifier);
				IVisualizationWidget* l_pParentVisualizationWindow = m_rVisualizationTree.getVisualizationWidget(l_pParentVisualizationWidget->getParentIdentifier());
				uint32_t l_oPanelIndex;
				l_pParentVisualizationWindow->getChildIndex(l_pParentVisualizationWidget->getIdentifier(), l_oPanelIndex);

				//remove temporary page
				gtk_notebook_remove_page(GTK_NOTEBOOK(l_pParentWidget), l_oPanelIndex);

				//insert final page
				gtk_notebook_insert_page(GTK_NOTEBOOK(l_pParentWidget), GTK_WIDGET(widgetBox), gtk_label_new(static_cast<const char*>(l_pParentVisualizationWidget->getName())), l_oPanelIndex);
			}

			//resize widgets once they are allocated : this is the case when they are shown on an expose event
			//FIXME : perform resizing only once (when it is done as many times as there are widgets in the tree here)
			if (l_pParentWidget != nullptr)
			{
				gtk_signal_connect(GTK_OBJECT(l_pParentWidget), "expose-event", G_CALLBACK(widget_expose_event_cb), this);
			}

			//show window (and realize widget if it owns a 3D context)
			//--------------------------------------------------------
			//get panel containing widget
			GtkTreeIter l_oPanelIter = l_oParentIter;
			if (m_rVisualizationTree.findParentNode(&l_oPanelIter, EVisualizationTreeNode_VisualizationPanel))
			{
				//get panel identifier
				CIdentifier l_oPanelIdentifier;
				m_rVisualizationTree.getIdentifierFromTreeIter(&l_oPanelIter, l_oPanelIdentifier, EVisualizationTreeColumn_StringIdentifier);

				//get panel index in window
				uint32_t l_ui32PanelIndex;
				m_rVisualizationTree.getVisualizationWidgetIndex(l_oPanelIdentifier, l_ui32PanelIndex);

				//get notebook pointer
				void* l_pPanelWidget = nullptr;
				m_rVisualizationTree.getPointerValueFromTreeIter(&l_oPanelIter, l_pPanelWidget, EVisualizationTreeColumn_PointerWidget);

				//get parent window
				GtkTreeIter l_oWindowIter = l_oPanelIter;
				if (m_rVisualizationTree.findParentNode(&l_oWindowIter, EVisualizationTreeNode_VisualizationWindow))
				{
					//get parent window pointer
					void* l_pWindowWidget = nullptr;
					m_rVisualizationTree.getPointerValueFromTreeIter(&l_oWindowIter, l_pWindowWidget, EVisualizationTreeColumn_PointerWidget);

					//show parent window
					gtk_widget_show(GTK_WIDGET(l_pWindowWidget));
					// gtk_widget_realize(GTK_WIDGET(l_pWindowWidget));
					// gdk_flush();

					//set panel containing widget as current (this realizes the widget)
					gtk_notebook_set_current_page(GTK_NOTEBOOK(l_pPanelWidget), l_ui32PanelIndex);

					//then reset first panel as current
					gtk_notebook_set_current_page(GTK_NOTEBOOK(l_pPanelWidget), 0);
				}
			}
		}
	}

	return true;
}

//called upon Player start
void CPlayerVisualization::showTopLevelWindows()

{
	for (auto& window : m_vWindows) { gtk_widget_show(GTK_WIDGET(window)); }
	if (m_pActiveToolbarButton != nullptr)
	{
		//show active toolbar
		gtk_widget_show(m_mToolbars[m_pActiveToolbarButton]);
	}
	auto it = m_mPlugins.begin();
	while (it != m_mPlugins.end())
	{
		if (GTK_IS_WIDGET(it->second.m_widget))
		{
			gtk_widget_show(it->second.m_widget);
		}
		++it;
	}
}

//called upon Player stop
void CPlayerVisualization::hideTopLevelWindows()

{
	auto it = m_mPlugins.begin();
	while (it != m_mPlugins.end())
	{
		if (GTK_IS_WIDGET(it->second.m_widget))
		{
			gtk_widget_hide(it->second.m_widget);
		}
		++it;
	}

	for (auto& window : m_vWindows) { gtk_widget_hide(GTK_WIDGET(window)); }

	if (m_pActiveToolbarButton != nullptr)
	{
		//hide active toolbar
		gtk_widget_hide(m_mToolbars[m_pActiveToolbarButton]);
	}
}

//event generated whenever window changes size, including when it is first created
gboolean CPlayerVisualization::configure_event_cb(GtkWidget* widget, GdkEventConfigure* /*event*/, gpointer data)
{
	//paned positions aren't to be saved, they are to be read once only
	g_signal_handlers_disconnect_by_func(G_OBJECT(widget), G_CALLBACK2(CPlayerVisualization::configure_event_cb), data);

	if (GTK_IS_CONTAINER(widget))
	{
		static_cast<CPlayerVisualization*>(data)->resizeCB(GTK_CONTAINER(widget));
	}

	return FALSE;
}

gboolean CPlayerVisualization::widget_expose_event_cb(GtkWidget* widget, GdkEventExpose* /*event*/, gpointer data)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(widget), G_CALLBACK2(CPlayerVisualization::widget_expose_event_cb), data);
	/*
		//retrieve topmost widget
		while(gtk_widget_get_parent(widget) != nullptr && GTK_IS_CONTAINER(gtk_widget_get_parent(widget)))
			widget = gtk_widget_get_parent(widget);
	*/
	if (GTK_IS_CONTAINER(widget))
	{
		static_cast<CPlayerVisualization*>(data)->resizeCB(GTK_CONTAINER(widget));
	}

	return FALSE;
}

void CPlayerVisualization::resizeCB(GtkContainer* container)
{
	if (GTK_IS_WINDOW(container))
	{
		gpointer data = g_list_first(gtk_container_get_children(container))->data;

		if (GTK_IS_CONTAINER(data)) { resizeCB(GTK_CONTAINER(data)); }
	}
	else if (GTK_IS_NOTEBOOK(container))
	{
		GtkNotebook* l_pNotebook = GTK_NOTEBOOK(container);

		for (int i = 0; i < gtk_notebook_get_n_pages(l_pNotebook); ++i)
		{
			GtkWidget* l_widget = gtk_notebook_get_nth_page(l_pNotebook, i);
			if (GTK_IS_CONTAINER(l_widget)) { resizeCB(GTK_CONTAINER(l_widget)); }
		}
	}
	else if (GTK_IS_PANED(container))
	{
		GtkPaned* l_pPaned = GTK_PANED(container);

		//retrieve paned identifier from paned map
		CIdentifier& l_oPanedIdentifier = m_mSplitWidgets[GTK_PANED(l_pPaned)];

		//retrieve its attributes
		IVisualizationWidget* l_pVisualizationWidget = m_rVisualizationTree.getVisualizationWidget(l_oPanedIdentifier);
		const int l_i32HandlePos                     = l_pVisualizationWidget->getDividerPosition();
		const int l_i32MaxHandlePos                  = l_pVisualizationWidget->getMaxDividerPosition();

		if (l_i32MaxHandlePos > 0)
		{
			//retrieve current maximum handle position
			const int l_i32CurrentMaxHandlePos = GTK_IS_VPANED(l_pPaned) ? l_pPaned->container.widget.allocation.height : l_pPaned->container.widget.allocation.width;

			//set new paned handle position
			gtk_paned_set_position(l_pPaned, l_i32HandlePos * l_i32CurrentMaxHandlePos / l_i32MaxHandlePos);
		}

		//go down each child
		GtkWidget* l_pChild = gtk_paned_get_child1(l_pPaned);
		if (GTK_IS_CONTAINER(l_pChild))
		{
			resizeCB(GTK_CONTAINER(l_pChild));
		}

		l_pChild = gtk_paned_get_child2(l_pPaned);
		if (GTK_IS_CONTAINER(l_pChild))
		{
			resizeCB(GTK_CONTAINER(l_pChild));
		}
	}
}

void CPlayerVisualization::drag_data_get_from_widget_cb(GtkWidget* pSrcWidget, GdkDragContext* /*pDragContext*/, GtkSelectionData* pSelectionData, guint /*uiInfo*/, guint /*uiTime*/, gpointer /*data*/)
{
	char l_sString[1024];
	sprintf(l_sString, "%p", pSrcWidget);
	gtk_selection_data_set_text(pSelectionData, l_sString, gint(strlen(l_sString)));
}

void CPlayerVisualization::drag_data_received_in_widget_cb(GtkWidget* pDstWidget, GdkDragContext* /*pDragContext*/, gint /*iX*/, gint /*iY*/, GtkSelectionData* pSelectionData, guint /*uiInfo*/, guint /*uiTime*/, gpointer /*data*/)
{
	void* srcWidget = nullptr;
	sscanf(reinterpret_cast<const char*>(gtk_selection_data_get_text(pSelectionData)), "%p", &srcWidget);

	//retrieve source box and parent widgets
	GtkWidget* srcBoxWidget = nullptr;
	do
	{
		srcBoxWidget = gtk_widget_get_parent(GTK_WIDGET(srcWidget));
	} while (srcBoxWidget != nullptr && !GTK_IS_VBOX(srcBoxWidget));

	if (srcBoxWidget == nullptr) { return; }

	GtkWidget* srcParentWidget = gtk_widget_get_parent(srcBoxWidget);

	if (srcParentWidget == nullptr) { return; }

	//retrieve dest box and parent widgets
	GtkWidget* dstBoxWidget = nullptr;
	do
	{
		dstBoxWidget = gtk_widget_get_parent(pDstWidget);
	} while (dstBoxWidget != nullptr && !GTK_IS_VBOX(dstBoxWidget));

	if (dstBoxWidget == nullptr) { return; }

	GtkWidget* dstParentWidget = gtk_widget_get_parent(dstBoxWidget);

	if (dstParentWidget == nullptr) { return; }

	//ensure src and dst widgets are different
	if (srcBoxWidget == dstBoxWidget) { return; }

	//remove src box from parent
	int srcIndex;
	GtkWidget* srcTabLabel = nullptr;

	if (GTK_IS_WINDOW(srcParentWidget)) { srcIndex = 0; }
	else if (GTK_IS_NOTEBOOK(srcParentWidget))
	{
		srcIndex    = gtk_notebook_page_num(GTK_NOTEBOOK(srcParentWidget), srcBoxWidget);
		srcTabLabel = gtk_label_new(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(srcParentWidget), srcBoxWidget));
	}
	else if (GTK_IS_PANED(srcParentWidget))
	{
		srcIndex = reinterpret_cast<GtkPaned*>(srcParentWidget)->child1 == srcBoxWidget ? 1 : 2;
	}
	else { return; }

	//remove dst box from parent
	int dstIndex;
	GtkWidget* l_pDstTabLabel = nullptr;
	if (GTK_IS_WINDOW(dstParentWidget)) { dstIndex = 0; }
	else if (GTK_IS_NOTEBOOK(dstParentWidget))
	{
		dstIndex       = gtk_notebook_page_num(GTK_NOTEBOOK(dstParentWidget), dstBoxWidget);
		l_pDstTabLabel = gtk_label_new(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(dstParentWidget), dstBoxWidget));
	}
	else if (GTK_IS_PANED(dstParentWidget))
	{
		dstIndex = reinterpret_cast<GtkPaned*>(dstParentWidget)->child1 == dstBoxWidget ? 1 : 2;
	}
	else { return; }

	gtk_object_ref(GTK_OBJECT(srcBoxWidget));
	gtk_container_remove(GTK_CONTAINER(srcParentWidget), srcBoxWidget);

	gtk_object_ref(GTK_OBJECT(dstBoxWidget));
	gtk_container_remove(GTK_CONTAINER(dstParentWidget), dstBoxWidget);

	//parent src box to dst parent
	if (GTK_IS_WINDOW(dstParentWidget))
	{
		gtk_container_add(GTK_CONTAINER(dstParentWidget), srcBoxWidget);
	}
	else if (GTK_IS_NOTEBOOK(dstParentWidget))
	{
		gtk_notebook_insert_page(GTK_NOTEBOOK(dstParentWidget), srcBoxWidget, l_pDstTabLabel, dstIndex);
	}
	else //dst parent is a paned
	{
		if (dstIndex == 1) { gtk_paned_pack1(GTK_PANED(dstParentWidget), srcBoxWidget, TRUE, TRUE); }
		else { gtk_paned_pack2(GTK_PANED(dstParentWidget), srcBoxWidget, TRUE, TRUE); }
	}

	//parent dst box to src parent
	if (GTK_IS_WINDOW(srcParentWidget))
	{
		gtk_container_add(GTK_CONTAINER(srcParentWidget), dstBoxWidget);
	}
	else if (GTK_IS_NOTEBOOK(srcParentWidget))
	{
		gtk_notebook_insert_page(GTK_NOTEBOOK(srcParentWidget), dstBoxWidget, srcTabLabel, srcIndex);
	}
	else //src parent is a paned
	{
		if (srcIndex == 1)
		{
			gtk_paned_pack1(GTK_PANED(srcParentWidget), dstBoxWidget, TRUE, TRUE);
		}
		else
		{
			gtk_paned_pack2(GTK_PANED(srcParentWidget), dstBoxWidget, TRUE, TRUE);
		}
	}
}

void CPlayerVisualization::toolbar_button_toggled_cb(GtkToggleButton* button, gpointer data)
{
	static_cast<CPlayerVisualization*>(data)->toggleToolbarCB(button);
}

bool CPlayerVisualization::toggleToolbarCB(GtkToggleButton* button)
{
	//retrieve toolbar
	if (m_mToolbars.find(button) == m_mToolbars.end()) { return false; }
	GtkWidget* l_pToolbar = m_mToolbars[button];

	//if current toolbar is toggled on or off, update toolbar state accordingly
	if (button == m_pActiveToolbarButton)
	{
		//hiding active toolbar
		if (gtk_toggle_button_get_active(button) == FALSE)
		{
			gtk_widget_hide(l_pToolbar);
			m_pActiveToolbarButton = nullptr;
		}
		else //showing active toolbar
		{
			gtk_widget_show(l_pToolbar);
		}
	}
	else //a new toolbar is to be shown
	{
		//hide previously active toolbar, if any
		if (m_pActiveToolbarButton != nullptr)
		{
			gtk_widget_hide(m_mToolbars[m_pActiveToolbarButton]);

			g_signal_handlers_disconnect_by_func(m_pActiveToolbarButton, G_CALLBACK2(toolbar_button_toggled_cb), this);
			gtk_toggle_button_set_active(m_pActiveToolbarButton, FALSE);
			g_signal_connect(m_pActiveToolbarButton, "toggled", G_CALLBACK(toolbar_button_toggled_cb), this);
		}
		/*
				//set toolbar transient for plugin window
				::GtkWidget* l_widget = GTK_WIDGET(pToolbarButton);
				while(l_widget!=nullptr && !GTK_IS_WINDOW(l_widget))
				{
					l_widget = gtk_widget_get_parent(l_widget);
				}
				if(l_widget != nullptr && GTK_IS_WINDOW(l_pToolbar))
				{
					gtk_window_set_transient_for(GTK_WINDOW(l_pToolbar), GTK_WINDOW(l_widget));
				}
		*/
		//show new toolbar
		gtk_widget_show(l_pToolbar);

		//update active toolbar button
		m_pActiveToolbarButton = button;
	}

	return true;
}

gboolean CPlayerVisualization::toolbar_delete_event_cb(GtkWidget* widget, GdkEvent* /*event*/, gpointer data)
{
	if (data != nullptr) { static_cast<CPlayerVisualization*>(data)->deleteToolbarCB(widget); }
	return TRUE;
}

bool CPlayerVisualization::deleteToolbarCB(GtkWidget* widget)
{
	if (m_pActiveToolbarButton == nullptr || m_mToolbars[m_pActiveToolbarButton] != widget)
	{
		//error : active toolbar isn't the one registered as such
		gtk_widget_hide(widget);
		return FALSE;
	}

	//toggle toolbar button off
	g_signal_handlers_disconnect_by_func(m_pActiveToolbarButton, G_CALLBACK2(toolbar_button_toggled_cb), this);
	gtk_toggle_button_set_active(m_pActiveToolbarButton, FALSE);
	g_signal_connect(m_pActiveToolbarButton, "toggled", G_CALLBACK(toolbar_button_toggled_cb), this);

	//hide toolbar
	gtk_widget_hide(widget);

	//clear active toolbar button pointer
	m_pActiveToolbarButton = nullptr;

	return TRUE;
}
