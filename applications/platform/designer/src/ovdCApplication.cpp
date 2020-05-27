#include "ovd_base.h"

#include <boost/filesystem.hpp>

#include <system/ovCTime.h>
#include <system/ovCMemory.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <numeric>

#if defined TARGET_OS_Windows
#include "system/WindowsUtilities.h"
#include "Windows.h"
#endif

#include <communication/ovCMessagingProtocol.h>
#include <visualization-toolkit/ovviz_defines.h>
#include <visualization-toolkit/ovvizIVisualizationContext.h>
#include <ovp_global_defines.h>
#include <fs/Files.h>
#include <json/json.h>

#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
#include <strings.h>
#define _strcmpi strcasecmp
#endif

#define OVD_GUI_File						OpenViBE::Directories::getDataDir() + "/applications/designer/interface.ui"
#define OVD_GUI_AboutDialog_File			OpenViBE::Directories::getDataDir() + "/applications/designer/about-dialog.ui"
#define OVD_GUI_Settings_File				OpenViBE::Directories::getDataDir() + "/applications/designer/interface-settings.ui"
#define OVD_AttributeId_ScenarioFilename	OpenViBE::CIdentifier(0x4C536D0A, 0xB23DC545)
#define OVD_README_File						OpenViBE::Directories::getDistRootDir() + "/ReadMe.txt"


#define OVD_SCENARIOS_PATH					"${Path_Data}/scenarios"
#define OVD_WORKING_SCENARIOS_PATH			"${Designer_DefaultWorkingDirectory}/scenarios"

static const size_t s_RecentFileNumber = 10;

#include "ovdCDesignerVisualization.h"
#include "ovdCPlayerVisualization.h"
#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"
#include "ovdAssert.h"
#include "ovdCLogListenerDesigner.h"

#include "visualization/ovdCVisualizationManager.h"

#define OV_ClassId_Selected OpenViBE::CIdentifier(0xC67A01DC, 0x28CE06C1)

// because std::tolower has multiple signatures, it can not be easily used in std::transform this workaround is taken from http://www.gcek.net/ref/books/sw/cpp/ticppv2/
template <class TChar>
static TChar to_lower(TChar c) { return tolower(c); }

namespace OpenViBE {
namespace Designer {

namespace {

struct SBoxProto final : Kernel::IBoxProto
{
	explicit SBoxProto(Kernel::ITypeManager& typeManager) : typeManager(typeManager) { }

	bool addInput(const CString& /*name*/, const CIdentifier& typeID, const CIdentifier& id, const bool /*notify*/) override
	{
		uint64_t v = typeID.toUInteger();
		swap_byte(v, nInputHash);
		swap_byte(nInputHash, 0x7936A0F3BD12D936LL);
		hash = hash.toUInteger() ^ v;
		if (id != OV_UndefinedIdentifier)
		{
			v = id.toUInteger();
			swap_byte(v, 0x2BD1D158F340014D);
			hash = hash.toUInteger() ^ v;
		}
		return true;
	}
	//
	bool addOutput(const CString& /*name*/, const CIdentifier& typeID, const CIdentifier& id, const bool /*notify*/) override
	{
		uint64_t v = typeID.toUInteger();
		swap_byte(v, nOutputHash);
		swap_byte(nOutputHash, 0xCBB66A5B893AA4E9LL);
		hash = hash.toUInteger() ^ v;
		if (id != OV_UndefinedIdentifier)
		{
			v = id.toUInteger();
			swap_byte(v, 0x87CA0F5EFC4FAC68);
			hash = hash.toUInteger() ^ v;
		}
		return true;
	}

	bool addSetting(const CString& /*name*/, const CIdentifier& typeID, const CString& /*defaultValue*/, const bool /*modifiable*/, const CIdentifier& id,
					const bool /*notify*/) override
	{
		uint64_t v = typeID.toUInteger();
		swap_byte(v, nSettingHash);
		swap_byte(nSettingHash, 0x3C87F3AAE9F8303BLL);
		hash = hash.toUInteger() ^ v;
		if (id != OV_UndefinedIdentifier)
		{
			v = id.toUInteger();
			swap_byte(v, 0x17185F7CDA63A9FA);
			hash = hash.toUInteger() ^ v;
		}
		return true;
	}

	bool addInputSupport(const CIdentifier& /*typeID*/) override { return true; }

	bool addOutputSupport(const CIdentifier& /*typeID*/) override { return true; }

	bool addFlag(const Kernel::EBoxFlag flag) override
	{
		switch (flag)
		{
			case Kernel::BoxFlag_CanAddInput: hash = hash.toUInteger() ^ CIdentifier(0x07507AC8, 0xEB643ACE).toUInteger();
				break;
			case Kernel::BoxFlag_CanModifyInput: hash = hash.toUInteger() ^ CIdentifier(0x5C985376, 0x8D74CDB8).toUInteger();
				break;
			case Kernel::BoxFlag_CanAddOutput: hash = hash.toUInteger() ^ CIdentifier(0x58DEA69B, 0x12411365).toUInteger();
				break;
			case Kernel::BoxFlag_CanModifyOutput: hash = hash.toUInteger() ^ CIdentifier(0x6E162C01, 0xAC979F22).toUInteger();
				break;
			case Kernel::BoxFlag_CanAddSetting: hash = hash.toUInteger() ^ CIdentifier(0xFA7A50DC, 0x2140C013).toUInteger();
				break;
			case Kernel::BoxFlag_CanModifySetting: hash = hash.toUInteger() ^ CIdentifier(0x624D7661, 0xD8DDEA0A).toUInteger();
				break;
			case Kernel::BoxFlag_IsDeprecated: isDeprecated = true;
				break;
			default: return false;
		}
		return true;
	}

	bool addFlag(const CIdentifier& flag) override
	{
		const uint64_t value = typeManager.getEnumerationEntryValueFromName(OV_TypeId_BoxAlgorithmFlag, flag.toString());
		return value != OV_UndefinedIdentifier;
	}

	void swap_byte(uint64_t& v, const uint64_t s)
	{
		uint8_t V[sizeof(v)];
		uint8_t S[sizeof(s)];
		System::Memory::hostToLittleEndian(v, V);
		System::Memory::hostToLittleEndian(s, S);
		for (size_t i = 0; i < sizeof(s); i += 2)
		{
			const size_t j  = S[i] % sizeof(v);
			const size_t k  = S[i + 1] % sizeof(v);
			const uint8_t t = V[j];
			V[j]            = V[k];
			V[k]            = t;
		}
		System::Memory::littleEndianToHost(V, &v);
	}

	_IsDerivedFromClass_Final_(IBoxProto, OV_UndefinedIdentifier)

	CIdentifier hash;
	bool isDeprecated     = false;
	uint64_t nInputHash   = 0x64AC3CB54A35888CLL;
	uint64_t nOutputHash  = 0x21E0FAAFE5CAF1E1LL;
	uint64_t nSettingHash = 0x6BDFB15B54B09F63LL;
	Kernel::ITypeManager& typeManager;
};

extern "C" G_MODULE_EXPORT void open_url_mensia_cb(GtkWidget* /*widget*/, gpointer /*data*/)
{
#if defined(TARGET_OS_Windows) && defined(MENSIA_DISTRIBUTION)
		system("start http://mensiatech.com");
#endif
}
}  // namespace

static guint idle_add_cb(GSourceFunc callback, gpointer data, gint /*priority*/  = G_PRIORITY_DEFAULT_IDLE)
{
	GSource* src = g_idle_source_new();
	g_source_set_priority(src, G_PRIORITY_LOW);
	g_source_set_callback(src, callback, data, nullptr);
	return g_source_attach(src, nullptr);
}

static guint timeout_add_cb(const guint interval, GSourceFunc callback, gpointer data, gint /*iPriority*/  = G_PRIORITY_DEFAULT)
{
	GSource* src = g_timeout_source_new(interval);
	g_source_set_priority(src, G_PRIORITY_LOW);
	g_source_set_callback(src, callback, data, nullptr);
	return g_source_attach(src, nullptr);
}

static void drag_data_get_cb(GtkWidget* widget, GdkDragContext* dc, GtkSelectionData* selectionData, const guint info, const guint time, const gpointer data)
{
	static_cast<CApplication*>(data)->dragDataGetCB(widget, dc, selectionData, info, time);
}

static void menu_undo_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->undoCB(); }

static void menu_redo_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->redoCB(); }

static void menu_focus_search_cb(GtkMenuItem* /*item*/, gpointer data)
{
	gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-box_algorithm_searchbox")));
}

static void menu_copy_selection_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->copySelectionCB(); }

static void menu_cut_selection_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->cutSelectionCB(); }

static void menu_paste_selection_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->pasteSelectionCB(); }

static void menu_delete_selection_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->deleteSelectionCB(); }

static void menu_preferences_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->preferencesCB(); }

static void menu_new_scenario_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->newScenarioCB(); }

static void menu_open_scenario_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->openScenarioCB(); }

static void menu_open_recent_scenario_cb(GtkMenuItem* item, gpointer data)
{
	const gchar* fileName = gtk_menu_item_get_label(item);
	static_cast<CApplication*>(data)->openScenario(fileName);
}

static void menu_save_scenario_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioCB(); }

static void menu_save_scenario_as_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioAsCB(); }

static void menu_restore_default_scenarios_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->restoreDefaultScenariosCB(); }

static void menu_close_scenario_cb(GtkMenuItem* /*item*/, gpointer data)
{
	static_cast<CApplication*>(data)->closeScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
}

static void menu_quit_application_cb(GtkMenuItem* /*item*/, gpointer data) { if (static_cast<CApplication*>(data)->quitApplicationCB()) { gtk_main_quit(); } }

static void menu_about_scenario_cb(GtkMenuItem* /*item*/, gpointer data)
{
	static_cast<CApplication*>(data)->aboutScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
}

static void menu_about_openvibe_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->aboutOpenViBECB(); }

#if defined(TARGET_OS_Windows)
static void menu_about_link_clicked_cb(GtkAboutDialog* /*dialog*/, const gchar* linkPtr, gpointer data)
{
	static_cast<CApplication*>(data)->aboutLinkClickedCB(linkPtr);
}
#endif

static void menu_browse_documentation_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->browseDocumentationCB(); }

#ifdef MENSIA_DISTRIBUTION
static void menu_register_license_cb(::GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->registerLicenseCB(); }
#endif

static void menu_report_issue_cb(GtkMenuItem* /*item*/, gpointer data) { static_cast<CApplication*>(data)->reportIssueCB(); }

static void menu_display_changelog_cb(GtkMenuItem* /*item*/, gpointer data)
{
	if (!static_cast<CApplication*>(data)->displayChangelogWhenAvailable())
	{
		const std::string version = static_cast<CApplication*>(data)->m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString();
		GtkWidget* dialog         = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
														   "No boxes were added or updated in version %s of " DESIGNER_NAME ".",
														   version != "${Application_Version}" ? version.c_str() : ProjectVersion);
		gtk_window_set_title(GTK_WINDOW(dialog), "No new boxes");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

static void button_new_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->newScenarioCB(); }

static void button_open_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->openScenarioCB(); }

static void button_save_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioCB(); }

static void button_save_scenario_as_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioAsCB(); }

static void button_close_scenario_cb(GtkButton* /*button*/, gpointer data)
{
	static_cast<CApplication*>(data)->closeScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
}

static void button_undo_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->undoCB(); }

static void button_redo_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->redoCB(); }

#ifdef MENSIA_DISTRIBUTION
static void button_toggle_neurort_engine_configuration_cb(::GtkMenuItem* item, gpointer data)
{
	static_cast<CApplication*>(data)->m_ArchwayHandlerGUI->toggleNeuroRTEngineConfigurationDialog(
		bool(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(item)) == TRUE));
}
#endif

static void delete_designer_visualisation_cb(gpointer data) { static_cast<CApplication*>(data)->deleteDesignerVisualizationCB(); }

static void button_toggle_window_manager_cb(GtkToggleToolButton* /*button*/, gpointer data)
{
	static_cast<CApplication*>(data)->toggleDesignerVisualizationCB();
}

static void button_comment_cb(GtkButton* /*button*/, CApplication* app) { app->addCommentCB(app->getCurrentInterfacedScenario()); }

static void button_about_scenario_cb(GtkButton* /*button*/, gpointer data)
{
	static_cast<CApplication*>(data)->aboutScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
}

static void stop_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->stopScenarioCB(); }

static void play_pause_scenario_cb(GtkButton* button, gpointer data)
{
	if (std::string(gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(button))) == GTK_STOCK_MEDIA_PLAY) { static_cast<CApplication*>(data)->playScenarioCB(); }
	else { static_cast<CApplication*>(data)->pauseScenarioCB(); }
}

static void next_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->nextScenarioCB(); }

static void forward_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->forwardScenarioCB(); }

static void button_configure_current_scenario_settings_cb(GtkButton* /*button*/, CApplication* app)
{
	app->configureScenarioSettingsCB(app->getCurrentInterfacedScenario());
}

static gboolean button_quit_application_cb(GtkWidget* /*widget*/, GdkEvent* /*pEvent*/, gpointer data)
{
	if (static_cast<CApplication*>(data)->quitApplicationCB())
	{
		gtk_main_quit();
		return FALSE;
	}
	return TRUE;
}

static gboolean window_state_changed_cb(GtkWidget* /*widget*/, GdkEventWindowState* event, gpointer data)
{
	if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
	{
		// window has changed from maximized to not maximized or the other way around
		static_cast<CApplication*>(data)->windowStateChangedCB((event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0);
	}
	return TRUE;
}

static void log_level_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->logLevelCB(); }

static void cpu_usage_cb(GtkToggleButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->cpuUsageCB(); }

static gboolean change_current_scenario_cb(GtkNotebook* /*notebook*/, GtkNotebookPage* /*notebookPage*/, const guint pageNumber, gpointer data)
{
	static_cast<CApplication*>(data)->changeCurrentScenario(int(pageNumber));
	return TRUE;
}

static gboolean reorder_scenario_cb(GtkNotebook* /*notebook*/, GtkNotebookPage* /*notebookPage*/, const guint pageNumber, gpointer data)
{
	static_cast<CApplication*>(data)->reorderCurrentScenario(int(pageNumber));
	return TRUE;
}

static void box_algorithm_title_button_expand_cb(GtkButton* /*button*/, gpointer data)
{
	gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-box_algorithm_tree")));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-resource_notebook")), 0);
}

static void box_algorithm_title_button_collapse_cb(GtkButton* /*button*/, gpointer data)
{
	gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-box_algorithm_tree")));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-resource_notebook")), 0);
}

static void algorithm_title_button_expand_cb(GtkButton* /*button*/, gpointer data)
{
	gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-algorithm_tree")));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-resource_notebook")), 1);
}

