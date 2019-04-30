#include "ovdCSettingEditorDialog.h"

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;

static void type_changed_cb(GtkComboBox* pWidget, gpointer pUserData)
{
	static_cast<CSettingEditorDialog*>(pUserData)->typeChangedCB();
}

CSettingEditorDialog::CSettingEditorDialog(const IKernelContext& rKernelContext, IBox& rBox, uint32_t ui32SettingIndex, const char* sTitle, const char* sGUIFilename, const char* sGUISettingsFilename)
	:m_rKernelContext(rKernelContext)
	,m_rBox(rBox)
	,m_oHelper(rKernelContext, sGUIFilename)
	,m_ui32SettingIndex(ui32SettingIndex)
	,m_sGUIFilename(sGUIFilename)
	,m_sGUISettingsFilename(sGUISettingsFilename)
	,m_sTitle(sTitle)
	,m_pDefaultValue(nullptr) { }

CSettingEditorDialog::~CSettingEditorDialog() { }

bool CSettingEditorDialog::run()

{
	GtkBuilder* l_pBuilderInterfaceSetting=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "setting_editor", nullptr);
	gtk_builder_add_from_file(l_pBuilderInterfaceSetting, m_sGUIFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(l_pBuilderInterfaceSetting, nullptr);

	GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "setting_editor"));
	GtkWidget* l_pName=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "setting_editor-setting_name_entry"));
	m_pTable=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "setting_editor-table"));
	m_pType=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceSetting, "setting_editor-setting_type_combobox"));
	gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(m_pType))));
	g_object_unref(l_pBuilderInterfaceSetting);

	gtk_window_set_title(GTK_WINDOW(l_pDialog), m_sTitle.c_str());

	g_signal_connect(G_OBJECT(m_pType), "changed", G_CALLBACK(type_changed_cb), this);

	CString l_sSettingName;
	CIdentifier l_oSettingType;
	m_rBox.getSettingName(m_ui32SettingIndex, l_sSettingName);
	m_rBox.getSettingType(m_ui32SettingIndex, l_oSettingType);

	gtk_entry_set_text(GTK_ENTRY(l_pName), l_sSettingName.toASCIIString());

	gint l_iActive=-1;
	uint32_t numSettings = 0; // Cannot rely on m_vSettingTypes.size() -- if there are any duplicates, it wont increment properly (and should be an error anyway) ...
	
	for (auto l_oCurrentTypeIdentifier: m_rKernelContext.getTypeManager().getSortedTypes())
	{
		if(!m_rKernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier.first))
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(m_pType), l_oCurrentTypeIdentifier.second.toASCIIString());
			if(l_oCurrentTypeIdentifier.first == l_oSettingType)
			{
				l_iActive = numSettings;
			}
			m_vSettingTypes[l_oCurrentTypeIdentifier.second.toASCIIString()] = l_oCurrentTypeIdentifier.first;
			numSettings++;
		}
	}

	if(l_iActive!=-1)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_pType), l_iActive);
	}

	bool l_bFinished=false;
	bool l_bResult=false;
	while(!l_bFinished)
	{
		gint l_iResult=gtk_dialog_run(GTK_DIALOG(l_pDialog));
		if(l_iResult==GTK_RESPONSE_APPLY)
		{
			char* l_sActiveText=gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pType));
			if(l_sActiveText)
			{
				CIdentifier l_oSettingType=m_vSettingTypes[l_sActiveText];
				m_rBox.setSettingName(m_ui32SettingIndex, gtk_entry_get_text(GTK_ENTRY(l_pName)));
				m_rBox.setSettingType(m_ui32SettingIndex, l_oSettingType);
				m_rBox.setSettingValue(m_ui32SettingIndex, m_oHelper.getValue(l_oSettingType, m_pDefaultValue));
				m_rBox.setSettingDefaultValue(m_ui32SettingIndex, m_oHelper.getValue(l_oSettingType, m_pDefaultValue));
				l_bFinished=true;
				l_bResult=true;
			}
		}
		else if(l_iResult==2) // revert
		{
			gtk_entry_set_text(GTK_ENTRY(l_pName), l_sSettingName.toASCIIString());

			if(l_iActive!=-1)
			{
				gtk_combo_box_set_active(GTK_COMBO_BOX(m_pType), l_iActive);
			}
		}
		else
		{
			l_bFinished=true;
			l_bResult=false;
		}
	}

	gtk_widget_destroy(l_pDialog);

	return l_bResult;
}

void CSettingEditorDialog::typeChangedCB()

{
	CIdentifier l_oSettingType=m_vSettingTypes[gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pType))];

	CString l_sWidgetName=m_oHelper.getSettingWidgetName(l_oSettingType).toASCIIString();
	GtkBuilder* l_pBuilderInterfaceDefaultValueDummy=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), l_sWidgetName.toASCIIString(), nullptr);
	gtk_builder_add_from_file(l_pBuilderInterfaceDefaultValueDummy, m_sGUISettingsFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(l_pBuilderInterfaceDefaultValueDummy, nullptr);

	if(m_pDefaultValue) gtk_container_remove(GTK_CONTAINER(m_pTable), m_pDefaultValue);
	m_pDefaultValue=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceDefaultValueDummy, l_sWidgetName.toASCIIString()));
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(m_pDefaultValue)), m_pDefaultValue);
	gtk_table_attach(GTK_TABLE(m_pTable), m_pDefaultValue, 1, 2, 2, 3, GtkAttachOptions(GTK_FILL|GTK_EXPAND), GtkAttachOptions(GTK_FILL|GTK_EXPAND), 0, 0);
	g_object_unref(l_pBuilderInterfaceDefaultValueDummy);

	CString l_sDefaultValue;
	m_rBox.getSettingDefaultValue(m_ui32SettingIndex, l_sDefaultValue);
	m_oHelper.setValue(l_oSettingType, m_pDefaultValue, l_sDefaultValue);
}
