#include "ovdCStringSettingView.h"
#include "../ovd_base.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CStringSettingView *>(data)->onChange(); }

CStringSettingView::CStringSettingView(Kernel::IBox& box, const size_t index, CString& builderName)
	: CAbstractSettingView(box, index, builderName, "settings_collection-entry_setting_string")
{
	GtkWidget* settingWidget = CAbstractSettingView::getEntryFieldWidget();

	m_entry = GTK_ENTRY(settingWidget);
	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnChange), this);

	CAbstractSettingView::initializeValue();
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
		const gchar* value = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), value);
	}
}
