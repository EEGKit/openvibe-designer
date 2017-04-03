#include "ovdCLogListenerDesigner.h"

#include <iostream>
#include <sstream>

#include <openvibe/ovITimeArithmetics.h>

#define OVD_GUI_File OpenViBE::Directories::getDataDir() + "/applications/designer/interface.ui"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	void close_messages_alert_window_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_widget_hide(GTK_WIDGET(pUserData));
	}

	void focus_message_window_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CLogListenerDesigner*>(pUserData)->focusMessageWindow();
	}

	void refresh_search_log_entry(::GtkEntry* pTextfield, gpointer pUserData)
	{
		CLogListenerDesigner* designerLog_ptr = static_cast<CLogListenerDesigner*>(pUserData);
		designerLog_ptr->m_sSearchTerm = gtk_entry_get_text(pTextfield);
		designerLog_ptr->searchMessages(designerLog_ptr->m_sSearchTerm);
	}
	void focus_on_box_cidentifier_clicked(::GtkWidget* pWidget, GdkEventButton *pEvent, gpointer pData)
	{
		//log text view grab the focus so isLogAreaClicked() return true and CTRL+F will focus on the log searchEntry
		gtk_widget_grab_focus(pWidget);

		CLogListenerDesigner* designerLog_ptr = static_cast<CLogListenerDesigner*>(pData);

		//if left click
		if (pEvent->button == 1)
		{
			GtkTextView* l_pTextView = GTK_TEXT_VIEW(pWidget);
			GtkTextWindowType l_oWindowType = gtk_text_view_get_window_type(l_pTextView, pEvent->window);
			gint l_iBufferX, l_iBufferY;
			//convert event coord (mouse position) in buffer coord (character in buffer)
			gtk_text_view_window_to_buffer_coords(l_pTextView, l_oWindowType, round(pEvent->x), round(pEvent->y), &l_iBufferX, &l_iBufferY);
			//get the text iter corresponding to that position
			GtkTextIter l_oIter;
			gtk_text_view_get_iter_at_location(l_pTextView, &l_oIter, l_iBufferX, l_iBufferY);

			//if this position is not tagged, exit
			if(!gtk_text_iter_has_tag(&l_oIter, designerLog_ptr->m_pCIdentifierTag))
			{
				return;
			}
			//the position is tagged, we are on a CIdentifier
			GtkTextIter l_oStart = l_oIter;
			GtkTextIter l_oEnd = l_oIter;

			while(gtk_text_iter_has_tag(&l_oEnd, designerLog_ptr->m_pCIdentifierTag))
			{
				gtk_text_iter_forward_char(&l_oEnd);
			}
			while(gtk_text_iter_has_tag(&l_oStart, designerLog_ptr->m_pCIdentifierTag))
			{
				gtk_text_iter_backward_char(&l_oStart);
			}
			//we went one char to far for start
			gtk_text_iter_forward_char(&l_oStart);
			//this contains the CIdentifier
			gchar * l_sLink=gtk_text_iter_get_text(&l_oStart, &l_oEnd);
			//cout << "cid is |" << link << "|" << endl;
			CIdentifier l_oId;
			l_oId.fromString(CString(l_sLink));
			designerLog_ptr->m_CenterOnBoxFun(l_oId);
		}
	}
}


void CLogListenerDesigner::searchMessages(CString l_sSearchTerm)
{
	//clear displayed buffer
	gtk_text_buffer_set_text(m_pBuffer, "", -1);
	m_sSearchTerm = l_sSearchTerm;
	for(CLogObject* log : m_vStoredLog)
	{
		if(log->Filter(l_sSearchTerm))
		{
			//display the log
			appendLog(log);
		}
	}
}

void CLogListenerDesigner::appendLog(CLogObject *oLog)
{
	GtkTextIter l_oEndter, l_oLogBegin, l_oLogEnd;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndter);
	//get log buffer bounds
	gtk_text_buffer_get_start_iter(oLog->getTextBuffer(), &l_oLogBegin);
	gtk_text_buffer_get_end_iter(oLog->getTextBuffer(), &l_oLogEnd);
	//copy at the end of the displayed buffer
	gtk_text_buffer_insert_range(m_pBuffer, &l_oEndter, &l_oLogBegin, &l_oLogEnd );
}

