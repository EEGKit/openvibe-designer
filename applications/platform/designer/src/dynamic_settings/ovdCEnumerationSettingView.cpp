#include "ovdCEnumerationSettingView.h"
#include "../ovd_base.h"

#include <algorithm> // std::sort
#include <map>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CEnumerationSettingView *>(data)->onChange(); }

CEnumerationSettingView::CEnumerationSettingView(Kernel::IBox& box, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& ctx, const CIdentifier& typeID)
	: CAbstractSettingView(box, index, rBuilderName, "settings_collection-comboboxentry_setting_enumeration"), m_typeID(typeID), m_kernelContext(ctx)
{
	p                        = false;
	GtkWidget* settingWidget = this->getEntryFieldWidget();

	m_comboBox = GTK_COMBO_BOX(settingWidget);

	std::vector<std::string> entries;

	for (uint64_t i = 0; i < m_kernelContext.getTypeManager().getEnumerationEntryCount(m_typeID); ++i)
	{
		CString name;
		uint64_t value;
		if (m_kernelContext.getTypeManager().getEnumerationEntry(m_typeID, i, name, value))
		{
			entries.push_back(name.toASCIIString());
		}
	}

	std::sort(entries.begin(), entries.end());

	GtkTreeIter listIter;
	GtkListStore* list = GTK_LIST_STORE(gtk_combo_box_get_model(m_comboBox));
	gtk_combo_box_set_wrap_width(m_comboBox, 0);
	gtk_list_store_clear(list);

	for (size_t i = 0; i < entries.size(); ++i)
	{
		gtk_list_store_append(list, &listIter);
		gtk_list_store_set(list, &listIter, 0, entries[i].c_str(), -1);

		m_entriesIdx[CString(entries[i].c_str())] = uint64_t(i);
	}

	CString settingValue;
	box.getSettingValue(index, settingValue);
	if (m_entriesIdx.count(settingValue.toASCIIString()) == 0)
	{
		gtk_list_store_append(list, &listIter);
		gtk_list_store_set(list, &listIter, 0, settingValue.toASCIIString(), -1);
	}

	initializeValue();

	g_signal_connect(G_OBJECT(m_comboBox), "changed", G_CALLBACK(OnChange), this);
}


void CEnumerationSettingView::getValue(CString& value) const { value = CString(gtk_combo_box_get_active_text(m_comboBox)); }


void CEnumerationSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;

	// If the current value of the setting is not in the enumeration list, we will add or replace the last value in the list, so it can be set to this value
	if (m_entriesIdx.count(value) == 0)
	{
		GtkTreeIter listIter;
		GtkListStore* list = GTK_LIST_STORE(gtk_combo_box_get_model(m_comboBox));
		int valuesInModel     = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list), nullptr);
		if (valuesInModel == int(m_entriesIdx.size()))
		{
			gtk_list_store_append(list, &listIter);
			valuesInModel += 1;
		}
		else
		{
			// We just set the iterator at the end
			GtkTreePath* treePath = gtk_tree_path_new_from_indices(valuesInModel - 1, -1);
			gtk_tree_model_get_iter(GTK_TREE_MODEL(list), &listIter, treePath);
			gtk_tree_path_free(treePath);
		}
		gtk_list_store_set(list, &listIter, 0, value.toASCIIString(), -1);
		gtk_combo_box_set_active(m_comboBox, valuesInModel - 1);
	}
	else { gtk_combo_box_set_active(m_comboBox, gint(m_entriesIdx[value])); }
	m_onValueSetting = false;
}

void CEnumerationSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		gchar* value = gtk_combo_box_get_active_text(m_comboBox);
		getBox().setSettingValue(getSettingIndex(), value);
		g_free(value);
	}
}
