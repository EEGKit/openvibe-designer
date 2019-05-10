#include "ovdCIntegerSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_button_setting_integer_up_pressed(GtkButton* /*pButton*/, gpointer pUserData)
{
	static_cast<CIntegerSettingView*>(pUserData)->adjustValue(1);
}

static void on_button_setting_integer_down_pressed(GtkButton* /*pButton*/, gpointer pUserData)
{
	static_cast<CIntegerSettingView*>(pUserData)->adjustValue(-1);
}

static void on_insertion(GtkEntry* /*entry*/, gpointer pUserData)
{
	static_cast<CIntegerSettingView*>(pUserData)->onChange();
}


CIntegerSettingView::CIntegerSettingView(Kernel::IBox& rBox, uint32_t ui32Index, CString& rBuilderName, const Kernel::IKernelContext& rKernelContext) :
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_integer"), m_rKernelContext(rKernelContext), m_bOnValueSetting(false)
{
	GtkWidget* l_pSettingWidget = this->CAbstractSettingView::getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	CAbstractSettingView::extractWidget(l_pSettingWidget, l_vWidget);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_insertion), this);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_integer_up_pressed), this);
	g_signal_connect(G_OBJECT(l_vWidget[2]), "clicked", G_CALLBACK(on_button_setting_integer_down_pressed), this);

	CAbstractSettingView::initializeValue();
}


void CIntegerSettingView::getValue(CString& rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CIntegerSettingView::setValue(const CString& rValue)
{
	m_bOnValueSetting = true;
	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting = false;
}

void CIntegerSettingView::adjustValue(int amount)
{
	char l_sValue[1024];
	int64_t l_i64lValue = m_rKernelContext.getConfigurationManager().expandAsInteger(gtk_entry_get_text(m_pEntry), 0);
	l_i64lValue += amount;
	sprintf(l_sValue, "%lli", l_i64lValue);

	getBox().setSettingValue(getSettingIndex(), l_sValue);
	setValue(l_sValue);
}

void CIntegerSettingView::onChange()
{
	if (!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
