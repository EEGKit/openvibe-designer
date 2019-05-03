#include "ovdCBooleanSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_checkbutton_setting_bool_pressed(GtkToggleButton* pButton, gpointer pUserData)
{
	static_cast<CBooleanSettingView*>(pUserData)->toggleButtonClick();
}

static void on_insertion(GtkEntry* entry, gpointer pUserData)
{
	static_cast<CBooleanSettingView*>(pUserData)->onChange();
}

CBooleanSettingView::CBooleanSettingView(Kernel::IBox& rBox, uint32_t ui32Index, CString& rBuilderName) :
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_bool"), m_bOnValueSetting(false)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_pToggle = GTK_TOGGLE_BUTTON(l_vWidget[1]);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_insertion), this);

	g_signal_connect(G_OBJECT(m_pToggle), "toggled", G_CALLBACK(on_checkbutton_setting_bool_pressed), this);

	initializeValue();
}


void CBooleanSettingView::getValue(CString& rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CBooleanSettingView::setValue(const CString& rValue)
{
	m_bOnValueSetting = true;
	if (rValue == CString("true"))
	{
		gtk_toggle_button_set_active(m_pToggle, true);
		gtk_toggle_button_set_inconsistent(m_pToggle, false);
	}
	else if (rValue == CString("false"))
	{
		gtk_toggle_button_set_active(m_pToggle, false);
		gtk_toggle_button_set_inconsistent(m_pToggle, false);
	}
	else
	{
		gtk_toggle_button_set_inconsistent(m_pToggle, true);
	}

	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting = false;
}


void CBooleanSettingView::toggleButtonClick()
{
	if (!m_bOnValueSetting)
	{
		if (gtk_toggle_button_get_active(m_pToggle))
		{
			getBox().setSettingValue(getSettingIndex(), "true");
			setValue("true");
		}
		else
		{
			getBox().setSettingValue(getSettingIndex(), "false");
			setValue("false");
		}
	}
}

void CBooleanSettingView::onChange()
{
	if (!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
