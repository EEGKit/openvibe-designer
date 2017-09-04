#include "ovdCBoxConfigurationDialog.h"
#include "ovdCSettingCollectionHelper.h"

#include <vector>
#include <string>
#include <fstream>

#include <xml/IReader.h>
#include <xml/IWriter.h>
#include <xml/IXMLHandler.h>
#include <xml/IXMLNode.h>

#include <fs/Files.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEDesigner;
using namespace std;

#include <iostream>

namespace
{
	const char * const c_sRootName = "OpenViBE-SettingsOverride";
	const char *const c_sSettingName = "SettingValue";
}

static void on_file_override_check_toggled(::GtkToggleButton* pToggleButton, gpointer pUserData)
{
	gtk_widget_set_sensitive((::GtkWidget*)pUserData, !gtk_toggle_button_get_active(pToggleButton));
}

static void on_button_load_clicked(::GtkButton*, gpointer pUserData)
{
	static_cast<CBoxConfigurationDialog *>(pUserData)->loadConfiguration();
}

static void on_button_save_clicked(::GtkButton*, gpointer pUserData)
{
	static_cast<CBoxConfigurationDialog *>(pUserData)->saveConfiguration();
}

static void on_override_browse_clicked(::GtkButton* pButton, gpointer pUserData)
{
	static_cast<CBoxConfigurationDialog *>(pUserData)->onOverrideBrowse();
}

