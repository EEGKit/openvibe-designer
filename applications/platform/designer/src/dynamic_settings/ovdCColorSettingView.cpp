#include "ovdCColorSettingView.h"
#include "../ovd_base.h"

#include <cmath>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static int Color2Percent(const guint16 color) { return int(round(color / 655.350)); }	// c * 100 / 65535
static guint16 Percent2Color(const int color) { return guint16(color * 655.35); }		// c * 65535 / 100

static void OnButtonSettingColorChoosePressed(GtkColorButton* /*button*/, gpointer data) { static_cast<CColorSettingView *>(data)->selectColor(); }
static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CColorSettingView*>(data)->onChange(); }

CColorSettingView::CColorSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx)
	: CAbstractSettingView(box, index, builderName, "settings_collection-hbox_setting_color"), m_kernelCtx(ctx)
{
	GtkWidget* settingWidget = CAbstractSettingView::getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	CAbstractSettingView::extractWidget(settingWidget, widgets);
	m_entry  = GTK_ENTRY(widgets[0]);
	m_button = GTK_COLOR_BUTTON(widgets[1]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnChange), this);
	g_signal_connect(G_OBJECT(m_button), "color-set", G_CALLBACK(OnButtonSettingColorChoosePressed), this);

	CAbstractSettingView::initializeValue();
}


void CColorSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }

void CColorSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	int r            = 0, g = 0, b = 0;
	sscanf(m_kernelCtx.getConfigurationManager().expand(value).toASCIIString(), "%i,%i,%i", &r, &g, &b);

	GdkColor color;
	color.red   = Percent2Color(r);
	color.green = Percent2Color(g);
	color.blue  = Percent2Color(b);
	gtk_color_button_set_color(m_button, &color);

	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CColorSettingView::selectColor()
{
	GdkColor color;
	gtk_color_button_get_color(m_button, &color);
	const std::string value = std::to_string(Color2Percent(color.red)) + "," + std::to_string(Color2Percent(color.green)) + ","
							  + std::to_string(Color2Percent(color.blue));
	getBox().setSettingValue(getSettingIndex(), value.c_str());
	setValue(value.c_str());
}

void CColorSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* value = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), value);
	}
}
