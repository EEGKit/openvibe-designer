#include "ovdCIntegerSettingView.h"
#include "../ovd_base.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void OnButtonSettingIntegerUpPressed(GtkButton* /*button*/, gpointer data) { static_cast<CIntegerSettingView *>(data)->adjustValue(1); }

static void OnButtonSettingIntegerDownPressed(GtkButton* /*button*/, gpointer data) { static_cast<CIntegerSettingView *>(data)->adjustValue(-1); }

static void OnInsertion(GtkEntry* /*entry*/, gpointer data) { static_cast<CIntegerSettingView *>(data)->onChange(); }


CIntegerSettingView::CIntegerSettingView(Kernel::IBox& box, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& ctx)
	: CAbstractSettingView(box, index, rBuilderName, "settings_collection-hbox_setting_integer"), m_kernelContext(ctx)
{
	GtkWidget* settingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	extractWidget(settingWidget, widgets);
	m_entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnInsertion), this);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingIntegerUpPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingIntegerDownPressed), this);

	initializeValue();
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
	char l_sValue[1024];
	int64_t l_i64lValue = m_kernelContext.getConfigurationManager().expandAsInteger(gtk_entry_get_text(m_entry), 0);
	l_i64lValue += amount;
	sprintf(l_sValue, "%lli", l_i64lValue);

	getBox().setSettingValue(getSettingIndex(), l_sValue);
	setValue(l_sValue);
}

void CIntegerSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
