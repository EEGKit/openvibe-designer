#include "ovdCStringSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_change(GtkEntry *entry, gpointer pUserData)
{
	static_cast<CStringSettingView *>(pUserData)->onChange();
}

CStringSettingView::CStringSettingView(Kernel::IBox &rBox, uint32_t ui32Index, CString &rBuilderName):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-entry_setting_string"), m_bOnValueSetting(false)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	m_pEntry = GTK_ENTRY(l_pSettingWidget);
	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_change), this);

	initializeValue();
}


void CStringSettingView::getValue(CString &rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CStringSettingView::setValue(const CString &rValue)
{
	m_bOnValueSetting = true;
	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting =false;
}

void CStringSettingView::onChange()
{
	if(!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
