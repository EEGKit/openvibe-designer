#include "ovdCIntegerSettingView.h"
#include "../ovd_base.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace Setting;

static void OnButtonSettingIntegerUpPressed(GtkButton* /*button*/, gpointer data) { static_cast<CIntegerSettingView *>(data)->adjustValue(1); }

static void OnButtonSettingIntegerDownPressed(GtkButton* /*button*/, gpointer data) { static_cast<CIntegerSettingView *>(data)->adjustValue(-1); }

static void OnInsertion(GtkEntry* /*entry*/, gpointer data) { static_cast<CIntegerSettingView *>(data)->onChange(); }


CIntegerSettingView::CIntegerSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx)
	: CAbstractSettingView(box, index, builderName, "settings_collection-hbox_setting_integer"), m_kernelCtx(ctx)
{
	GtkWidget* settingWidget = CAbstractSettingView::getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	CAbstractSettingView::extractWidget(settingWidget, widgets);
	m_entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnInsertion), this);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingIntegerUpPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingIntegerDownPressed), this);

	CAbstractSettingView::initializeValue();
}


void CIntegerSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }


void CIntegerSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CIntegerSettingView::adjustValue(const int amount)
{
	const int64_t value   = m_kernelCtx.getConfigurationManager().expandAsInteger(gtk_entry_get_text(m_entry), 0) + amount;
	const std::string res = std::to_string(value);

	getBox().setSettingValue(getSettingIndex(), res.c_str());
	setValue(res.c_str());
}

void CIntegerSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* value = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), value);
	}
}
