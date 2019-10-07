#include "ovdCAbstractSettingView.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;


static void CollectWidgetCB(GtkWidget* widget, gpointer data) { static_cast<std::vector<GtkWidget*> *>(data)->push_back(widget); }

CAbstractSettingView::~CAbstractSettingView()
{
	if (GTK_IS_WIDGET(m_nameWidget)) { gtk_widget_destroy(m_nameWidget); }
	if (GTK_IS_WIDGET(m_entryNameWidget)) { gtk_widget_destroy(m_entryNameWidget); }
	if (G_IS_OBJECT(m_pBuilder)) { g_object_unref(m_pBuilder); }
}

CAbstractSettingView::CAbstractSettingView(Kernel::IBox& box, const uint32_t index, const char* sBuilderName, const char* sWidgetName)
	: m_box(box), m_index(index), m_settingWidgetName("")
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
	GtkWidget* settingName = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-label_setting_name"));
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(settingName)), settingName);
	setNameWidget(settingName);

	CString l_sSettingName;
	getBox().getSettingName(m_index, l_sSettingName);
	gtk_label_set_text(GTK_LABEL(settingName), l_sSettingName);
}

GtkWidget* CAbstractSettingView::generateEntryWidget()
{
	GtkTable* m_table = GTK_TABLE(gtk_table_new(1, 3, false));

	GtkWidget* settingWidget  = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, m_settingWidgetName.toASCIIString()));
	GtkWidget* settingRevert  = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-button_setting_revert"));
	GtkWidget* settingDefault = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-button_setting_default"));

	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(settingWidget)), settingWidget);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(settingRevert)), settingRevert);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(settingDefault)), settingDefault);

	gtk_table_attach(m_table, settingWidget, 0, 1, 0, 1, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(GTK_FILL | GTK_EXPAND), 0, 0);
	//gtk_table_attach(m_table, settingDefault, 1, 2, 0, 1, ::GtkAttachOptions(GTK_SHRINK),          ::GtkAttachOptions(GTK_SHRINK),          0, 0);
	//gtk_table_attach(m_table, settingRevert,  2, 3, 0, 1, ::GtkAttachOptions(GTK_SHRINK),          ::GtkAttachOptions(GTK_SHRINK),          0, 0);

	setEntryWidget(GTK_WIDGET(m_table));
	gtk_widget_set_visible(getEntryWidget(), true);
	//If we don't increase the ref counter it will cause trouble when we gonna move it later
	g_object_ref(G_OBJECT(m_table));
	return settingWidget;
}

void CAbstractSettingView::initializeValue()
{
	CString l_sSettingValue;
	getBox().getSettingValue(m_index, l_sSettingValue);
	setValue(l_sSettingValue);
}

void CAbstractSettingView::extractWidget(GtkWidget* widget, std::vector<GtkWidget*>& rVector)
{
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &rVector);
}