static void collect_widget_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	static_cast< std::vector< ::GtkWidget* > *>(pUserData)->push_back(pWidget);
}
CBoxConfigurationDialog::CBoxConfigurationDialog(const IKernelContext& rKernelContext, IBox& rBox, const char* sGUIFilename, const char* sGUISettingsFilename, bool isScenarioRunning)
	:m_rKernelContext(rKernelContext)
	,m_rBox(rBox)
	,m_sGUIFilename(sGUIFilename)
	,m_sGUISettingsFilename(sGUISettingsFilename)
	,m_oSettingFactory(m_sGUISettingsFilename.toASCIIString(), rKernelContext)
	,m_pSettingsTable(nullptr)
	,m_pViewPort(nullptr)
	,m_pScrolledWindow(nullptr)
	,m_bIsScenarioRunning(isScenarioRunning)
	,m_pOverrideEntryContainer(nullptr)
	,m_pSettingDialog(nullptr)
	,m_pFileOverrideCheck(nullptr)
{
	m_rBox.addObserver(this);

	if(m_rBox.getSettingCount())
	{
		::GtkBuilder* l_pBuilderInterfaceSetting=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "box_configuration", NULL);
		gtk_builder_add_from_file(l_pBuilderInterfaceSetting, m_sGUIFilename.toASCIIString(), NULL);
		gtk_builder_connect_signals(l_pBuilderInterfaceSetting, NULL);

		if (!m_bIsScenarioRunning)
		{
			// TODO : This is not a modal dialog. It would be better if it was.
			m_pSettingDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration"));
			char l_sTitle[1024];
			sprintf(l_sTitle, "Configure %s settings", m_rBox.getName().toASCIIString());
			gtk_window_set_title(GTK_WINDOW(m_pSettingDialog), l_sTitle);
		}
		else
		{
			// This is actually *not* a dialog
			m_pSettingDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-scrolledwindow"));
		}
		m_pSettingsTable=GTK_TABLE(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-table"));
		m_pScrolledWindow=GTK_SCROLLED_WINDOW(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-scrolledwindow"));
		m_pViewPort=GTK_VIEWPORT(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-viewport"));

		gtk_table_resize(m_pSettingsTable, m_rBox.getSettingCount(), 4);

		generateSettingsTable();

		CSettingCollectionHelper l_oHelper(m_rKernelContext, m_sGUISettingsFilename.toASCIIString());

		if (!m_bIsScenarioRunning)
		{
			::GtkContainer* l_pFileOverrideContainer = GTK_CONTAINER(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-hbox_filename_override"));
			m_pFileOverrideCheck = GTK_CHECK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-checkbutton_filename_override"));
			::GtkButton* l_pButtonLoad = GTK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-button_load_current_from_file"));
			::GtkButton* l_pButtonSave = GTK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-button_save_current_to_file"));

			string l_sSettingOverrideWidgetName=l_oHelper.getSettingWidgetName(OV_TypeId_Filename).toASCIIString();
			::GtkBuilder* l_pBuilderInterfaceSettingCollection=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), l_sSettingOverrideWidgetName.c_str(), NULL);
			gtk_builder_add_from_file(l_pBuilderInterfaceSettingCollection, m_sGUISettingsFilename.toASCIIString(), NULL);
//			gtk_builder_connect_signals(l_pBuilderInterfaceSettingCollection, NULL);

			m_pOverrideEntryContainer = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSettingCollection, l_sSettingOverrideWidgetName.c_str()));

			std::vector < ::GtkWidget* > l_vWidget;
			gtk_container_foreach(GTK_CONTAINER(m_pOverrideEntryContainer), collect_widget_cb, &l_vWidget);

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(m_pOverrideEntryContainer)), m_pOverrideEntryContainer);
			gtk_container_add(l_pFileOverrideContainer, m_pOverrideEntryContainer);
			m_pOverrideEntry = GTK_ENTRY(l_vWidget[0]);


			g_signal_connect(G_OBJECT(m_pFileOverrideCheck), "toggled", G_CALLBACK(on_file_override_check_toggled), GTK_WIDGET(m_pSettingsTable));
			g_signal_connect(G_OBJECT(l_vWidget[1]),         "clicked", G_CALLBACK(on_override_browse_clicked), this);
			g_signal_connect(G_OBJECT(l_pButtonLoad),        "clicked", G_CALLBACK(on_button_load_clicked), this);
			g_signal_connect(G_OBJECT(l_pButtonSave),        "clicked", G_CALLBACK(on_button_save_clicked), this);

			if(m_rBox.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
			{
				::GtkExpander *l_pExpander = GTK_EXPANDER(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-expander"));
				gtk_expander_set_expanded(l_pExpander, true);

				gtk_entry_set_text(m_pOverrideEntry, m_rBox.getAttributeValue(OV_AttributeId_Box_SettingOverrideFilename).toASCIIString());
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), true);
				gtk_widget_set_sensitive(GTK_WIDGET(m_pSettingsTable), false);
			}
			else
			{
				gtk_entry_set_text(m_pOverrideEntry, "");
			}

			g_object_unref(l_pBuilderInterfaceSetting);
			g_object_unref(l_pBuilderInterfaceSettingCollection);

		}

	}
}

CBoxConfigurationDialog::~CBoxConfigurationDialog(void)
{
	m_rBox.deleteObserver(this);
	gtk_widget_destroy(m_pSettingDialog);
}

