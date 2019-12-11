#include "ovdCRenameDialog.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

bool CRenameDialog::run()
{
	GtkBuilder* interface = gtk_builder_new(); // glade_xml_new(m_guiFilename.toASCIIString(), "rename", nullptr);
	gtk_builder_add_from_file(interface, m_guiFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(interface, nullptr);

	GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(interface, "rename"));
	GtkWidget* name   = GTK_WIDGET(gtk_builder_get_object(interface, "rename-entry"));
	g_object_unref(interface);

	gtk_entry_set_text(GTK_ENTRY(name), m_initialName.toASCIIString());

	bool finished = false;
	bool res      = false;
	while (!finished)
	{
		const gint valid = gtk_dialog_run(GTK_DIALOG(dialog));
		if (valid == GTK_RESPONSE_APPLY)
		{
			m_result = gtk_entry_get_text(GTK_ENTRY(name));
			finished = true;
			res      = true;
		}
		else if (valid == 1) { gtk_entry_set_text(GTK_ENTRY(name), m_defaultName.toASCIIString()); } // default
		else if (valid == 2) { gtk_entry_set_text(GTK_ENTRY(name), m_initialName.toASCIIString()); } // revert
		else
		{
			finished = true;
			res      = false;
		}
	}

	gtk_widget_destroy(dialog);

	return res;
}