static void algorithm_title_button_collapse_cb(GtkButton* /*button*/, gpointer data)
{
	gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-algorithm_tree")));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_Builder, "openvibe-resource_notebook")), 1);
}

static void clear_messages_cb(GtkButton* /*button*/, gpointer data) { static_cast<CLogListenerDesigner*>(data)->clearMessages(); }

static void add_scenario_input_cb(GtkButton* /*button*/, CApplication* app) { app->getCurrentInterfacedScenario()->addScenarioInputCB(); }

static void add_scenario_output_cb(GtkButton* /*button*/, CApplication* app) { app->getCurrentInterfacedScenario()->addScenarioOutputCB(); }

static void add_scenario_setting_cb(GtkButton* /*button*/, CApplication* app) { app->getCurrentInterfacedScenario()->addScenarioSettingCB(); }

static std::string strtoupper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), std::ptr_fun<int, int>(std::toupper));
	return str;
}

static gboolean box_algorithm_search_func(GtkTreeModel* model, GtkTreeIter* iter, gpointer data)
{
	auto* app = static_cast<CApplication*>(data);
	/* Visible if row is non-empty and first column is "HI" */


	gboolean visible = false;
	gchar* name;
	gchar* desc;
	gboolean unstable;

	gtk_tree_model_get(model, iter, Resource_StringName, &name, Resource_StringShortDescription, &desc, Resource_BooleanIsUnstable, &unstable, -1);

	// consider only leaf nodes which match the search term
	if (name != nullptr && desc != nullptr)
	{
		if (!unstable && (std::string::npos != strtoupper(name).find(strtoupper(app->m_SearchTerm))
						  || std::string::npos != strtoupper(desc).find(strtoupper(app->m_SearchTerm)) || gtk_tree_model_iter_has_child(model, iter)))
		{
			//std::cout << "value : " << app->m_searchTerm << "\n";
			visible = true;
		}

		g_free(name);
		g_free(desc);
	}
	else { visible = true; }

	return visible;
}

static gboolean box_algorithm_prune_empty_folders(GtkTreeModel* model, GtkTreeIter* iter, gpointer /*data*/)
{
	gboolean isPlugin;
	gtk_tree_model_get(model, iter, Resource_BooleanIsPlugin, &isPlugin, -1);

	if (gtk_tree_model_iter_has_child(model, iter) || isPlugin) { return true; }

	return false;
}

static gboolean do_refilter(CApplication* app)
{
	//if (0 == strcmp(app->m_searchTerm, "")) { gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(app->m_BoxAlgorithmTreeModel)); } // reattach the old model
	//else {
	app->m_BoxAlgorithmTreeModelFilter  = gtk_tree_model_filter_new(GTK_TREE_MODEL(app->m_BoxAlgorithmTreeModel), nullptr);
	app->m_BoxAlgorithmTreeModelFilter2 = gtk_tree_model_filter_new(GTK_TREE_MODEL(app->m_BoxAlgorithmTreeModelFilter), nullptr);
	app->m_BoxAlgorithmTreeModelFilter3 = gtk_tree_model_filter_new(GTK_TREE_MODEL(app->m_BoxAlgorithmTreeModelFilter2), nullptr);
	app->m_BoxAlgorithmTreeModelFilter4 = gtk_tree_model_filter_new(GTK_TREE_MODEL(app->m_BoxAlgorithmTreeModelFilter3), nullptr);
	// detach the normal model from the treeview
	gtk_tree_view_set_model(app->m_BoxAlgorithmTreeView, nullptr);

	// clear the model

	// add a filtering function to the model
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(app->m_BoxAlgorithmTreeModelFilter), box_algorithm_search_func, app, nullptr);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(app->m_BoxAlgorithmTreeModelFilter2), box_algorithm_prune_empty_folders, app, nullptr);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(app->m_BoxAlgorithmTreeModelFilter3), box_algorithm_prune_empty_folders, app, nullptr);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(app->m_BoxAlgorithmTreeModelFilter4), box_algorithm_prune_empty_folders, app, nullptr);

	// attach the model to the treeview
	gtk_tree_view_set_model(app->m_BoxAlgorithmTreeView, GTK_TREE_MODEL(app->m_BoxAlgorithmTreeModelFilter4));

	if (0 == strcmp(app->m_SearchTerm, "")) { gtk_tree_view_collapse_all(app->m_BoxAlgorithmTreeView); }
	else { gtk_tree_view_expand_all(app->m_BoxAlgorithmTreeView); }
	//}

	app->m_FilterTimeout = 0;

	return false;
}

static void queue_refilter(CApplication* app)
{
	if (app->m_FilterTimeout) { g_source_remove(app->m_FilterTimeout); }
	app->m_FilterTimeout = g_timeout_add(300, GSourceFunc(do_refilter), app);
}

static void refresh_search_cb(GtkEntry* textfield, CApplication* app)
{
	app->m_SearchTerm = gtk_entry_get_text(textfield);
	queue_refilter(app);
}

static void refresh_search_no_data_cb(GtkToggleButton* /*button*/, CApplication* app)
{
	app->m_SearchTerm = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(app->m_Builder, "openvibe-box_algorithm_searchbox")));
	queue_refilter(app);
}

static gboolean searchbox_select_all_cb(GtkWidget* widget, GdkEvent* /*event*/, CApplication* /*app*/)
{
	// we select the current search
	gtk_widget_grab_focus(widget); // we must grab or selection wont work. It also triggers the other CBs.
	gtk_editable_select_region(GTK_EDITABLE(widget), 0, -1);
	return false;
}

static gboolean searchbox_focus_in_cb(GtkWidget* /*widget*/, GdkEvent* /*event*/, CApplication* app)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(app->m_Builder, "openvibe-menu_edit")), false);
	return false;
}

static gboolean searchbox_focus_out_cb(GtkWidget* widget, GdkEvent* /*event*/, CApplication* app)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(app->m_Builder, "openvibe-menu_edit")), true);
	gtk_editable_select_region(GTK_EDITABLE(widget), 0, 0);
	return false;
}

#if defined TARGET_OS_Windows
static void about_newversion_button_display_changelog_cb(GtkButton* /*button*/, gpointer /*data*/)
{
	System::WindowsUtilities::utf16CompliantShellExecute(nullptr, "open", (OVD_README_File).toASCIIString(), nullptr, nullptr, SHOW_OPENWINDOW);
}
#endif

static gboolean idle_application_loop(gpointer data)
{
	auto* app = static_cast<CApplication*>(data);
#ifdef MENSIA_DISTRIBUTION
	if (app->m_ArchwayHandler->isEngineStarted()) { app->m_ArchwayHandler->loopEngine(); }
#endif

	CInterfacedScenario* scenario = app->getCurrentInterfacedScenario();
	if (scenario)
	{
		if (app->getPlayer() && scenario->m_PlayerStatus != app->getPlayer()->getStatus())
		{
			switch (app->getPlayer()->getStatus())
			{
				case Kernel::EPlayerStatus::Stop:
					gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(app->m_Builder, "openvibe-button_stop")), "clicked");
					break;
				case Kernel::EPlayerStatus::Pause:
					while (scenario->m_PlayerStatus != Kernel::EPlayerStatus::Pause)
					{
						gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(app->m_Builder, "openvibe-button_play_pause")), "clicked");
					}
					break;
				case Kernel::EPlayerStatus::Step: break;
				case Kernel::EPlayerStatus::Play:
					while (scenario->m_PlayerStatus != Kernel::EPlayerStatus::Play)
					{
						gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(app->m_Builder, "openvibe-button_play_pause")), "clicked");
					}
					break;
				case Kernel::EPlayerStatus::Forward:
					gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(app->m_Builder, "openvibe-button_forward")), "clicked");
					break;
				default:
					std::cout << "unhandled player status : " << toString(app->getPlayer()->getStatus()) << " :(\n";
					break;
			}
		}
		else
		{
			const double time = (scenario->m_Player ? CTime(scenario->m_Player->getCurrentSimulatedTime()).toSeconds() : 0.0);
			if (app->m_LastTimeRefresh != time)
			{
				app->m_LastTimeRefresh = uint64_t(time);

				const size_t milli   = (size_t(time * 1000) % 1000);
				const size_t seconds = (size_t(time)) % 60;
				const size_t minutes = (size_t(time) / 60) % 60;
				const size_t hours   = (size_t(time) / 60) / 60;

				const double cpuUsage = (scenario->m_Player ? scenario->m_Player->getCPUUsage() : 0);

				std::stringstream ss("Time : ");
				ss.fill('0');
				if (hours)
				{
					ss << std::setw(2) << hours << "h " << std::setw(2) << minutes << "m " << std::setw(2) << seconds << "s " << std::setw(3) << milli << "ms";
				}
				else if (minutes) { ss << std::setw(2) << minutes << "m " << std::setw(2) << seconds << "s " << std::setw(3) << milli << "ms"; }
				else if (seconds) { ss << std::setw(2) << seconds << "s " << std::setw(3) << milli << "ms"; }
				else { ss << std::setw(3) << milli << "ms"; }


				gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(app->m_Builder, "openvibe-label_current_time")), ss.str().c_str());

				ss.str("");
				ss.fill('0');
				ss.precision(1);
				ss << std::fixed << cpuUsage << "%";

				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gtk_builder_get_object(app->m_Builder, "openvibe-progressbar_cpu_usage")), cpuUsage * .01);
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gtk_builder_get_object(app->m_Builder, "openvibe-progressbar_cpu_usage")), ss.str().c_str());
				if (scenario->m_Player && scenario->m_DebugCPUUsage)
				{
					// redraws scenario
					scenario->redraw();
				}
			}
		}
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(app->m_Builder, "openvibe-label_current_time")), "Time : 000ms");
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gtk_builder_get_object(app->m_Builder, "openvibe-progressbar_cpu_usage")), "");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gtk_builder_get_object(app->m_Builder, "openvibe-progressbar_cpu_usage")), 0);
	}

	if (!app->hasRunningScenario()) { System::Time::sleep(50); }

	return TRUE;
}

static gboolean idle_scenario_loop(gpointer data)
{
	auto* scenario      = static_cast<CInterfacedScenario*>(data);
	const uint64_t time = System::Time::zgetTime();
	if (scenario->m_LastLoopTime == uint64_t(-1)) { scenario->m_LastLoopTime = time; }
	scenario->m_Player->setFastForwardMaximumFactor(gtk_spin_button_get_value(scenario->m_Application.m_FastForwardFactor));
	if (!scenario->m_Player->loop(time - scenario->m_LastLoopTime)) { scenario->m_Application.stopInterfacedScenarioAndReleasePlayer(scenario); }
	scenario->m_LastLoopTime = time;
	return TRUE;
}

static gboolean timeout_application_loop(gpointer data)
{
	auto* app = static_cast<CApplication*>(data);
	if (!app->hasRunningScenario() && app->m_CmdLineFlags & CommandLineFlag_NoGui)
	{
		app->quitApplicationCB();
		gtk_main_quit();
		return FALSE;
	}
	return TRUE;
}

#ifdef NDEBUG
/**
* Function called in gtk loop: to check each 0.1second if a message was sent by a second instance of Designer
* (Meaning that someone tried to reopen Designer and this instance has to do something)
**/
static gboolean receiveSecondInstanceMessage(gpointer data)
{
	try
	{	// Open or create ensures that
		boost::interprocess::named_mutex mutex(boost::interprocess::open_or_create, MUTEX_NAME);
		{
			boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex);
			CApplication* app = static_cast<CApplication*>(data);
			//Tries to open a message, if fails, go to catch
			boost::interprocess::message_queue message(boost::interprocess::open_only, MESSAGE_NAME);
			app->m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- A message was detected \n";

			// Whatever contains the message the first instance should try to take the focus
			gtk_window_present(GTK_WINDOW(app->m_MainWindow));
			size_t size;
			uint32_t priority = 0;
			char buffer[2048];
			if (message.try_receive(&buffer, sizeof(buffer), size, priority))
			{
				boost::interprocess::message_queue::remove(MESSAGE_NAME);

				int mode = 0;
				char path[2048];
				char* msg = strtok(buffer, ";");
				while (msg != nullptr)
				{
					sscanf(msg, "%1d : <%2048[^>]> ", &mode, &path);
					switch (mode)
					{
						case CommandLineFlag_Open:
							app->openScenario(path);
							break;
						case CommandLineFlag_Play:
							if (app->openScenario(path)) { app->playScenarioCB(); }
							break;
						case CommandLineFlag_PlayFast:
							if (app->openScenario(path)) { app->forwardScenarioCB(); }
							break;
						default: break;
					}
					mode = 0;
					msg  = strtok(nullptr, ";");
				}
			}
			else { boost::interprocess::message_queue::remove(MESSAGE_NAME); }
		}
	}
		// An interprocess_exception is throwed if no messages could be found, in this case we do nothing
	catch (boost::interprocess::interprocess_exception) {}
	return TRUE;
}
#endif

static void zoom_in_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->zoomInCB(); }
static void zoom_out_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->zoomOutCB(); }

static void spinner_zoom_changed_cb(GtkSpinButton* button, gpointer data)
{
	static_cast<CApplication*>(data)->spinnerZoomChangedCB(size_t(gtk_spin_button_get_value(button)));
}

static const GtkTargetEntry TARGET_ENTRY[] = { { static_cast<gchar*>("STRING"), 0, 0 }, { static_cast<gchar*>("text/plain"), 0, 0 } };


