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
			CLogObject(GtkTextBuffer* buffer)
			{
				m_buffer       = gtk_text_buffer_new(gtk_text_buffer_get_tag_table(buffer));
				m_bPassedFilter = false;//by default the log does not pass the filter;
			}

			GtkTextBuffer* getTextBuffer() const { return m_buffer; }

			//determine if the log contains the sSearchTerm and tag the part with the sSerachTerm in gray
			bool Filter(const OpenViBE::CString& sSearchTerm)
			{
				m_bPassedFilter = false;
				GtkTextIter start_find, end_find;
				gtk_text_buffer_get_start_iter(m_buffer, &start_find);
				gtk_text_buffer_get_end_iter(m_buffer, &end_find);

				//tag for highlighting the search term
				GtkTextTag* tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(m_buffer), "gray_bg");
				if (tag == nullptr) { gtk_text_buffer_create_tag(m_buffer, "gray_bg", "background", "gray", nullptr); }

				//remove previous tagging
				gtk_text_buffer_remove_tag_by_name(m_buffer, "gray_bg", &start_find, &end_find);

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
					gtk_text_buffer_apply_tag_by_name(m_buffer, "gray_bg", &start_match, &end_match);
					//offset to end_match
					const int offset = gtk_text_iter_get_offset(&end_match);
					//begin next search at end match
					gtk_text_buffer_get_iter_at_offset(m_buffer, &start_find, offset);
					m_bPassedFilter = true;
				}
				return m_bPassedFilter;
			}

			void appendToCurrentLog(const char* textColor, const char* logMessage, bool bIsLink /* = false */) const
			{
				GtkTextIter l_oEndLogIter;
				gtk_text_buffer_get_end_iter(m_buffer, &l_oEndLogIter);

				if (bIsLink) { gtk_text_buffer_insert_with_tags_by_name(m_buffer, &l_oEndLogIter, logMessage, -1, "f_mono", textColor, "link", nullptr); }
				else { gtk_text_buffer_insert_with_tags_by_name(m_buffer, &l_oEndLogIter, logMessage, -1, "f_mono", textColor, nullptr); }
			}

			GtkTextBuffer* m_buffer = nullptr;
			bool m_bPassedFilter     = false;
		};

		CLogListenerDesigner(const OpenViBE::Kernel::IKernelContext& ctx, GtkBuilder* pBuilderInterface);

		bool isActive(OpenViBE::Kernel::ELogLevel level) override;
		bool activate(OpenViBE::Kernel::ELogLevel level, bool active) override;
		bool activate(OpenViBE::Kernel::ELogLevel eStartLogLevel, OpenViBE::Kernel::ELogLevel eEndLogLevel, bool active) override;
		bool activate(bool active) override;

		void log(const OpenViBE::time64 value) override;

		void log(const uint64_t value) override;
		void log(const uint32_t value) override;

		void log(const int64_t value) override;
		void log(const int value) override;

		void log(const double value) override;
		void log(const float value) override;

		void log(const bool value) override;

		void log(const OpenViBE::CIdentifier& value) override;
		void log(const OpenViBE::CString& value) override;
		void log(const std::string& value) override;
		void log(const char* value) override;

		void log(const OpenViBE::Kernel::ELogLevel level) override;
		void log(const OpenViBE::Kernel::ELogColor color) override;

		void clearMessages();
		void focusMessageWindow() const;

		// TODO
		void searchMessages(const OpenViBE::CString& l_sSearchTerm);
		void displayLog(CLogObject* oLog) const;
		void appendLog(CLogObject* oLog) const;
		void scrollToBottom();

		_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier)

		OpenViBE::CString m_sSearchTerm;
		GtkTextTag* m_pCIdentifierTag = nullptr;
		std::function<void(OpenViBE::CIdentifier&)> m_CenterOnBoxFun;

	protected:

		std::map<OpenViBE::Kernel::ELogLevel, bool> m_vActiveLevel;

		//logs
		std::vector<CLogObject*> m_vStoredLog;

	private:

		GtkBuilder* m_pBuilderInterface = nullptr;
		GtkBuilder* m_pAlertBuilder     = nullptr;
		GtkTextView* m_pTextView        = nullptr;
		GtkTextBuffer* m_buffer        = nullptr;

		GtkToggleButton* m_pToggleButtonPopup = nullptr;

		GtkToggleToolButton* m_pToggleButtonActive_Debug            = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Benchmark        = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Trace            = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Info             = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Warning          = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_ImportantWarning = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Error            = nullptr;
		GtkToggleToolButton* m_pToggleButtonActive_Fatal            = nullptr;

		GtkLabel* m_labelnMessages       = nullptr;
		GtkLabel* m_labelnWarnings       = nullptr;
		GtkLabel* m_labelnErrors         = nullptr;
		GtkLabel* m_labelDialognWarnings = nullptr;
		GtkLabel* m_labelDialognErrors   = nullptr;

		GtkWidget* m_pImageWarnings = nullptr;
		GtkWidget* m_pImageErrors   = nullptr;

		GtkWindow* m_pAlertWindow = nullptr;

		bool m_bIngnoreMessages = false;

		size_t m_nMessages = 0;
		size_t m_nWarnings = 0;
		size_t m_nErrors   = 0;

		bool m_logWithHexa          = false;
		bool m_logTimeInSecond      = false;
		uint32_t m_logTimePrecision = 0;

		CLogObject* m_pCurrentLog = nullptr;

		void updateMessageCounts() const;
		void checkAppendFilterCurrentLog(const char* textColor, const char* logMessage, bool bIsLink = false) const;
	};
} // namespace OpenViBEDesigner
