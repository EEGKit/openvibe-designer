#include "ovdCEnumerationSettingView.h"
#include "../ovd_base.h"

#include <algorithm> // std::sort
#include <map>

using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace Setting;

static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CEnumerationSettingView*>(data)->onChange(); }

CEnumerationSettingView::CEnumerationSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx,
												 const CIdentifier& typeID)
	: CAbstractSettingView(box, index, builderName, "settings_collection-comboboxentry_setting_enumeration"), m_typeID(typeID), m_kernelCtx(ctx)
{
	m_comboBox = GTK_COMBO_BOX_TEXT(CAbstractSettingView::getEntryFieldWidget());

	std::vector<std::string> entries;
	for (size_t i = 0; i < m_kernelCtx.getTypeManager().getEnumerationEntryCount(m_typeID); ++i)
	{
		CString name;
		uint64_t value;
		if (m_kernelCtx.getTypeManager().getEnumerationEntry(m_typeID, i, name, value)) { entries.push_back(name.toASCIIString()); }
	}

	std::sort(entries.begin(), entries.end());

	gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(m_comboBox), 0);
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(m_comboBox));

	for (size_t i = 0; i < entries.size(); ++i)
	{
        gtk_combo_box_text_append_text(m_comboBox, entries[i].c_str());
		m_entriesIdx[CString(entries[i].c_str())] = uint64_t(i);
	}

	CString value;
	box.getSettingValue(index, value);
	if (m_entriesIdx.count(value.toASCIIString()) == 0)
	{
        gtk_combo_box_text_append_text(m_comboBox, value.toASCIIString());
	}

	CAbstractSettingView::initializeValue();

	g_signal_connect(G_OBJECT(m_comboBox), "changed", G_CALLBACK(OnChange), this);
}


void CEnumerationSettingView::getValue(CString& value) const { value = CString(gtk_combo_box_text_get_active_text(m_comboBox)); }


void CEnumerationSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;

	// If the current value of the setting is not in the enumeration list, we will add or replace the last value in the list, so it can be set to this value
	if (m_entriesIdx.count(value) == 0)
	{
		GtkListStore* list = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(m_comboBox)));
		int valuesInModel  = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list), nullptr);
		if (valuesInModel == int(m_entriesIdx.size()))
		{
			valuesInModel += 1;
		}
		else
		{
			// We just remove the item at the end
            gtk_combo_box_text_remove(m_comboBox, valuesInModel);
		}
        gtk_combo_box_text_append_text(m_comboBox, value.toASCIIString());
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboBox), valuesInModel - 1);
	}
	else { gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboBox), gint(m_entriesIdx[value])); }
	m_onValueSetting = false;
}

void CEnumerationSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		gchar* value = gtk_combo_box_text_get_active_text(m_comboBox);
		getBox().setSettingValue(getSettingIndex(), value);
		g_free(value);
	}
}
