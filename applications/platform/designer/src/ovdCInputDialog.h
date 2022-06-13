#pragma once

#include "ovd_base.h"

namespace OpenViBE {
namespace Designer {
typedef void (*fpButtonCB)(GtkWidget* pWidget, gpointer data);

class CInputDialog
{
public:
	CInputDialog(const char* gtkBuilder, fpButtonCB okButtonCB, void* data = nullptr, const char* title = nullptr, const char* label = nullptr,
				 const char* entry                                         = nullptr);
	~CInputDialog();

	void run();
	void* getUserData() const { return m_userData; }
	const char* getEntry() const { return static_cast<const char*>(gtk_entry_get_text(m_dialogEntry)); }

private:
	static gboolean keyPressEventCB(GtkWidget* widget, GdkEventKey* eventKey, gpointer data);
	static void buttonClickedCB(GtkButton* button, gpointer data);
	void buttonClicked(GtkButton* button) const;

	void* m_userData = nullptr;
	fpButtonCB m_okButtonCB;
	GtkDialog* m_dialog             = nullptr;
	GtkLabel* m_dialogLabel         = nullptr;
	GtkEntry* m_dialogEntry         = nullptr;
	GtkButton* m_dialogOkButton     = nullptr;
	GtkButton* m_dialogCancelButton = nullptr;
};
}  // namespace Designer
}  // namespace OpenViBE