CLogListenerDesigner::CLogListenerDesigner(const IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface)
	:m_pBuilderInterface( pBuilderInterface )
	,m_pAlertWindow(nullptr)
	,m_bIngnoreMessages( false )
	,m_ui32CountMessages( 0 )
	,m_ui32CountWarnings( 0 )
	,m_ui32CountErrors( 0 )
	,m_sSearchTerm("")
	,m_CenterOnBoxFun([](CIdentifier& id){})
{
	m_pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-textview_messages"));
	m_pAlertWindow = GTK_WINDOW(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup"));

	m_pToggleButtonPopup = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_alert_on_error"));

	m_pLabelCountMessages = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_message_label"));
	m_pLabelCountWarnings = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_label"));
	m_pLabelCountErrors = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_label"));
	m_pLabelDialogCountWarnings = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-warning_count"));
	m_pLabelDialogCountErrors = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-error_count"));

	m_pImageWarnings = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_image"));
	m_pImageErrors = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_image"));

	m_pToggleButtonActive_Debug = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug"));
	m_pToggleButtonActive_Benchmark = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench"));
	m_pToggleButtonActive_Trace = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace"));
	m_pToggleButtonActive_Info = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info"));
	m_pToggleButtonActive_Warning = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning"));
	m_pToggleButtonActive_ImportantWarning = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning"));
	m_pToggleButtonActive_Error = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error"));
	m_pToggleButtonActive_Fatal = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal"));

	// set the popup-on-error checkbox according to the configuration token
	gtk_toggle_button_set_active(m_pToggleButtonPopup, (OpenViBE::boolean)(rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_PopUpOnError}")));

	g_signal_connect(G_OBJECT(m_pAlertWindow), "delete_event", G_CALLBACK(::gtk_widget_hide), nullptr);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-button_view")), "clicked", G_CALLBACK(::focus_message_window_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-button_ok")), "clicked", G_CALLBACK(::close_messages_alert_window_cb), m_pAlertWindow);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "searchEntry")), "changed", G_CALLBACK(::refresh_search_log_entry), this);
	g_signal_connect(G_OBJECT(m_pTextView), "button_press_event", G_CALLBACK(focus_on_box_cidentifier_clicked), this);
	m_pBuffer = gtk_text_view_get_buffer( m_pTextView );

	gtk_text_buffer_create_tag(m_pBuffer, "f_mono", "family", "monospace", nullptr);
	gtk_text_buffer_create_tag(m_pBuffer, "w_bold", "weight", PANGO_WEIGHT_BOLD, nullptr);
	gtk_text_buffer_create_tag(m_pBuffer, "c_blue", "foreground", "#0000FF", nullptr); // debug
	gtk_text_buffer_create_tag(m_pBuffer, "c_magenta", "foreground", "#FF00FF", nullptr); // benchmark
	gtk_text_buffer_create_tag(m_pBuffer, "c_darkOrange", "foreground", "#FF9000", nullptr); // important warning
	gtk_text_buffer_create_tag(m_pBuffer, "c_red", "foreground", "#FF0000", nullptr); // error, fatal
	gtk_text_buffer_create_tag(m_pBuffer, "c_watercourse", "foreground", "#008238", nullptr); // trace
	gtk_text_buffer_create_tag(m_pBuffer, "c_aqua", "foreground", "#00FFFF", nullptr); // number
	gtk_text_buffer_create_tag(m_pBuffer, "c_darkViolet", "foreground", "#6900D7", nullptr); // warning
	gtk_text_buffer_create_tag(m_pBuffer, "c_blueChill", "foreground", "#3d889b", nullptr); // information
	gtk_text_buffer_create_tag(m_pBuffer, "link", "underline", PANGO_UNDERLINE_SINGLE, NULL); // link for CIdentifier

	GtkTextTagTable* l_pTagtable =  gtk_text_buffer_get_tag_table(m_pBuffer);
	m_pCIdentifierTag = gtk_text_tag_table_lookup(l_pTagtable, "link");

	m_bConsoleLogWithHexa = rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ConsoleLogWithHexa}",false);
	m_bConsoleLogTimeInSecond = rKernelContext.getConfigurationManager().expandAsBoolean("${Kernel_ConsoleLogTimeInSecond}",false);
	m_ui32ConsoleLogTimePrecision = (uint32) rKernelContext.getConfigurationManager().expandAsUInteger("${Designer_ConsoleLogTimePrecision}",3);
}

