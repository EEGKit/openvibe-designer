#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	typedef void (*fpButtonCB)(GtkWidget* pWidget, gpointer pUserData);

	class CInputDialog
	{
	public:

		CInputDialog(const char* sGtkBuilder, fpButtonCB fpOKButtonCB, void* pUserData = nullptr, const char* sTitle = nullptr, const char* sLabel = nullptr, const char* sEntry = nullptr);
		~CInputDialog();

		void run();
		void* getUserData() { return m_pUserData; }
		const char* getEntry() { return static_cast<const char*>(gtk_entry_get_text(m_pInputDialogEntry)); }

	private:

		static gboolean key_press_event_cb(GtkWidget* pWidget, GdkEventKey* pEventKey, gpointer pUserData);
		static void button_clicked_cb(GtkButton* pButton, gpointer pUserData);
		void buttonClickedCB(GtkButton* pButton);

		void* m_pUserData = nullptr;
		fpButtonCB m_fpOKButtonCB;
		GtkDialog* m_pInputDialog = nullptr;
		GtkLabel* m_pInputDialogLabel = nullptr;
		GtkEntry* m_pInputDialogEntry = nullptr;
		GtkButton* m_pInputDialogOKButton = nullptr;
		GtkButton* m_pInputDialogCancelButton = nullptr;
	};
};
