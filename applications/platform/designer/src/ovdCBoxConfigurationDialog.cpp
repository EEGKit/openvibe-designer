#include "ovdCBoxConfigurationDialog.h"
#include "ovdCSettingCollectionHelper.h"

#include <vector>
#include <string>

#include <xml/IXMLHandler.h>
#include <xml/IXMLNode.h>

#include <fs/Files.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	const char* const ROOT_NAME    = "OpenViBE-SettingsOverride";
	const char* const SETTING_NAME = "SettingValue";
} // namespace

static void onFileOverrideCheckToggled(GtkToggleButton* pToggleButton, gpointer data)
{
	gtk_widget_set_sensitive(static_cast<GtkWidget*>(data), !gtk_toggle_button_get_active(pToggleButton));
}

static void OnButtonLoadClicked(GtkButton*, gpointer data) { static_cast<CBoxConfigurationDialog*>(data)->loadConfiguration(); }

static void OnButtonSaveClicked(GtkButton*, gpointer data) { static_cast<CBoxConfigurationDialog*>(data)->saveConfiguration(); }

static void OnOverrideBrowseClicked(GtkButton* /*button*/, gpointer data) { static_cast<CBoxConfigurationDialog*>(data)->onOverrideBrowse(); }

static void CollectWidgetCB(GtkWidget* widget, gpointer data) { static_cast<std::vector<GtkWidget*>*>(data)->push_back(widget); }

CBoxConfigurationDialog::CBoxConfigurationDialog(const IKernelContext& ctx, IBox& box, const char* sGUIFilename, const char* sGUISettingsFilename,
												 const bool isScenarioRunning)
	: m_kernelCtx(ctx), m_box(box), m_sGUIFilename(sGUIFilename), m_sGUISettingsFilename(sGUISettingsFilename)
	  , m_oSettingFactory(m_sGUISettingsFilename.toASCIIString(), ctx), m_isScenarioRunning(isScenarioRunning)
{
	m_box.addObserver(this);

	if (m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting))
	{
		GtkBuilder* l_pBuilderInterfaceSetting = gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "box_configuration", nullptr);
		gtk_builder_add_from_file(l_pBuilderInterfaceSetting, m_sGUIFilename.toASCIIString(), nullptr);
		gtk_builder_connect_signals(l_pBuilderInterfaceSetting, nullptr);

		if (!m_isScenarioRunning)
		{
			// TODO : This is not a modal dialog. It would be better if it was.
			m_pSettingDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration"));
			char l_sTitle[1024];
			sprintf(l_sTitle, "Configure %s settings", m_box.getName().toASCIIString());
			gtk_window_set_title(GTK_WINDOW(m_pSettingDialog), l_sTitle);
		}
		else
		{
			// This is actually *not* a dialog
			m_pSettingDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-scrolledwindow"));
		}
		m_pSettingsTable  = GTK_TABLE(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-table"));
		m_pScrolledWindow = GTK_SCROLLED_WINDOW(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-scrolledwindow"));
		m_pViewPort       = GTK_VIEWPORT(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-viewport"));

		gtk_table_resize(m_pSettingsTable, m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting), 4);

		generateSettingsTable();

		const CSettingCollectionHelper l_oHelper(m_kernelCtx, m_sGUISettingsFilename.toASCIIString());

		if (!m_isScenarioRunning)
		{
			GtkContainer* l_pFileOverrideContainer = GTK_CONTAINER(
				gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-hbox_filename_override"));
			m_pFileOverrideCheck     = GTK_CHECK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-checkbutton_filename_override"));
			GtkButton* l_pButtonLoad = GTK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-button_load_current_from_file"));
			GtkButton* l_pButtonSave = GTK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-button_save_current_to_file"));

			const string l_sSettingOverrideWidgetName        = l_oHelper.getSettingWidgetName(OV_TypeId_Filename).toASCIIString();
			GtkBuilder* l_pBuilderInterfaceSettingCollection =
					gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), l_sSettingOverrideWidgetName.c_str(), nullptr);
			gtk_builder_add_from_file(l_pBuilderInterfaceSettingCollection, m_sGUISettingsFilename.toASCIIString(), nullptr);
			//			gtk_builder_connect_signals(l_pBuilderInterfaceSettingCollection, nullptr);

			m_pOverrideEntryContainer = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSettingCollection, l_sSettingOverrideWidgetName.c_str()));

			std::vector<GtkWidget*> widgets;
			gtk_container_foreach(GTK_CONTAINER(m_pOverrideEntryContainer), CollectWidgetCB, &widgets);

			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(m_pOverrideEntryContainer)), m_pOverrideEntryContainer);
			gtk_container_add(l_pFileOverrideContainer, m_pOverrideEntryContainer);
			m_pOverrideEntry = GTK_ENTRY(widgets[0]);


			g_signal_connect(G_OBJECT(m_pFileOverrideCheck), "toggled", G_CALLBACK(onFileOverrideCheckToggled), GTK_WIDGET(m_pSettingsTable));
			g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnOverrideBrowseClicked), this);
			g_signal_connect(G_OBJECT(l_pButtonLoad), "clicked", G_CALLBACK(OnButtonLoadClicked), this);
			g_signal_connect(G_OBJECT(l_pButtonSave), "clicked", G_CALLBACK(OnButtonSaveClicked), this);

			if (m_box.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
			{
				GtkExpander* l_pExpander = GTK_EXPANDER(gtk_builder_get_object(l_pBuilderInterfaceSetting, "box_configuration-expander"));
				gtk_expander_set_expanded(l_pExpander, true);

				gtk_entry_set_text(m_pOverrideEntry, m_box.getAttributeValue(OV_AttributeId_Box_SettingOverrideFilename).toASCIIString());
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), true);
				gtk_widget_set_sensitive(GTK_WIDGET(m_pSettingsTable), false);
			}
			else { gtk_entry_set_text(m_pOverrideEntry, ""); }

			g_object_unref(l_pBuilderInterfaceSetting);
			g_object_unref(l_pBuilderInterfaceSettingCollection);
		}
	}
}