CApplication::CApplication(const Kernel::IKernelContext& ctx) : m_kernelCtx(ctx)
{
	m_PluginMgr   = &m_kernelCtx.getPluginManager();
	m_ScenarioMgr = &m_kernelCtx.getScenarioManager();
	m_ScenarioMgr->registerScenarioImporter(OVD_ScenarioImportContext_OpenScenario, ".xml", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	m_ScenarioMgr->registerScenarioImporter(OVD_ScenarioImportContext_OpenScenario, ".mxs", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	m_ScenarioMgr->registerScenarioImporter(OVD_ScenarioImportContext_OpenScenario, ".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	m_ScenarioMgr->registerScenarioExporter(OVD_ScenarioExportContext_SaveScenario, ".xml", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
	m_ScenarioMgr->registerScenarioExporter(OVD_ScenarioExportContext_SaveScenario, ".mxs", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
	m_ScenarioMgr->registerScenarioExporter(OVD_ScenarioExportContext_SaveMetabox, ".xml", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
	m_ScenarioMgr->registerScenarioExporter(OVD_ScenarioExportContext_SaveMetabox, ".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);

	m_VisualizationMgr = new CVisualizationManager(m_kernelCtx);
	m_visualizationCtx = dynamic_cast<VisualizationToolkit::IVisualizationContext*>(
		m_kernelCtx.getPluginManager().createPluginObject(OVP_ClassId_Plugin_VisualizationCtx));
	m_visualizationCtx->setManager(m_VisualizationMgr);
	m_logListener = nullptr;

	m_kernelCtx.getConfigurationManager().createConfigurationToken("Player_ScenarioDirectory", "");
	m_kernelCtx.getConfigurationManager().createConfigurationToken("__volatile_ScenarioDir", "");

#ifdef MENSIA_DISTRIBUTION
	m_ArchwayHandler    = new OpenViBE::CArchwayHandler(ctx);
	m_ArchwayHandlerGUI = new OpenViBE::CArchwayHandlerGUI(*m_ArchwayHandler, this);
#endif
}

CApplication::~CApplication()
{
	if (m_Builder)
	{
		m_kernelCtx.getLogManager().removeListener(m_logListener);
		// @FIXME this likely still does not deallocate the actual widgets allocated by add_from_file
		g_object_unref(G_OBJECT(m_Builder));
		m_Builder = nullptr;
	}

	m_kernelCtx.getPluginManager().releasePluginObject(m_visualizationCtx);

#ifdef MENSIA_DISTRIBUTION
	delete m_ArchwayHandlerGUI;
	delete m_ArchwayHandler;
#endif
}

void CApplication::initialize(const ECommandLineFlag cmdLineFlags)
{
	m_CmdLineFlags = cmdLineFlags;
	m_SearchTerm   = "";

	// Load metaboxes from metabox path
	m_kernelCtx.getMetaboxManager().addMetaboxesFromFiles(m_kernelCtx.getConfigurationManager().expand("${Kernel_Metabox}"));

	// Copy recursively default scenario directory to the default working directory if not exists
	const CString defaultWorkingDirectory   = m_kernelCtx.getConfigurationManager().expand(OVD_WORKING_SCENARIOS_PATH);
	const CString defaultScenariosDirectory = m_kernelCtx.getConfigurationManager().expand(OVD_SCENARIOS_PATH);
	if (!FS::Files::directoryExists(defaultWorkingDirectory) && FS::Files::directoryExists(defaultScenariosDirectory))
	{
		if (!FS::Files::copyDirectory(defaultScenariosDirectory, defaultWorkingDirectory))
		{
			m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "Could not create " << defaultWorkingDirectory << " folder\n";
		}
	}


	// Prepares scenario clipboard
	CIdentifier clipboardScenarioID;
	if (m_ScenarioMgr->createScenario(clipboardScenarioID)) { m_ClipboardScenario = &m_ScenarioMgr->getScenario(clipboardScenarioID); }

	m_Builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "openvibe", nullptr);
	gtk_builder_add_from_file(m_Builder, OVD_GUI_File, nullptr);
	gtk_builder_connect_signals(m_Builder, nullptr);

	std::string version = m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString();
	if (version == "${Application_Version}")
	{
		version = m_kernelCtx.getConfigurationManager().expand("${ProjectVersion_Major}.${ProjectVersion_Minor}.${ProjectVersion_Patch}").toASCIIString();
	}
	const std::string defaultWindowTitle = BRAND_NAME " " DESIGNER_NAME " " + version;

	m_MainWindow = GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe"));
	gtk_window_set_title(GTK_WINDOW(m_MainWindow), defaultWindowTitle.c_str());
	gtk_menu_item_set_label(
		GTK_MENU_ITEM(gtk_builder_get_object(m_Builder, "openvibe-menu_display_changelog")),
		("What's new in " + version + " version of " + BRAND_NAME " " DESIGNER_NAME).c_str());

	// Catch delete events when close button is clicked
	g_signal_connect(m_MainWindow, "delete_event", G_CALLBACK(button_quit_application_cb), this);
	// be notified on maximize/minimize events
	g_signal_connect(m_MainWindow, "window-state-event", G_CALLBACK(window_state_changed_cb), this);

	// Connects menu actions
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_undo")), "activate", G_CALLBACK(menu_undo_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_redo")), "activate", G_CALLBACK(menu_redo_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_focus_search")), "activate", G_CALLBACK(menu_focus_search_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_copy")), "activate", G_CALLBACK(menu_copy_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_cut")), "activate", G_CALLBACK(menu_cut_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_paste")), "activate", G_CALLBACK(menu_paste_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_delete")), "activate", G_CALLBACK(menu_delete_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_preferences")), "activate", G_CALLBACK(menu_preferences_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_new")), "activate", G_CALLBACK(menu_new_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_open")), "activate", G_CALLBACK(menu_open_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_save")), "activate", G_CALLBACK(menu_save_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_save_as")), "activate", G_CALLBACK(menu_save_scenario_as_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_close")), "activate", G_CALLBACK(menu_close_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_quit")), "activate", G_CALLBACK(menu_quit_application_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_restore_default_scenarios")), "activate",
					 G_CALLBACK(menu_restore_default_scenarios_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_about")), "activate", G_CALLBACK(menu_about_openvibe_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_scenario_about")), "activate", G_CALLBACK(menu_about_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_documentation")), "activate", G_CALLBACK(menu_browse_documentation_cb), this);
#ifdef MENSIA_DISTRIBUTION
	if (FS::Files::fileExists(Directories::getBinDir() + "/mensia-flexnet-activation.exe"))
	{
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_register_license")), "activate", G_CALLBACK(menu_register_license_cb), this);
	}
	else { gtk_widget_hide(GTK_WIDGET((gtk_builder_get_object(m_Builder, "openvibe-menu_register_license")))); }
#else
	gtk_widget_hide(GTK_WIDGET((gtk_builder_get_object(m_Builder, "openvibe-menu_register_license"))));
#endif

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_issue_report")), "activate", G_CALLBACK(menu_report_issue_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-menu_display_changelog")), "activate", G_CALLBACK(menu_display_changelog_cb), this);

	// g_signal_connect(G_OBJECT(gtk_builder_get_object(m_builderInterface, "openvibe-menu_test")), "activate", G_CALLBACK(menu_test_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_new")), "clicked", G_CALLBACK(button_new_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_open")), "clicked", G_CALLBACK(button_open_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_save")), "clicked", G_CALLBACK(button_save_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_save_as")), "clicked", G_CALLBACK(button_save_scenario_as_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_close")), "clicked", G_CALLBACK(button_close_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_undo")), "clicked", G_CALLBACK(button_undo_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_redo")), "clicked", G_CALLBACK(button_redo_cb), this);


	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_log_level")), "clicked", G_CALLBACK(log_level_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), "toggled", G_CALLBACK(button_toggle_window_manager_cb),
					 this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_comment")), "clicked", G_CALLBACK(button_comment_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_aboutscenario")), "clicked", G_CALLBACK(button_about_scenario_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), "clicked", G_CALLBACK(stop_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), "clicked", G_CALLBACK(play_pause_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_next")), "clicked", G_CALLBACK(next_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), "clicked", G_CALLBACK(forward_scenario_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_title_button_expand")), "clicked",
					 G_CALLBACK(box_algorithm_title_button_expand_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_title_button_collapse")), "clicked",
					 G_CALLBACK(box_algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-algorithm_title_button_expand")), "clicked",
					 G_CALLBACK(algorithm_title_button_expand_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-algorithm_title_button_collapse")), "clicked",
					 G_CALLBACK(algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_searchbox")), "icon-press", G_CALLBACK(searchbox_select_all_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_searchbox")), "changed", G_CALLBACK(refresh_search_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_searchbox")), "focus-in-event", G_CALLBACK(searchbox_focus_in_cb),
					 this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_searchbox")), "focus-out-event", G_CALLBACK(searchbox_focus_out_cb),
					 this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-scenario_configuration_button_configure")), "clicked",
					 G_CALLBACK(button_configure_current_scenario_settings_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_zoomin")), "clicked", G_CALLBACK(zoom_in_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-button_zoomout")), "clicked", G_CALLBACK(zoom_out_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-zoom_spinner")), "value-changed", G_CALLBACK(spinner_zoom_changed_cb), this);
#ifdef MENSIA_DISTRIBUTION
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "neurort-toggle_engine_configuration")), "clicked",
					 G_CALLBACK(button_toggle_neurort_engine_configuration_cb), this);
	m_ArchwayHandlerGUI->m_ButtonOpenEngineConfigurationDialog = GTK_WIDGET(gtk_builder_get_object(m_Builder, "neurort-toggle_engine_configuration"));
#endif
	// Prepares fast forward feature
	const double fastForwardFactor = m_kernelCtx.getConfigurationManager().expandAsFloat("${Designer_FastForwardFactor}", -1);
	m_FastForwardFactor            = GTK_SPIN_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-spinbutton_fast-forward-factor"));
	if (fastForwardFactor == -1) { gtk_spin_button_set_value(m_FastForwardFactor, 100); }
	else { gtk_spin_button_set_value(m_FastForwardFactor, fastForwardFactor); }

#if defined(TARGET_OS_Windows)
#if GTK_CHECK_VERSION(2, 24, 0)
	// expect it to work */
#else
	gtk_about_dialog_set_url_hook((GtkAboutDialogActivateLinkFunc)menu_about_link_clicked_cb, this, nullptr);
#endif
#endif

	idle_add_cb(idle_application_loop, this);
	timeout_add_cb(1000, timeout_application_loop, this);
#ifdef NDEBUG
	timeout_add_cb(100, receiveSecondInstanceMessage, this);
