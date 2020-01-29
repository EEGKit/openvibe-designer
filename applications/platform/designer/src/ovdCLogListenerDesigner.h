#pragma once

#include "ovd_base.h"

#include <vector>
#include <map>

#define OVK_ClassId_Designer_LogListener		OpenViBE::CIdentifier(0x0FE155FA, 0x313C17A7)

namespace OpenViBEDesigner
{
	class CApplication;

	class CLogListenerDesigner final : public OpenViBE::Kernel::ILogListener
	{
	public:
		class CLogObject
		{
		public:
			explicit CLogObject(GtkTextBuffer* buffer)
			{
				m_Buffer       = gtk_text_buffer_new(gtk_text_buffer_get_tag_table(buffer));
				m_PassedFilter = false;//by default the log does not pass the filter;
			}

			GtkTextBuffer* getTextBuffer() const { return m_Buffer; }

			//determine if the log contains the searchTerm and tag the part with the sSerachTerm in gray
			bool filter(const OpenViBE::CString& searchTerm)
			{
				m_PassedFilter = false;
				GtkTextIter startFind, endFind;
				gtk_text_buffer_get_start_iter(m_Buffer, &startFind);
				gtk_text_buffer_get_end_iter(m_Buffer, &endFind);

				//tag for highlighting the search term
				GtkTextTag* tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(m_Buffer), "gray_bg");
				if (tag == nullptr) { gtk_text_buffer_create_tag(m_Buffer, "gray_bg", "background", "gray", nullptr); }

				//remove previous tagging
				gtk_text_buffer_remove_tag_by_name(m_Buffer, "gray_bg", &startFind, &endFind);

				//no term means no research so no filter we let all pass
				if (searchTerm == OpenViBE::CString(""))
				{
					m_PassedFilter = true;
					return m_PassedFilter;
				}


				GtkTextIter startMatch, endMatch;
				const gchar* text = searchTerm.toASCIIString();
				while (gtk_text_iter_forward_search(&startFind, text, GTK_TEXT_SEARCH_TEXT_ONLY, &startMatch, &endMatch, nullptr))
				{
					gtk_text_buffer_apply_tag_by_name(m_Buffer, "gray_bg", &startMatch, &endMatch);
					//offset to end_match
					const int offset = gtk_text_iter_get_offset(&endMatch);
					//begin next search at end match
					gtk_text_buffer_get_iter_at_offset(m_Buffer, &startFind, offset);
					m_PassedFilter = true;
				}
				return m_PassedFilter;
			}

			void appendToCurrentLog(const char* textColor, const char* logMessage, const bool isLink /* = false */) const
			{
				GtkTextIter endIter;
				gtk_text_buffer_get_end_iter(m_Buffer, &endIter);

				if (isLink) { gtk_text_buffer_insert_with_tags_by_name(m_Buffer, &endIter, logMessage, -1, "f_mono", textColor, "link", nullptr); }
				else { gtk_text_buffer_insert_with_tags_by_name(m_Buffer, &endIter, logMessage, -1, "f_mono", textColor, nullptr); }
			}

			GtkTextBuffer* m_Buffer = nullptr;
			bool m_PassedFilter     = false;
		};

		CLogListenerDesigner(const OpenViBE::Kernel::IKernelContext& ctx, GtkBuilder* builder);

		bool isActive(const OpenViBE::Kernel::ELogLevel level) override;
		bool activate(const OpenViBE::Kernel::ELogLevel level, const bool active) override;
		bool activate(const OpenViBE::Kernel::ELogLevel startLevel, const OpenViBE::Kernel::ELogLevel endLevel, const bool active) override;
		bool activate(const bool active) override;

		void log(const OpenViBE::time64 value) override;

		void log(const uint64_t value) override;
		void log(const uint32_t value) override;

		void log(const int64_t value) override;
		void log(const int value) override;

		void log(const double value) override;

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
		void searchMessages(const OpenViBE::CString& searchTerm);
		void displayLog(CLogObject* log) const;
		void appendLog(CLogObject* log) const;

		_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier)

		OpenViBE::CString m_SearchTerm;
		GtkTextTag* m_IdTag = nullptr;
		std::function<void(OpenViBE::CIdentifier&)> m_CenterOnBoxFun;

	protected:

		std::map<OpenViBE::Kernel::ELogLevel, bool> m_activeLevels;

		//logs
		std::vector<CLogObject*> m_storedLogs;

	private:

		GtkBuilder* m_builder   = nullptr;
		GtkTextView* m_textView = nullptr;
		GtkTextBuffer* m_buffer = nullptr;

		GtkToggleButton* m_buttonPopup = nullptr;

		GtkToggleToolButton* m_buttonActiveDebug            = nullptr;
		GtkToggleToolButton* m_buttonActiveBenchmark        = nullptr;
		GtkToggleToolButton* m_buttonActiveTrace            = nullptr;
		GtkToggleToolButton* m_buttonActiveInfo             = nullptr;
		GtkToggleToolButton* m_buttonActiveWarning          = nullptr;
		GtkToggleToolButton* m_buttonActiveImportantWarning = nullptr;
		GtkToggleToolButton* m_buttonActiveError            = nullptr;
		GtkToggleToolButton* m_buttonActiveFatal            = nullptr;

		GtkLabel* m_labelnMsg            = nullptr;
		GtkLabel* m_labelnWarnings       = nullptr;
		GtkLabel* m_labelnErrors         = nullptr;
		GtkLabel* m_labelDialognWarnings = nullptr;
		GtkLabel* m_labelDialognErrors   = nullptr;

		GtkWidget* m_imageWarnings = nullptr;
		GtkWidget* m_imageErrors   = nullptr;

		GtkWindow* m_alertWindow = nullptr;

		bool m_ignoreMsg = false;

		size_t m_nMsg     = 0;
		size_t m_nWarning = 0;
		size_t m_nError   = 0;

		bool m_logWithHexa        = false;
		bool m_logTimeInSecond    = false;
		size_t m_logTimePrecision = 0;

		CLogObject* m_currentLog = nullptr;

		void updateMessageCounts() const;
		void checkAppendFilterCurrentLog(const char* textColor, const char* logMessage, bool isLink = false) const;
	};
} // namespace OpenViBEDesigner