bool CBoxConfigurationDialog::run(void)
{
	bool l_bModified=false;
	if(m_rBox.getSettingCount())
	{
		CSettingCollectionHelper l_oHelper(m_rKernelContext, m_sGUISettingsFilename.toASCIIString());
		storeState();
		bool l_bFinished=false;
		while(!l_bFinished)
		{
			gint l_iResult=gtk_dialog_run(GTK_DIALOG(m_pSettingDialog));
			if(l_iResult==GTK_RESPONSE_APPLY)
			{
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck)))
				{
					const gchar* l_sFilename = gtk_entry_get_text(m_pOverrideEntry);
					if(m_rBox.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
					{
						m_rBox.setAttributeValue(OV_AttributeId_Box_SettingOverrideFilename, l_sFilename);
					}
					else
					{
						m_rBox.addAttribute(OV_AttributeId_Box_SettingOverrideFilename, l_sFilename);
					}
				}
				else
				{
					if(m_rBox.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
					{
						m_rBox.removeAttribute(OV_AttributeId_Box_SettingOverrideFilename);
					}
				}

				l_bFinished=true;
				l_bModified=true;
			}
			else if(l_iResult == GTK_RESPONSE_CANCEL)
			{
				restoreState();
				l_bFinished=true;
			}
			else if(l_iResult==1) // default
			{
				// Some settings will add/remove other settings;
				// by evaluating m_rBox.getSettingCount() each time we ensure not ending somewhere in the oblivion
				for(uint32_t i = 0; i < m_vSettingViewVector.size(); i++)
				{
					CString l_oSettingValue;
					m_rBox.getSettingDefaultValue(i, l_oSettingValue);
					m_rBox.setSettingValue(i, l_oSettingValue);
//					m_vSettingViewVector[i]->setValue(l_oSettingValue);
//					l_oHelper.setValue(l_oSettingType, i < m_vSettingViewVector.size()? m_vSettingViewVector[i]->getEntryWidget() : NULL, l_oSettingValue);
				}
				gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntryContainer), "");
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), false);
//				gtk_widget_set_sensitive(GTK_WIDGET(m_pSettingsTable), true);
				l_bModified=false;
			}
			else if(l_iResult==2) // revert
			{
				restoreState();

				if(m_rBox.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
				{
					gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntryContainer), m_rBox.getAttributeValue(OV_AttributeId_Box_SettingOverrideFilename).toASCIIString());
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), true);
				}
				else
				{
					gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntryContainer), "");
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), false);
				}
			}
			else if(l_iResult==3) // load
			{
				l_bModified=true;
			}
			else if(l_iResult==4) // save
			{
			}
			else
			{
				l_bFinished=true;
			}
		}
	}
	return l_bModified;
}

void CBoxConfigurationDialog::update(OpenViBE::CObservable &o, void* data)
{
	const BoxEventMessage *l_pEvent = static_cast< BoxEventMessage * > (data);

	switch(l_pEvent->m_eType)
	{
		case SettingsAllChange:
			generateSettingsTable();
			break;

		case SettingValueUpdate:
		{
			CString l_sSettingValue;
			m_rBox.getSettingValue(l_pEvent->m_i32FirstIndex, l_sSettingValue);

			m_vSettingViewVector[l_pEvent->m_i32FirstIndex]->setValue(l_sSettingValue);
			break;
		}

		case SettingDelete:
			removeSetting(l_pEvent->m_i32FirstIndex);
			break;

		case SettingAdd:
			addSetting(l_pEvent->m_i32FirstIndex);
			break;

		case SettingChange:
			settingChange(l_pEvent->m_i32FirstIndex);
			break;

		default:
//			OV_ERROR_KRF("wtf", ErrorType::BadSetting);
			break;
	}
}

void CBoxConfigurationDialog::generateSettingsTable()
{
	std::for_each(m_vSettingViewVector.begin(), m_vSettingViewVector.end(), [](Setting::CAbstractSettingView* elem){ delete elem;});
	m_vSettingViewVector.clear();
	//Remove rows
	gtk_container_foreach(GTK_CONTAINER(GTK_WIDGET(m_pSettingsTable)),
						  [](GtkWidget *widget, gpointer data)
							{
								gtk_container_remove(GTK_CONTAINER(data), widget);
							},
						  GTK_WIDGET(m_pSettingsTable));

	uint32_t l_ui32TableSize = 0;
	if (m_bIsScenarioRunning)
	{
		for(uint32_t i = 0; i < m_rBox.getSettingCount(); i++)
		{
			bool l_IsMod = false;
			m_rBox.getSettingMod(i, l_IsMod);
			if (l_IsMod)
			{
				l_ui32TableSize++;
			}
		}
	}
	else
	{
		l_ui32TableSize = m_rBox.getSettingCount();
	}
	gtk_table_resize(m_pSettingsTable, l_ui32TableSize + 2, 4);

	// Iterate over box settings, generate corresponding gtk widgets. If the scenario is running, we are making a
	// 'modifiable settings' dialog and use a subset of widgets with a slightly different layout and buttons.
	for(uint32 settingIndex = 0, tableIndex = 0; settingIndex < m_rBox.getSettingCount(); settingIndex++)
	{
		if(addSettingsToView(settingIndex, tableIndex))
		{
			++tableIndex;
		}
	}
	updateSize();
}