#endif

	// Prepares main notebooks
	m_scenarioNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_Builder, "openvibe-scenario_notebook"));
	// optional behavior: vertically stacked scenarios (filename trimming mandatory in that case).
	if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ScenarioTabsVerticalStack}", false))
	{
		gtk_notebook_set_tab_pos(m_scenarioNotebook, GTK_POS_LEFT);
	}

	g_signal_connect(G_OBJECT(m_scenarioNotebook), "switch-page", G_CALLBACK(change_current_scenario_cb), this);
	g_signal_connect(G_OBJECT(m_scenarioNotebook), "page-reordered", G_CALLBACK(reorder_scenario_cb), this);
	m_resourceNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_Builder, "openvibe-resource_notebook"));

	// Creates an empty scnenario
	gtk_notebook_remove_page(m_scenarioNotebook, 0);

	// Initialize menu open recent
	m_menuOpenRecent = GTK_CONTAINER(gtk_builder_get_object(m_Builder, "openvibe-menu_recent_content"));

	//newScenarioCB();
	{
		// Prepares box algorithm view
		m_BoxAlgorithmTreeView                = GTK_TREE_VIEW(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_tree"));
		GtkTreeViewColumn* treeViewColumnName = gtk_tree_view_column_new();
		GtkTreeViewColumn* treeViewColumnDesc = gtk_tree_view_column_new();
		GtkCellRenderer* cellRendererIcon     = gtk_cell_renderer_pixbuf_new();
		GtkCellRenderer* cellRendererName     = gtk_cell_renderer_text_new();
		GtkCellRenderer* cellRendererDesc     = gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(treeViewColumnName, "Name");
		gtk_tree_view_column_set_title(treeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(treeViewColumnName, cellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(treeViewColumnName, cellRendererName, TRUE);
		gtk_tree_view_column_pack_start(treeViewColumnDesc, cellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(treeViewColumnName, cellRendererIcon, "stock-id", Resource_StringStockIcon, nullptr);
		gtk_tree_view_column_set_attributes(treeViewColumnName, cellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, "font",
											Resource_StringFont, "background", Resource_BackGroundColor, nullptr);
		gtk_tree_view_column_set_attributes(treeViewColumnDesc, cellRendererDesc, "text", Resource_StringShortDescription, "foreground", Resource_StringColor,
											"background", Resource_BackGroundColor, nullptr);
		gtk_tree_view_column_set_sizing(treeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sizing(treeViewColumnDesc, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand(treeViewColumnName, FALSE);
		gtk_tree_view_column_set_expand(treeViewColumnDesc, FALSE);
		gtk_tree_view_column_set_resizable(treeViewColumnName, TRUE);
		gtk_tree_view_column_set_resizable(treeViewColumnDesc, TRUE);
		gtk_tree_view_column_set_min_width(treeViewColumnName, 64);
		gtk_tree_view_column_set_min_width(treeViewColumnDesc, 64);
		gtk_tree_view_column_set_fixed_width(treeViewColumnName, 256);
		gtk_tree_view_column_set_fixed_width(treeViewColumnDesc, 512);
		gtk_tree_view_append_column(m_BoxAlgorithmTreeView, treeViewColumnName);
		gtk_tree_view_append_column(m_BoxAlgorithmTreeView, treeViewColumnDesc);

		// Prepares box algorithm model
		m_BoxAlgorithmTreeModel = gtk_tree_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
													 G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING);

		// Tree Storage for the searches
		gtk_tree_view_set_model(m_BoxAlgorithmTreeView, GTK_TREE_MODEL(m_BoxAlgorithmTreeModel));
	}

	{
		// Prepares algorithm view
		m_algorithmTreeView                   = GTK_TREE_VIEW(gtk_builder_get_object(m_Builder, "openvibe-algorithm_tree"));
		GtkTreeViewColumn* treeViewColumnName = gtk_tree_view_column_new();
		GtkTreeViewColumn* treeViewColumnDesc = gtk_tree_view_column_new();
		GtkCellRenderer* cellRendererIcon     = gtk_cell_renderer_pixbuf_new();
		GtkCellRenderer* cellRendererName     = gtk_cell_renderer_text_new();
		GtkCellRenderer* cellRendererDesc     = gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(treeViewColumnName, "Name");
		gtk_tree_view_column_set_title(treeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(treeViewColumnName, cellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(treeViewColumnName, cellRendererName, TRUE);
		gtk_tree_view_column_pack_start(treeViewColumnDesc, cellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(treeViewColumnName, cellRendererIcon, "stock-id", Resource_StringStockIcon, nullptr);
		gtk_tree_view_column_set_attributes(treeViewColumnName, cellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, nullptr);
		gtk_tree_view_column_set_attributes(treeViewColumnDesc, cellRendererDesc, "text", Resource_StringShortDescription, "foreground", Resource_StringColor,
											nullptr);

		gtk_tree_view_column_set_sizing(treeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sizing(treeViewColumnDesc, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand(treeViewColumnName, FALSE);
		gtk_tree_view_column_set_expand(treeViewColumnDesc, FALSE);
		gtk_tree_view_column_set_resizable(treeViewColumnName, TRUE);
		gtk_tree_view_column_set_resizable(treeViewColumnDesc, TRUE);
		gtk_tree_view_column_set_min_width(treeViewColumnName, 64);
		gtk_tree_view_column_set_min_width(treeViewColumnDesc, 64);
		gtk_tree_view_column_set_fixed_width(treeViewColumnName, 256);
		gtk_tree_view_column_set_fixed_width(treeViewColumnDesc, 512);
		gtk_tree_view_append_column(m_algorithmTreeView, treeViewColumnName);
		gtk_tree_view_append_column(m_algorithmTreeView, treeViewColumnDesc);

		// Prepares algorithm model
		m_AlgorithmTreeModel = gtk_tree_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN,
												  G_TYPE_BOOLEAN, G_TYPE_STRING);
		gtk_tree_view_set_model(m_algorithmTreeView, GTK_TREE_MODEL(m_AlgorithmTreeModel));
	}

	m_configureSettingsAddSettingButton = GTK_WIDGET(gtk_builder_get_object(m_Builder, "dialog_scenario_configuration-button_add_setting"));
	g_signal_connect(G_OBJECT(m_configureSettingsAddSettingButton), "clicked", G_CALLBACK(add_scenario_setting_cb), this);
	// Set up the UI for adding Inputs and Outputs to the scenario

	GtkWidget* scenarioLinksVBox = GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-scenario_links_vbox"));

	m_Inputs  = gtk_table_new(1, 3, FALSE);
	m_Outputs = gtk_table_new(1, 3, FALSE);

	GtkWidget* scrolledWindowInputs = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledWindowInputs), GTK_WIDGET(m_Inputs));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindowInputs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* scrolledWindowOutputs = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledWindowOutputs), GTK_WIDGET(m_Outputs));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindowOutputs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* addInputButton  = gtk_button_new_with_label("Add Input");
	GtkWidget* addOutputButton = gtk_button_new_with_label("Add Output");

	g_signal_connect(G_OBJECT(addInputButton), "clicked", G_CALLBACK(add_scenario_input_cb), this);
	g_signal_connect(G_OBJECT(addOutputButton), "clicked", G_CALLBACK(add_scenario_output_cb), this);

	gtk_box_pack_start(GTK_BOX(scenarioLinksVBox), gtk_label_new("Inputs"), FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(scenarioLinksVBox), scrolledWindowInputs, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(scenarioLinksVBox), addInputButton, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(scenarioLinksVBox), gtk_label_new("Outputs"), FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(scenarioLinksVBox), scrolledWindowOutputs, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(scenarioLinksVBox), addOutputButton, FALSE, FALSE, 4);

	gtk_widget_show_all(scenarioLinksVBox);

	// Prepares drag& drop for box creation
	gtk_drag_source_set(GTK_WIDGET(m_BoxAlgorithmTreeView), GDK_BUTTON1_MASK, TARGET_ENTRY, sizeof(TARGET_ENTRY) / sizeof(GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_BoxAlgorithmTreeView), "drag_data_get", G_CALLBACK(drag_data_get_cb), this);

	// Shows main window
	gtk_builder_connect_signals(m_Builder, nullptr);
	m_isMaximized = false;

	const int height = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${Designer_EditorSizeHeight}"));
	const int width  = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${Designer_EditorSizeWidth}"));
	if (height > 0 && width > 0) { gtk_window_resize(GTK_WINDOW(m_MainWindow), width, height); }

	if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_FullscreenEditor}")) { gtk_window_maximize(GTK_WINDOW(m_MainWindow)); }

	const int panedPosition = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${Designer_EditorPanedPosition}"));
	if (panedPosition > 0) { gtk_paned_set_position(GTK_PANED(gtk_builder_get_object(m_Builder, "openvibe-horizontal_container")), panedPosition); }

	GtkNotebook* sidebar = GTK_NOTEBOOK(gtk_builder_get_object(m_Builder, "openvibe-resource_notebook"));


	// List the notebook pages, cycle through them in reverse so we can remove pages without modifying indexes
	for (int notebookIdx = gtk_notebook_get_n_pages(sidebar) - 1; notebookIdx >= 0; notebookIdx--)
	{
		GtkWidget* tabWidget = gtk_notebook_get_nth_page(sidebar, notebookIdx);
		GtkWidget* tabLabel  = gtk_notebook_get_tab_label(sidebar, tabWidget);
		if (!m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ShowAlgorithms}"))
		{
			if (tabLabel == GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-algorithm_title_container")))
			{
				gtk_notebook_remove_page(sidebar, notebookIdx);
			}
		}
	}

	// gtk_window_set_icon_name(GTK_WINDOW(m_MainWindow), "ov-logo");
	gtk_window_set_icon_from_file(GTK_WINDOW(m_MainWindow), Directories::getDataDir() + "/applications/designer/designer.ico", nullptr);
	gtk_window_set_default_icon_from_file(Directories::getDataDir() + "/applications/designer/designer.ico", nullptr);

	if (!(m_CmdLineFlags & CommandLineFlag_NoManageSession))
	{
		CIdentifier id;
		char name[1024];
		unsigned i = 0;
		do
		{
			sprintf(name, "Designer_LastScenarioFilename_%03u", ++i);
			if ((id = m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier(name)) != OV_UndefinedIdentifier)
			{
				CString filename;
				filename = m_kernelCtx.getConfigurationManager().getConfigurationTokenValue(id);
				filename = m_kernelCtx.getConfigurationManager().expand(filename);
				m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "Restoring scenario [" << filename << "]\n";
				if (!this->openScenario(filename.toASCIIString()))
				{
					m_kernelCtx.getLogManager() << Kernel::LogLevel_ImportantWarning << "Failed to restore scenario [" << filename << "]\n";
				}
			}
		} while (id != OV_UndefinedIdentifier);
	}

	CIdentifier tokenID;
	char str[1024];
	unsigned i = 0;
	do
	{
		sprintf(str, "Designer_RecentScenario_%03u", ++i);
		if ((tokenID = m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier(str)) != OV_UndefinedIdentifier)
		{
			CString fileName;
			fileName = m_kernelCtx.getConfigurationManager().getConfigurationTokenValue(tokenID);
			fileName = m_kernelCtx.getConfigurationManager().expand(fileName);

			GtkWidget* newRecentItem = gtk_image_menu_item_new_with_label(fileName.toASCIIString());
			g_signal_connect(G_OBJECT(newRecentItem), "activate", G_CALLBACK(menu_open_recent_scenario_cb), this);
			gtk_menu_shell_append(GTK_MENU_SHELL(m_menuOpenRecent), newRecentItem);
			gtk_widget_show(newRecentItem);
			m_recentScenarios.push_back(newRecentItem);
		}
	} while (tokenID != OV_UndefinedIdentifier);

	refresh_search_no_data_cb(nullptr, this);
	// Add the designer log listener
	const CString logLevel = m_kernelCtx.getConfigurationManager().expand("${Kernel_ConsoleLogLevel}");
	std::string value(logLevel.toASCIIString());
	transform(value.begin(), value.end(), value.begin(), ::to_lower<std::string::value_type>);

	Kernel::ELogLevel level = Kernel::LogLevel_Debug;
	if (value == "debug") { level = Kernel::LogLevel_Debug; }
	if (value == "benchmarking / profiling") { level = Kernel::LogLevel_Benchmark; }
	if (value == "trace") { level = Kernel::LogLevel_Trace; }
	if (value == "information") { level = Kernel::LogLevel_Info; }
	if (value == "warning") { level = Kernel::LogLevel_Warning; }
	if (value == "important warning") { level = Kernel::LogLevel_ImportantWarning; }
	if (value == "error") { level = Kernel::LogLevel_Error; }
	if (value == "fatal error") { level = Kernel::LogLevel_Fatal; }

	switch (level)
	{
		case Kernel::LogLevel_Debug: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_debug")), true);
		case Kernel::LogLevel_Benchmark: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_bench")), true);
		case Kernel::LogLevel_Trace: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_trace")), true);
		case Kernel::LogLevel_Info: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_info")), true);
		case Kernel::LogLevel_Warning: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_warning")), true);
		case Kernel::LogLevel_ImportantWarning: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_impwarning")), true);
		case Kernel::LogLevel_Error: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_error")), true);
		case Kernel::LogLevel_Fatal: gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_fatal")), true);
		default: break;
	}

	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_debug")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Debug));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_bench")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Benchmark));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_trace")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Trace));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_info")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Info));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_warning")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Warning));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_impwarning")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_ImportantWarning));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_error")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Error));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_fatal")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Fatal));

	if (!(m_CmdLineFlags & CommandLineFlag_NoGui))
	{
		m_logListener                   = new CLogListenerDesigner(m_kernelCtx, m_Builder);
		m_logListener->m_CenterOnBoxFun = [this](CIdentifier& id) { this->getCurrentInterfacedScenario()->centerOnBox(id); };
		m_kernelCtx.getLogManager().addListener(m_logListener);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_clear")), "clicked", G_CALLBACK(clear_messages_cb), m_logListener);
		gtk_widget_show(m_MainWindow);
	}
	// If last version of Designer used is anterior or null, then consider it as a new version
	const CString lastUsedVersion = m_kernelCtx.getConfigurationManager().expand("${Designer_LastVersionUsed}");
	int lastUsedVersionMajor      = 0;
	int lastUsedVersionMinor      = 0;
	int lastUsedVersionPatch      = 0;
	const int currentVersionMajor = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${ProjectVersion_Major}"));
	const int currentVersionMinor = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${ProjectVersion_Minor}"));
	const int currentVersionPatch = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${ProjectVersion_Patch}"));

	sscanf(lastUsedVersion.toASCIIString(), "%d.%d.%d", &lastUsedVersionMajor, &lastUsedVersionMinor, &lastUsedVersionPatch);
	if (lastUsedVersionMajor < currentVersionMajor
		|| (lastUsedVersionMajor == currentVersionMajor && lastUsedVersionMinor < currentVersionMinor)
		|| (lastUsedVersionMinor == currentVersionMinor && lastUsedVersionPatch < currentVersionPatch)
		|| (lastUsedVersionMajor == 0 && lastUsedVersionMinor == 0 && lastUsedVersionPatch == 0)) { m_IsNewVersion = true; }

	std::string defaultURLBaseString = std::string(m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}"));
#ifdef MENSIA_DISTRIBUTION
	if (m_ArchwayHandler->initialize() == OpenViBE::EEngineInitialisationStatus::NotAvailable)
	{
		gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_Builder, "neurort-toggle_engine_configuration")));
	}
#else
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_Builder, "neurort-toggle_engine_configuration")));
#endif
}

bool CApplication::displayChangelogWhenAvailable()
{
	// If last version used is ulterior as current version, and at least one box was added/updated, show the list
	if (!m_NewBoxes.empty() || !m_UpdatedBoxes.empty())
	{
		GtkBuilder* builder = gtk_builder_new();
		gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);

		GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "aboutdialog-newversion"));
		gtk_window_set_title(GTK_WINDOW(dialog), "Changelog");


		std::string version = m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString();
		version             = (version != "${Application_Version}") ? version
								  : m_kernelCtx.getConfigurationManager().expand("${ProjectVersion_Major}.${ProjectVersion_Minor}.${ProjectVersion_Patch}").
												toASCIIString();

		gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), version.c_str());

		std::string labelNewBoxesList = "<big><b>Changes in version " + version + " of the software:</b></big>";
		if (!m_NewBoxes.empty())
		{
			labelNewBoxesList += "\n<big>The following boxes were added:</big>\n";
			for (auto pNewBoxDesc : m_NewBoxes)
			{
				labelNewBoxesList += "    <b>" + pNewBoxDesc->getName() + ":</b> " + pNewBoxDesc->getShortDescription() + "\n";
			}
		}
		if (!m_UpdatedBoxes.empty())
		{
			labelNewBoxesList += "\n<big>The following boxes were updated:</big>\n";
			for (auto pUpdatedBoxDesc : m_UpdatedBoxes)
			{
				labelNewBoxesList += "    <b>" + pUpdatedBoxDesc->getName() + ":</b> " + pUpdatedBoxDesc->getShortDescription() + "\n";
			}
		}
#if defined TARGET_OS_Windows // This function makes Windows calls only, hide button on other OSs
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "button-display_changelog")), "clicked",
						 G_CALLBACK(about_newversion_button_display_changelog_cb), this);
#else
		gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "button-display_changelog")));
#endif
		if (!FS::Files::fileExists((OVD_README_File).toASCIIString()))
		{
			gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "button-display_changelog")));
		}
		GtkLabel* label = GTK_LABEL(gtk_builder_get_object(builder, "label-newversion"));
		gtk_label_set_markup(label, labelNewBoxesList.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		if (m_IsNewVersion)
		{
			gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_searchbox")), "(New)");
			gtk_window_set_focus(GTK_WINDOW(m_MainWindow), GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_searchbox")));
		}

		gtk_widget_destroy(dialog);
		g_object_unref(builder);
	}
	else { return false; }
	return true;
}

