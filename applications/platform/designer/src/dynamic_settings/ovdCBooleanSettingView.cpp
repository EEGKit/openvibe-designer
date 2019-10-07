#include "ovdCBooleanSettingView.h"
#include "../ovd_base.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void OnCheckbuttonSettingBooleanPressed(GtkToggleButton* /*button*/, gpointer data)
{
	static_cast<CBooleanSettingView *>(data)->toggleButtonClick();
}

static void OnInsertion(GtkEntry* /*entry*/, gpointer data) { static_cast<CBooleanSettingView *>(data)->onChange(); }

CBooleanSettingView::CBooleanSettingView(Kernel::IBox& box, const uint32_t index, CString& rBuilderName)
	: CAbstractSettingView(box, index, rBuilderName, "settings_collection-hbox_setting_boolean")
{
	GtkWidget* settingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	extractWidget(settingWidget, widgets);
	m_toggle = GTK_TOGGLE_BUTTON(widgets[1]);
	m_entry  = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnInsertion), this);

	g_signal_connect(G_OBJECT(m_toggle), "toggled", G_CALLBACK(OnCheckbuttonSettingBooleanPressed), this);

	initializeValue();
}


void CBooleanSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }


void CBooleanSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	if (value == CString("true"))
	{
		gtk_toggle_button_set_active(m_toggle, true);
		gtk_toggle_button_set_inconsistent(m_toggle, false);
	}
	else if (value == CString("false"))
	{
		gtk_toggle_button_set_active(m_toggle, false);
		gtk_toggle_button_set_inconsistent(m_toggle, false);
	}
	else { gtk_toggle_button_set_inconsistent(m_toggle, true); }

	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}


void CBooleanSettingView::toggleButtonClick()
{
	if (!m_onValueSetting)
	{
		if (gtk_toggle_button_get_active(m_toggle))
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
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
