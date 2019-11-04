#include "ovdCLogListenerDesigner.h"

#include <iostream>
#include <sstream>

#include <openvibe/ovTimeArithmetics.h>

#define OVD_GUI_File		OpenViBE::Directories::getDataDir() + "/applications/designer/interface.ui"

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	void close_messages_alert_window_cb(GtkButton* /*button*/, gpointer data) { gtk_widget_hide(GTK_WIDGET(data)); }

	void focus_message_window_cb(GtkButton* /*button*/, gpointer data) { static_cast<CLogListenerDesigner*>(data)->focusMessageWindow(); }

	void refresh_search_log_entry(GtkEntry* pTextfield, gpointer data)
	{
		auto* designerLog_ptr          = static_cast<CLogListenerDesigner*>(data);
		designerLog_ptr->m_sSearchTerm = gtk_entry_get_text(pTextfield);
		designerLog_ptr->searchMessages(designerLog_ptr->m_sSearchTerm);
	}

	void focus_on_box_cidentifier_clicked(GtkWidget* widget, GdkEventButton* pEvent, gpointer data)
	{
		//log text view grab the focus so isLogAreaClicked() return true and CTRL+F will focus on the log searchEntry
		gtk_widget_grab_focus(widget);

		auto* designerLog_ptr = static_cast<CLogListenerDesigner*>(data);

		//if left click
		if (pEvent->button == 1)
		{
			GtkTextView* textView              = GTK_TEXT_VIEW(widget);
			const GtkTextWindowType windowType = gtk_text_view_get_window_type(textView, pEvent->window);
			gint bufferX, bufferY;
			//convert event coord (mouse position) in buffer coord (character in buffer)
			gtk_text_view_window_to_buffer_coords(textView, windowType, gint(round(pEvent->x)), gint(round(pEvent->y)), &bufferX, &bufferY);
			//get the text iter corresponding to that position
			GtkTextIter iter;
			gtk_text_view_get_iter_at_location(textView, &iter, bufferX, bufferY);

			//if this position is not tagged, exit
			if (!gtk_text_iter_has_tag(&iter, designerLog_ptr->m_pCIdentifierTag)) { return; }
			//the position is tagged, we are on a CIdentifier
			GtkTextIter start = iter;
			GtkTextIter end   = iter;

			while (gtk_text_iter_has_tag(&end, designerLog_ptr->m_pCIdentifierTag)) { gtk_text_iter_forward_char(&end); }
			while (gtk_text_iter_has_tag(&start, designerLog_ptr->m_pCIdentifierTag)) { gtk_text_iter_backward_char(&start); }
			//we went one char to far for start
			gtk_text_iter_forward_char(&start);
			//this contains the CIdentifier
			gchar* link = gtk_text_iter_get_text(&start, &end);
			//cout << "cid is |" << link << "|" << endl;
			CIdentifier id;
			id.fromString(CString(link));
			designerLog_ptr->m_CenterOnBoxFun(id);
		}
	}
} // namespace


void CLogListenerDesigner::searchMessages(const CString& searchTerm)
{
	//clear displayed buffer
	gtk_text_buffer_set_text(m_buffer, "", -1);
	m_sSearchTerm = searchTerm;
	for (CLogObject* log : m_vStoredLog)
	{
		if (log->Filter(searchTerm))
		{
			//display the log
			appendLog(log);
		}
	}
}

void CLogListenerDesigner::appendLog(CLogObject* oLog) const
{
	GtkTextIter endIter, begin, end;
	gtk_text_buffer_get_end_iter(m_buffer, &endIter);
	//get log buffer bounds
	gtk_text_buffer_get_start_iter(oLog->getTextBuffer(), &begin);
	gtk_text_buffer_get_end_iter(oLog->getTextBuffer(), &end);
	//copy at the end of the displayed buffer
	gtk_text_buffer_insert_range(m_buffer, &endIter, &begin, &end);
}

