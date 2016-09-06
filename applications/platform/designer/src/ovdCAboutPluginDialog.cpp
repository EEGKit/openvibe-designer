#include "ovdCAboutPluginDialog.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;

CAboutPluginDialog::CAboutPluginDialog(const IKernelContext& rKernelContext, const CIdentifier& rPluginClassIdentifier, const char* sGUIFilename)
    : m_rKernelContext(rKernelContext),
      m_oPluginClassIdentifier(rPluginClassIdentifier),
      m_sGUIFilename(sGUIFilename),
      m_pPluginObjectDescriptor(NULL)
{
}

CAboutPluginDialog::CAboutPluginDialog(const IKernelContext& rKernelContext, const IPluginObjectDesc* pPluginObjectDescriptor, const char* sGUIFilename)
    : m_rKernelContext(rKernelContext),
      m_oPluginClassIdentifier(OV_UndefinedIdentifier),
      m_sGUIFilename(sGUIFilename),
      m_pPluginObjectDescriptor(pPluginObjectDescriptor)
{
}

CAboutPluginDialog::~CAboutPluginDialog(void)
{
}

boolean CAboutPluginDialog::run(void)
{
	if (m_pPluginObjectDescriptor == NULL)
	{
		m_pPluginObjectDescriptor = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(m_oPluginClassIdentifier);
	}

	if(!m_pPluginObjectDescriptor) return false;

	::GtkBuilder* l_pInterface=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "plugin_about", NULL);
	gtk_builder_add_from_file(l_pInterface, m_sGUIFilename.toASCIIString(), NULL);
	gtk_builder_connect_signals(l_pInterface, NULL);

	::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about"));
	::GtkWidget* l_pType=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_type"));
	::GtkWidget* l_pName=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_name"));
	::GtkWidget* l_pAuthorName=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_author_name"));
	::GtkWidget* l_pAuthorCompanyName=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_company_name"));
	::GtkWidget* l_pCategory=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_category"));
	::GtkWidget* l_pVersion=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_version"));
	::GtkWidget* l_pAddedSoftwareVersion=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_added_software_version"));
	::GtkWidget* l_pUpdatedSoftwareVersion=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-entry_update_software_version"));
	::GtkWidget* l_pShortDescription=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-textview_short_description"));
	::GtkWidget* l_pDetailedDescription=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "plugin_about-textview_detailed_description"));
	g_object_unref(l_pInterface);

	if(m_pPluginObjectDescriptor->isDerivedFromClass(OV_ClassId_Plugins_AlgorithmDesc))
	{
		gtk_entry_set_text(GTK_ENTRY(l_pType), "Algorithm");
	}
	else if(m_pPluginObjectDescriptor->isDerivedFromClass(OV_ClassId_Plugins_BoxAlgorithmDesc))
	{
		gtk_entry_set_text(GTK_ENTRY(l_pType), "Box algorithm");
	}
	else if(m_pPluginObjectDescriptor->isDerivedFromClass(OV_ClassId_Plugins_ScenarioImporterDesc))
	{
		gtk_entry_set_text(GTK_ENTRY(l_pType), "Scenario importer");
	}
	else if(m_pPluginObjectDescriptor->isDerivedFromClass(OV_ClassId_Plugins_ScenarioExporterDesc))
	{
		gtk_entry_set_text(GTK_ENTRY(l_pType), "Scenario exporter");
	}

	::GtkTextBuffer* l_pShortDescriptionBuffer=gtk_text_buffer_new(NULL);
	::GtkTextBuffer* l_pDetailedDescriptionBuffer=gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(l_pShortDescriptionBuffer, m_pPluginObjectDescriptor->getShortDescription(), -1);
	gtk_text_buffer_set_text(l_pDetailedDescriptionBuffer, m_pPluginObjectDescriptor->getDetailedDescription(), -1);
	gtk_entry_set_text(GTK_ENTRY(l_pName), m_pPluginObjectDescriptor->getName());
	gtk_entry_set_text(GTK_ENTRY(l_pAuthorName), m_pPluginObjectDescriptor->getAuthorName());
	gtk_entry_set_text(GTK_ENTRY(l_pAuthorCompanyName), m_pPluginObjectDescriptor->getAuthorCompanyName());
	gtk_entry_set_text(GTK_ENTRY(l_pCategory), m_pPluginObjectDescriptor->getCategory());
	gtk_entry_set_text(GTK_ENTRY(l_pVersion), m_pPluginObjectDescriptor->getVersion());
	gtk_entry_set_text(GTK_ENTRY(l_pAddedSoftwareVersion), m_pPluginObjectDescriptor->getAddedSoftwareVersion());
	gtk_entry_set_text(GTK_ENTRY(l_pUpdatedSoftwareVersion), m_pPluginObjectDescriptor->getUpdatedSoftwareVersion());
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(l_pShortDescription), l_pShortDescriptionBuffer);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(l_pDetailedDescription), l_pDetailedDescriptionBuffer);
	g_object_unref(l_pShortDescriptionBuffer);
	g_object_unref(l_pDetailedDescriptionBuffer);

	gtk_dialog_run(GTK_DIALOG(l_pDialog));
	gtk_widget_destroy(l_pDialog);

	return true;
}
