#include "ovdCScriptSettingView.h"
#include "../ovd_base.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_button_setting_filename_browse_pressed(GtkButton* /*pButton*/, gpointer pUserData)
{
	static_cast<CScriptSettingView*>(pUserData)->browse();
}

static void on_button_setting_script_edit_pressed(GtkButton* /*pButton*/, gpointer pUserData)
{
	static_cast<CScriptSettingView*>(pUserData)->edit();
}

static void on_change(GtkEntry* /*entry*/, gpointer pUserData)
{
	static_cast<CScriptSettingView*>(pUserData)->onChange();
}

#if defined TARGET_OS_Windows
static gboolean on_focus_out_event(GtkEntry* /*entry*/, GdkEvent* /*event*/, gpointer pUserData)
{
	static_cast<CScriptSettingView*>(pUserData)->onFocusLost();
	return FALSE;
}
#endif

CScriptSettingView::CScriptSettingView(Kernel::IBox& rBox, uint32_t ui32Index, CString& rBuilderName, const Kernel::IKernelContext& rKernelContext) :
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_script"), m_rKernelContext(rKernelContext), m_bOnValueSetting(false)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_change), this);
#if defined TARGET_OS_Windows
	// Only called for Windows path
	g_signal_connect(G_OBJECT(m_pEntry), "focus_out_event", G_CALLBACK(on_focus_out_event), this);
#endif
	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_script_edit_pressed), this);
	g_signal_connect(G_OBJECT(l_vWidget[2]), "clicked", G_CALLBACK(on_button_setting_filename_browse_pressed), this);

	initializeValue();
}


void CScriptSettingView::getValue(CString& rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CScriptSettingView::setValue(const CString& rValue)
{
	m_bOnValueSetting = true;
	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting = false;
}

void CScriptSettingView::browse()
{
	GtkWidget* l_pWidgetDialogOpen = gtk_file_chooser_dialog_new(
		"Select file to open...",
		nullptr,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		nullptr);

	const CString l_sInitialFileName = m_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_pEntry));
	if (g_path_is_absolute(l_sInitialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sInitialFileName.toASCIIString());
	}
	else
	{
		char* l_sFullPath = g_build_filename(g_get_current_dir(), l_sInitialFileName.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sFullPath);
		g_free(l_sFullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* l_sFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		char* l_pBackslash = nullptr;
		while ((l_pBackslash = strchr(l_sFileName, '\\')) != nullptr)
		{
			*l_pBackslash = '/';
		}
		gtk_entry_set_text(m_pEntry, l_sFileName);
		g_free(l_sFileName);
	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
}

void CScriptSettingView::edit()
{
	const CString l_sFileName = m_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_pEntry));
	const CString l_sEditorCommand = m_rKernelContext.getConfigurationManager().expand("${Designer_ScriptEditorCommand}");

	if (l_sEditorCommand != CString(""))
	{
		CString l_sFullCommand = l_sEditorCommand + CString(" \"") + l_sFileName + CString("\"");
#if defined TARGET_OS_Windows
		l_sFullCommand = "START " + l_sFullCommand;
#elif defined TARGET_OS_Linux
		l_sFullCommand = l_sFullCommand + " &";
#else
#endif
		if (system(l_sFullCommand.toASCIIString()) < 0)
		{
			m_rKernelContext.getLogManager() << Kernel::LogLevel_Warning << "Could not run command " << l_sFullCommand << "\n";
		}
	}
}

void CScriptSettingView::onChange()
{
	if (!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}

#if defined TARGET_OS_Windows
void CScriptSettingView::onFocusLost()
{
	// We replace antislash, interpreted as escape, by slash in Windows path
	if (!m_bOnValueSetting)
	{
		std::string fileName = gtk_entry_get_text(m_pEntry);
		std::string::iterator iter = fileName.begin();

		while ((iter = std::find(iter, fileName.end(), '\\')) != fileName.end())
		{
			if (iter == std::prev(fileName.end()))
			{
				*iter = '/';
				break;
			}
			if (*std::next(iter) != '{' && *std::next(iter) != '$' && *std::next(iter) != '}')
			{
				*iter = '/';
			}

			std::advance(iter, 1);
		}

		gtk_entry_set_text(m_pEntry, fileName.c_str());
		getBox().setSettingValue(this->getSettingIndex(), fileName.c_str());
	}
}
#endif