bool CBoxConfigurationDialog::addSettingsToView(uint32_t ui32SettingIndex, uint32_t ui32TableIndex)
{
	bool l_bSettingModifiable;
	m_rBox.getSettingMod(ui32SettingIndex, l_bSettingModifiable);

	if((!m_bIsScenarioRunning) || (m_bIsScenarioRunning && l_bSettingModifiable) )
	{
		CString l_sSettingName;

		m_rBox.getSettingName(ui32SettingIndex, l_sSettingName);
		Setting::CAbstractSettingView* l_oView = m_oSettingFactory.getSettingView(m_rBox, ui32SettingIndex);

		gtk_table_attach(m_pSettingsTable, l_oView->getNameWidget() ,   0, 1, ui32TableIndex, ui32TableIndex+1, ::GtkAttachOptions(GTK_FILL), ::GtkAttachOptions(GTK_FILL), 0, 0);
		gtk_table_attach(m_pSettingsTable, l_oView->getEntryWidget(),   1, 4, ui32TableIndex, ui32TableIndex+1, ::GtkAttachOptions(GTK_SHRINK|GTK_FILL|GTK_EXPAND), ::GtkAttachOptions(GTK_SHRINK), 0, 0);

		m_vSettingViewVector.insert(m_vSettingViewVector.begin()+ui32TableIndex, l_oView);

		return true;
	}
	return false;
}

void CBoxConfigurationDialog::settingChange(uint32_t ui32SettingIndex)
{
	//We remeber the place to add the new setting at the same place
	uint32 l_ui32IndexTable = getTableIndex(ui32SettingIndex);

	removeSetting(ui32SettingIndex, false);
	addSettingsToView(ui32SettingIndex, l_ui32IndexTable);
}

void CBoxConfigurationDialog::addSetting(uint32_t ui32SettingIndex)
{
	boolean l_bSettingModifiable;
	m_rBox.getSettingMod(ui32SettingIndex, l_bSettingModifiable);

	if( (!m_bIsScenarioRunning) || (m_bIsScenarioRunning && l_bSettingModifiable) )
	{
		uint32 l_ui32TableSize = m_vSettingViewVector.size();
		/*There is two case.
		1) we just add at the end of the setting box
		2) we add it in the middle end we need to shift
		*/
		uint32 l_ui32TableIndex;
		if(ui32SettingIndex > m_vSettingViewVector[l_ui32TableSize-1]->getSettingIndex()){
			l_ui32TableIndex = l_ui32TableSize;
		}
		else
		{
			l_ui32TableIndex = getTableIndex(ui32SettingIndex);
		}

		gtk_table_resize(m_pSettingsTable, l_ui32TableSize+2, 4);

		if(ui32SettingIndex <= m_vSettingViewVector[l_ui32TableSize-1]->getSettingIndex())
		{
			for(size_t i = l_ui32TableSize-1; i >= l_ui32TableIndex ; --i)
			{
				Setting::CAbstractSettingView *l_oView = m_vSettingViewVector[i];

				//We need to update the index
				l_oView->setSettingIndex(l_oView->getSettingIndex() + 1);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getNameWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getNameWidget() ,   0, 1, i+1, i+2, ::GtkAttachOptions(GTK_FILL), ::GtkAttachOptions(GTK_FILL), 0, 0);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getEntryWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getEntryWidget(),   1, 4, i+1, i+2, ::GtkAttachOptions(GTK_SHRINK|GTK_FILL|GTK_EXPAND), ::GtkAttachOptions(GTK_SHRINK), 0, 0);
			}
		}
		addSettingsToView(l_ui32TableIndex, ui32SettingIndex);
		updateSize();
	}
	//Even if nothing is add to the interface, we still need to update index
	else
	{
		for(Setting::CAbstractSettingView* l_oView : m_vSettingViewVector)
		{
			if(l_oView->getSettingIndex() >= ui32SettingIndex)
			{
				l_oView->setSettingIndex(l_oView->getSettingIndex() + 1);
			}
		}
	}
}