CLogListenerDesigner::CLogListenerDesigner(const IKernelContext& ctx, GtkBuilder* pBuilderInterface)
	: m_sSearchTerm(""), m_CenterOnBoxFun([](CIdentifier& /*id*/) {}), m_builderInterface(pBuilderInterface)
{
	m_pTextView    = GTK_TEXT_VIEW(gtk_builder_get_object(m_builderInterface, "openvibe-textview_messages"));
	m_pAlertWindow = GTK_WINDOW(gtk_builder_get_object(m_builderInterface, "dialog_error_popup"));

	m_pToggleButtonPopup = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_alert_on_error"));

	m_labelnMessages       = GTK_LABEL(gtk_builder_get_object(m_builderInterface, "openvibe-messages_count_message_label"));
	m_labelnWarnings       = GTK_LABEL(gtk_builder_get_object(m_builderInterface, "openvibe-messages_count_warning_label"));
	m_labelnErrors         = GTK_LABEL(gtk_builder_get_object(m_builderInterface, "openvibe-messages_count_error_label"));
	m_labelDialognWarnings = GTK_LABEL(gtk_builder_get_object(m_builderInterface, "dialog_error_popup-warning_count"));
	m_labelDialognErrors   = GTK_LABEL(gtk_builder_get_object(m_builderInterface, "dialog_error_popup-error_count"));

	m_pImageWarnings = GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "openvibe-messages_count_warning_image"));
	m_pImageErrors   = GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "openvibe-messages_count_error_image"));

	m_pToggleButtonActive_Debug            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_debug"));
	m_pToggleButtonActive_Benchmark        = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_bench"));
	m_pToggleButtonActive_Trace            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_trace"));
	m_pToggleButtonActive_Info             = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_info"));
	m_pToggleButtonActive_Warning          = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_warning"));
	m_pToggleButtonActive_ImportantWarning = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_impwarning"));
	m_pToggleButtonActive_Error            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_error"));
	m_pToggleButtonActive_Fatal            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "openvibe-messages_tb_fatal"));

	// set the popup-on-error checkbox according to the configuration token
	gtk_toggle_button_set_active(m_pToggleButtonPopup, bool(ctx.getConfigurationManager().expandAsBoolean("${Designer_PopUpOnError}")));

	g_signal_connect(G_OBJECT(m_pAlertWindow), "delete_event", G_CALLBACK(::gtk_widget_hide), nullptr);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_builderInterface, "dialog_error_popup-button_view")), "clicked", G_CALLBACK(::focus_message_window_cb),
					 this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_builderInterface, "dialog_error_popup-button_ok")), "clicked",
					 G_CALLBACK(::close_messages_alert_window_cb), m_pAlertWindow);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_builderInterface, "searchEntry")), "changed", G_CALLBACK(::refresh_search_log_entry), this);
	g_signal_connect(G_OBJECT(m_pTextView), "button_press_event", G_CALLBACK(focus_on_box_cidentifier_clicked), this);
	m_buffer = gtk_text_view_get_buffer(m_pTextView);

	gtk_text_buffer_create_tag(m_buffer, "f_mono", "family", "monospace", nullptr);
	gtk_text_buffer_create_tag(m_buffer, "w_bold", "weight", PANGO_WEIGHT_BOLD, nullptr);
	gtk_text_buffer_create_tag(m_buffer, "c_blue", "foreground", "#0000FF", nullptr);				// debug
	gtk_text_buffer_create_tag(m_buffer, "c_magenta", "foreground", "#FF00FF", nullptr);			// benchmark
	gtk_text_buffer_create_tag(m_buffer, "c_darkOrange", "foreground", "#FF9000", nullptr);		// important warning
	gtk_text_buffer_create_tag(m_buffer, "c_red", "foreground", "#FF0000", nullptr);				// error, fatal
	gtk_text_buffer_create_tag(m_buffer, "c_watercourse", "foreground", "#008238", nullptr);		// trace
	gtk_text_buffer_create_tag(m_buffer, "c_aqua", "foreground", "#00FFFF", nullptr);				// number
	gtk_text_buffer_create_tag(m_buffer, "c_darkViolet", "foreground", "#6900D7", nullptr);		// warning
	gtk_text_buffer_create_tag(m_buffer, "c_blueChill", "foreground", "#3d889b", nullptr);			// information
	gtk_text_buffer_create_tag(m_buffer, "link", "underline", PANGO_UNDERLINE_SINGLE, nullptr);	// link for CIdentifier

	GtkTextTagTable* tagTable = gtk_text_buffer_get_tag_table(m_buffer);
	m_pCIdentifierTag         = gtk_text_tag_table_lookup(tagTable, "link");

	m_logWithHexa      = ctx.getConfigurationManager().expandAsBoolean("${Designer_ConsoleLogWithHexa}", false);
	m_logTimeInSecond  = ctx.getConfigurationManager().expandAsBoolean("${Kernel_ConsoleLogTimeInSecond}", false);
	m_logTimePrecision = size_t(ctx.getConfigurationManager().expandAsUInteger("${Designer_ConsoleLogTimePrecision}", 3));
}