boolean CLogListenerDesigner::isActive(ELogLevel eLogLevel)
{
	map<ELogLevel, boolean>::iterator itLogLevel=m_vActiveLevel.find(eLogLevel);
	if(itLogLevel==m_vActiveLevel.end())
	{
		return true;
	}
	return itLogLevel->second;
}

boolean CLogListenerDesigner::activate(ELogLevel eLogLevel, boolean bActive)
{
	m_vActiveLevel[eLogLevel]=bActive;
	return true;
}

boolean CLogListenerDesigner::activate(ELogLevel eStartLogLevel, ELogLevel eEndLogLevel, boolean bActive)
{
	for(int i=eStartLogLevel; i<=eEndLogLevel; i++)
	{
		m_vActiveLevel[ELogLevel(i)]=bActive;
	}
	return true;
}

boolean CLogListenerDesigner::activate(boolean bActive)
{
	return activate(LogLevel_First, LogLevel_Last, bActive);
}

void CLogListenerDesigner::log(const time64 time64Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	if(m_bConsoleLogTimeInSecond)
	{
		float64 l_f64Time=ITimeArithmetics::timeToSeconds(time64Value.m_ui64TimeValue);
		std::stringstream ss;
		ss.precision(m_ui32ConsoleLogTimePrecision);
		ss.setf(std::ios::fixed,std::ios::floatfield);
		ss << l_f64Time;
		ss << " sec";
		if(m_bConsoleLogWithHexa)
		{
			ss << " (0x" << hex << time64Value.m_ui64TimeValue << ")";
		}

		l_sText << ss.str().c_str();
	}
	else
	{
		l_sText << dec << time64Value.m_ui64TimeValue;
		if(m_bConsoleLogWithHexa)
		{
			l_sText << " (0x" << hex << time64Value.m_ui64TimeValue << ")";
		}
	}

	checkAppendFilterCurrentLog("c_watercourse", l_sText.str().c_str());
}

void CLogListenerDesigner::log(const uint64 ui64Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui64Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui64Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse", l_sText.str().c_str());
}

void CLogListenerDesigner::log(const uint32 ui32Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui32Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui32Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse", l_sText.str().c_str());
}

void CLogListenerDesigner::log(const uint16 ui16Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui16Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui16Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse", l_sText.str().c_str());
}

