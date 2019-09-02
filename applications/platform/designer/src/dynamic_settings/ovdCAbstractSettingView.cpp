#include "ovdCAbstractSettingView.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;


static void collect_widget_cb(GtkWidget* widget, gpointer data) { static_cast<std::vector<GtkWidget*> *>(data)->push_back(widget); }

CAbstractSettingView::~CAbstractSettingView()
{
	if (GTK_IS_WIDGET(m_nameWidget)) { gtk_widget_destroy(m_nameWidget); }
	if (GTK_IS_WIDGET(m_entryNameWidget)) { gtk_widget_destroy(m_entryNameWidget); }
	if (G_IS_OBJECT(m_pBuilder)) { g_object_unref(m_pBuilder); }
}

CAbstractSettingView::
CAbstractSettingView(Kernel::IBox& box, const uint32_t index, const char* sBuilderName, const char* sWidgetName): m_rBox(box), m_index(index),
																												  m_settingWidgetName("")
{
	if (sBuilderName != nullptr)
	{
		m_pBuilder = gtk_builder_new();
		gtk_builder_add_from_file(m_pBuilder, sBuilderName, nullptr);
		gtk_builder_connect_signals(m_pBuilder, nullptr);

		if (sWidgetName != nullptr)
		{
			m_settingWidgetName = sWidgetName;
			generateNameWidget();
			m_entryFieldWidget = generateEntryWidget();
		}
	}
}

void CAbstractSettingView::setNameWidget(GtkWidget* widget)
{
	if (m_nameWidget) { gtk_widget_destroy(m_nameWidget); }
	m_nameWidget = widget;
}

void CAbstractSettingView::setEntryWidget(GtkWidget* widget)
{
	if (m_entryNameWidget) { gtk_widget_destroy(m_entryNameWidget); }
	m_entryNameWidget = widget;
}

void CAbstractSettingView::generateNameWidget()
{
	GtkWidget* l_pSettingName = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-label_setting_name"));
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingName)), l_pSettingName);
	setNameWidget(l_pSettingName);

	CString l_sSettingName;
	getBox().getSettingName(m_index, l_sSettingName);
	gtk_label_set_text(GTK_LABEL(l_pSettingName), l_sSettingName);
}

GtkWidget* CAbstractSettingView::generateEntryWidget()
{
	GtkTable* m_pTable = GTK_TABLE(gtk_table_new(1, 3, false));

	GtkWidget* l_pSettingWidget  = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, m_settingWidgetName.toASCIIString()));
	GtkWidget* l_pSettingRevert  = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-button_setting_revert"));
	GtkWidget* l_pSettingDefault = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-button_setting_default"));

	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingWidget)), l_pSettingWidget);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingRevert)), l_pSettingRevert);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingDefault)), l_pSettingDefault);

	gtk_table_attach(m_pTable, l_pSettingWidget, 0, 1, 0, 1, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(GTK_FILL | GTK_EXPAND), 0, 0);
	//gtk_table_attach(m_pTable, l_pSettingDefault, 1, 2, 0, 1, ::GtkAttachOptions(GTK_SHRINK),          ::GtkAttachOptions(GTK_SHRINK),          0, 0);
	//gtk_table_attach(m_pTable, l_pSettingRevert,  2, 3, 0, 1, ::GtkAttachOptions(GTK_SHRINK),          ::GtkAttachOptions(GTK_SHRINK),          0, 0);

	setEntryWidget(GTK_WIDGET(m_pTable));
	gtk_widget_set_visible(getEntryWidget(), true);
	//If we don't increase the ref counter it will cause trouble when we gonna move it later
	g_object_ref(G_OBJECT(m_pTable));
	return l_pSettingWidget;
}

void CAbstractSettingView::initializeValue()
{
	CString l_sSettingValue;
	getBox().getSettingValue(m_index, l_sSettingValue);
	setValue(l_sSettingValue);
}

void CAbstractSettingView::extractWidget(GtkWidget* widget, std::vector<GtkWidget*>& rVector)
{
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &rVector);
}