bool CApplication::openScenario(const char* filename)
{
	// Prevent opening twice the same scenario
	for (size_t i = 0; i < m_Scenarios.size(); ++i)
	{
		const auto scenario = m_Scenarios[i];
		if (scenario->m_Filename == std::string(filename))
		{
			gtk_notebook_set_current_page(m_scenarioNotebook, i);
			return true;
		}
	}

	CIdentifier scenarioID;
	if (m_ScenarioMgr->importScenarioFromFile(scenarioID, OVD_ScenarioImportContext_OpenScenario, filename))
	{
		// Closes first unnamed scenario
		if (m_Scenarios.size() == 1)
		{
			if (!m_Scenarios[0]->m_HasBeenModified && !m_Scenarios[0]->m_HasFileName)
			{
				const CIdentifier tmp = m_Scenarios[0]->m_ScenarioID;
				delete m_Scenarios[0];
				m_ScenarioMgr->releaseScenario(tmp);
				m_Scenarios.clear();
			}
		}

		Kernel::IScenario& scenario = m_ScenarioMgr->getScenario(scenarioID);

		// Creates interfaced scenario
		CInterfacedScenario* interfacedScenario = new CInterfacedScenario(m_kernelCtx, *this, scenario, scenarioID, *m_scenarioNotebook, OVD_GUI_File,
																		  OVD_GUI_Settings_File);

		// Deserialize the visualization tree from the scenario metadata, if it exists

		// Find the VisualizationTree metadata
		Kernel::IMetadata* vizTreeMetadata = nullptr;
		CIdentifier metadataID             = OV_UndefinedIdentifier;
		while ((metadataID = scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
		{
			vizTreeMetadata = scenario.getMetadataDetails(metadataID);
			if (vizTreeMetadata && vizTreeMetadata->getType() == OVVIZ_MetadataIdentifier_VisualizationTree) { break; }
		}
		VisualizationToolkit::IVisualizationTree* vizTree = interfacedScenario->m_Tree;
		if (vizTreeMetadata && vizTree) { vizTree->deserialize(vizTreeMetadata->getData()); }

		CIdentifier id;

		// Ensure visualization widgets contained in the scenario (if any) appear in the window manager
		//  even when the VisualizationTree section of a scenario file is missing, erroneous or deprecated

		// no visualization widget was added to visualization tree : ensure there aren't any in scenario
		CIdentifier boxID;
		while ((boxID = scenario.getNextBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
		{
			if (!vizTree->getVisualizationWidgetFromBoxIdentifier(boxID))
			{
				const Kernel::IBox* box                            = scenario.getBoxDetails(boxID);
				const Plugins::IPluginObjectDesc* boxAlgorithmDesc = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(
					box->getAlgorithmClassIdentifier());
				if (boxAlgorithmDesc && boxAlgorithmDesc->hasFunctionality(Plugins::EPluginFunctionality::Visualization))
				{
					//a visualization widget was found in scenario : manually add it to visualization tree
					vizTree->addVisualizationWidget(id, box->getName(), VisualizationToolkit::EVisualizationWidget::Box,
													OV_UndefinedIdentifier, 0, box->getIdentifier(), 0, OV_UndefinedIdentifier);
				}
			}
		}

		if (interfacedScenario->m_DesignerVisualization != nullptr)
		{
			interfacedScenario->m_DesignerVisualization->setDeleteEventCB(&delete_designer_visualisation_cb, this);
			interfacedScenario->m_DesignerVisualization->load();
		}
		//interfacedScenario->snapshotCB(); --> a snapshot is already created in CInterfacedScenario builder !
		interfacedScenario->m_Filename        = filename;
		interfacedScenario->m_HasFileName     = true;
		interfacedScenario->m_HasBeenModified = false;
		interfacedScenario->snapshotCB(false);

		m_Scenarios.push_back(interfacedScenario);

		interfacedScenario->redrawScenarioSettings();

		gtk_notebook_set_current_page(m_scenarioNotebook, gtk_notebook_get_n_pages(m_scenarioNotebook) - 1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_scenarioNotebook)-1);

		interfacedScenario->updateScenarioLabel();

		this->saveOpenedScenarios();
		return true;
	}
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "Importing scenario from file [" << filename << "] failed... "
			<< " Current file either is corrupted or is not compatible with the selected scenario importer (ie not an OpenViBE scenario file)\n";

	if (!(m_CmdLineFlags & CommandLineFlag_NoGui))
	{
		std::stringstream ss;
		ss << "The requested file: " << filename << "\n";
		ss << "may either not be an OpenViBE scenario file, \n";
		ss << "a " + std::string(BRAND_NAME) + " scenario file, \n";
		ss << "be corrupted or not compatible with \n";
		ss << "the selected scenario importer...";
		GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Scenario importation process failed !");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", ss.str().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	return false;
}

CString CApplication::getWorkingDirectory()
{
	CString workingDirectory = m_kernelCtx.getConfigurationManager().expand("${Designer_DefaultWorkingDirectory}/scenarios");

	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario)
	{
		if (scenario->m_HasFileName)
		{
			std::string directory = std::string(g_path_get_dirname(scenario->m_Filename.c_str()));
#if defined TARGET_OS_Windows
			std::replace(directory.begin(), directory.end(), '\\', '/');
#endif
			workingDirectory = directory.c_str();
		}
	}

	if (!g_path_is_absolute(workingDirectory.toASCIIString()))
	{
		std::string directory = g_get_current_dir();
#if defined TARGET_OS_Windows
		std::replace(directory.begin(), directory.end(), '\\', '/');
#endif
		workingDirectory = directory.c_str() + CString("/") + workingDirectory;
	}

	return workingDirectory;
}

bool CApplication::hasRunningScenario()
{
	return std::any_of(m_Scenarios.begin(), m_Scenarios.end(), [](CInterfacedScenario* elem) { return elem->m_Player != nullptr; });
}

bool CApplication::hasUnsavedScenario()
{
	return std::any_of(m_Scenarios.begin(), m_Scenarios.end(), [](CInterfacedScenario* elem) { return elem->m_HasBeenModified; });
}

CInterfacedScenario* CApplication::getCurrentInterfacedScenario()
{
	if (m_currentScenarioIdx < m_Scenarios.size()) { return m_Scenarios[m_currentScenarioIdx]; }
	return nullptr;
}

void CApplication::saveOpenedScenarios()
{
	// Saves opened scenarios
	if (!(m_CmdLineFlags & CommandLineFlag_NoManageSession))
	{
		const CString appConfigFile = m_kernelCtx.getConfigurationManager().expand("${Designer_CustomConfigurationFile}");

		FILE* file = FS::Files::open(appConfigFile.toASCIIString(), "wt");
		if (file)
		{
			size_t i = 1;
			fprintf(file, "# This file is generated\n");
			fprintf(file, "# Do not modify\n");
			fprintf(file, "\n");

			int width, height;
			gtk_window_get_size(GTK_WINDOW(m_MainWindow), &width, &height);
			fprintf(file, "Designer_EditorSizeWidth = %i\n", width);
			fprintf(file, "Designer_EditorSizeHeight = %i\n", height);
			fprintf(file, "Designer_EditorPanedPosition = %i\n",
					gtk_paned_get_position(GTK_PANED(gtk_builder_get_object(m_Builder, "openvibe-horizontal_container"))));
			fprintf(file, "Designer_FullscreenEditor = %s\n", m_isMaximized ? "True" : "False");

			fprintf(file, "# Last files opened in %s\n", std::string(DESIGNER_NAME).c_str());

			for (CInterfacedScenario* scenario : m_Scenarios)
			{
				if (!scenario->m_Filename.empty())
				{
					fprintf(file, "Designer_LastScenarioFilename_%03u = %s\n", i, scenario->m_Filename.c_str());
					i++;
				}
			}
			fprintf(file, "\n");

			const CString projectVersion = m_kernelCtx.getConfigurationManager().expand(
				"${ProjectVersion_Major}.${ProjectVersion_Minor}.${ProjectVersion_Patch}");
			const CString componentVersions = m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenValue("ProjectVersion_Components");
			fprintf(file, "# Last version of " DESIGNER_NAME " used:\n");
			fprintf(file, "Designer_LastVersionUsed = %s\n", projectVersion.toASCIIString());
			fprintf(file, "Designer_LastComponentVersionsUsed = %s\n", componentVersions.toASCIIString());
			fprintf(file, "\n");

			fprintf(file, "# Recently opened scenario\n");
			size_t scenarioID = 1;
			for (const GtkWidget* recentScenario : m_recentScenarios)
			{
				const gchar* recentScenarioPath = gtk_menu_item_get_label(GTK_MENU_ITEM(recentScenario));
				fprintf(file, "Designer_RecentScenario_%03u = %s\n", scenarioID, recentScenarioPath);
				++scenarioID;
			}
			fprintf(file, "\n");

			fclose(file);
		}
		else { m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "Error writing to '" << appConfigFile << "'\n"; }
	}
}

void CApplication::dragDataGetCB(GtkWidget* /*widget*/, GdkDragContext* /*dc*/, GtkSelectionData* selectionData, guint /*info*/, guint /*time*/) const
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "dragDataGetCB\n";

	GtkTreeView* view           = GTK_TREE_VIEW(gtk_builder_get_object(m_Builder, "openvibe-box_algorithm_tree"));
	GtkTreeSelection* selection = gtk_tree_view_get_selection(view);
	GtkTreeModel* model         = nullptr;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		const char* boxAlgorithmID = nullptr;
		gtk_tree_model_get(model, &iter, Resource_StringIdentifier, &boxAlgorithmID, -1);
		if (boxAlgorithmID)
		{
			gtk_selection_data_set(selectionData, GDK_SELECTION_TYPE_STRING, 8, reinterpret_cast<const guchar*>(boxAlgorithmID),
								   gint(strlen(boxAlgorithmID) + 1));
		}
	}
}

void CApplication::undoCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "undoCB\n";

	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario) { scenario->undoCB(); }
}

void CApplication::redoCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "redoCB\n";

	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario) { scenario->redoCB(); }
}

void CApplication::copySelectionCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "copySelectionCB\n";
	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario) { scenario->copySelection(); }
}

void CApplication::cutSelectionCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "cutSelectionCB\n";
	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario) { scenario->cutSelection(); }
}

void CApplication::pasteSelectionCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "pasteSelectionCB\n";
	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario) { scenario->pasteSelection(); }
}

void CApplication::deleteSelectionCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "deleteSelectionCB\n";
	CInterfacedScenario* scenario = this->getCurrentInterfacedScenario();
	if (scenario) { scenario->deleteSelection(); }
}

void CApplication::preferencesCB() const
{
	enum { Resource_TokenName, Resource_TokenValue, Resource_TokenExpand };

	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "preferencesCB\n";
	GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "configuration_manager", nullptr);
	gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
	gtk_builder_connect_signals(builder, nullptr);

	GtkWidget* configMgr           = GTK_WIDGET(gtk_builder_get_object(builder, "configuration_manager"));
	GtkTreeView* configMgrTreeView = GTK_TREE_VIEW(gtk_builder_get_object(builder, "configuration_manager-treeview"));

	// Prepares tree view
	GtkTreeViewColumn* treeViewColumnTokenName   = gtk_tree_view_column_new();
	GtkTreeViewColumn* treeViewColumnTokenValue  = gtk_tree_view_column_new();
	GtkTreeViewColumn* treeViewColumnTokenExpand = gtk_tree_view_column_new();
	GtkCellRenderer* cellRendererTokenName       = gtk_cell_renderer_text_new();
	GtkCellRenderer* cellRendererTokenValue      = gtk_cell_renderer_text_new();
	GtkCellRenderer* cellRendererTokenExpand     = gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(treeViewColumnTokenName, "Token name");
	gtk_tree_view_column_set_title(treeViewColumnTokenValue, "Token value");
	gtk_tree_view_column_set_title(treeViewColumnTokenExpand, "Expanded token value");
	gtk_tree_view_column_pack_start(treeViewColumnTokenName, cellRendererTokenName, TRUE);
	gtk_tree_view_column_pack_start(treeViewColumnTokenValue, cellRendererTokenValue, TRUE);
	gtk_tree_view_column_pack_start(treeViewColumnTokenExpand, cellRendererTokenExpand, TRUE);
	gtk_tree_view_column_set_attributes(treeViewColumnTokenName, cellRendererTokenName, "text", Resource_TokenName, nullptr);
	gtk_tree_view_column_set_attributes(treeViewColumnTokenValue, cellRendererTokenValue, "text", Resource_TokenValue, nullptr);
	gtk_tree_view_column_set_attributes(treeViewColumnTokenExpand, cellRendererTokenExpand, "text", Resource_TokenExpand, nullptr);
	gtk_tree_view_column_set_sort_column_id(treeViewColumnTokenName, Resource_TokenName);
	gtk_tree_view_column_set_sort_column_id(treeViewColumnTokenValue, Resource_TokenValue);
	gtk_tree_view_column_set_sort_column_id(treeViewColumnTokenExpand, Resource_TokenExpand);
	gtk_tree_view_column_set_sizing(treeViewColumnTokenName, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sizing(treeViewColumnTokenValue, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sizing(treeViewColumnTokenExpand, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(treeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_expand(treeViewColumnTokenValue, TRUE);
	gtk_tree_view_column_set_expand(treeViewColumnTokenExpand, TRUE);
	gtk_tree_view_column_set_resizable(treeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_resizable(treeViewColumnTokenValue, TRUE);
	gtk_tree_view_column_set_resizable(treeViewColumnTokenExpand, TRUE);
	gtk_tree_view_column_set_min_width(treeViewColumnTokenName, 256);
	gtk_tree_view_column_set_min_width(treeViewColumnTokenValue, 256);
	gtk_tree_view_column_set_min_width(treeViewColumnTokenExpand, 256);
	gtk_tree_view_append_column(configMgrTreeView, treeViewColumnTokenName);
	gtk_tree_view_append_column(configMgrTreeView, treeViewColumnTokenValue);
	gtk_tree_view_append_column(configMgrTreeView, treeViewColumnTokenExpand);
	gtk_tree_view_column_set_sort_indicator(treeViewColumnTokenName, TRUE);

	// Prepares tree model
	CIdentifier tokenID;
	GtkTreeStore* configMgrTreeModel = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	while ((tokenID = m_kernelCtx.getConfigurationManager().getNextConfigurationTokenIdentifier(tokenID)) != OV_UndefinedIdentifier)
	{
		GtkTreeIter iter;
		CString name   = m_kernelCtx.getConfigurationManager().getConfigurationTokenName(tokenID);
		CString value  = m_kernelCtx.getConfigurationManager().getConfigurationTokenValue(tokenID);
		CString expand = m_kernelCtx.getConfigurationManager().expand(value);
		gtk_tree_store_append(configMgrTreeModel, &iter, nullptr);
		gtk_tree_store_set(configMgrTreeModel, &iter, Resource_TokenName, name.toASCIIString(), Resource_TokenValue, value.toASCIIString(),
						   Resource_TokenExpand, expand.toASCIIString(), -1);
	}
	gtk_tree_view_set_model(configMgrTreeView, GTK_TREE_MODEL(configMgrTreeModel));
	g_signal_emit_by_name(treeViewColumnTokenName, "clicked");

	gtk_dialog_run(GTK_DIALOG(configMgr));
	gtk_widget_destroy(configMgr);

	g_object_unref(configMgrTreeModel);
	g_object_unref(builder);
}

void CApplication::testCB() const { m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "testCB\n"; }

void CApplication::newScenarioCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "newScenarioCB\n";

	CIdentifier scenarioID;
	if (m_ScenarioMgr->createScenario(scenarioID))
	{
		Kernel::IScenario& scenario             = m_ScenarioMgr->getScenario(scenarioID);
		CInterfacedScenario* interfacedScenario = new CInterfacedScenario(m_kernelCtx, *this, scenario, scenarioID, *m_scenarioNotebook, OVD_GUI_File,
																		  OVD_GUI_Settings_File);
		if (interfacedScenario->m_DesignerVisualization != nullptr)
		{
			interfacedScenario->m_DesignerVisualization->setDeleteEventCB(&delete_designer_visualisation_cb, this);
			interfacedScenario->m_DesignerVisualization->newVisualizationWindow("Default window");
		}
		interfacedScenario->updateScenarioLabel();
		m_Scenarios.push_back(interfacedScenario);
		gtk_notebook_set_current_page(m_scenarioNotebook, gtk_notebook_get_n_pages(m_scenarioNotebook) - 1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_scenarioNotebook)-1);
	}
}

void CApplication::openScenarioCB()
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "openScenarioCB\n";

	GtkFileFilter* specific = gtk_file_filter_new();
	GtkFileFilter* all      = gtk_file_filter_new();

	std::string allFileFormatsString = "All available formats (";
	CString fileNameExtension        = "";
	while ((fileNameExtension = m_kernelCtx.getScenarioManager().getNextScenarioImporter(OVD_ScenarioImportContext_OpenScenario, fileNameExtension)) !=
		   CString(""))
	{
		std::string currentFileFormatMask = "*" + std::string(fileNameExtension.toASCIIString());
		gtk_file_filter_add_pattern(specific, currentFileFormatMask.c_str());
		allFileFormatsString += "*" + std::string(fileNameExtension) + ", ";
	}

	allFileFormatsString.erase(allFileFormatsString.size() - 2); // because the loop adds one ", " too much
	allFileFormatsString += ")";

	gtk_file_filter_set_name(specific, allFileFormatsString.c_str());

	gtk_file_filter_set_name(all, "All files");
	gtk_file_filter_add_pattern(all, "*");

	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select scenario to open...", nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
															  GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogOpen), specific);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogOpen), all);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(widgetDialogOpen), specific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(widgetDialogOpen), true);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(widgetDialogOpen), this->getWorkingDirectory().toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(widgetDialogOpen), true);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		//char* fileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		GSList* list;
		GSList* file = list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(widgetDialogOpen));
		while (file)
		{
			char* filename = static_cast<char*>(file->data);
			char* backslash;
			while ((backslash = strchr(filename, '\\')) != nullptr) { *backslash = '/'; }
			this->openScenario(filename);
			g_free(file->data);
			file = file->next;
		}
		g_slist_free(list);
	}
	gtk_widget_destroy(widgetDialogOpen);
	//	g_object_unref(fileFilterSpecific);
	//	g_object_unref(fileFilterAll);
}

