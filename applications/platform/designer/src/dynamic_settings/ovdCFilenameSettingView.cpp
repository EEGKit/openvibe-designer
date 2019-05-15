#include "ovdCFilenameSettingView.h"
#include "../ovd_base.h"

#include <iostream>
#include <cstring>
#include <iterator>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_button_setting_filename_browse_pressed(GtkButton* /*button*/, gpointer data)
{
	static_cast<CFilenameSettingView*>(data)->browse();
}

static void on_change(GtkEntry* /*entry*/, gpointer data)
{
	static_cast<CFilenameSettingView*>(data)->onChange();
}

#if defined TARGET_OS_Windows
static gboolean on_focus_out_event(GtkEntry* /*entry*/, GdkEvent* /*event*/, gpointer data)
{
	static_cast<CFilenameSettingView*>(data)->onFocusLost();
	return FALSE;
}
#endif

CFilenameSettingView::CFilenameSettingView(Kernel::IBox& rBox, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& rKernelContext) :
	CAbstractSettingView(rBox, index, rBuilderName, "settings_collection-hbox_setting_filename"), m_rKernelContext(rKernelContext)
{
	GtkWidget* l_pSettingWidget = this->CAbstractSettingView::getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	CAbstractSettingView::extractWidget(l_pSettingWidget, l_vWidget);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_change), this);
#if defined TARGET_OS_Windows
	// Only called for Windows path
	g_signal_connect(G_OBJECT(m_pEntry), "focus_out_event", G_CALLBACK(on_focus_out_event), this);
#endif
	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_filename_browse_pressed), this);

	CAbstractSettingView::initializeValue();
}

void CFilenameSettingView::getValue(CString& rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}

void CFilenameSettingView::setValue(const CString& rValue)
{
	m_bOnValueSetting = true;
	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting = false;
}

void CFilenameSettingView::browse() const
{
	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

	const CString initialFileName = m_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_pEntry));
	if (g_path_is_absolute(initialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
	}
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		char* l_pBackslash = nullptr;
		while ((l_pBackslash = strchr(fileName, '\\')) != nullptr)
		{
			*l_pBackslash = '/';
		}
		gtk_entry_set_text(m_pEntry, fileName);
		g_free(fileName);
	}
	gtk_widget_destroy(widgetDialogOpen);
}

void CFilenameSettingView::onChange()
{
	if (!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}

#if defined TARGET_OS_Windows
void CFilenameSettingView::onFocusLost()
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
