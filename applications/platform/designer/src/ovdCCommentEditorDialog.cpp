#include "ovdCCommentEditorDialog.h"

#include <cstdlib>
#include <cstring>

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;


namespace
{
	void button_comment_bold_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<b>", "</b>");
	}

	void button_comment_italic_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<i>", "</i>");
	}

	void button_comment_underline_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<u>", "</u>");
	}

	void button_comment_strikethrough_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<s>", "</s>");
	}

	void button_comment_mono_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<tt>", "</tt>");
	}

	void button_comment_subscript_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<sub>", "</sub>");
	}

	void button_comment_superscript_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<sup>", "</sup>");
	}

	void button_comment_big_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<big>", "</big>");
	}

	void button_comment_small_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<small>", "</small>");
	}

	void button_comment_red_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<span color=\"red\">", "</span>");
	}

	void button_comment_green_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<span color=\"green\">", "</span>");
	}

	void button_comment_blue_selection_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->applyTagCB("<span color=\"blue\">", "</span>");
	}

	void button_comment_info_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CCommentEditorDialog*>(data)->infoCB();
	}
} // namespace

CCommentEditorDialog::CCommentEditorDialog(const IKernelContext& rKernelContext, IComment& rComment, const char* sGUIFilename)
	: m_kernelContext(rKernelContext), m_rComment(rComment), m_sGUIFilename(sGUIFilename) { }

bool CCommentEditorDialog::run()

{
	bool l_bResult = false;

	m_pInterface = gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "comment", nullptr);
	gtk_builder_add_from_file(m_pInterface, m_sGUIFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(m_pInterface, nullptr);

	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_bold"), "clicked", G_CALLBACK(button_comment_bold_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_italic"), "clicked", G_CALLBACK(button_comment_italic_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_underline"), "clicked", G_CALLBACK(button_comment_underline_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_strikethrough"), "clicked", G_CALLBACK(button_comment_strikethrough_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_mono"), "clicked", G_CALLBACK(button_comment_mono_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_subscript"), "clicked", G_CALLBACK(button_comment_subscript_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_superscript"), "clicked", G_CALLBACK(button_comment_superscript_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_big"), "clicked", G_CALLBACK(button_comment_big_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_small"), "clicked", G_CALLBACK(button_comment_small_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_red"), "clicked", G_CALLBACK(button_comment_red_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_green"), "clicked", G_CALLBACK(button_comment_green_selection_cb), this);
	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_blue"), "clicked", G_CALLBACK(button_comment_blue_selection_cb), this);

	::g_signal_connect(::gtk_builder_get_object(m_pInterface, "comment_toolbutton_info"), "clicked", G_CALLBACK(button_comment_info_cb), this);

	m_pDialog      = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "comment"));
	m_pDescription = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "comment-textview_description"));

	m_pInfoDialog = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "messagedialog_howto_comment"));
	::g_signal_connect(m_pInfoDialog, "close", G_CALLBACK(::gtk_widget_hide), nullptr);
	::g_signal_connect(m_pInfoDialog, "delete-event", G_CALLBACK(::gtk_widget_hide), nullptr);

	//::g_signal_connect(GTK_WIDGET(gtk_builder_get_object(m_pInterface, "messagedialog_howto_comment_button_close")), "clicked", G_CALLBACK(::gtk_widget_hide), nullptr);

	g_object_unref(m_pInterface);

	m_pDescriptionBuffer = gtk_text_buffer_new(nullptr);
	gtk_text_buffer_set_text(m_pDescriptionBuffer, m_rComment.getText().toASCIIString(), -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_pDescription), m_pDescriptionBuffer);
	g_object_unref(m_pDescriptionBuffer);

	gtk_widget_grab_focus(m_pDescription);

	const gint l_iResult = gtk_dialog_run(GTK_DIALOG(m_pDialog));
	if (l_iResult == GTK_RESPONSE_APPLY)
	{
		l_bResult = true;

		GtkTextIter l_oStartIter;
		GtkTextIter l_oEndIter;
		m_pDescriptionBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_pDescription));
		gtk_text_buffer_get_start_iter(m_pDescriptionBuffer, &l_oStartIter);
		gtk_text_buffer_get_end_iter(m_pDescriptionBuffer, &l_oEndIter);
		m_rComment.setText(gtk_text_buffer_get_text(m_pDescriptionBuffer, &l_oStartIter, &l_oEndIter, TRUE));
	}
	gtk_widget_destroy(m_pInfoDialog);
	gtk_widget_destroy(m_pDialog);

	return l_bResult;
}

//-----------------------------------------------------------------------------------
void CCommentEditorDialog::applyTagCB(const char* sTagIn, const char* sTagOut) const
{
	GtkTextIter l_oStartIter;
	GtkTextIter l_oEndIter;

	if (gtk_text_buffer_get_has_selection(m_pDescriptionBuffer))
	{
		gtk_text_buffer_get_selection_bounds(m_pDescriptionBuffer, &l_oStartIter, &l_oEndIter);
		gtk_text_buffer_insert(m_pDescriptionBuffer, &l_oStartIter, sTagIn, gint(strlen(sTagIn)));
		gtk_text_buffer_get_selection_bounds(m_pDescriptionBuffer, &l_oStartIter, &l_oEndIter);
		gtk_text_buffer_insert(m_pDescriptionBuffer, &l_oEndIter, sTagOut, gint(strlen(sTagOut)));

		// reset selection to the selected text, as the tagOut is now selected
		gtk_text_buffer_get_selection_bounds(m_pDescriptionBuffer, &l_oStartIter, &l_oEndIter);
		gtk_text_iter_backward_chars(&l_oEndIter, gint(strlen(sTagOut)));
		gtk_text_buffer_select_range(m_pDescriptionBuffer, &l_oStartIter, &l_oEndIter);
	}
	else
	{
		gtk_text_buffer_get_selection_bounds(m_pDescriptionBuffer, &l_oStartIter, &l_oEndIter);
		const gint l_iOffset = gtk_text_iter_get_offset(&l_oStartIter);

		gtk_text_buffer_insert_at_cursor(m_pDescriptionBuffer, sTagIn, gint(strlen(sTagIn)));
		gtk_text_buffer_insert_at_cursor(m_pDescriptionBuffer, sTagOut, gint(strlen(sTagOut)));


		gtk_text_buffer_get_iter_at_offset(m_pDescriptionBuffer, &l_oStartIter, gint(l_iOffset + strlen(sTagIn)));
		gtk_text_buffer_place_cursor(m_pDescriptionBuffer, &l_oStartIter);
	}

	// set focus on the text, to get back in edition mode directly
	gtk_widget_grab_focus(m_pDescription);
}