void CApplication::saveScenarioCB(CInterfacedScenario* scenario)
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "saveScenarioCB\n";

	CInterfacedScenario* currScenario = scenario ? scenario : getCurrentInterfacedScenario();
	if (!currScenario) { return; }

	if (currScenario->m_Scenario.containsBoxWithDeprecatedInterfacors())
	{
		cannotSaveScenarioBeforeUpdate();
		return;
	}

	if (!currScenario->m_HasFileName) { saveScenarioAsCB(scenario); }
	else
	{
		// If the current scenario is a metabox, we will save its prototype hash into an attribute of the scenario
		// that way the standalone scheduler can check whether metaboxes included inside need updating.
		if (currScenario->m_Scenario.isMetabox())
		{
			SBoxProto metaboxProto(m_kernelCtx.getTypeManager());

			Kernel::IScenario& tmp = currScenario->m_Scenario;
			for (size_t i = 0; i < tmp.getInputCount(); ++i)
			{
				CString name;
				CIdentifier typeID;
				CIdentifier id;

				tmp.getInputType(i, typeID);
				tmp.getInputName(i, name);
				tmp.getInterfacorIdentifier(Kernel::Input, i, id);

				metaboxProto.addInput(name, typeID, id, true);
			}

			for (size_t i = 0; i < tmp.getOutputCount(); ++i)
			{
				CString name;
				CIdentifier typeID;
				CIdentifier id;

				tmp.getOutputType(i, typeID);
				tmp.getOutputName(i, name);
				tmp.getInterfacorIdentifier(Kernel::Output, i, id);

				metaboxProto.addOutput(name, typeID, id, true);
			}

			for (size_t i = 0; i < tmp.getSettingCount(); ++i)
			{
				CString name;
				CIdentifier typeID;
				CString value;

				tmp.getSettingName(i, name);
				tmp.getSettingType(i, typeID);
				tmp.getSettingDefaultValue(i, value);

				metaboxProto.addSetting(name, typeID, value, false, OV_UndefinedIdentifier, true);
			}

			if (tmp.hasAttribute(OV_AttributeId_Scenario_MetaboxHash))
			{
				tmp.setAttributeValue(OV_AttributeId_Scenario_MetaboxHash, metaboxProto.hash.toString());
			}
			else { tmp.addAttribute(OV_AttributeId_Scenario_MetaboxHash, metaboxProto.hash.toString()); }

			if (!tmp.hasAttribute(OVP_AttributeId_Metabox_ID)) { tmp.setAttributeValue(OVP_AttributeId_Metabox_ID, CIdentifier::random().str().c_str()); }

			m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "This metaboxes Hash : " << metaboxProto.hash << "\n";
		}

		const char* scenarioFileName = currScenario->m_Filename.c_str();

		// Remove attributes that were added to links and boxes by the designer and which are used only for interal functionality.
		// This way the scenarios do not change if, for example somebody opens them on a system with different font metrics.
		currScenario->m_Scenario.removeAttribute(OV_AttributeId_ScenarioFilename);

		CIdentifier linkID;
		while ((linkID = currScenario->m_Scenario.getNextLinkIdentifier(linkID)) != OV_UndefinedIdentifier)
		{
			auto link = currScenario->m_Scenario.getLinkDetails(linkID);
			link->removeAttribute(OV_AttributeId_Link_XSrc);
			link->removeAttribute(OV_AttributeId_Link_YSrc);
			link->removeAttribute(OV_AttributeId_Link_XDst);
			link->removeAttribute(OV_AttributeId_Link_YDst);
			link->removeAttribute(OV_ClassId_Selected);
		}

		CIdentifier boxID;
		while ((boxID = currScenario->m_Scenario.getNextBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
		{
			auto box = currScenario->m_Scenario.getBoxDetails(boxID);
			box->removeAttribute(OV_AttributeId_Box_XSize);
			box->removeAttribute(OV_AttributeId_Box_YSize);
			box->removeAttribute(OV_ClassId_Selected);
		}

		CIdentifier commentID;
		while ((commentID = currScenario->m_Scenario.getNextCommentIdentifier(commentID)) != OV_UndefinedIdentifier)
		{
			auto comment = currScenario->m_Scenario.getCommentDetails(commentID);
			comment->removeAttribute(OV_ClassId_Selected);
		}

		// Remove all VisualizationTree type metadata
		// We save the last found identifier if there was one, this allows us to not modify it on subsequent saves
		CIdentifier metadataID              = OV_UndefinedIdentifier;
		CIdentifier lastFoundTreeIdentifier = OV_UndefinedIdentifier;
		while ((metadataID = currScenario->m_Scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
		{
			if (currScenario->m_Scenario.getMetadataDetails(metadataID)->getType() == OVVIZ_MetadataIdentifier_VisualizationTree)
			{
				currScenario->m_Scenario.removeMetadata(metadataID);
				lastFoundTreeIdentifier = metadataID;
				metadataID              = OV_UndefinedIdentifier;
			}
		}

		// Insert new metadata
		currScenario->m_Scenario.addMetadata(metadataID, lastFoundTreeIdentifier);
		currScenario->m_Scenario.getMetadataDetails(metadataID)->setType(OVVIZ_MetadataIdentifier_VisualizationTree);
		currScenario->m_Scenario.getMetadataDetails(metadataID)->setData(currScenario->m_Tree->serialize());

		CIdentifier scenarioExportContext = OVD_ScenarioExportContext_SaveScenario;
		if (currScenario->m_Scenario.isMetabox()) { scenarioExportContext = OVD_ScenarioExportContext_SaveMetabox; }

		m_kernelCtx.getErrorManager().releaseErrors();
		if (m_ScenarioMgr->exportScenarioToFile(scenarioExportContext, scenarioFileName, currScenario->m_ScenarioID))
		{
			currScenario->snapshotCB();
			currScenario->m_HasFileName     = true;
			currScenario->m_HasBeenModified = false;
			currScenario->updateScenarioLabel();
			this->saveOpenedScenarios();
		}
		else
		{
			m_kernelCtx.getLogManager() << Kernel::LogLevel_Warning << "Exporting scenario failed...\n";
			GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
			gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
			gtk_builder_connect_signals(builder, nullptr);
			GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_error_popup_saving"));
			// Reset the labels
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "dialog_error_popup_saving-label1")), "");
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "dialog_error_popup_saving-label2")), "");
			if (m_kernelCtx.getErrorManager().hasError())
			{
				gtk_label_set_text(
					GTK_LABEL(gtk_builder_get_object(builder, "dialog_error_popup_saving-label1")), m_kernelCtx.getErrorManager().getLastErrorString());
			}
			gtk_builder_connect_signals(builder, nullptr);
			const gint responseId = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			g_object_unref(builder);

			switch (responseId)
			{
				case GTK_RESPONSE_OK:
					this->saveScenarioCB(scenario);
					break;
				case GTK_RESPONSE_DELETE_EVENT:
				case GTK_RESPONSE_CANCEL:			//return; //useless no loop and default make nothing
				default: break;
			}
		}
	}
}

void CApplication::restoreDefaultScenariosCB() const
{
	const CString defaultScenariosDirectory = m_kernelCtx.getConfigurationManager().expand(OVD_SCENARIOS_PATH);
	const CString defaultWorkingDirectory   = m_kernelCtx.getConfigurationManager().expand(OVD_WORKING_SCENARIOS_PATH);
	const CString message                   = "Default scenarios will be restored in '" + defaultWorkingDirectory + "' folder.\n"
											  + "All previous scenarios in this folder will be removed.\n"
											  + "Do you want to continue ?\n";

	GtkWidget* widgetDialogRestoreScenarios = gtk_message_dialog_new(nullptr, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
																	 message.toASCIIString());


	const CString backupFolder = defaultWorkingDirectory + m_kernelCtx.getConfigurationManager().expand("-$core{date}-$core{time}");

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogRestoreScenarios)) == GTK_RESPONSE_YES)
	{
		// to avoid to loose old data, make a backup
		FS::Files::removeAll(backupFolder);
		if (FS::Files::copyDirectory(defaultWorkingDirectory, backupFolder))
		{
			m_kernelCtx.getLogManager() << Kernel::LogLevel_Info << "Old scenario folder backed up into "
					<< backupFolder << " folder\n";
			// make the copy
			FS::Files::removeAll(defaultWorkingDirectory);
			if (FS::Files::copyDirectory(defaultScenariosDirectory, defaultWorkingDirectory))
			{
				m_kernelCtx.getLogManager() << Kernel::LogLevel_Info << "Default scenarios restored into " << defaultWorkingDirectory << " folder\n";
			}
			else { m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "Could not copy " << defaultWorkingDirectory << " folder\n"; }
		}
		else { m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "Could not back up " << defaultWorkingDirectory << " folder. Copy has aborted.\n"; }
	}

	gtk_widget_destroy(widgetDialogRestoreScenarios);
}

void CApplication::saveScenarioAsCB(CInterfacedScenario* scenario)
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "saveScenarioAsCB\n";

	CInterfacedScenario* currScenario = scenario ? scenario : getCurrentInterfacedScenario();
	if (!currScenario) { return; }

	if (currScenario->m_Scenario.containsBoxWithDeprecatedInterfacors())
	{
		cannotSaveScenarioBeforeUpdate();
		return;
	}

	const bool isCurrentScenarioAMetabox = currScenario->m_Scenario.isMetabox();

	GtkFileFilter* all = gtk_file_filter_new(); // All files
	gtk_file_filter_set_name(all, "All files");
	gtk_file_filter_add_pattern(all, "*");

	GtkFileFilter* allCompatibleFormatsFileFilter = gtk_file_filter_new(); // All compatible files

	std::map<GtkFileFilter*, std::string> fileFilters;


	std::set<std::string> compatibleExtensions;
	CString ext;
	if (!isCurrentScenarioAMetabox)
	{
		while ((ext = m_kernelCtx.getScenarioManager().getNextScenarioExporter(OVD_ScenarioExportContext_SaveScenario, ext)) != CString(""))
		{
			compatibleExtensions.emplace(ext);
		}
	}
	while ((ext = m_kernelCtx.getScenarioManager().getNextScenarioExporter(OVD_ScenarioExportContext_SaveMetabox, ext)) != CString(""))
	{
		compatibleExtensions.emplace(ext);
	}

	std::string allCompatibleFormatsFilterName = "All compatible formats (";

	for (auto& extension : compatibleExtensions)
	{
		GtkFileFilter* fileFilter = gtk_file_filter_new();
		std::string filterName    = m_kernelCtx.getConfigurationManager().expand(std::string("${ScenarioFileNameExtension" + extension + "}").c_str()).
												toASCIIString() + std::string(" (*") + extension + ")";
		gtk_file_filter_set_name(fileFilter, filterName.c_str());
		std::string filterWildcard = "*" + extension;
		gtk_file_filter_add_pattern(fileFilter, filterWildcard.c_str());
		fileFilters[fileFilter] = extension;

		allCompatibleFormatsFilterName += filterWildcard + ", ";
		gtk_file_filter_add_pattern(allCompatibleFormatsFileFilter, filterWildcard.c_str());
	}

	allCompatibleFormatsFilterName.erase(allCompatibleFormatsFilterName.size() - 2); // because the loop adds one ", " too much
	allCompatibleFormatsFilterName += ")";

	gtk_file_filter_set_name(allCompatibleFormatsFileFilter, allCompatibleFormatsFilterName.c_str());

	// gtk_file_filter_set_name(fileFilterSVG, "SVG image");
	// gtk_file_filter_add_pattern(fileFilterSVG, "*.svg");

	GtkWidget* widgetDialogSaveAs = gtk_file_chooser_dialog_new("Select scenario to save...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
																GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, nullptr);

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), allCompatibleFormatsFileFilter);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), all);

	for (const auto& fileFilter : fileFilters) { gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), fileFilter.first); }

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), allCompatibleFormatsFileFilter);
	// gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_widgetDialogSaveAs), true);
	if (currScenario->m_HasFileName) { gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogSaveAs), currScenario->m_Filename.c_str()); }
	else
	{
		// Put metaboxes to the User metabox folder by default
		if (isCurrentScenarioAMetabox)
		{
			gtk_file_chooser_set_current_folder(
				GTK_FILE_CHOOSER(widgetDialogSaveAs), m_kernelCtx.getConfigurationManager().expand("${Path_UserData}/metaboxes").toASCIIString());
		}
		else { gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(widgetDialogSaveAs), this->getWorkingDirectory().toASCIIString()); }
	}

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogSaveAs)) == GTK_RESPONSE_ACCEPT)
	{
		char* tmp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogSaveAs));

		// replaces \ with / on windows
		char* backslash;
		while ((backslash = strchr(tmp, '\\')) != nullptr) { *backslash = '/'; }

		// stores filename in a local variable
		char filename[1024];
		int length = sprintf(filename, "%s", tmp);
		g_free(tmp);

		GtkFileFilter* filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs));
		if (fileFilters.count(filter) != 0)
		{
			// User chose a specific filter
			const std::string expectedExtension = fileFilters[filter];
			if (_strcmpi(filename + length - 4, (std::string(".") + expectedExtension).c_str()) != 0)
			{
				// If filename already has an extension, remove it
				if (filename[length - 4] == '.')
				{
					length -= 4;
					filename[length] = '\0';
				}

				// When user did not put appropriate extension, append it
				strcat(filename, expectedExtension.c_str());
				//filenameLength += int(1 + expectedExtension.length());
			}
		}

		// Set a default extension in case the current one is not compatible or there is none
		const std::string scenarioFilenameExtension = boost::filesystem::extension(filename);
		if (!compatibleExtensions.count(scenarioFilenameExtension))
		{
			//if (isCurrentScenarioAMetabox) { strcat(filename, ".mxb"); }
			//else { strcat(filename, ".mxs"); }
			//filenameLength += 4;
			strcat(filename, ".xml");
		}

		// We ensure the file does not exist
		bool isSaveActionValid = true;
		FILE* file             = FS::Files::open(filename, "r");
		if (file)
		{
			fclose(file);
			GtkDialog* confirmationDialog = GTK_DIALOG(
				::gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "The file already exists"));
			gtk_message_dialog_format_secondary_text(
				GTK_MESSAGE_DIALOG(confirmationDialog),
				"%s\n\nThe file you are trying to save-as already exists, confirming this action will overwrite the existing file. Please confirm you want to overwrite the existing file.",
				filename);
			isSaveActionValid = (gtk_dialog_run(GTK_DIALOG(confirmationDialog)) == GTK_RESPONSE_OK);
			gtk_widget_destroy(GTK_WIDGET(confirmationDialog));
		}

		// Finally save the scenario
		if (isSaveActionValid)
		{
			currScenario->m_Filename        = filename;
			currScenario->m_HasFileName     = true;
			currScenario->m_HasBeenModified = false;
			currScenario->updateScenarioLabel();
			saveScenarioCB(currScenario);
		}
		else { m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "Canceled 'save-as' action for filename [" << filename << "]\n"; }
	}

	gtk_widget_destroy(widgetDialogSaveAs);
	//	g_object_unref(fileFilterSpecific);
	//	g_object_unref(fileFilterAll);
}