CBoxConfigurationDialog::~CBoxConfigurationDialog()
{
	m_box.deleteObserver(this);
	if (m_pSettingDialog) { gtk_widget_destroy(m_pSettingDialog); }
}

bool CBoxConfigurationDialog::run()
{
	bool modified = false;
	if (m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting))
	{
		CSettingCollectionHelper l_oHelper(m_kernelCtx, m_sGUISettingsFilename.toASCIIString());
		storeState();
		bool finished = false;
		while (!finished)
		{
			const gint result = gtk_dialog_run(GTK_DIALOG(m_pSettingDialog));
			if (result == GTK_RESPONSE_APPLY)
			{
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck)))
				{
					const gchar* fileName = gtk_entry_get_text(m_pOverrideEntry);
					if (m_box.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
					{
						m_box.setAttributeValue(OV_AttributeId_Box_SettingOverrideFilename, fileName);
					}
					else { m_box.addAttribute(OV_AttributeId_Box_SettingOverrideFilename, fileName); }
				}
				else
				{
					if (m_box.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename)) { m_box.removeAttribute(OV_AttributeId_Box_SettingOverrideFilename); }
				}

				finished = true;
				modified = true;
			}
			else if (result == GTK_RESPONSE_CANCEL)
			{
				restoreState();
				finished = true;
			}
			else if (result == 1) // default
			{
				// Some settings will add/remove other settings;
				// by evaluating m_box.getSettingCount() each time we ensure not ending somewhere in the oblivion
				for (uint32_t i = 0; i < m_vSettingViewVector.size(); ++i)
				{
					CString l_oSettingValue;
					m_box.getSettingDefaultValue(i, l_oSettingValue);
					m_box.setSettingValue(i, l_oSettingValue);
					//					m_vSettingViewVector[i]->setValue(l_oSettingValue);
					//					l_oHelper.setValue(l_oSettingType, i < m_vSettingViewVector.size()? m_vSettingViewVector[i]->getEntryWidget() : nullptr, l_oSettingValue);
				}
				gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntryContainer), "");
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), false);
				//				gtk_widget_set_sensitive(GTK_WIDGET(m_pSettingsTable), true);
				modified = false;
			}
			else if (result == 2) // revert
			{
				restoreState();

				if (m_box.hasAttribute(OV_AttributeId_Box_SettingOverrideFilename))
				{
					gtk_entry_set_text(
						GTK_ENTRY(m_pOverrideEntryContainer), m_box.getAttributeValue(OV_AttributeId_Box_SettingOverrideFilename).toASCIIString());
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), true);
				}
				else
				{
					gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntryContainer), "");
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pFileOverrideCheck), false);
				}
			}
			else if (result == 3) { modified = true; }	// load
			else if (result == 4) {}					// save 
			else { finished = true; }
		}
	}
	return modified;
}

