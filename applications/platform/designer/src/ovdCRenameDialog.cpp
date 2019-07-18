#include "ovdCRenameDialog.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

CRenameDialog::CRenameDialog(const IKernelContext& rKernelContext, const CString& rInitialName, const CString& rDefaultName, const char* sGUIFilename)
	: m_kernelContext(rKernelContext), m_sInitialName(rInitialName), m_sDefaultName(rDefaultName), m_sResult(rInitialName), m_sGUIFilename(sGUIFilename) { }

CRenameDialog::~CRenameDialog() = default;

bool CRenameDialog::run()
{
	GtkBuilder* interface = gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "rename", nullptr);
	gtk_builder_add_from_file(interface, m_sGUIFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(interface, nullptr);

	GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(interface, "rename"));
	GtkWidget* l_pName = GTK_WIDGET(gtk_builder_get_object(interface, "rename-entry"));
	g_object_unref(interface);

	gtk_entry_set_text(GTK_ENTRY(l_pName), m_sInitialName.toASCIIString());

	bool l_bFinished = false;
	bool l_bResult = false;
	while (!l_bFinished)
	{
		const gint l_iResult = gtk_dialog_run(GTK_DIALOG(l_pDialog));
		if (l_iResult == GTK_RESPONSE_APPLY)
		{
			m_sResult = gtk_entry_get_text(GTK_ENTRY(l_pName));
			l_bFinished = true;
			l_bResult = true;
		}
		else if (l_iResult == 1) // default
		{
			gtk_entry_set_text(GTK_ENTRY(l_pName), m_sDefaultName.toASCIIString());
		}
		else if (l_iResult == 2) // revert
		{
			gtk_entry_set_text(GTK_ENTRY(l_pName), m_sInitialName.toASCIIString());
		}
		else
		{
			l_bFinished = true;
			l_bResult = false;
		}
	}

	gtk_widget_destroy(l_pDialog);

	return l_bResult;
}