void CLogListenerDesigner::log(const uint8 ui8Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui8Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui8Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse", l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int64 i64Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i64Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i64Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int32 i32Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i32Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i32Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int16 i16Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i16Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i16Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int8 i8Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i8Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i8Value << ")";
	}

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const float32 f32Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << f32Value;

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const float64 f64Value)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << f64Value;

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const boolean bValue)
{
	if(m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << (bValue ? "true" : "false");

	checkAppendFilterCurrentLog("c_watercourse",l_sText.str().c_str());
}

void CLogListenerDesigner::log(const CIdentifier& rValue)
{
	if(m_bIngnoreMessages) return;

	checkAppendFilterCurrentLog("c_blueChill", rValue.toString(), true);
}

void CLogListenerDesigner::log(const CString& rValue)
{
	if(m_bIngnoreMessages) return;

	checkAppendFilterCurrentLog("c_blueChill", rValue);
}

void CLogListenerDesigner::log(const char* pValue)
{
	if(m_bIngnoreMessages) return;

	checkAppendFilterCurrentLog(nullptr, pValue);
}

void CLogListenerDesigner::log(const ELogLevel eLogLevel)
{
//	GtkTextIter l_oTextIter;
//	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);

	//new log, will be deleted when m_vStoredLog is cleared
	m_pCurrentLog = new CLogObject(m_pBuffer);//m_pNonFilteredBuffer);

	//copy this newly added content in the current log
	GtkTextIter l_oEndLogIter;
	gtk_text_buffer_get_end_iter(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter);

	auto addTagName = [this, &l_oEndLogIter](GtkToggleToolButton* activeButton, OpenViBE::uint32& countVariable, const char* state, const char* color)
	{
		m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(activeButton);
		if(m_bIngnoreMessages) return;

		countVariable++;
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[ ", -1, "w_bold", "f_mono", nullptr);
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, state, -1, "w_bold", "f_mono", color, nullptr);
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, " ] ", -1, "w_bold", "f_mono", nullptr);
	};

	switch(eLogLevel)
	{
		case LogLevel_Debug:
			addTagName(m_pToggleButtonActive_Debug, m_ui32CountMessages, "DEBUG", "c_blue");
			break;

		case LogLevel_Benchmark:
			addTagName(m_pToggleButtonActive_Benchmark, m_ui32CountMessages, "BENCH", "c_magenta");
			break;

		case LogLevel_Trace:
			addTagName(m_pToggleButtonActive_Trace, m_ui32CountMessages, "TRACE", "c_watercourse");
			break;

		case LogLevel_Info:
			addTagName(m_pToggleButtonActive_Info, m_ui32CountMessages, "INF", "c_blueChill");
			break;

		case LogLevel_Warning:
			addTagName(m_pToggleButtonActive_Warning, m_ui32CountWarnings, "WARNING", "c_darkViolet");
			break;

		case LogLevel_ImportantWarning:
			addTagName(m_pToggleButtonActive_ImportantWarning, m_ui32CountWarnings, "WARNING", "c_darkOrange");
			break;

		case LogLevel_Error:
			addTagName(m_pToggleButtonActive_Error, m_ui32CountErrors, "ERROR", "c_red");
			break;

		case LogLevel_Fatal:
			addTagName(m_pToggleButtonActive_Fatal, m_ui32CountErrors, "FATAL", "c_red");
			break;

		default:
			addTagName(nullptr, m_ui32CountMessages, "UNKNOWN", nullptr);
			break;
	}

	if(gtk_toggle_button_get_active(m_pToggleButtonPopup) && (eLogLevel == LogLevel_Warning || eLogLevel == LogLevel_ImportantWarning || eLogLevel == LogLevel_Error || eLogLevel == LogLevel_Fatal))
	{
		if(!gtk_widget_get_visible(GTK_WIDGET(m_pAlertWindow)))
		{
			gtk_window_set_position(GTK_WINDOW(m_pAlertWindow), GTK_WIN_POS_CENTER);
			gtk_window_present(GTK_WINDOW(m_pAlertWindow));
			gtk_window_set_keep_above(GTK_WINDOW(m_pAlertWindow), true);
		}
	}
	m_vStoredLog.push_back(m_pCurrentLog);

	//see if the log passes the filter
	boolean l_bPassFilter = m_pCurrentLog->Filter(m_sSearchTerm);
	//if it does mark this position and insert the log in the text buffer displayed
	GtkTextIter l_oEndDisplayedTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndDisplayedTextIter);
	gtk_text_buffer_create_mark(m_pBuffer, "current_log", &l_oEndDisplayedTextIter, true );//creating a mark will erase the previous one with the same name so no worry here
	if(l_bPassFilter)
	{
		displayLog(m_pCurrentLog);
	}

	this->updateMessageCounts();

	::GtkTextMark l_oMark;
	l_oMark = *(gtk_text_buffer_get_mark (gtk_text_view_get_buffer( m_pTextView ), "insert"));
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (m_pTextView), &l_oMark, 0.0, FALSE, 0.0, 0.0);
}

void CLogListenerDesigner::log(const ELogColor eLogColor)
{
	if(m_bIngnoreMessages) return;
}

