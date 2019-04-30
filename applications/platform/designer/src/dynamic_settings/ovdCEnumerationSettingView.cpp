#include "ovdCEnumerationSettingView.h"
#include "../ovd_base.h"

#include <algorithm> // std::sort
#include <iostream>
#include <map>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_change(GtkEntry *entry, gpointer pUserData)
{
	static_cast<CEnumerationSettingView *>(pUserData)->onChange();
}

CEnumerationSettingView::CEnumerationSettingView(Kernel::IBox &rBox, uint32_t ui32Index,
												 CString &rBuilderName, const Kernel::IKernelContext &rKernelContext,
												 const CIdentifier &rTypeIdentifier):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-comboboxentry_setting_enumeration"),
	m_oTypeIdentifier(rTypeIdentifier),
	m_rKernelContext(rKernelContext),
	m_bOnValueSetting(false)
{
	p=false;
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	m_pComboBox = GTK_COMBO_BOX(l_pSettingWidget);

	std::vector<std::string> l_vEntries;

	for(uint64_t i=0; i<m_rKernelContext.getTypeManager().getEnumerationEntryCount(m_oTypeIdentifier); i++)
	{
		CString l_sEntryName;
		uint64_t l_ui64EntryValue;
		if(m_rKernelContext.getTypeManager().getEnumerationEntry(m_oTypeIdentifier, i, l_sEntryName, l_ui64EntryValue))
		{
			l_vEntries.push_back(l_sEntryName.toASCIIString());
		}
	}

	std::sort(l_vEntries.begin(), l_vEntries.end());

	GtkTreeIter l_oListIter;
	GtkListStore* l_pList=GTK_LIST_STORE(gtk_combo_box_get_model(m_pComboBox));
	gtk_combo_box_set_wrap_width(m_pComboBox, 0);
	gtk_list_store_clear(l_pList);

	for (size_t i = 0; i < l_vEntries.size(); i++)
	{
		gtk_list_store_append(l_pList, &l_oListIter);
		gtk_list_store_set(l_pList, &l_oListIter, 0, l_vEntries[i].c_str(), -1);

		m_mEntriesIndex[CString(l_vEntries[i].c_str())] = static_cast<uint64_t>(i);
	}

	CString settingValue;
	rBox.getSettingValue(ui32Index, settingValue);
	if (m_mEntriesIndex.count(settingValue.toASCIIString()) == 0)
	{
		gtk_list_store_append(l_pList, &l_oListIter);
		gtk_list_store_set(l_pList, &l_oListIter, 0, settingValue.toASCIIString(), -1);
	}

	initializeValue();

	g_signal_connect(G_OBJECT(m_pComboBox), "changed", G_CALLBACK(on_change), this);
}


void CEnumerationSettingView::getValue(CString &rValue) const
{
	rValue = CString(gtk_combo_box_get_active_text(m_pComboBox));
}


void CEnumerationSettingView::setValue(const CString &rValue)
{
	m_bOnValueSetting = true;

	// If the current value of the setting is not in the enumeration list, we will add or replace the last value in the list, so it can be set to this value
	if (m_mEntriesIndex.count(rValue) == 0)
	{
		GtkTreeIter l_oListIter;
		GtkListStore* l_pList = GTK_LIST_STORE(gtk_combo_box_get_model(m_pComboBox));
		int valuesInModel = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(l_pList), nullptr);
		if (valuesInModel == static_cast<int>(m_mEntriesIndex.size()))
		{
			gtk_list_store_append(l_pList, &l_oListIter);
			valuesInModel += 1;
		}
		else
		{
			// We just set the iterator at the end
			GtkTreePath* treePath = gtk_tree_path_new_from_indices(valuesInModel - 1, -1);
			gtk_tree_model_get_iter(GTK_TREE_MODEL(l_pList), &l_oListIter, treePath);
			gtk_tree_path_free(treePath);

		}
		gtk_list_store_set(l_pList, &l_oListIter, 0, rValue.toASCIIString(), -1);
		gtk_combo_box_set_active(m_pComboBox, valuesInModel - 1);
	}
	else
	{
		gtk_combo_box_set_active(m_pComboBox, (gint)m_mEntriesIndex[rValue]);
	}
	m_bOnValueSetting =false;
}

void CEnumerationSettingView::onChange()
{
	if(!m_bOnValueSetting)
	{
		gchar* l_sValue = gtk_combo_box_get_active_text(m_pComboBox);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
		g_free(l_sValue);
	}
}

