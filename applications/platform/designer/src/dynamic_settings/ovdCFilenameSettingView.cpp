#include "ovdCFilenameSettingView.h"
#include "../ovd_base.h"

#include <cstring>
#include <iterator>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void OnButtonSettingFilenameBrowsePressed(GtkButton* /*button*/, gpointer data) { static_cast<CFilenameSettingView *>(data)->browse(); }

static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CFilenameSettingView *>(data)->onChange(); }

#if defined TARGET_OS_Windows
static gboolean OnFocusOutEvent(GtkEntry* /*entry*/, GdkEvent* /*event*/, gpointer data)
{
	static_cast<CFilenameSettingView *>(data)->onFocusLost();
	return FALSE;
}
#endif

CFilenameSettingView::CFilenameSettingView(Kernel::IBox& box, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& ctx)
	: CAbstractSettingView(box, index, rBuilderName, "settings_collection-hbox_setting_filename"), m_kernelContext(ctx)
{
	GtkWidget* settingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	extractWidget(settingWidget, widgets);
	m_entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnChange), this);
#if defined TARGET_OS_Windows
	// Only called for Windows path
	g_signal_connect(G_OBJECT(m_entry), "focus_out_event", G_CALLBACK(OnFocusOutEvent), this);
#endif
	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingFilenameBrowsePressed), this);

	initializeValue();
}

void CFilenameSettingView::getValue(CString& value) const { value = CString(gtk_entry_get_text(m_entry)); }

void CFilenameSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CFilenameSettingView::browse() const
{
	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE,
																 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

	const CString initialFileName = m_kernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_entry));
	if (g_path_is_absolute(initialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
	}
	else
	{
		char* l_sFullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), l_sFullPath);
		g_free(l_sFullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* l_sFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		char* l_pBackslash;
		while ((l_pBackslash = strchr(l_sFileName, '\\')) != nullptr) { *l_pBackslash = '/'; }
		gtk_entry_set_text(m_entry, l_sFileName);
		g_free(l_sFileName);
	}
	gtk_widget_destroy(widgetDialogOpen);
}

void CFilenameSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}

#if defined TARGET_OS_Windows
void CFilenameSettingView::onFocusLost()
{
	// We replace antislash, interpreted as escape, by slash in Windows path
	if (!m_onValueSetting)
	{
		std::string fileName       = gtk_entry_get_text(m_entry);
		auto iter = fileName.begin();

		while ((iter = std::find(iter, fileName.end(), '\\')) != fileName.end())
		{
			if (iter == std::prev(fileName.end()))
			{
				*iter = '/';
				break;
			}
			if (*std::next(iter) != '{' && *std::next(iter) != '$' && *std::next(iter) != '}') { *iter = '/'; }

			std::advance(iter, 1);
		}

		gtk_entry_set_text(m_entry, fileName.c_str());
		getBox().setSettingValue(this->getSettingIndex(), fileName.c_str());
	}
}
#endif
