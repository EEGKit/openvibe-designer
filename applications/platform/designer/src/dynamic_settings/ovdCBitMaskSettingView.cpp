#include "ovdCBitMaskSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_checkbutton__pressed(GtkToggleButton* /*button*/, gpointer data)
{
	static_cast<CBitMaskSettingView *>(data)->onChange();
}

CBitMaskSettingView::CBitMaskSettingView(Kernel::IBox& rBox, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& rKernelContext, const CIdentifier& rTypeIdentifier): CAbstractSettingView(rBox, index, rBuilderName, "settings_collection-table_setting_bitmask"), m_oTypeIdentifier(rTypeIdentifier), m_kernelContext(rKernelContext)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	const gint tableSize      = guint((m_kernelContext.getTypeManager().getBitMaskEntryCount(m_oTypeIdentifier) + 1) >> 1);
	GtkTable* l_pBitMaskTable = GTK_TABLE(l_pSettingWidget);
	gtk_table_resize(l_pBitMaskTable, 2, tableSize);

	for (uint64_t i = 0; i < m_kernelContext.getTypeManager().getBitMaskEntryCount(m_oTypeIdentifier); i++)
	{
		CString l_sEntryName;
		uint64_t l_ui64EntryValue;
		if (m_kernelContext.getTypeManager().getBitMaskEntry(m_oTypeIdentifier, i, l_sEntryName, l_ui64EntryValue))
		{
			GtkWidget* l_pSettingButton = gtk_check_button_new();
			gtk_table_attach_defaults(l_pBitMaskTable, l_pSettingButton, guint(i & 1), guint((i & 1) + 1), guint(i >> 1), guint((i >> 1) + 1));
			gtk_button_set_label(GTK_BUTTON(l_pSettingButton), static_cast<const char*>(l_sEntryName));
			m_toggleButton.push_back(GTK_TOGGLE_BUTTON(l_pSettingButton));
			g_signal_connect(G_OBJECT(l_pSettingButton), "toggled", G_CALLBACK(on_checkbutton__pressed), this);
		}
	}
	gtk_widget_show_all(GTK_WIDGET(l_pBitMaskTable));

	initializeValue();
}


void CBitMaskSettingView::getValue(CString& value) const
{
	std::string res;
	for (auto& toggle : m_toggleButton)
	{
		if (gtk_toggle_button_get_active(toggle))
		{
			if (!res.empty()) { res += ':'; }
			res += gtk_button_get_label(GTK_BUTTON(toggle));
		}
	}
	value = CString(res.c_str());
}


void CBitMaskSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	const std::string sValue(value);

	for (auto& toggle : m_toggleButton)
	{
		const gchar* l_sLabel = gtk_button_get_label(GTK_BUTTON(toggle));
		if (sValue.find(l_sLabel) != std::string::npos)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), true);
		}
		else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), false);
		}
	}

	m_onValueSetting = false;
}

void CBitMaskSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		CString l_sValue;
		this->getValue(l_sValue);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
