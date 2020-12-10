#include "ovdCFilenameSettingView.h"
#include "../ovd_base.h"

#include <cstring>
#include <iterator>

using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace Setting;

static void OnButtonSettingFilenameBrowsePressed(GtkButton* /*button*/, gpointer data) { static_cast<CFilenameSettingView*>(data)->browse(); }

static void OnChange(GtkEntry* /*entry*/, gpointer data) { static_cast<CFilenameSettingView*>(data)->onChange(); }

#if defined TARGET_OS_Windows
static gboolean OnFocusOutEvent(GtkEntry* /*entry*/, GdkEvent* /*event*/, gpointer data)
{
	static_cast<CFilenameSettingView*>(data)->onFocusLost();
	return FALSE;
}
#endif

CFilenameSettingView::CFilenameSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx)
	: CAbstractSettingView(box, index, builderName, "settings_collection-hbox_setting_filename"), m_kernelCtx(ctx)
{
	GtkWidget* settingWidget = CAbstractSettingView::getEntryFieldWidget();

	std::vector<GtkWidget*> widgets;
	CAbstractSettingView::extractWidget(settingWidget, widgets);
	m_entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(OnChange), this);
#if defined TARGET_OS_Windows
	// Only called for Windows path
	g_signal_connect(G_OBJECT(m_entry), "focus_out_event", G_CALLBACK(OnFocusOutEvent), this);
#endif
	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingFilenameBrowsePressed), this);

	CAbstractSettingView::initializeValue();
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

	const CString initialFilename = m_kernelCtx.getConfigurationManager().expand(gtk_entry_get_text(m_entry));
	if (g_path_is_absolute(initialFilename.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFilename.toASCIIString());
	}
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), initialFilename.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		char* backslash;
		while ((backslash = strchr(fileName, '\\')) != nullptr) { *backslash = '/'; }
		gtk_entry_set_text(m_entry, fileName);
		g_free(fileName);
	}
	gtk_widget_destroy(widgetDialogOpen);
}

void CFilenameSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* value = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), value);
	}
}

#if defined TARGET_OS_Windows
void CFilenameSettingView::onFocusLost()
{
	// We replace antislash, interpreted as escape, by slash in Windows path
	if (!m_onValueSetting)
	{
		std::string fileName = gtk_entry_get_text(m_entry);
		auto it              = fileName.begin();

		while ((it = std::find(it, fileName.end(), '\\')) != fileName.end())
		{
			if (it == std::prev(fileName.end()))
			{
				*it = '/';
				break;
			}
			if (*std::next(it) != '{' && *std::next(it) != '$' && *std::next(it) != '}') { *it = '/'; }

			std::advance(it, 1);
		}

		gtk_entry_set_text(m_entry, fileName.c_str());
		getBox().setSettingValue(this->getSettingIndex(), fileName.c_str());
	}
}
#endif