void CBoxConfigurationDialog::removeSetting(uint32_t ui32SettingIndex, bool bShift)
{
	int32 i32TableIndex = getTableIndex(ui32SettingIndex);

	if(i32TableIndex != -1)
	{
		Setting::CAbstractSettingView *l_oView = m_vSettingViewVector[i32TableIndex];
		::GtkWidget* l_pName = l_oView->getNameWidget();
		::GtkWidget* l_pEntry = l_oView->getEntryWidget();

		gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_pName);
		gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_pEntry);

		delete l_oView;
		m_vSettingViewVector.erase(m_vSettingViewVector.begin() + i32TableIndex);

		//Now if we need to do it we shift everything to avoid an empty row in the table
		if(bShift){

			for(size_t i = i32TableIndex; i < m_vSettingViewVector.size() ; ++i)
			{
				Setting::CAbstractSettingView *l_oView = m_vSettingViewVector[i];
				l_oView->setSettingIndex(l_oView->getSettingIndex() - 1);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getNameWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getNameWidget() ,   0, 1, i, i+1, ::GtkAttachOptions(GTK_FILL), ::GtkAttachOptions(GTK_FILL), 0, 0);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getEntryWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getEntryWidget(),   1, 4, i, i+1, ::GtkAttachOptions(GTK_SHRINK|GTK_FILL|GTK_EXPAND), ::GtkAttachOptions(GTK_SHRINK), 0, 0);
			}
			//Now let's resize everything
			gtk_table_resize(m_pSettingsTable, m_vSettingViewVector.size()+2, 4);
			updateSize();
		}
	}
	//Even if we delete an "invisible" setting we need to update every index.
	else
	{
		for(Setting::CAbstractSettingView* l_oView : m_vSettingViewVector)
		{
			if(l_oView->getSettingIndex() >= ui32SettingIndex)
			{
				l_oView->setSettingIndex(l_oView->getSettingIndex() - 1);
			}
		}
	}
}

int32_t CBoxConfigurationDialog::getTableIndex(uint32_t ui32SettingIndex)
{
	uint32 ui32TableIndex=0;
	for (auto it = m_vSettingViewVector.begin() ; it != m_vSettingViewVector.end(); ++it, ++ui32TableIndex)
	{
		Setting::CAbstractSettingView *l_pView = *it;
		if(l_pView->getSettingIndex() == ui32SettingIndex){
			return ui32SettingIndex;
		}
	}

	return -1;
}

void CBoxConfigurationDialog::updateSize()
{
	// Resize the window to fit as much of the table as possible, but keep the max size
	// limited so it doesn't get outside the screen. For safety, we cap to 800x600
	// anyway to hopefully prevent the window from going under things such as the gnome toolbar.
	// The ui file at the moment does not allow resize of this window because the result
	// looked ugly if the window was made overly large, and no satisfying solution at the time was
	// found by the limited intellectual resources available.
	const uint32 l_ui32MaxWidth = std::min(800,gdk_screen_get_width(gdk_screen_get_default()));
	const uint32 l_ui32MaxHeight = std::min(600,gdk_screen_get_height(gdk_screen_get_default()));
	GtkRequisition l_oSize;
	gtk_widget_size_request(GTK_WIDGET(m_pViewPort), &l_oSize);
	gtk_widget_set_size_request(GTK_WIDGET(m_pScrolledWindow),
		std::min(l_ui32MaxWidth,(uint32)l_oSize.width),
								std::min(l_ui32MaxHeight,(uint32)l_oSize.height));
}