void CBoxConfigurationDialog::update(CObservable& /*o*/, void* data)
{
	const BoxEventMessage* l_pEvent = static_cast<BoxEventMessage*>(data);

	switch (l_pEvent->m_Type)
	{
		case SettingsAllChange:
			generateSettingsTable();
			break;

		case SettingValueUpdate:
		{
			CString l_sSettingValue;
			m_box.getSettingValue(l_pEvent->m_FirstIdx, l_sSettingValue);

			m_vSettingViewVector[l_pEvent->m_FirstIdx]->setValue(l_sSettingValue);
			break;
		}

		case SettingDelete:
			removeSetting(l_pEvent->m_FirstIdx);
			break;

		case SettingAdd:
			addSetting(l_pEvent->m_FirstIdx);
			break;

		case SettingChange:
			settingChange(l_pEvent->m_FirstIdx);
			break;

		default:
			//			OV_ERROR_KRF("wtf", ErrorType::BadSetting);
			break;
	}
}

void CBoxConfigurationDialog::generateSettingsTable()
{
	std::for_each(m_vSettingViewVector.begin(), m_vSettingViewVector.end(), [](Setting::CAbstractSettingView* elem) { delete elem; });
	m_vSettingViewVector.clear();
	//Remove rows
	gtk_container_foreach(GTK_CONTAINER(GTK_WIDGET(m_pSettingsTable)),
						  [](GtkWidget* widget, gpointer data) { gtk_container_remove(GTK_CONTAINER(data), widget); },
						  GTK_WIDGET(m_pSettingsTable));

	uint32_t l_ui32TableSize = 0;
	if (m_isScenarioRunning)
	{
		for (uint32_t i = 0; i < m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting); ++i)
		{
			bool mod = false;
			m_box.getSettingMod(i, mod);
			if (mod) { l_ui32TableSize++; }
		}
	}
	else { l_ui32TableSize = m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting); }
	gtk_table_resize(m_pSettingsTable, l_ui32TableSize + 2, 4);

	// Iterate over box settings, generate corresponding gtk widgets. If the scenario is running, we are making a
	// 'modifiable settings' dialog and use a subset of widgets with a slightly different layout and buttons.
	for (uint32_t settingIndex = 0, tableIndex = 0; settingIndex < m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting); ++settingIndex)
	{
		if (addSettingsToView(settingIndex, tableIndex)) { ++tableIndex; }
	}
	updateSize();
}

bool CBoxConfigurationDialog::addSettingsToView(const uint32_t settingIndex, const uint32_t tableIndex)
{
	bool l_bSettingModifiable;
	m_box.getSettingMod(settingIndex, l_bSettingModifiable);

	if ((!m_isScenarioRunning) || (m_isScenarioRunning && l_bSettingModifiable))
	{
		CString l_sSettingName;

		m_box.getSettingName(settingIndex, l_sSettingName);
		Setting::CAbstractSettingView* l_oView = m_oSettingFactory.getSettingView(m_box, settingIndex);

		bool isSettingDeprecated = false;
		m_box.getInterfacorDeprecatedStatus(EBoxInterfacorType::Setting, settingIndex, isSettingDeprecated);
		if (isSettingDeprecated)
		{
			gtk_widget_set_sensitive(GTK_WIDGET(l_oView->getNameWidget()), false);
			gtk_widget_set_sensitive(GTK_WIDGET(l_oView->getEntryWidget()), false);
		}

		gtk_table_attach(m_pSettingsTable, l_oView->getNameWidget(), 0, 1, tableIndex, tableIndex + 1, GtkAttachOptions(GTK_FILL), GtkAttachOptions(GTK_FILL),
						 0, 0);
		gtk_table_attach(m_pSettingsTable, l_oView->getEntryWidget(), 1, 4, tableIndex, tableIndex + 1, GtkAttachOptions(GTK_SHRINK | GTK_FILL | GTK_EXPAND),
						 GtkAttachOptions(GTK_SHRINK), 0, 0);

		m_vSettingViewVector.insert(m_vSettingViewVector.begin() + tableIndex, l_oView);

		return true;
	}
	return false;
}

void CBoxConfigurationDialog::settingChange(const uint32_t settingIndex)
{
	//We remeber the place to add the new setting at the same place
	const uint32_t indexTable = getTableIndex(settingIndex);

	removeSetting(settingIndex, false);
	addSettingsToView(settingIndex, indexTable);
}