bool CLogListenerDesigner::isActive(const ELogLevel level)
{
	const auto it = m_vActiveLevel.find(level);
	if (it == m_vActiveLevel.end()) { return true; }
	return it->second;
}

bool CLogListenerDesigner::activate(const ELogLevel level, const bool active)
{
	m_vActiveLevel[level] = active;
	return true;
}

bool CLogListenerDesigner::activate(const ELogLevel eStartLogLevel, const ELogLevel eEndLogLevel, const bool active)
{
	for (int i = eStartLogLevel; i <= eEndLogLevel; ++i) { m_vActiveLevel[ELogLevel(i)] = active; }
	return true;
}

bool CLogListenerDesigner::activate(const bool active) { return activate(LogLevel_First, LogLevel_Last, active); }

void CLogListenerDesigner::log(const time64 value)
{
	if (m_bIngnoreMessages) { return; }

	stringstream txt;
	if (m_logTimeInSecond)
	{
		const double time = TimeArithmetics::timeToSeconds(value.timeValue);
		std::stringstream ss;
		ss.precision(m_logTimePrecision);
		ss.setf(std::ios::fixed, std::ios::floatfield);
		ss << time;
		ss << " sec";
		if (m_logWithHexa) { ss << " (0x" << hex << value.timeValue << ")"; }

		txt << ss.str();
	}
	else
	{
		txt << dec << value.timeValue;
		if (m_logWithHexa) { txt << " (0x" << hex << value.timeValue << ")"; }
	}

	checkAppendFilterCurrentLog("c_watercourse", txt.str().c_str());
}

void CLogListenerDesigner::log(const uint64_t value)
{
	if (m_bIngnoreMessages) { return; }

	stringstream txt;
	txt << dec << value;
	if (m_logWithHexa) { txt << " (0x" << hex << value << ")"; }

	checkAppendFilterCurrentLog("c_watercourse", txt.str().c_str());
}

void CLogListenerDesigner::log(const uint32_t value)
{
	if (m_bIngnoreMessages) { return; }

	stringstream txt;
	txt << dec << value;
	if (m_logWithHexa) { txt << " (0x" << hex << value << ")"; }

	checkAppendFilterCurrentLog("c_watercourse", txt.str().c_str());
}

void CLogListenerDesigner::log(const int64_t value)
{
	if (m_bIngnoreMessages) { return; }

	stringstream txt;
	txt << dec << value;
	if (m_logWithHexa) { txt << " (0x" << hex << value << ")"; }

	checkAppendFilterCurrentLog("c_watercourse", txt.str().c_str());
}

void CLogListenerDesigner::log(const int value)
{
	if (m_bIngnoreMessages) { return; }

	stringstream txt;
	txt << dec << value;
	if (m_logWithHexa) { txt << " (0x" << hex << value << ")"; }

	checkAppendFilterCurrentLog("c_watercourse", txt.str().c_str());
}

void CLogListenerDesigner::log(const double value)
{
	if (m_bIngnoreMessages) { return; }
	checkAppendFilterCurrentLog("c_watercourse", std::to_string(value).c_str());
}

void CLogListenerDesigner::log(const bool value)
{
	if (m_bIngnoreMessages) { return; }
	checkAppendFilterCurrentLog("c_watercourse", (value ? "true" : "false"));
}

void CLogListenerDesigner::log(const CIdentifier& value)
{
	if (m_bIngnoreMessages) { return; }
	checkAppendFilterCurrentLog("c_blueChill", value.str().c_str(), true);
}

void CLogListenerDesigner::log(const CString& value)
{
	if (m_bIngnoreMessages) { return; }
	checkAppendFilterCurrentLog("c_blueChill", value.toASCIIString());
}

void CLogListenerDesigner::log(const std::string& value)
{
	if (m_bIngnoreMessages) { return; }
	checkAppendFilterCurrentLog(nullptr, value.c_str());
}

