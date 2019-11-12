#include "ovdCInputDialog.h"
#include <gdk/gdkkeysyms.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;

CInputDialog::CInputDialog(const char* sGtkBuilder, const fpButtonCB fpOKButtonCB, void* data, const char* sTitle, const char* sLabel, const char* sEntry)
{
	m_fpOKButtonCB = fpOKButtonCB;
	m_pUserData    = data;

	//retrieve input dialog
	GtkBuilder* inputDialogInterface = gtk_builder_new(); // glade_xml_new(sGtkBuilder, "input", nullptr);
	gtk_builder_add_from_file(inputDialogInterface, sGtkBuilder, nullptr);
	gtk_builder_connect_signals(inputDialogInterface, nullptr);

	m_iDialog             = GTK_DIALOG(gtk_builder_get_object(inputDialogInterface, "input"));
	m_iDialogLabel        = GTK_LABEL(gtk_builder_get_object(inputDialogInterface, "input-label"));
	m_iDialogEntry        = GTK_ENTRY(gtk_builder_get_object(inputDialogInterface, "input-entry"));
	m_iDialogOKButton     = GTK_BUTTON(gtk_builder_get_object(inputDialogInterface, "input-button_ok"));
	m_iDialogCancelButton = GTK_BUTTON(gtk_builder_get_object(inputDialogInterface, "input-button_cancel"));

	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(m_iDialogEntry), GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(m_iDialogEntry), "key-press-event", G_CALLBACK(key_press_event_cb), m_iDialog);

	if (sLabel != nullptr) { gtk_label_set(m_iDialogLabel, sLabel); }
	if (sEntry != nullptr) { gtk_entry_set_text(m_iDialogEntry, sEntry); }

	g_signal_connect(G_OBJECT(m_iDialogOKButton), "clicked", G_CALLBACK(button_clicked_cb), this);
	g_signal_connect(G_OBJECT(m_iDialogCancelButton), "clicked", G_CALLBACK(button_clicked_cb), this);

	gtk_window_set_position(GTK_WINDOW(m_iDialog), GTK_WIN_POS_MOUSE);
	gtk_window_set_title(GTK_WINDOW(m_iDialog), sTitle);
}

CInputDialog::~CInputDialog() { gtk_widget_destroy(GTK_WIDGET(m_iDialog)); }

void CInputDialog::run()
{
	const gint res = gtk_dialog_run(m_iDialog);

	if (res == GTK_RESPONSE_ACCEPT) { if (m_fpOKButtonCB != nullptr) { m_fpOKButtonCB(GTK_WIDGET(m_iDialogOKButton), this); } }

	gtk_widget_hide_all(GTK_WIDGET(m_iDialog));
}

gboolean CInputDialog::key_press_event_cb(GtkWidget* /*widget*/, GdkEventKey* eventKey, gpointer data)
{
	if (eventKey->keyval == GDK_Return || eventKey->keyval == GDK_KP_Enter)
	{
		gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_ACCEPT);
		return TRUE;
	}
	if (eventKey->keyval == GDK_Escape)
	{
		gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_REJECT);
		return TRUE;
	}

	return FALSE;
}

void CInputDialog::button_clicked_cb(GtkButton* button, gpointer data) { static_cast<CInputDialog*>(data)->buttonClickedCB(button); }

void CInputDialog::buttonClickedCB(GtkButton* button) const
{
	if (button == m_iDialogOKButton) { gtk_dialog_response(m_iDialog, GTK_RESPONSE_ACCEPT); }
	else { gtk_dialog_response(m_iDialog, GTK_RESPONSE_REJECT); }
}
