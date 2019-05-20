#include "ovdCFloatSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_button_setting_float_up_pressed(GtkButton* /*button*/, gpointer data)
{
	static_cast<CFloatSettingView *>(data)->adjustValue(1.0);
}

static void on_button_setting_float_down_pressed(GtkButton* /*button*/, gpointer data)
{
	static_cast<CFloatSettingView *>(data)->adjustValue(-1.0);
}

static void on_change(GtkEntry* /*entry*/, gpointer data)
{
	static_cast<CFloatSettingView *>(data)->onChange();
}


CFloatSettingView::CFloatSettingView(Kernel::IBox& rBox, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& rKernelContext):
	CAbstractSettingView(rBox, index, rBuilderName, "settings_collection-hbox_setting_float"), m_rKernelContext(rKernelContext)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_entry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(on_change), this);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_float_up_pressed), this);
	g_signal_connect(G_OBJECT(l_vWidget[2]), "clicked", G_CALLBACK(on_button_setting_float_down_pressed), this);

	initializeValue();
}


void CFloatSettingView::getValue(CString& value) const
{
	value = CString(gtk_entry_get_text(m_entry));
}


void CFloatSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CFloatSettingView::adjustValue(const double amount)
{
	char l_sValue[1024];
	double l_f64lValue = m_rKernelContext.getConfigurationManager().expandAsFloat(gtk_entry_get_text(m_entry), 0);
	l_f64lValue += amount;
	sprintf(l_sValue, "%lf", l_f64lValue);

	getBox().setSettingValue(getSettingIndex(), l_sValue);
	setValue(l_sValue);
}

void CFloatSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