void CApplication::addRecentScenario(const std::string& scenarioPath)
{
	bool scenarioFound = false;
	// If scenario path is already in menu, remove it from menu shell and re-add it on top of list
	for (size_t i = 0; i < m_recentScenarios.size(); ++i)
	{
		const gchar* fileName = gtk_menu_item_get_label(GTK_MENU_ITEM(m_recentScenarios[i]));
		if (strcmp(fileName, scenarioPath.c_str()) == 0)
		{
			gtk_container_remove(m_menuOpenRecent, GTK_WIDGET(m_recentScenarios[i]));
			gtk_menu_shell_prepend(GTK_MENU_SHELL(m_menuOpenRecent), GTK_WIDGET(m_recentScenarios[i]));
			scenarioFound = true;
			m_recentScenarios.insert(m_recentScenarios.begin(), m_recentScenarios[i]);
			m_recentScenarios.erase(m_recentScenarios.begin() + i + 1);
			break;
		}
	}
	// If scenario is not in menu, create new widget and add it to menu shell
	if (!scenarioFound)
	{
		GtkWidget* newRecentItem = gtk_image_menu_item_new_with_label(scenarioPath.c_str());

		g_signal_connect(G_OBJECT(newRecentItem), "activate", G_CALLBACK(menu_open_recent_scenario_cb), this);
		gtk_menu_shell_prepend(GTK_MENU_SHELL(m_menuOpenRecent), newRecentItem);
		gtk_widget_show(newRecentItem);
		m_recentScenarios.insert(m_recentScenarios.begin(), newRecentItem);
	}

	if (m_recentScenarios.size() > s_RecentFileNumber)
	{
		for (auto it = m_recentScenarios.begin() + s_RecentFileNumber; it != m_recentScenarios.end(); ++it)
		{
			gtk_container_remove(m_menuOpenRecent, GTK_WIDGET(*it));
			gtk_widget_destroy(GTK_WIDGET(*it));
		}
		m_recentScenarios.erase(m_recentScenarios.begin() + s_RecentFileNumber, m_recentScenarios.end());
	}
}

void CApplication::closeScenarioCB(CInterfacedScenario* scenario)
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "closeScenarioCB\n";

	if (!scenario) { return; }
	if (scenario->isLocked())
	{
		GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(builder, nullptr);

		GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_running_scenario"));
		gtk_builder_connect_signals(builder, nullptr);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_object_unref(builder);
		return;
	}
	if (scenario->m_HasBeenModified)
	{
		GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(builder, nullptr);

		GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_unsaved_scenario"));
		gtk_builder_connect_signals(builder, nullptr);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE, true);
		const gint responseId = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_object_unref(builder);

		switch (responseId)
		{
			case GTK_RESPONSE_OK:
				this->saveScenarioCB(scenario);
				if (scenario->m_HasBeenModified) { return; }
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				return;
			default:
				break;
		}
	}
	// Add scenario to recently opened:
	this->addRecentScenario(scenario->m_Filename);

	const auto it = std::find(m_Scenarios.begin(), m_Scenarios.end(), scenario);
	if (it != m_Scenarios.end())
	{
		// We need to erase the scenario from the list first, because deleting the scenario will launch a "switch-page"
		// callback accessing this array with the identifier of the deleted scenario (if its not the last one) -> boom.
		m_Scenarios.erase(it);
		const CIdentifier id = scenario->m_ScenarioID;
		delete scenario;
		m_ScenarioMgr->releaseScenario(id);
		//when closing last open scenario, no "switch-page" event is triggered so we manually handle this case
		if (m_Scenarios.empty()) { newScenarioCB(); }
		else { changeCurrentScenario(gtk_notebook_get_current_page(m_scenarioNotebook)); }
	}

	this->saveOpenedScenarios();
}

void CApplication::deleteDesignerVisualizationCB()
{
	//untoggle window manager button when its associated dialog is closed
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), FALSE);

	CInterfacedScenario* scenario = getCurrentInterfacedScenario();
	if (scenario) { scenario->snapshotCB(); }
}

void CApplication::toggleDesignerVisualizationCB()
{
	CInterfacedScenario* scenario = getCurrentInterfacedScenario();
	if (scenario != nullptr && !scenario->isLocked())
	{
		const auto index = size_t(gtk_notebook_get_current_page(m_scenarioNotebook));
		if (index < m_Scenarios.size()) { m_Scenarios[index]->toggleDesignerVisualization(); }
	}
}

void CApplication::aboutOpenViBECB() const

{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::aboutOpenViBECB\n";
	GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
	gtk_builder_add_from_file(builder, OVD_GUI_AboutDialog_File, nullptr);
	gtk_builder_connect_signals(builder, nullptr);
	//gtk_builder_connect_signals(builder, nullptr);
	GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "about"));
	if (dialog == nullptr)
	{
		m_kernelCtx.getLogManager() << Kernel::LogLevel_Warning << "Dialog could not be opened\n";
		return;
	}

	if (m_kernelCtx.getConfigurationManager().expand("${Application_Name}").length() > 0)
	{
		gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), m_kernelCtx.getConfigurationManager().expand("${Application_Name}").toASCIIString());
	}
	if (m_kernelCtx.getConfigurationManager().expand("${Application_Version}").length() > 0)
	{
		gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString());
	}

	gchar* strval;
	g_object_get(dialog, "comments", &strval, nullptr);
	// We use a lookup instead of expansion as JSON can contain { } characters
	const std::string componentVersionsJSON = m_kernelCtx.getConfigurationManager().expand("${ProjectVersion_Components}").toASCIIString();
	if (!componentVersionsJSON.empty())
	{
		// This check is necessary because the asignemt operator would fail with an assert
		if (json::Deserialize(componentVersionsJSON).GetType() == json::ObjectVal)
		{
			json::Object components  = json::Deserialize(componentVersionsJSON);
			const std::string update = std::accumulate(components.begin(), components.end(), std::string(strval) + "\nComponents :\n",
													   [](const std::string& a, const std::pair<std::string, json::Value>& b)
													   {
														   return a + b.first + " version " + b.second.ToString() + "\n";
													   });
			g_object_set(dialog, "comments", update.c_str(), nullptr);
		}
	}
	g_free(strval);
	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE, true);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_object_unref(builder);
}

void CApplication::aboutScenarioCB(CInterfacedScenario* scenario) const
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::aboutScenarioCB\n";
	if (scenario && !scenario->isLocked()) { scenario->contextMenuScenarioAboutCB(); }
}

void CApplication::aboutLinkClickedCB(const gchar* url) const
{
	if (!url) { return; }
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::aboutLinkClickedCB\n";
	const CString command = m_kernelCtx.getConfigurationManager().expand("${Designer_WebBrowserCommand} " + CString(url));
	const int result      = system(command.toASCIIString());
	if (result < 0) { m_kernelCtx.getLogManager() << Kernel::LogLevel_Warning << "Could not launch command " << command << "\n"; }
}

void CApplication::browseDocumentationCB() const
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::browseDocumentationCB\n";
	const CString command = m_kernelCtx.getConfigurationManager().expand(
		"${Designer_HelpBrowserCommand} \"${Designer_HelpBrowserDocumentationIndex}\" ${Designer_HelpBrowserCommandPostfix}");

	const int result = system(command.toASCIIString());
	OV_WARNING_UNLESS((result == 0), "Could not launch command " << command << "\n", m_kernelCtx.getLogManager());
}

void CApplication::registerLicenseCB()
{
#if defined TARGET_OS_Windows && defined(MENSIA_DISTRIBUTION)
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::registerLicenseCB\n";
	const std::string cmd = (Directories::getBinDir() + "/mensia-flexnet-activation.exe").toASCIIString();
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	GetStartupInfo(&startupInfo);
	if (!System::WindowsUtilities::utf16CompliantCreateProcess(nullptr, const_cast<char*>(cmd.c_str()), nullptr, nullptr, 0, 0, nullptr, nullptr, &startupInfo,
															   &processInfo)) { exit(1); }
#elif defined TARGET_OS_Linux && defined(MENSIA_DISTRIBUTION)
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Info << "Register License application's GUI cannot run on Linux. In order to activate your license,"
		<< " you can use the tool 'mensia-flexnet-activation' in command line.\n";
#endif
}

void CApplication::reportIssueCB() const

{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::reportIssueCB\n";
	const CString command = m_kernelCtx.getConfigurationManager().expand(
		"${Designer_WebBrowserCommand} ${Designer_WebBrowserSupportURL} ${Designer_WebBrowserCommandPostfix}");
	const int result = system(command.toASCIIString());

	OV_WARNING_UNLESS((result == 0), "Could not launch command " << command << "\n", m_kernelCtx.getLogManager());
}

void CApplication::addCommentCB(CInterfacedScenario* scenario) const
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::addCommentCB\n";
	if (scenario && !scenario->isLocked()) { scenario->addCommentCB(); }
}

void CApplication::configureScenarioSettingsCB(CInterfacedScenario* scenario) const
{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "CApplication::configureScenarioSettingsCB " << m_currentScenarioIdx << "\n";

	if (scenario && !scenario->isLocked()) { scenario->configureScenarioSettingsCB(); }
}

Kernel::IPlayer* CApplication::getPlayer()
{
	CInterfacedScenario* scenario = getCurrentInterfacedScenario();
	return (scenario ? scenario->m_Player : nullptr);
}

bool CApplication::createPlayer()

{
	CInterfacedScenario* scenario = getCurrentInterfacedScenario();
	if (scenario && !scenario->m_Player)
	{
		// create a snapshot so settings override does not modify the scenario !
		scenario->snapshotCB(false);

		// set filename attribute to scenario so delayed configuration can be used
		if (scenario->m_HasFileName)
		{
			if (scenario->m_Scenario.hasAttribute(OV_AttributeId_ScenarioFilename))
			{
				scenario->m_Scenario.setAttributeValue(OV_AttributeId_ScenarioFilename, scenario->m_Filename.c_str());
			}
			else { scenario->m_Scenario.addAttribute(OV_AttributeId_ScenarioFilename, scenario->m_Filename.c_str()); }
		}

		m_kernelCtx.getPlayerManager().createPlayer(scenario->m_PlayerID);
		const CIdentifier scenarioID = scenario->m_ScenarioID;
		const CIdentifier playerID   = scenario->m_PlayerID;
		scenario->m_Player           = &m_kernelCtx.getPlayerManager().getPlayer(playerID);
		if (!scenario->m_Player->setScenario(scenarioID))
		{
			scenario->m_PlayerID = OV_UndefinedIdentifier;
			scenario->m_Player   = nullptr;
			m_kernelCtx.getPlayerManager().releasePlayer(playerID);
			OV_ERROR_DRF("The current scenario could not be loaded by the player.\n", Kernel::ErrorType::BadCall);
		}

		// The visualization manager needs to know the visualization tree in which the widgets should be inserted
		scenario->m_Player->getRuntimeConfigurationManager().
				  createConfigurationToken("VisualizationContext_VisualizationTreeId", scenario->m_TreeID.toString());

		// TODO_JL: This should be a copy of the tree containing visualizations from the metaboxes
		scenario->createPlayerVisualization(scenario->m_Tree);


		if (scenario->m_Player->initialize() != Kernel::EPlayerReturnCodes::Success)
		{
			scenario->releasePlayerVisualization();
			m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "The player could not be initialized.\n";
			scenario->m_PlayerID = OV_UndefinedIdentifier;
			scenario->m_Player   = nullptr;
			m_kernelCtx.getPlayerManager().releasePlayer(playerID);
			return false;
		}
		scenario->m_LastLoopTime = uint64_t(-1);

		//set up idle function
		idle_add_cb(idle_scenario_loop, scenario);

		// redraws scenario
		scenario->redraw();
	}
	return true;
}

void CApplication::stopInterfacedScenarioAndReleasePlayer(CInterfacedScenario* scenario)
{
	if (!(scenario && scenario->m_Player))
	{
		m_kernelCtx.getLogManager() << Kernel::LogLevel_Warning << "Trying to stop a non-started scenario" << "\n";
		return;
	}

	scenario->stopAndReleasePlayer();

	if (scenario == this->getCurrentInterfacedScenario())
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
	}
}

void CApplication::stopScenarioCB()

{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "stopScenarioCB\n";

	const Kernel::EPlayerStatus currentState = this->getCurrentInterfacedScenario()->m_PlayerStatus;
	if (currentState == Kernel::EPlayerStatus::Play || currentState == Kernel::EPlayerStatus::Pause || currentState == Kernel::EPlayerStatus::Forward)
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());

		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-toggle_button_loop"))))
		{
			switch (currentState)
			{
				case Kernel::EPlayerStatus::Play:
					playScenarioCB();
					break;
				case Kernel::EPlayerStatus::Forward:
					forwardScenarioCB();
					break;
				default:
					break;
			}
		}
	}
}

