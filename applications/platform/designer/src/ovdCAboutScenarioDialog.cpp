#include "ovdCAboutScenarioDialog.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

CAboutScenarioDialog::CAboutScenarioDialog(const IKernelContext& rKernelContext, IScenario& rScenario, const char* sGUIFilename)
	: m_kernelContext(rKernelContext), m_rScenario(rScenario), m_sGUIFilename(sGUIFilename) { }

CAboutScenarioDialog::~CAboutScenarioDialog() = default;

namespace
{
	void buttonMetaboxReset_clicked(GtkWidget* /*widget*/, gpointer data)
	{
		gtk_entry_set_text(GTK_ENTRY(data), CIdentifier::random().toString().toASCIIString());
	}
} // namespace

bool CAboutScenarioDialog::run()

{
	GtkBuilder* interface = gtk_builder_new(); // glade_xml_new(m_sGUIFilename.toASCIIString(), "scenario_about", nullptr);
	gtk_builder_add_from_file(interface, m_sGUIFilename.toASCIIString(), nullptr);
	gtk_builder_connect_signals(interface, nullptr);

	GtkWidget* l_pDialog                 = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about"));
	GtkWidget* l_pName                   = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_name"));
	GtkWidget* l_pAuthorName             = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_author_name"));
	GtkWidget* l_pAuthorCompanyName      = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_company_name"));
	GtkWidget* l_pCategory               = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_category"));
	GtkWidget* l_pVersion                = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_version"));
	GtkWidget* l_pDocumentationPage      = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_documentation_page"));
	GtkWidget* l_pAddedSoftwareVersion   = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_added_software_version"));
	GtkWidget* l_pUpdatedSoftwareVersion = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_update_software_version"));

	GtkWidget* l_pMetaboxId = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-entry_metabox_id"));

	GtkWidget* l_pResetMetaboxId = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-button_reset_metabox_id"));
	const gulong signalHandlerId = g_signal_connect(G_OBJECT(l_pResetMetaboxId), "clicked", G_CALLBACK(buttonMetaboxReset_clicked), l_pMetaboxId);

	GtkWidget* l_pShortDescription    = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-textview_short_description"));
	GtkWidget* l_pDetailedDescription = GTK_WIDGET(gtk_builder_get_object(interface, "scenario_about-textview_detailed_description"));

	g_object_unref(interface);

	gtk_entry_set_text(GTK_ENTRY(l_pName), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_Name).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pAuthorName), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_Author).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pAuthorCompanyName), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_Company).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pCategory), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_Category).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pVersion), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_Version).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pDocumentationPage), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_DocumentationPage).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pAddedSoftwareVersion), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_AddedSoftwareVersion).toASCIIString());
	gtk_entry_set_text(GTK_ENTRY(l_pUpdatedSoftwareVersion), m_rScenario.getAttributeValue(OV_AttributeId_Scenario_UpdatedSoftwareVersion).toASCIIString());

	if (m_rScenario.isMetabox())
	{
		gtk_entry_set_text(GTK_ENTRY(l_pMetaboxId), m_rScenario.getAttributeValue(OVP_AttributeId_Metabox_Identifier).toASCIIString());
	}
	else
	{
		gtk_widget_set_sensitive(l_pMetaboxId, FALSE);
		gtk_widget_set_sensitive(l_pResetMetaboxId, FALSE);
	}

	GtkTextBuffer* l_pShortDescriptionBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(l_pShortDescription));
	gtk_text_buffer_set_text(l_pShortDescriptionBuffer, m_rScenario.getAttributeValue(OV_AttributeId_Scenario_ShortDescription).toASCIIString(), -1);
	GtkTextBuffer* l_pDetailedDescriptionBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(l_pDetailedDescription));
	gtk_text_buffer_set_text(l_pDetailedDescriptionBuffer, m_rScenario.getAttributeValue(OV_AttributeId_Scenario_DetailedDescription).toASCIIString(), -1);

	gtk_dialog_run(GTK_DIALOG(l_pDialog));

	g_signal_handler_disconnect(G_OBJECT(l_pResetMetaboxId), signalHandlerId);
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_Name, gtk_entry_get_text(GTK_ENTRY(l_pName)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_Author, gtk_entry_get_text(GTK_ENTRY(l_pAuthorName)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_Company, gtk_entry_get_text(GTK_ENTRY(l_pAuthorCompanyName)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_Category, gtk_entry_get_text(GTK_ENTRY(l_pCategory)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_Version, gtk_entry_get_text(GTK_ENTRY(l_pVersion)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_DocumentationPage, gtk_entry_get_text(GTK_ENTRY(l_pDocumentationPage)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_AddedSoftwareVersion, gtk_entry_get_text(GTK_ENTRY(l_pAddedSoftwareVersion)));
	m_rScenario.setAttributeValue(OV_AttributeId_Scenario_UpdatedSoftwareVersion, gtk_entry_get_text(GTK_ENTRY(l_pUpdatedSoftwareVersion)));

	if (m_rScenario.isMetabox())
	{
		const CString textId(gtk_entry_get_text(GTK_ENTRY(l_pMetaboxId)));
		CIdentifier cid;
		if (!cid.fromString(textId))
		{
			m_kernelContext.getLogManager() << LogLevel_Error << "Invalid identifier " << textId <<
					" is not in the \"(0x[0-9a-f]{1-8}, 0x[0-9a-f]{1-8})\" format. ";
			m_kernelContext.getLogManager() << "Reverting to " << m_rScenario.getAttributeValue(OVP_AttributeId_Metabox_Identifier).toASCIIString() << ".\n";
		}
		else { m_rScenario.setAttributeValue(OVP_AttributeId_Metabox_Identifier, textId); }
	}

	GtkTextIter l_pTextIterStart;
	GtkTextIter l_pTextIterEnd;

	gtk_text_buffer_get_start_iter(l_pShortDescriptionBuffer, &l_pTextIterStart);
	gtk_text_buffer_get_end_iter(l_pShortDescriptionBuffer, &l_pTextIterEnd);
	m_rScenario.setAttributeValue(
		OV_AttributeId_Scenario_ShortDescription,
		gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(l_pShortDescription)), &l_pTextIterStart, &l_pTextIterEnd, FALSE));

	gtk_text_buffer_get_start_iter(l_pDetailedDescriptionBuffer, &l_pTextIterStart);
	gtk_text_buffer_get_end_iter(l_pDetailedDescriptionBuffer, &l_pTextIterEnd);
	m_rScenario.setAttributeValue(
		OV_AttributeId_Scenario_DetailedDescription,
		gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(l_pDetailedDescription)), &l_pTextIterStart, &l_pTextIterEnd, FALSE));

	gtk_widget_destroy(l_pDialog);

	return true;
}
