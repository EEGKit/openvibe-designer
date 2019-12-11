#include "ovdCFloatSettingView.h"
#include "../ovd_base.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void OnButtonSettingFloatUpPressed(GtkButton* /*button*/, gpointer data) { static_cast<CFloatSettingView *>(data)->adjustValue(1.0); }

static void OnButtonSettingFloatDownPressed(GtkButton* /*button*/, gpointer data) { static_cast<CFloatSettingView *>(data)->adjustValue(-1.0); }

static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CFloatSettingView *>(data)->onChange(); }


CFloatSettingView::CFloatSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx)
	: CAbstractSettingView(box, index, builderName, "settings_collection-hbox_setting_float"), m_kernelCtx(ctx)
{
	GtkWidget* settingWidget = CAbstractSettingView::getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	CAbstractSettingView::extractWidget(settingWidget, widgets);
	m_entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnChange), this);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingFloatUpPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingFloatDownPressed), this);

	CAbstractSettingView::initializeValue();
}


void CFloatSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }


void CFloatSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CFloatSettingView::adjustValue(const double amount)
{
	const double value = m_kernelCtx.getConfigurationManager().expandAsFloat(gtk_entry_get_text(m_entry), 0) + amount;
	const std::string str = std::to_string(value);
	getBox().setSettingValue(getSettingIndex(), str.c_str());
	setValue(str.c_str());
}

void CFloatSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* value = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), value);
	}
}
