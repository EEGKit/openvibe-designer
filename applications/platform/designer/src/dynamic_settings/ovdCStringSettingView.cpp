#include "ovdCStringSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_change(GtkEntry* /*entry*/, gpointer data)
{
	static_cast<CStringSettingView *>(data)->onChange();
}

CStringSettingView::CStringSettingView(Kernel::IBox& rBox, const uint32_t index, CString& rBuilderName):
	CAbstractSettingView(rBox, index, rBuilderName, "settings_collection-entry_setting_string")
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	m_entry = GTK_ENTRY(l_pSettingWidget);
	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(on_change), this);

	initializeValue();
}


void CStringSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }


void CStringSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CStringSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