void CBoxConfigurationDialog::addSetting(const uint32_t settingIndex)
{
	bool l_bSettingModifiable;
	m_box.getSettingMod(settingIndex, l_bSettingModifiable);

	if ((!m_isScenarioRunning) || (m_isScenarioRunning && l_bSettingModifiable))
	{
		const size_t tableSize = m_vSettingViewVector.size();
		/*There is two case.
		1) we just add at the end of the setting box
		2) we add it in the middle end we need to shift
		*/
		const size_t tableIndex = (settingIndex > m_vSettingViewVector[tableSize - 1]->getSettingIndex())
									  ? tableSize
									  : getTableIndex(settingIndex);

		gtk_table_resize(m_pSettingsTable, guint(tableSize + 2), 4);

		if (settingIndex <= m_vSettingViewVector[tableSize - 1]->getSettingIndex())
		{
			for (size_t i = tableSize - 1; i >= tableIndex; --i)
			{
				Setting::CAbstractSettingView* l_oView = m_vSettingViewVector[i];

				//We need to update the index
				l_oView->setSettingIndex(l_oView->getSettingIndex() + 1);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getNameWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getNameWidget(), 0, 1, guint(i + 1), guint(i + 2), GtkAttachOptions(GTK_FILL),
								 GtkAttachOptions(GTK_FILL), 0, 0);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getEntryWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getEntryWidget(), 1, 4, guint(i + 1), guint(i + 2),
								 GtkAttachOptions(GTK_SHRINK | GTK_FILL | GTK_EXPAND), GtkAttachOptions(GTK_SHRINK), 0, 0);
			}
		}
		addSettingsToView(uint32_t(tableIndex), settingIndex);
		updateSize();
	}
		//Even if nothing is add to the interface, we still need to update index
	else
	{
		for (Setting::CAbstractSettingView* l_oView : m_vSettingViewVector)
		{
			if (l_oView->getSettingIndex() >= settingIndex) { l_oView->setSettingIndex(l_oView->getSettingIndex() + 1); }
		}
	}
}

void CBoxConfigurationDialog::removeSetting(const uint32_t settingIndex, const bool shift)
{
	const int tableIndex = getTableIndex(settingIndex);

	if (tableIndex != -1)
	{
		Setting::CAbstractSettingView* l_oView = m_vSettingViewVector[tableIndex];
		GtkWidget* l_pName                     = l_oView->getNameWidget();
		GtkWidget* l_pEntry                    = l_oView->getEntryWidget();

		gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_pName);
		gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_pEntry);

		delete l_oView;
		m_vSettingViewVector.erase(m_vSettingViewVector.begin() + tableIndex);

		//Now if we need to do it we shift everything to avoid an empty row in the table
		if (shift)
		{
			for (size_t i = tableIndex; i < m_vSettingViewVector.size(); ++i)
			{
				l_oView = m_vSettingViewVector[i];
				l_oView->setSettingIndex(l_oView->getSettingIndex() - 1);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getNameWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getNameWidget(), 0, 1, guint(i), guint(i + 1), GtkAttachOptions(GTK_FILL),
								 GtkAttachOptions(GTK_FILL), 0, 0);

				gtk_container_remove(GTK_CONTAINER(m_pSettingsTable), l_oView->getEntryWidget());
				gtk_table_attach(m_pSettingsTable, l_oView->getEntryWidget(), 1, 4, guint(i), guint(i + 1),
								 GtkAttachOptions(GTK_SHRINK | GTK_FILL | GTK_EXPAND), GtkAttachOptions(GTK_SHRINK), 0, 0);
			}
			//Now let's resize everything
			gtk_table_resize(m_pSettingsTable, guint(m_vSettingViewVector.size() + 2), 4);
			updateSize();
		}
	}
		//Even if we delete an "invisible" setting we need to update every index.
	else
	{
		for (Setting::CAbstractSettingView* l_oView : m_vSettingViewVector)
		{
			if (l_oView->getSettingIndex() >= settingIndex) { l_oView->setSettingIndex(l_oView->getSettingIndex() - 1); }
		}
	}
}

int CBoxConfigurationDialog::getTableIndex(const uint32_t settingIndex)
{
	uint32_t ui32TableIndex = 0;
	for (auto it = m_vSettingViewVector.begin(); it != m_vSettingViewVector.end(); ++it, ++ui32TableIndex)
	{
		Setting::CAbstractSettingView* l_pView = *it;
		if (l_pView->getSettingIndex() == settingIndex) { return settingIndex; }
	}

	return -1;
}

