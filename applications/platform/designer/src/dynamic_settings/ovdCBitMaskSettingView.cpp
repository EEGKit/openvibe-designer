#include "ovdCBitMaskSettingView.h"
#include "../ovd_base.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace Setting;

static void OnCheckbuttonPressed(GtkToggleButton* /*button*/, gpointer data) { static_cast<CBitMaskSettingView*>(data)->onChange(); }

CBitMaskSettingView::CBitMaskSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx,
										 const CIdentifier& typeID)
	: CAbstractSettingView(box, index, builderName, "settings_collection-table_setting_bitmask"), m_typeID(typeID), m_kernelCtx(ctx)
{
	GtkWidget* settingWidget = CAbstractSettingView::getEntryFieldWidget();
	GtkGrid* bitMaskTable = GTK_GRID(settingWidget);

	for (size_t i = 0; i < m_kernelCtx.getTypeManager().getBitMaskEntryCount(m_typeID); ++i)
	{
		CString name;
		uint64_t value;
		if (m_kernelCtx.getTypeManager().getBitMaskEntry(m_typeID, i, name, value))
		{
			GtkWidget* button = gtk_check_button_new();
			gtk_grid_attach(bitMaskTable, button, guint(i & 1), guint(i >> 1), 1, 1);
			gtk_button_set_label(GTK_BUTTON(button), name.toASCIIString());
			m_toggleButton.push_back(GTK_TOGGLE_BUTTON(button));
			g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(OnCheckbuttonPressed), this);
		}
	}
	gtk_widget_show_all(GTK_WIDGET(bitMaskTable));

	CAbstractSettingView::initializeValue();
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
	value = res.c_str();
}


void CBitMaskSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	const std::string str(value);

	for (auto& toggle : m_toggleButton)
	{
		const gchar* label = gtk_button_get_label(GTK_BUTTON(toggle));
		if (str.find(label) != std::string::npos) { gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), true); }
		else { gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), false); }
	}

	m_onValueSetting = false;
}

void CBitMaskSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		CString value;
		this->getValue(value);
		getBox().setSettingValue(getSettingIndex(), value);
	}
}
