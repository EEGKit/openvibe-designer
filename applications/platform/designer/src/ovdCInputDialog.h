#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	typedef void (*fpButtonCB)(GtkWidget* pWidget, gpointer data);

	class CInputDialog
	{
	public:

		CInputDialog(const char* sGtkBuilder, fpButtonCB fpOKButtonCB, void* data = nullptr, const char* sTitle = nullptr, const char* sLabel = nullptr, const char* sEntry = nullptr);
		~CInputDialog();

		void run();
		void* getUserData() { return m_pUserData; }
		const char* getEntry() { return static_cast<const char*>(gtk_entry_get_text(m_iDialogEntry)); }

	private:

		static gboolean key_press_event_cb(GtkWidget* widget, GdkEventKey* eventKey, gpointer data);
		static void button_clicked_cb(GtkButton* button, gpointer data);
		void buttonClickedCB(GtkButton* button) const;

		void* m_pUserData = nullptr;
		fpButtonCB m_fpOKButtonCB;
		GtkDialog* m_iDialog = nullptr;
		GtkLabel* m_iDialogLabel = nullptr;
		GtkEntry* m_iDialogEntry = nullptr;
		GtkButton* m_iDialogOKButton = nullptr;
		GtkButton* m_iDialogCancelButton = nullptr;
	};
}  // namespace OpenViBEDesigner
