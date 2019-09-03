#include "ovdCColorSettingView.h"
#include "../ovd_base.h"

#include <iostream>
#include <cmath>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_button_setting_color_choose_pressed(GtkColorButton* /*button*/, gpointer data) { static_cast<CColorSettingView *>(data)->selectColor(); }

static void on_change(GtkEntry* /*entry*/, gpointer data) { static_cast<CColorSettingView*>(data)->onChange(); }


CColorSettingView::
CColorSettingView(Kernel::IBox& box, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& ctx): CAbstractSettingView(
																																	 box, index, rBuilderName,
																																	 "settings_collection-hbox_setting_color"),
																																 m_kernelContext(ctx)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_entry  = GTK_ENTRY(l_vWidget[0]);
	m_button = GTK_COLOR_BUTTON(l_vWidget[1]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(on_change), this);
	g_signal_connect(G_OBJECT(m_button), "color-set", G_CALLBACK(on_button_setting_color_choose_pressed), this);

	initializeValue();
}


void CColorSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }

void CColorSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	int r            = 0, g = 0, b = 0;
	sscanf(m_kernelContext.getConfigurationManager().expand(value).toASCIIString(), "%i,%i,%i", &r, &g, &b);

	GdkColor l_oColor;
	l_oColor.red   = (r * 65535) / 100;
	l_oColor.green = (g * 65535) / 100;
	l_oColor.blue  = (b * 65535) / 100;
	gtk_color_button_set_color(m_button, &l_oColor);

	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CColorSettingView::selectColor()
{
	GdkColor l_oColor;
	gtk_color_button_get_color(m_button, &l_oColor);

	char l_sBuffer[1024];
	sprintf(l_sBuffer, "%i,%i,%i", int(round((l_oColor.red * 100) / 65535.)), int(round((l_oColor.green * 100) / 65535.)),
			int(round((l_oColor.blue * 100) / 65535.)));

	getBox().setSettingValue(getSettingIndex(), l_sBuffer);
	setValue(l_sBuffer);
}

void CColorSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
