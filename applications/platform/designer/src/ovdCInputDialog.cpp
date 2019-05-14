#include "ovdCInputDialog.h"
#include <gdk/gdkkeysyms.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;

CInputDialog::CInputDialog(const char* sGtkBuilder, const fpButtonCB fpOKButtonCB, void* data, const char* sTitle, const char* sLabel, const char* sEntry)
{
	m_fpOKButtonCB = fpOKButtonCB;
	m_pUserData = data;

	//retrieve input dialog
	GtkBuilder* l_pInputDialogInterface = gtk_builder_new(); // glade_xml_new(sGtkBuilder, "input", nullptr);
	gtk_builder_add_from_file(l_pInputDialogInterface, sGtkBuilder, nullptr);
	gtk_builder_connect_signals(l_pInputDialogInterface, nullptr);

	m_pInputDialog = GTK_DIALOG(gtk_builder_get_object(l_pInputDialogInterface, "input"));
	m_pInputDialogLabel = GTK_LABEL(gtk_builder_get_object(l_pInputDialogInterface, "input-label"));
	m_pInputDialogEntry = GTK_ENTRY(gtk_builder_get_object(l_pInputDialogInterface, "input-entry"));
	m_pInputDialogOKButton = GTK_BUTTON(gtk_builder_get_object(l_pInputDialogInterface, "input-button_ok"));
	m_pInputDialogCancelButton = GTK_BUTTON(gtk_builder_get_object(l_pInputDialogInterface, "input-button_cancel"));

	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(m_pInputDialogEntry), GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(m_pInputDialogEntry), "key-press-event", G_CALLBACK(key_press_event_cb), m_pInputDialog);

	if (sLabel != nullptr) { gtk_label_set(m_pInputDialogLabel, sLabel); }
	if (sEntry != nullptr) { gtk_entry_set_text(m_pInputDialogEntry, sEntry); }

	g_signal_connect(G_OBJECT(m_pInputDialogOKButton), "clicked", G_CALLBACK(button_clicked_cb), this);
	g_signal_connect(G_OBJECT(m_pInputDialogCancelButton), "clicked", G_CALLBACK(button_clicked_cb), this);

	gtk_window_set_position(GTK_WINDOW(m_pInputDialog), GTK_WIN_POS_MOUSE);
	gtk_window_set_title(GTK_WINDOW(m_pInputDialog), sTitle);
}

CInputDialog::~CInputDialog()
{
	gtk_widget_destroy(GTK_WIDGET(m_pInputDialog));
}

void CInputDialog::run()
{
	const gint l_iResult = gtk_dialog_run(m_pInputDialog);

	if (l_iResult == GTK_RESPONSE_ACCEPT)
	{
		if (m_fpOKButtonCB != nullptr) { m_fpOKButtonCB(GTK_WIDGET(m_pInputDialogOKButton), this); }
	}

	gtk_widget_hide_all(GTK_WIDGET(m_pInputDialog));
}

gboolean CInputDialog::key_press_event_cb(GtkWidget* /*pWidget*/, GdkEventKey* pEventKey, gpointer data)
{
	if (pEventKey->keyval == GDK_Return || pEventKey->keyval == GDK_KP_Enter)
	{
		gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_ACCEPT);
		return TRUE;
	}
	if (pEventKey->keyval == GDK_Escape)
	{
		gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_REJECT);
		return TRUE;
	}

	return FALSE;
}

void CInputDialog::button_clicked_cb(GtkButton* pButton, gpointer data)
{
	static_cast<CInputDialog*>(data)->buttonClickedCB(pButton);
}

void CInputDialog::buttonClickedCB(GtkButton* pButton)
{
	if (pButton == m_pInputDialogOKButton)
	{
		gtk_dialog_response(m_pInputDialog, GTK_RESPONSE_ACCEPT);
	}
	else
	{
		gtk_dialog_response(m_pInputDialog, GTK_RESPONSE_REJECT);
	}
}