void CLogListenerDesigner::log(const char* value)
{
	if (m_bIngnoreMessages) { return; }
	checkAppendFilterCurrentLog(nullptr, value);
}

void CLogListenerDesigner::log(const ELogLevel level)
{
	// GtkTextIter textIter;
	// gtk_text_buffer_get_end_iter(m_Buffer, &l_oTextIter);

	//new log, will be deleted when m_vStoredLog is cleared
	m_pCurrentLog = new CLogObject(m_buffer);//m_pNonFilteredBuffer);

	//copy this newly added content in the current log
	GtkTextIter endIter;
	gtk_text_buffer_get_end_iter(m_pCurrentLog->getTextBuffer(), &endIter);

	const auto addTagName = [this, &endIter](GtkToggleToolButton* activeButton, size_t& countVariable, const char* state, const char* color)
	{
		m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(activeButton);
		if (m_bIngnoreMessages) { return; }

		countVariable++;
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &endIter, "[ ", -1, "w_bold", "f_mono", nullptr);
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &endIter, state, -1, "w_bold", "f_mono", color, nullptr);
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &endIter, " ] ", -1, "w_bold", "f_mono", nullptr);
	};

	switch (level)
	{
		case LogLevel_Debug:
			addTagName(m_pToggleButtonActive_Debug, m_nMessages, "DEBUG", "c_blue");
			break;

		case LogLevel_Benchmark:
			addTagName(m_pToggleButtonActive_Benchmark, m_nMessages, "BENCH", "c_magenta");
			break;

		case LogLevel_Trace:
			addTagName(m_pToggleButtonActive_Trace, m_nMessages, "TRACE", "c_watercourse");
			break;

		case LogLevel_Info:
			addTagName(m_pToggleButtonActive_Info, m_nMessages, "INF", "c_blueChill");
			break;

		case LogLevel_Warning:
			addTagName(m_pToggleButtonActive_Warning, m_nWarnings, "WARNING", "c_darkViolet");
			break;

		case LogLevel_ImportantWarning:
			addTagName(m_pToggleButtonActive_ImportantWarning, m_nWarnings, "WARNING", "c_darkOrange");
			break;

		case LogLevel_Error:
			addTagName(m_pToggleButtonActive_Error, m_nErrors, "ERROR", "c_red");
			break;

		case LogLevel_Fatal:
			addTagName(m_pToggleButtonActive_Fatal, m_nErrors, "FATAL", "c_red");
			break;

		default:
			addTagName(nullptr, m_nMessages, "UNKNOWN", nullptr);
			break;
	}

	if (gtk_toggle_button_get_active(m_pToggleButtonPopup) && (level == LogLevel_Warning || level == LogLevel_ImportantWarning || level ==
															   LogLevel_Error || level == LogLevel_Fatal))
	{
		if (!gtk_widget_get_visible(GTK_WIDGET(m_pAlertWindow)))
		{
			gtk_window_set_position(GTK_WINDOW(m_pAlertWindow), GTK_WIN_POS_CENTER);
			gtk_window_present(GTK_WINDOW(m_pAlertWindow));
			gtk_window_set_keep_above(GTK_WINDOW(m_pAlertWindow), true);
		}
	}
	m_vStoredLog.push_back(m_pCurrentLog);

	//see if the log passes the filter
	const bool passFilter = m_pCurrentLog->Filter(m_sSearchTerm);
	//if it does mark this position and insert the log in the text buffer displayed
	GtkTextIter endDisplayedTextIter;
	gtk_text_buffer_get_end_iter(m_buffer, &endDisplayedTextIter);
	gtk_text_buffer_create_mark(m_buffer, "current_log", &endDisplayedTextIter,
								true);//creating a mark will erase the previous one with the same name so no worry here
	if (passFilter) { displayLog(m_pCurrentLog); }

	this->updateMessageCounts();

	GtkTextMark mark;
	mark = *(gtk_text_buffer_get_mark(gtk_text_view_get_buffer(m_pTextView), "insert"));
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(m_pTextView), &mark, 0.0, FALSE, 0.0, 0.0);
}

void CLogListenerDesigner::log(const ELogColor /*color*/) { }

