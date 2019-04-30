#ifndef __OpenViBEDesigner_CLogListenerDesigner_H__
#define __OpenViBEDesigner_CLogListenerDesigner_H__

#include "ovd_base.h"

#include <vector>
#include <map>

#define OVK_ClassId_Designer_LogListener OpenViBE::CIdentifier(0x0FE155FA, 0x313C17A7)

namespace OpenViBEDesigner
{
	class CApplication;

	class CLogListenerDesigner : public OpenViBE::Kernel::ILogListener
	{
		public:
			class CLogObject
			{
			public:
				CLogObject(GtkTextBuffer* pBuffer)
				{
					m_pBuffer = gtk_text_buffer_new(gtk_text_buffer_get_tag_table(pBuffer));
					m_bPassedFilter = false;//by default the log does not pass the filter;
				}

				::GtkTextBuffer* getTextBuffer()
				{
					return m_pBuffer;
				}

				//determine if the log contains the sSearchTerm and tag the part with the sSerachTerm in gray
				const bool Filter(OpenViBE::CString sSearchTerm)
				{
					m_bPassedFilter = false;
					GtkTextIter start_find, end_find;
					gtk_text_buffer_get_start_iter(m_pBuffer, &start_find);
					gtk_text_buffer_get_end_iter(m_pBuffer, &end_find);

					//tag for highlighting the search term
					GtkTextTag* tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(m_pBuffer), "gray_bg");
					if(tag==nullptr)
					{
						gtk_text_buffer_create_tag(m_pBuffer, "gray_bg", "background", "gray", nullptr);
					}

					//remove previous tagging
					gtk_text_buffer_remove_tag_by_name(m_pBuffer, "gray_bg", &start_find, &end_find);

					//no term means no research so no filter we let all pass
					if(sSearchTerm==OpenViBE::CString(""))
					{
						m_bPassedFilter = true;
						return m_bPassedFilter;
					}


					GtkTextIter start_match, end_match;
					const gchar *text = sSearchTerm.toASCIIString();
					while ( gtk_text_iter_forward_search(&start_find, text, GTK_TEXT_SEARCH_TEXT_ONLY, &start_match, &end_match, nullptr) )
					{
						gtk_text_buffer_apply_tag_by_name(m_pBuffer, "gray_bg", &start_match, &end_match);
						//offset to end_match
						int offset = gtk_text_iter_get_offset(&end_match);
						//begin next search at end match
						gtk_text_buffer_get_iter_at_offset(m_pBuffer, &start_find, offset);
						m_bPassedFilter = true;
					}
					return m_bPassedFilter;
				}

				void appendToCurrentLog(const char *textColor, const char *logMessage, bool bIsLink /* = false */)
				{

					GtkTextIter l_oEndLogIter;
					gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndLogIter);

					if(bIsLink)
					{
						gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oEndLogIter, logMessage, -1, "f_mono", textColor, "link", nullptr);
					}
					else
					{
						gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oEndLogIter, logMessage, -1, "f_mono", textColor, nullptr);
					}

				}

				public:
					::GtkTextBuffer* m_pBuffer;
					bool m_bPassedFilter;
			};

			CLogListenerDesigner(const OpenViBE::Kernel::IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface);

			virtual bool isActive(OpenViBE::Kernel::ELogLevel eLogLevel);
			virtual bool activate(OpenViBE::Kernel::ELogLevel eLogLevel, bool bActive);
			virtual bool activate(OpenViBE::Kernel::ELogLevel eStartLogLevel, OpenViBE::Kernel::ELogLevel eEndLogLevel, bool bActive);
			virtual bool activate(bool bActive);

			virtual void log(const OpenViBE::time64 time64Value);

			virtual void log(const uint64_t ui64Value);
			virtual void log(const uint32_t ui32Value);
			virtual void log(const uint16_t ui16Value);
			virtual void log(const uint8_t ui8Value);

			virtual void log(const int64_t i64Value);
			virtual void log(const int32_t i32Value);
			virtual void log(const int16_t i16Value);
			virtual void log(const int8_t i8Value);

			virtual void log(const double f64Value);
			virtual void log(const float f32Value);

			virtual void log(const bool bValue);

			virtual void log(const OpenViBE::CIdentifier& rValue);
			virtual void log(const OpenViBE::CString& rValue);
			virtual void log(const char* pValue);

			virtual void log(const OpenViBE::Kernel::ELogLevel eLogLevel);
			virtual void log(const OpenViBE::Kernel::ELogColor eLogColor);

			void clearMessages();
			void focusMessageWindow();

			// TODO
			void searchMessages(OpenViBE::CString l_sSearchTerm);
			void displayLog(CLogObject* oLog);
			void appendLog(CLogObject* oLog);
			void scrollToBottom();

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier);

			OpenViBE::CString m_sSearchTerm;
			::GtkTextTag* m_pCIdentifierTag;
			std::function<void(OpenViBE::CIdentifier&)> m_CenterOnBoxFun;

		protected:

			std::map<OpenViBE::Kernel::ELogLevel, bool> m_vActiveLevel;

			//logs
			std::vector<CLogObject*> m_vStoredLog;

		private:

			::GtkBuilder* m_pBuilderInterface;
			::GtkBuilder* m_pAlertBuilder;
			::GtkTextView* m_pTextView;
			::GtkTextBuffer* m_pBuffer;

			::GtkToggleButton* m_pToggleButtonPopup;

			::GtkToggleToolButton* m_pToggleButtonActive_Debug;
			::GtkToggleToolButton* m_pToggleButtonActive_Benchmark;
			::GtkToggleToolButton* m_pToggleButtonActive_Trace;
			::GtkToggleToolButton* m_pToggleButtonActive_Info;
			::GtkToggleToolButton* m_pToggleButtonActive_Warning;
			::GtkToggleToolButton* m_pToggleButtonActive_ImportantWarning;
			::GtkToggleToolButton* m_pToggleButtonActive_Error;
			::GtkToggleToolButton* m_pToggleButtonActive_Fatal;

			::GtkLabel* m_pLabelCountMessages;
			::GtkLabel* m_pLabelCountWarnings;
			::GtkLabel* m_pLabelCountErrors;
			::GtkLabel* m_pLabelDialogCountWarnings;
			::GtkLabel* m_pLabelDialogCountErrors;

			::GtkWidget* m_pImageWarnings;
			::GtkWidget* m_pImageErrors;

			::GtkWindow* m_pAlertWindow;

			bool m_bIngnoreMessages;

			uint32_t m_ui32CountMessages;
			uint32_t m_ui32CountWarnings;
			uint32_t m_ui32CountErrors;

			void updateMessageCounts();
			void checkAppendFilterCurrentLog(const char *textColor, const char *logMessage, bool bIsLink = false);


			bool m_bConsoleLogWithHexa;
			bool m_bConsoleLogTimeInSecond;
			uint32_t m_ui32ConsoleLogTimePrecision;

			CLogObject* m_pCurrentLog;

	};
};

#endif // __OpenViBEDesigner_CLogListenerDesigner_H__
