#include "ovdCConnectorEditor.h"

#include <map>
#include <string>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	void reset_scenario_connector_identifier(GtkWidget*, CConnectorEditor* self)
	{
		const CIdentifier newIdentifier = self->m_rBox.getUnusedInputIdentifier(OV_UndefinedIdentifier);
		if (self->m_ConnectorIdentifierEntry && newIdentifier != OV_UndefinedIdentifier)
		{
			gtk_entry_set_text(self->m_ConnectorIdentifierEntry, newIdentifier.toString().toASCIIString());
		}
	}
}  // namespace

CConnectorEditor::CConnectorEditor(const IKernelContext& rKernelContext, IBox& rBox, const uint32_t connectorType, const uint32_t connectorIndex, const char* sTitle, const char* sGUIFilename)
	: m_kernelContext(rKernelContext), m_rBox(rBox), m_connectorType(connectorType),
	  m_connectorIndex(connectorIndex), m_sGUIFilename(sGUIFilename), m_sTitle(sTitle ? sTitle : "") { }

CConnectorEditor::~CConnectorEditor() = default;

bool CConnectorEditor::run()
{
	//	t_getConnectorIdentifier getConnectorIdentifier = nullptr;
	t_setConnectorName setConnectorName = nullptr;
	t_setConnectorType setConnectorType = nullptr;
	t_isTypeSupported isTypeSupported   = nullptr;
	//	t_updateConnectorIdentifier updateConnectorIdentifier = nullptr;

	BoxInterfacorType interfacorType;
	switch (m_connectorType)
	{
		case Box_Input:
			setConnectorName = &IBox::setInputName;
			setConnectorType = &IBox::setInputType;
			isTypeSupported  = &IBox::hasInputSupport;
			interfacorType   = Input;
			break;

		case Box_Output:
			setConnectorName = &IBox::setOutputName;
			setConnectorType = &IBox::setOutputType;
			isTypeSupported  = &IBox::hasOutputSupport;
			interfacorType   = Output;
			break;

		default:
			return false;
	}

	CString l_oConnectorName;
	CIdentifier l_oConnectorType;
	CIdentifier connectorIdentifier;
	m_rBox.getInterfacorIdentifier(interfacorType, m_connectorIndex, connectorIdentifier);
	m_rBox.getInterfacorName(interfacorType, m_connectorIndex, l_oConnectorName);
	m_rBox.getInterfacorType(interfacorType, m_connectorIndex, l_oConnectorType);

	GtkBuilder* l_pBuilderInterfaceConnector = gtk_builder_new();
	gtk_builder_add_from_file(l_pBuilderInterfaceConnector, m_sGUIFilename.c_str(), nullptr);
	gtk_builder_connect_signals(l_pBuilderInterfaceConnector, nullptr);

	GtkWidget* l_pConnectorDialog             = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceConnector, "connector_editor"));
	GtkEntry* l_pConnectorNameEntry           = GTK_ENTRY(gtk_builder_get_object(l_pBuilderInterfaceConnector, "connector_editor-connector_name_entry"));
	GtkComboBox* l_pConnectorTypeComboBox     = GTK_COMBO_BOX(gtk_builder_get_object(l_pBuilderInterfaceConnector, "connector_editor-connector_type_combobox"));
	m_ConnectorIdentifierEntry                = GTK_ENTRY(gtk_builder_get_object(l_pBuilderInterfaceConnector, "connector_editor-connector_identifier_entry"));
	GtkButton* connectorIdentifierResetButton = GTK_BUTTON(gtk_builder_get_object(l_pBuilderInterfaceConnector, "connector_editor-connector_identifier_reset_button"));
	gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(l_pConnectorTypeComboBox)));
	gtk_window_set_title(GTK_WINDOW(l_pConnectorDialog), m_sTitle.c_str());

	if (m_rBox.getAlgorithmClassIdentifier() == OV_UndefinedIdentifier)
	{
		gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterfaceConnector, "connector_editor-connector_identifier_label")));
		gtk_widget_show(GTK_WIDGET(m_ConnectorIdentifierEntry));
		gtk_widget_show(GTK_WIDGET(connectorIdentifierResetButton));
		g_signal_connect(GTK_WIDGET(connectorIdentifierResetButton), "clicked", G_CALLBACK(reset_scenario_connector_identifier), this);
	}

	//get a list of stream types and display connector type
	map<string, CIdentifier> l_vStreamTypes;
	gint l_iActive = -1;

	for (const auto& l_oCurrentTypeIdentifier : m_kernelContext.getTypeManager().getSortedTypes())
	{
		//First check if the type is support by the connector
		if ((m_rBox.*isTypeSupported)(l_oCurrentTypeIdentifier.first))
		{
			//If the input type is support by the connector, let's add it to the list
			if (m_kernelContext.getTypeManager().isStream(l_oCurrentTypeIdentifier.first))
			{
				gtk_combo_box_append_text(l_pConnectorTypeComboBox, l_oCurrentTypeIdentifier.second.toASCIIString());
				if (l_oCurrentTypeIdentifier.first == l_oConnectorType)
				{
					l_iActive = gint(l_vStreamTypes.size());
					gtk_combo_box_set_active(l_pConnectorTypeComboBox, l_iActive);
				}
				l_vStreamTypes[l_oCurrentTypeIdentifier.second.toASCIIString()] = l_oCurrentTypeIdentifier.first;
			}
		}
	}

	//display connector name
	gtk_entry_set_text(l_pConnectorNameEntry, l_oConnectorName.toASCIIString());
	gtk_entry_set_text(m_ConnectorIdentifierEntry, connectorIdentifier.toString().toASCIIString());

	bool l_bFinished = false;
	bool l_bResult   = false;
	while (!l_bFinished)
	{
		const gint l_iResult = gtk_dialog_run(GTK_DIALOG(l_pConnectorDialog));
		if (l_iResult == GTK_RESPONSE_APPLY)
		{
			char* l_sActiveText = gtk_combo_box_get_active_text(l_pConnectorTypeComboBox);
			if (l_sActiveText)
			{
				const auto newName             = gtk_entry_get_text(l_pConnectorNameEntry);
				auto newType                   = l_vStreamTypes[l_sActiveText];
				const auto newIdentifierString = gtk_entry_get_text(m_ConnectorIdentifierEntry);

				(m_rBox.*setConnectorType)(m_connectorIndex, newType);
				(m_rBox.*setConnectorName)(m_connectorIndex, newName);

				// If the connector identifier is valid then create a new one and swap it with the edited one
				// this is because we can not change the identifier of a setting
				CIdentifier newIdentifier;
				if (newIdentifier.fromString(newIdentifierString) && (newIdentifier != connectorIdentifier))
				{
					m_rBox.updateInterfacorIdentifier(interfacorType, m_connectorIndex, newIdentifier);
				}
				//				(m_rBox.*addConnector)(newName, newType);
				l_bFinished = true;
				l_bResult   = true;
			}
		}
		else if (l_iResult == 2) // revert
		{
			m_rBox.getInterfacorName(interfacorType, m_connectorIndex, l_oConnectorName);
			m_rBox.getInterfacorType(interfacorType, m_connectorIndex, l_oConnectorType);

			gtk_entry_set_text(l_pConnectorNameEntry, l_oConnectorName.toASCIIString());
			gtk_combo_box_set_active(l_pConnectorTypeComboBox, l_iActive);
		}
		else
		{
			l_bFinished = true;
			l_bResult   = false;
		}
	}

	gtk_widget_destroy(l_pConnectorDialog);

	g_object_unref(l_pBuilderInterfaceConnector);

	return l_bResult;
}
