#pragma once

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

			GtkTextBuffer* getTextBuffer() { return m_pBuffer; }

			//determine if the log contains the sSearchTerm and tag the part with the sSerachTerm in gray
			const bool Filter(const OpenViBE::CString sSearchTerm)
			{
				m_bPassedFilter = false;
				GtkTextIter start_find, end_find;
				gtk_text_buffer_get_start_iter(m_pBuffer, &start_find);
				gtk_text_buffer_get_end_iter(m_pBuffer, &end_find);

				//tag for highlighting the search term
				GtkTextTag* tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(m_pBuffer), "gray_bg");
				if (tag == nullptr)
				{
					gtk_text_buffer_create_tag(m_pBuffer, "gray_bg", "background", "gray", nullptr);
				}

				//remove previous tagging
				gtk_text_buffer_remove_tag_by_name(m_pBuffer, "gray_bg", &start_find, &end_find);

				//no term means no research so no filter we let all pass
				if (sSearchTerm == OpenViBE::CString(""))
				{
					m_bPassedFilter = true;
					return m_bPassedFilter;
				}


				GtkTextIter start_match, end_match;
				const gchar* text = sSearchTerm.toASCIIString();
				while (gtk_text_iter_forward_search(&start_find, text, GTK_TEXT_SEARCH_TEXT_ONLY, &start_match, &end_match, nullptr))
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

			void appendToCurrentLog(const char* textColor, const char* logMessage, bool bIsLink /* = false */)
			{
				GtkTextIter l_oEndLogIter;
				gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndLogIter);

				if (bIsLink)
				{
					gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oEndLogIter, logMessage, -1, "f_mono", textColor, "link", nullptr);
				}
				else
				{
					gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oEndLogIter, logMessage, -1, "f_mono", textColor, nullptr);
				}
			}

			GtkTextBuffer* m_pBuffer;
			bool m_bPassedFilter;
		};

		CLogListenerDesigner(const OpenViBE::Kernel::IKernelContext& rKernelContext, GtkBuilder* pBuilderInterface);

		bool isActive(OpenViBE::Kernel::ELogLevel eLogLevel) override;
		bool activate(OpenViBE::Kernel::ELogLevel eLogLevel, bool bActive) override;
		bool activate(OpenViBE::Kernel::ELogLevel eStartLogLevel, OpenViBE::Kernel::ELogLevel eEndLogLevel, bool bActive) override;
		bool activate(bool bActive) override;

		void log(OpenViBE::time64 time64Value) override;

		void log(uint64_t ui64Value) override;
		void log(uint32_t ui32Value) override;
		void log(uint16_t ui16Value) override;
		void log(uint8_t ui8Value) override;

		void log(int64_t i64Value) override;
		void log(int32_t i32Value) override;
		void log(int16_t i16Value) override;
		void log(int8_t i8Value) override;

		void log(double f64Value) override;
		void log(float f32Value) override;

		void log(bool bValue) override;

		void log(const OpenViBE::CIdentifier& rValue) override;
		void log(const OpenViBE::CString& rValue) override;
		void log(const char* pValue) override;

		void log(OpenViBE::Kernel::ELogLevel eLogLevel) override;
		void log(OpenViBE::Kernel::ELogColor eLogColor) override;

		void clearMessages();
		void focusMessageWindow();

		// TODO
		void searchMessages(const OpenViBE::CString& l_sSearchTerm);
		void displayLog(CLogObject* oLog);
		void appendLog(CLogObject* oLog);
		void scrollToBottom();

		_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier);

		OpenViBE::CString m_sSearchTerm;
		GtkTextTag* m_pCIdentifierTag;
		std::function<void(OpenViBE::CIdentifier&)> m_CenterOnBoxFun;

	protected:

		std::map<OpenViBE::Kernel::ELogLevel, bool> m_vActiveLevel;

		//logs
		std::vector<CLogObject*> m_vStoredLog;

	private:

		GtkBuilder* m_pBuilderInterface = nullptr;
		GtkBuilder* m_pAlertBuilder = nullptr;
		GtkTextView* m_pTextView = nullptr;
		GtkTextBuffer* m_pBuffer = nullptr;

		GtkToggleButton* m_pToggleButtonPopup = nullptr;

		GtkToggleToolButton* m_pToggleButtonActive_Debug = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Benchmark = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Trace = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Info = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Warning = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_ImportantWarning = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Error = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Fatal = nullptr;

		GtkLabel* m_pLabelCountMessages = nullptr;
		GtkLabel* m_pLabelCountWarnings = nullptr;
		GtkLabel* m_pLabelCountErrors = nullptr;
		GtkLabel* m_pLabelDialogCountWarnings = nullptr;
		GtkLabel* m_pLabelDialogCountErrors = nullptr;

		GtkWidget* m_pImageWarnings = nullptr;
		GtkWidget* m_pImageErrors = nullptr;

		GtkWindow* m_pAlertWindow = nullptr;

		bool m_bIngnoreMessages = false;

		uint32_t m_ui32CountMessages = 0;
		uint32_t m_ui32CountWarnings = 0;
		uint32_t m_ui32CountErrors = 0;

		bool m_bConsoleLogWithHexa = false;
		bool m_bConsoleLogTimeInSecond = false;
		uint32_t m_ui32ConsoleLogTimePrecision = 0;

		CLogObject* m_pCurrentLog = nullptr;

		void updateMessageCounts();
		void checkAppendFilterCurrentLog(const char* textColor, const char* logMessage, bool bIsLink = false);
	};
};