void CLogListenerDesigner::updateMessageCounts()
{
	stringstream l_sCountMessages;
	l_sCountMessages << "<b>" << m_ui32CountMessages << "</b> Message";

	if(m_ui32CountMessages > 1)
	{
		l_sCountMessages << "s";
	}

	gtk_label_set_markup(m_pLabelCountMessages, l_sCountMessages.str().data());

	if(m_ui32CountWarnings > 0)
	{
		stringstream l_sCountWarnings;
		l_sCountWarnings << "<b>" << m_ui32CountWarnings << "</b> Warning";

		if(m_ui32CountWarnings > 1)
		{
			l_sCountWarnings << "s";
		}

		gtk_label_set_markup(m_pLabelCountWarnings, l_sCountWarnings.str().data());
		gtk_label_set_markup(m_pLabelDialogCountWarnings, l_sCountWarnings.str().data());
		gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountWarnings), true);
		gtk_widget_set_visible(GTK_WIDGET(m_pImageWarnings), true);
	}

	if(m_ui32CountErrors > 0)
	{
		stringstream l_sCountErrors;
		l_sCountErrors << "<b>" << m_ui32CountErrors << "</b> Error";

		if(m_ui32CountErrors > 1)
		{
			l_sCountErrors << "s";
		}

		gtk_label_set_markup(m_pLabelCountErrors, l_sCountErrors.str().data());
		gtk_label_set_markup(m_pLabelDialogCountErrors, l_sCountErrors.str().data());

		gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountErrors), true);
		gtk_widget_set_visible(GTK_WIDGET(m_pImageErrors), true);
	}
}

void CLogListenerDesigner::clearMessages()
{
	m_ui32CountMessages = 0;
	m_ui32CountWarnings = 0;
	m_ui32CountErrors = 0;

	gtk_label_set_markup(m_pLabelCountMessages, "<b>0</b> Messages");
	gtk_label_set_markup(m_pLabelCountWarnings, "<b>0</b> Warnings");
	gtk_label_set_markup(m_pLabelCountErrors, "<b>0</b> Errors");
	gtk_label_set_markup(m_pLabelDialogCountWarnings, "<b>0</b> Warnings");
	gtk_label_set_markup(m_pLabelDialogCountErrors, "<b>0</b> Errors");

	gtk_widget_set_visible(m_pImageWarnings, false);
	gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountWarnings), false);
	gtk_widget_set_visible(m_pImageErrors, false);
	gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountErrors), false);

	gtk_text_buffer_set_text(m_pBuffer, "", -1);

	std::for_each(m_vStoredLog.begin(), m_vStoredLog.end(), [](CLogObject* elem){ delete elem;});
	m_vStoredLog.clear();

	m_pCurrentLog = nullptr;
}

void CLogListenerDesigner::focusMessageWindow()
{
	gtk_widget_hide(GTK_WIDGET(m_pAlertWindow));
	gtk_expander_set_expanded(GTK_EXPANDER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-expander_messages")), true);
}


void CLogListenerDesigner::checkAppendFilterCurrentLog(const char *textColor, const char *logMessage, bool bIsLink)
{
	if(!m_pCurrentLog) {
		std::cout << "Ouch, current log had been deleted before creating new, this shouldn't happen...\n";
		return;
	}
	m_pCurrentLog->appendToCurrentLog(textColor, logMessage, bIsLink);

	if(m_pCurrentLog->Filter(m_sSearchTerm))
	{
		displayLog(m_pCurrentLog);
	}
}

void CLogListenerDesigner::displayLog(CLogObject *oLog)
{
	GtkTextMark* l_oMark = gtk_text_buffer_get_mark(m_pBuffer, "current_log");
	GtkTextIter l_oIter, l_oEndter, l_oLogBegin, l_oLogEnd;
	gtk_text_buffer_get_iter_at_mark(m_pBuffer, &l_oIter, l_oMark);
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndter);

	//delete what after the mark
	gtk_text_buffer_delete(m_pBuffer, &l_oIter, &l_oEndter);
	//get iter
	gtk_text_buffer_get_iter_at_mark(m_pBuffer, &l_oIter, l_oMark);
	//rewrite the log
	gtk_text_buffer_get_start_iter(oLog->getTextBuffer(), &l_oLogBegin);
	gtk_text_buffer_get_end_iter(oLog->getTextBuffer(), &l_oLogEnd);
	gtk_text_buffer_insert_range(m_pBuffer, &l_oIter, &l_oLogBegin, &l_oLogEnd );
}