void CBoxConfigurationDialog::saveConfiguration()
{
	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
		"Select file to save settings to...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	const gchar* l_sInitialFileNameToExpand = gtk_entry_get_text(GTK_ENTRY(m_pOverrideEntryContainer));
	CString l_sInitialFileName=m_rKernelContext.getConfigurationManager().expand(l_sInitialFileNameToExpand);
	if(g_path_is_absolute(l_sInitialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sInitialFileName.toASCIIString());
	}
	else
	{
		char* l_sFullPath=g_build_filename(g_get_current_dir(), l_sInitialFileName.toASCIIString(), NULL);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sFullPath);
		g_free(l_sFullPath);
	}

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));

		XML::IXMLHandler *l_pHandler = XML::createXMLHandler();
		XML::IXMLNode *l_pRootNode = XML::createNode(c_sRootName);
		for(size_t i = 0; i < m_rBox.getSettingCount() ; ++i)
		{
			XML::IXMLNode *l_pTempNode = XML::createNode(c_sSettingName);
			CString l_sValue;
			m_rBox.getSettingValue(i, l_sValue);
			l_pTempNode->setPCData(l_sValue.toASCIIString());

			l_pRootNode->addChild(l_pTempNode);
		}

		l_pHandler->writeXMLInFile(*l_pRootNode, l_sFileName);

		l_pHandler->release();
		l_pRootNode->release();
		g_free(l_sFileName);

	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
}

void CBoxConfigurationDialog::loadConfiguration()
{
	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
		"Select file to load settings from...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	const gchar* l_sInitialFileNameToExpand = gtk_entry_get_text(GTK_ENTRY(m_pOverrideEntryContainer));

	CString l_sInitialFileName=m_rKernelContext.getConfigurationManager().expand(l_sInitialFileNameToExpand);
	if(g_path_is_absolute(l_sInitialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sInitialFileName.toASCIIString());
	}
	else
	{
		char* l_sFullPath=g_build_filename(g_get_current_dir(), l_sInitialFileName.toASCIIString(), NULL);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sFullPath);
		g_free(l_sFullPath);
	}

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));

		XML::IXMLHandler *l_pHandler = XML::createXMLHandler();
		XML::IXMLNode *l_pRootNode = l_pHandler->parseFile(l_sFileName);

		for(size_t i = 0 ; i<l_pRootNode->getChildCount() ; ++i)
		{
			//Hope everything will fit in the right place
			m_rBox.setSettingValue(i, l_pRootNode->getChild(i)->getPCData());
		}

		l_pRootNode->release();
		l_pHandler->release();
		g_free(l_sFileName);

	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
}

void CBoxConfigurationDialog::onOverrideBrowse()
{
	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
		"Select file to open...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	CString l_sInitialFileName=m_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(GTK_ENTRY(m_pOverrideEntry)));
	if(g_path_is_absolute(l_sInitialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sInitialFileName.toASCIIString());
	}
	else
	{
		char* l_sFullPath=g_build_filename(g_get_current_dir(), l_sInitialFileName.toASCIIString(), NULL);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sFullPath);
		g_free(l_sFullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), false);

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		gchar* cFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		std::string fileName(cFileName);
		g_free(cFileName);
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
		gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntry), fileName.c_str());
	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
}


void CBoxConfigurationDialog::storeState(void)
{
	m_SettingsMemory.clear();
	for (uint32_t i =0; i < m_rBox.getSettingCount(); i++)
	{
		OpenViBE::CString temp;
		m_rBox.getSettingValue(i, temp);
		m_SettingsMemory.push_back(temp);
	}
}

void CBoxConfigurationDialog::restoreState(void)
{
	for (uint32_t i =0; i < m_SettingsMemory.size(); i++)
	{
		if (i >= m_rBox.getSettingCount())
		{
			// This is not supposed to happen
			return;
		}
		m_rBox.setSettingValue(i, m_SettingsMemory[i]);
	}
}

::GtkWidget* CBoxConfigurationDialog::getWidget()
{
	return m_pSettingDialog;
}

const CIdentifier CBoxConfigurationDialog::getBoxID() const
{
	return m_rBox.getIdentifier();
}