void CLogListenerDesigner::updateMessageCounts() const
{
	stringstream nMessages;
	nMessages << "<b>" << m_nMessages << "</b> Message";

	if (m_nMessages > 1) { nMessages << "s"; }

	gtk_label_set_markup(m_labelnMessages, nMessages.str().data());

	if (m_nWarnings > 0)
	{
		stringstream countWarnings;
		countWarnings << "<b>" << m_nWarnings << "</b> Warning";

		if (m_nWarnings > 1) { countWarnings << "s"; }

		gtk_label_set_markup(m_labelnWarnings, countWarnings.str().data());
		gtk_label_set_markup(m_labelDialognWarnings, countWarnings.str().data());
		gtk_widget_set_visible(GTK_WIDGET(m_labelnWarnings), true);
		gtk_widget_set_visible(GTK_WIDGET(m_pImageWarnings), true);
	}

	if (m_nErrors > 0)
	{
		stringstream nErrors;
		nErrors << "<b>" << m_nErrors << "</b> Error";

		if (m_nErrors > 1) { nErrors << "s"; }

		gtk_label_set_markup(m_labelnErrors, nErrors.str().data());
		gtk_label_set_markup(m_labelDialognErrors, nErrors.str().data());

		gtk_widget_set_visible(GTK_WIDGET(m_labelnErrors), true);
		gtk_widget_set_visible(GTK_WIDGET(m_pImageErrors), true);
	}
}

void CLogListenerDesigner::clearMessages()
{
	m_nMessages = 0;
	m_nWarnings = 0;
	m_nErrors   = 0;

	gtk_label_set_markup(m_labelnMessages, "<b>0</b> Messages");
	gtk_label_set_markup(m_labelnWarnings, "<b>0</b> Warnings");
	gtk_label_set_markup(m_labelnErrors, "<b>0</b> Errors");
	gtk_label_set_markup(m_labelDialognWarnings, "<b>0</b> Warnings");
	gtk_label_set_markup(m_labelDialognErrors, "<b>0</b> Errors");

	gtk_widget_set_visible(m_pImageWarnings, false);
	gtk_widget_set_visible(GTK_WIDGET(m_labelnWarnings), false);
	gtk_widget_set_visible(m_pImageErrors, false);
	gtk_widget_set_visible(GTK_WIDGET(m_labelnErrors), false);

	gtk_text_buffer_set_text(m_buffer, "", -1);

	std::for_each(m_vStoredLog.begin(), m_vStoredLog.end(), [](CLogObject* elem) { delete elem; });
	m_vStoredLog.clear();

	m_pCurrentLog = nullptr;
}

void CLogListenerDesigner::focusMessageWindow() const
{
	gtk_widget_hide(GTK_WIDGET(m_pAlertWindow));
	gtk_expander_set_expanded(GTK_EXPANDER(gtk_builder_get_object(m_builderInterface, "openvibe-expander_messages")), true);
}


void CLogListenerDesigner::checkAppendFilterCurrentLog(const char* textColor, const char* logMessage, const bool bIsLink) const
{
	if (!m_pCurrentLog)
	{
		std::cout << "Ouch, current log had been deleted before creating new, this shouldn't happen...\n";
		return;
	}
	m_pCurrentLog->appendToCurrentLog(textColor, logMessage, bIsLink);

	if (m_pCurrentLog->Filter(m_sSearchTerm)) { displayLog(m_pCurrentLog); }
}

void CLogListenerDesigner::displayLog(CLogObject* oLog) const
{
	GtkTextMark* mark = gtk_text_buffer_get_mark(m_buffer, "current_log");
	GtkTextIter iter, endIter, begin, end;
	gtk_text_buffer_get_iter_at_mark(m_buffer, &iter, mark);
	gtk_text_buffer_get_end_iter(m_buffer, &endIter);

	//delete what after the mark
	gtk_text_buffer_delete(m_buffer, &iter, &endIter);
	//get iter
	gtk_text_buffer_get_iter_at_mark(m_buffer, &iter, mark);
	//rewrite the log
	gtk_text_buffer_get_start_iter(oLog->getTextBuffer(), &begin);
	gtk_text_buffer_get_end_iter(oLog->getTextBuffer(), &end);
	gtk_text_buffer_insert_range(m_buffer, &iter, &begin, &end);
}
