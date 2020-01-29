#include "ovdCSettingEditorDialog.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Designer;
using namespace std;

static void TypeChangedCB(GtkComboBox* /*widget*/, gpointer data) { static_cast<CSettingEditorDialog*>(data)->typeChangedCB(); }

bool CSettingEditorDialog::run()

{
	GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(m_guiFilename.toASCIIString(), "setting_editor", nullptr);
	gtk_builder_add_from_file(builder, m_guiFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(builder, nullptr);

	GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor"));
	GtkWidget* name   = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-setting_name_entry"));
	m_table           = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-table"));
	m_type            = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-setting_type_combobox"));
	gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(m_type))));
	g_object_unref(builder);

	gtk_window_set_title(GTK_WINDOW(dialog), m_title.c_str());

	g_signal_connect(G_OBJECT(m_type), "changed", G_CALLBACK(TypeChangedCB), this);

	CString settingName;
	CIdentifier settingType;
	m_box.getSettingName(m_settingIdx, settingName);
	m_box.getSettingType(m_settingIdx, settingType);

	gtk_entry_set_text(GTK_ENTRY(name), settingName.toASCIIString());

	gint active      = -1;
	size_t nSettings = 0; // Cannot rely on m_settingTypes.size() -- if there are any duplicates, it wont increment properly (and should be an error anyway) ...

	for (const auto& currentTypeID : m_kernelCtx.getTypeManager().getSortedTypes())
	{
		if (!m_kernelCtx.getTypeManager().isStream(currentTypeID.first))
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(m_type), currentTypeID.second.toASCIIString());
			if (currentTypeID.first == settingType) { active = gint(nSettings); }
			m_settingTypes[currentTypeID.second.toASCIIString()] = currentTypeID.first;
			nSettings++;
		}
	}

	if (active != -1) { gtk_combo_box_set_active(GTK_COMBO_BOX(m_type), active); }

	bool finished = false;
	bool res      = false;
	while (!finished)
	{
		const gint result = gtk_dialog_run(GTK_DIALOG(dialog));
		if (result == GTK_RESPONSE_APPLY)
		{
			char* activeText = gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_type));
			if (activeText)
			{
				settingType = m_settingTypes[activeText];
				m_box.setSettingName(m_settingIdx, gtk_entry_get_text(GTK_ENTRY(name)));
				m_box.setSettingType(m_settingIdx, settingType);
				m_box.setSettingValue(m_settingIdx, m_helper.getValue(settingType, m_defaultValue));
				m_box.setSettingDefaultValue(m_settingIdx, m_helper.getValue(settingType, m_defaultValue));
				finished = true;
				res      = true;
			}
		}
		else if (result == 2) // revert
		{
			gtk_entry_set_text(GTK_ENTRY(name), settingName.toASCIIString());
			if (active != -1) { gtk_combo_box_set_active(GTK_COMBO_BOX(m_type), active); }
		}
		else
		{
			finished = true;
			res      = false;
		}
	}

	gtk_widget_destroy(dialog);
	return res;
}

void CSettingEditorDialog::typeChangedCB()

{
	const CIdentifier settingType = m_settingTypes[gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_type))];

	const char* name         = m_helper.getSettingWidgetName(settingType).toASCIIString();
	GtkBuilder* builderDummy = gtk_builder_new(); // glade_xml_new(m_guiFilename.toASCIIString(), name, nullptr);
	gtk_builder_add_from_file(builderDummy, m_guiSettingsFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(builderDummy, nullptr);

	if (m_defaultValue) { gtk_container_remove(GTK_CONTAINER(m_table), m_defaultValue); }
	m_defaultValue = GTK_WIDGET(gtk_builder_get_object(builderDummy, name));
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(m_defaultValue)), m_defaultValue);
	gtk_table_attach(GTK_TABLE(m_table), m_defaultValue, 1, 2, 2, 3, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(GTK_FILL | GTK_EXPAND), 0, 0);
	g_object_unref(builderDummy);

	CString value;
	m_box.getSettingDefaultValue(m_settingIdx, value);
	m_helper.setValue(settingType, m_defaultValue, value);
}