void CApplication::pauseScenarioCB()

{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "pauseScenarioCB\n";

	this->createPlayer();
	this->getPlayer()->pause();
	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::nextScenarioCB()

{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "nextScenarioCB\n";

	this->createPlayer();
	auto player = this->getPlayer();
	OV_ERROR_UNLESS_DRV(player, "Player did not initialize correctly", Kernel::ErrorType::BadCall);
	player->step();
	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::playScenarioCB()

{
	if (this->getCurrentInterfacedScenario() != nullptr)
	{
		Kernel::IScenario& scenario = this->getCurrentInterfacedScenario()->m_Scenario;
		m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "playScenarioCB\n";
		if (scenario.hasOutdatedBox())
		{
			if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Kernel_AbortPlayerWhenBoxIsOutdated}", false))
			{
				std::string outdatedBoxesList = "You can not start the scenario because following boxes need to be updated: \n";
				CIdentifier boxID;
				while ((boxID = scenario.getNextOutdatedBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
				{
					const Kernel::IBox* box = scenario.getBoxDetails(boxID);
					outdatedBoxesList += "\t[" + box->getName() + "]\n";
				}
				outdatedBoxesList += "To update a box you need to delete it from scenario, and add it again.";
				GtkWidget* dialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_MESSAGE_INFO,
														   GTK_BUTTONS_OK, "%s", outdatedBoxesList.c_str());
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				return;
			}
			if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ThrowPopUpWhenBoxIsOutdated}", false))
			{
				std::string outdatedBoxesList = "The following boxes need to be updated: \n";
				CIdentifier boxID;
				while ((boxID = scenario.getNextOutdatedBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
				{
					const Kernel::IBox* box = scenario.getBoxDetails(boxID);
					outdatedBoxesList += "\t[" + box->getName() + "]\n";
				}
				outdatedBoxesList += "Do you still want to play the scenario ?";
				GtkWidget* dialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_MESSAGE_QUESTION,
														   GTK_BUTTONS_YES_NO, "%s", outdatedBoxesList.c_str());
				const gint response = gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);

				if (response == GTK_RESPONSE_YES)
				{
					m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "CApplication::playScenarioCB - GTK_RESPONSE_YES: the scenario will be played. \n";
				}
				else
				{
					m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "CApplication::playScenarioCB - the scenario will not be played. \n";
					return;
				}
			}
		}
	}

	if (!this->createPlayer())
	{
		m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "The initialization of player failed. Check the above log messages to get the issue.\n";
		return;
	}
	if (!this->getPlayer()->play())
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
		return;
	}
	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PAUSE);

	if (m_CmdLineFlags & CommandLineFlag_NoVisualization) { for (auto& iScenario : m_Scenarios) { iScenario->hideCurrentVisualization(); } }
}

void CApplication::forwardScenarioCB()

{
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Trace << "forwardScenarioCB\n";

	if (!this->createPlayer())
	{
		m_kernelCtx.getLogManager() << Kernel::LogLevel_Error << "CreatePlayer failed\n";
		return;
	}

	if (!this->getPlayer()->forward())
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
		return;
	}

	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

	if (m_CmdLineFlags & CommandLineFlag_NoVisualization) { for (auto& iScenario : m_Scenarios) { iScenario->hideCurrentVisualization(); } }
}

bool CApplication::quitApplicationCB()

{
	CIdentifier id;
	m_kernelCtx.getLogManager() << Kernel::LogLevel_Debug << "quitApplicationCB\n";

	// can't quit while scenarios are running
	if (this->hasRunningScenario())
	{
		GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(builder, nullptr);

		GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_running_scenario_global"));
		gtk_builder_connect_signals(builder, nullptr);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_object_unref(builder);

		// prevent Gtk from handling delete_event and killing app
		return false;
	}

	// can't quit while scenarios are unsaved
	if (this->hasUnsavedScenario())
	{
		GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(builder, nullptr);

		GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_unsaved_scenario_global"));
		gtk_builder_connect_signals(builder, nullptr);
		const gint responseId = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_object_unref(builder);

		switch (responseId)
		{
			case GTK_RESPONSE_OK:
				for (auto i = m_Scenarios.begin(); i != m_Scenarios.end(); ++i) { this->saveScenarioCB(*i); }
				if (this->hasUnsavedScenario())
				{
					// prevent Gtk from handling delete_event and killing app
					return false;
				}
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				// prevent Gtk from handling delete_event and killing app
				return false;
			default:
				break;
		}
	}

	// Switch to quitting mode
	m_IsQuitting = true;

	// Saves opened scenarios
	this->saveOpenedScenarios();

	// Clears all existing interfaced scenarios
	for (auto interfacedScenario : m_Scenarios) { delete interfacedScenario; }

	// Clears all existing scenarios
	std::vector<CIdentifier> scenarioIDs;
	while ((id = m_kernelCtx.getScenarioManager().getNextScenarioIdentifier(id)) != OV_UndefinedIdentifier) { scenarioIDs.push_back(id); }

	for (auto& scenario : scenarioIDs) { m_kernelCtx.getScenarioManager().releaseScenario(scenario); }

	// release the log manager and free the memory
	if (m_logListener)
	{
		m_kernelCtx.getLogManager().removeListener(m_logListener);
		delete m_logListener;
		m_logListener = nullptr;
	}

	// OK to kill app
	return true;
}

void CApplication::windowStateChangedCB(const bool isMaximized)
{
	if (m_isMaximized != isMaximized && !isMaximized) // we switched to not maximized
	{
		//gtk_paned_set_position(GTK_PANED(gtk_builder_get_object(m_builderInterface, "openvibe-horizontal_container")), 640);
		gtk_window_resize(GTK_WINDOW(m_MainWindow), 1024, 768);
	}
	m_isMaximized = isMaximized;
}

void CApplication::logLevelCB() const
{
	// Loads log level dialog
	GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "loglevel", nullptr);
	gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);
	gtk_builder_connect_signals(builder, nullptr);

	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_fatal")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Fatal));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_error")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Error));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_important_warning")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_ImportantWarning));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_warning")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Warning));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_info")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Info));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_trace")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Trace));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_benchmark")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Benchmark));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_debug")),
		m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Debug));

	GtkDialog* logLevelDialog = GTK_DIALOG(gtk_builder_get_object(builder, "loglevel"));
	const gint result         = gtk_dialog_run(logLevelDialog);
	if (result == GTK_RESPONSE_APPLY)
	{
		m_kernelCtx.getLogManager().activate(
			Kernel::LogLevel_Fatal,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_fatal"))) != 0);
		m_kernelCtx.getLogManager().activate(
			Kernel::LogLevel_Error,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_error"))) != 0);
		m_kernelCtx.getLogManager().activate(Kernel::LogLevel_ImportantWarning,
											 gtk_toggle_button_get_active(
												 GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_important_warning"))) != 0);
		m_kernelCtx.getLogManager().activate(Kernel::LogLevel_Warning,
											 gtk_toggle_button_get_active(
												 GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_warning"))) != 0);
		m_kernelCtx.getLogManager().activate(
			Kernel::LogLevel_Info,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_info"))) != 0);
		m_kernelCtx.getLogManager().activate(
			Kernel::LogLevel_Trace,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_trace"))) != 0);
		m_kernelCtx.getLogManager().activate(Kernel::LogLevel_Benchmark,
											 gtk_toggle_button_get_active(
												 GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_benchmark"))) != 0);
		m_kernelCtx.getLogManager().activate(
			Kernel::LogLevel_Debug,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "loglevel-checkbutton_loglevel_debug"))) != 0);

		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_debug")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Debug));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_bench")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Benchmark));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_trace")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Trace));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_info")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Info));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_warning")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Warning));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_impwarning")),
			m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_ImportantWarning));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_error")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Error));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-messages_tb_fatal")), m_kernelCtx.getLogManager().isActive(Kernel::LogLevel_Fatal));
	}

	gtk_widget_destroy(GTK_WIDGET(logLevelDialog));
	g_object_unref(builder);
}

void CApplication::cpuUsageCB()

{
	CInterfacedScenario* scenario = getCurrentInterfacedScenario();
	if (scenario)
	{
		scenario->m_DebugCPUUsage = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage"))) != 0
		);
		scenario->redraw();
	}
}

void CApplication::changeCurrentScenario(const int pageIdx)
{
	if (m_IsQuitting) { return; }

	//hide window manager of previously active scenario, if any
	const int i = gtk_notebook_get_current_page(m_scenarioNotebook);
	if (i >= 0 && i < int(m_Scenarios.size())) { m_Scenarios[i]->hideCurrentVisualization(); }

	//closing last open scenario
	if (pageIdx == -1)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), false);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), false);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* windowManagerButton = GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(windowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(windowManagerButton), false);
		g_signal_connect(windowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		// toggle off and reset scenario settings
		GtkWidget* settingsVBox = GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-scenario_configuration_vbox"));

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-scenario_configuration_button_configure")), false);


		GList* settingWidgets = gtk_container_get_children(GTK_CONTAINER(gtk_builder_get_object(m_Builder, "openvibe-scenario_configuration_vbox")));
		for (GList* settingIterator = settingWidgets; settingIterator != nullptr; settingIterator = g_list_next(settingIterator))
		{
			gtk_widget_destroy(GTK_WIDGET(settingIterator->data));
		}
		g_list_free(settingWidgets);

		GtkWidget* settingPlaceholderLabel = gtk_label_new("This scenario has no settings");
		gtk_box_pack_end_defaults(GTK_BOX(settingsVBox), settingPlaceholderLabel);
		gtk_widget_show_all(settingsVBox);


		// current scenario is the current notebook page.
		m_currentScenarioIdx = i;
	}
		//switching to an existing scenario
	else if (pageIdx < m_Scenarios.size())
	{
		CInterfacedScenario* scenario            = m_Scenarios[pageIdx];
		const Kernel::EPlayerStatus playerStatus = (scenario->m_Player ? scenario->m_Player->getStatus() : Kernel::EPlayerStatus::Stop);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), playerStatus != Kernel::EPlayerStatus::Stop);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), playerStatus != Kernel::EPlayerStatus::Forward);
		gtk_tool_button_set_stock_id(
			GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")),
			(playerStatus == Kernel::EPlayerStatus::Stop || playerStatus == Kernel::EPlayerStatus::Pause) ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_undo")), scenario->m_StateStack->isUndoPossible());
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_redo")), scenario->m_StateStack->isRedoPossible());

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), playerStatus == Kernel::EPlayerStatus::Stop);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), scenario->m_DebugCPUUsage);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "openvibe-button_save")), scenario->m_hasFileName && scenario->m_hasBeenModified);
		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "openvibe-menu_save")),   scenario->m_hasFileName && scenario->m_hasBeenModified);

		//don't show window manager if in offline mode and it is toggled off
		if (playerStatus == Kernel::EPlayerStatus::Stop && !m_Scenarios[pageIdx]->isDesignerVisualizationToggled())
		{
			m_Scenarios[pageIdx]->hideCurrentVisualization();

			// we are in edition mode, updating internal configuration token
			std::string path = m_Scenarios[pageIdx]->m_Filename;
			path             = path.substr(0, path.rfind('/'));
			m_kernelCtx.getConfigurationManager().setConfigurationTokenValue(
				m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier("Player_ScenarioDirectory"), path.c_str());
			m_kernelCtx.getConfigurationManager().setConfigurationTokenValue(
				m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier("__volatile_ScenarioDir"), path.c_str());
		}
		else { m_Scenarios[pageIdx]->showCurrentVisualization(); }

		//update window manager button state
		GtkWidget* windowMgrButton = GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(windowMgrButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(windowMgrButton), m_Scenarios[pageIdx]->isDesignerVisualizationToggled());
		g_signal_connect(windowMgrButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-scenario_configuration_button_configure")), true);
		m_Scenarios[pageIdx]->redrawScenarioSettings();
		m_Scenarios[pageIdx]->redrawScenarioInputSettings();
		m_Scenarios[pageIdx]->redrawScenarioOutputSettings();


		// current scenario is the selected one
		m_currentScenarioIdx = pageIdx;
	}
		//first scenario is created (or a scenario is opened and replaces first unnamed unmodified scenario)
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_stop")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_next")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_forward")), true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_undo")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_redo")), false);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager")), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_Builder, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* windowMgrButton = GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(windowMgrButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(windowMgrButton), false);
		g_signal_connect(windowMgrButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_Builder, "openvibe-scenario_configuration_button_configure")), true);

		// we have a new notebook page
		m_currentScenarioIdx = pageIdx;

		// we are in edition mode, updating internal configuration token
		m_kernelCtx.getConfigurationManager().setConfigurationTokenValue(
			m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier("Player_ScenarioDirectory"), "");
		m_kernelCtx.getConfigurationManager().setConfigurationTokenValue(
			m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier("__volatile_ScenarioDir"), "");
	}

	// updates the trimming if need be
	for (auto& scenario : m_Scenarios) { scenario->updateScenarioLabel(); }
	// Reset zoom
	if (getCurrentInterfacedScenario())
	{
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-zoom_spinner")), round(getCurrentInterfacedScenario()->getScale() * 100.0));
	}
	else { gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-zoom_spinner")), 100); }
}

void CApplication::reorderCurrentScenario(const size_t newPageIdx)
{
	CInterfacedScenario* scenario = m_Scenarios[m_currentScenarioIdx];
	m_Scenarios.erase(m_Scenarios.begin() + m_currentScenarioIdx);
	m_Scenarios.insert(m_Scenarios.begin() + newPageIdx, scenario);

	this->changeCurrentScenario(newPageIdx);
}

//Increase the zoom of the current scenario
void CApplication::zoomInCB()
{
	gtk_spin_button_set_value(
		GTK_SPIN_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-zoom_spinner")), round(getCurrentInterfacedScenario()->getScale() * 100.0) + 5);
}

//Decrease the zoom of the current scenario
void CApplication::zoomOutCB()
{
	gtk_spin_button_set_value(
		GTK_SPIN_BUTTON(gtk_builder_get_object(m_Builder, "openvibe-zoom_spinner")), round(getCurrentInterfacedScenario()->getScale() * 100.0) - 5);
}

void CApplication::spinnerZoomChangedCB(const size_t scaleDelta)
{
	if (getCurrentInterfacedScenario() != nullptr) { getCurrentInterfacedScenario()->setScale(double(scaleDelta) / 100.0); }
}

void CApplication::cannotSaveScenarioBeforeUpdate()

{
	const CString message = "Cannot save a scenario if deprecated I/O or Settings are still pending.\n"
			"Please handle or delete all pending deprecated I/O before saving scenario.";
	GtkWidget* dialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
											   "%s", message.toASCIIString());
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

}  // namespace Designer
}  // namespace OpenViBE