void CBoxConfigurationDialog::updateSize() const
{
	// Resize the window to fit as much of the table as possible, but keep the max size
	// limited so it doesn't get outside the screen. For safety, we cap to 800x600
	// anyway to hopefully prevent the window from going under things such as the gnome toolbar.
	// The ui file at the moment does not allow resize of this window because the result
	// looked ugly if the window was made overly large, and no satisfying solution at the time was
	// found by the limited intellectual resources available.
	const uint32_t maxWidth  = std::min(800, gdk_screen_get_width(gdk_screen_get_default()));
	const uint32_t maxHeight = std::min(600, gdk_screen_get_height(gdk_screen_get_default()));
	GtkRequisition l_oSize;
	gtk_widget_size_request(GTK_WIDGET(m_pViewPort), &l_oSize);
	gtk_widget_set_size_request(GTK_WIDGET(m_pScrolledWindow), std::min(maxWidth, uint32_t(l_oSize.width)), std::min(maxHeight, uint32_t(l_oSize.height)));
}

void CBoxConfigurationDialog::saveConfiguration() const
{
	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to save settings to...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, nullptr);

	const gchar* initialFileNameToExpand = gtk_entry_get_text(GTK_ENTRY(m_pOverrideEntryContainer));
	const CString initialFileName        = m_kernelCtx.getConfigurationManager().expand(initialFileNameToExpand);
	if (g_path_is_absolute(initialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
	}
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));

		XML::IXMLHandler* l_pHandler = XML::createXMLHandler();
		XML::IXMLNode* l_pRootNode   = XML::createNode(ROOT_NAME);
		for (size_t i = 0; i < m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting); ++i)
		{
			XML::IXMLNode* l_pTempNode = XML::createNode(SETTING_NAME);
			CString l_sValue;
			m_box.getSettingValue(uint32_t(i), l_sValue);
			l_pTempNode->setPCData(l_sValue.toASCIIString());

			l_pRootNode->addChild(l_pTempNode);
		}

		l_pHandler->writeXMLInFile(*l_pRootNode, fileName);

		l_pHandler->release();
		l_pRootNode->release();
		g_free(fileName);
	}
	gtk_widget_destroy(widgetDialogOpen);
}

void CBoxConfigurationDialog::loadConfiguration() const
{
	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to load settings from...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

	const gchar* initialFileNameToExpand = gtk_entry_get_text(GTK_ENTRY(m_pOverrideEntryContainer));

	const CString initialFileName = m_kernelCtx.getConfigurationManager().expand(initialFileNameToExpand);
	if (g_path_is_absolute(initialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
	}
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));

		XML::IXMLHandler* l_pHandler = XML::createXMLHandler();
		XML::IXMLNode* l_pRootNode   = l_pHandler->parseFile(fileName);

		for (size_t i = 0; i < l_pRootNode->getChildCount(); ++i)
		{
			//Hope everything will fit in the right place
			m_box.setSettingValue(uint32_t(i), l_pRootNode->getChild(i)->getPCData());
		}

		l_pRootNode->release();
		l_pHandler->release();
		g_free(fileName);
	}
	gtk_widget_destroy(widgetDialogOpen);
}

void CBoxConfigurationDialog::onOverrideBrowse() const
{
	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

	const CString initialFileName = m_kernelCtx.getConfigurationManager().expand(gtk_entry_get_text(GTK_ENTRY(m_pOverrideEntry)));
	if (g_path_is_absolute(initialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
	}
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		gchar* cFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		std::string fileName(cFileName);
		g_free(cFileName);
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
		gtk_entry_set_text(GTK_ENTRY(m_pOverrideEntry), fileName.c_str());
	}
	gtk_widget_destroy(widgetDialogOpen);
}


void CBoxConfigurationDialog::storeState()
{
	m_SettingsMemory.clear();
	for (uint32_t i = 0; i < m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting); ++i)
	{
		CString temp;
		m_box.getSettingValue(i, temp);
		m_SettingsMemory.push_back(temp);
	}
}

void CBoxConfigurationDialog::restoreState()
{
	for (uint32_t i = 0; i < m_SettingsMemory.size(); ++i)
	{
		if (i >= m_box.getInterfacorCountIncludingDeprecated(EBoxInterfacorType::Setting))
		{
			// This is not supposed to happen
			return;
		}
		m_box.setSettingValue(i, m_SettingsMemory[i]);
	}
}
