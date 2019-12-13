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

#include <openvibe/ovTimeArithmetics.h>
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

static const uint32_t s_RecentFileNumber = 10;

#include "ovdCDesignerVisualization.h"
#include "ovdCPlayerVisualization.h"
#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"
#include "ovdAssert.h"
#include "ovdCLogListenerDesigner.h"

#include "visualization/ovdCVisualizationManager.h"

#define OV_ClassId_Selected OpenViBE::CIdentifier(0xC67A01DC, 0x28CE06C1)

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	// because std::tolower has multiple signatures,
	// it can not be easily used in std::transform
	// this workaround is taken from http://www.gcek.net/ref/books/sw/cpp/ticppv2/
	template <class charT>
	charT to_lower(charT c) { return tolower(c); }
} // namespace

namespace
{
	struct SBoxProto : IBoxProto
	{
		SBoxProto(ITypeManager& typeManager) : m_TypeManager(typeManager) { }

		bool addInput(const CString& /*name*/, const CIdentifier& typeID, const CIdentifier& identifier, const bool /*bNotify*/) override
		{
			uint64_t v = typeID.toUInteger();
			swap_byte(v, m_nInputHash);
			swap_byte(m_nInputHash, 0x7936A0F3BD12D936LL);
			m_oHash = m_oHash.toUInteger() ^ v;
			if (identifier != OV_UndefinedIdentifier)
			{
				v = identifier.toUInteger();
				swap_byte(v, 0x2BD1D158F340014D);
				m_oHash = m_oHash.toUInteger() ^ v;
			}
			return true;
		}
		//
		bool addOutput(const CString& /*name*/, const CIdentifier& typeID, const CIdentifier& identifier, const bool /*bNotify*/) override
		{
			uint64_t v = typeID.toUInteger();
			swap_byte(v, m_nOutputHash);
			swap_byte(m_nOutputHash, 0xCBB66A5B893AA4E9LL);
			m_oHash = m_oHash.toUInteger() ^ v;
			if (identifier != OV_UndefinedIdentifier)
			{
				v = identifier.toUInteger();
				swap_byte(v, 0x87CA0F5EFC4FAC68);
				m_oHash = m_oHash.toUInteger() ^ v;
			}
			return true;
		}

		bool addSetting(const CString& /*name*/, const CIdentifier& typeID, const CString& /*sDefaultValue*/, const bool /*bModifiable*/,
						const CIdentifier& identifier, const bool /*bNotify*/) override
		{
			uint64_t v = typeID.toUInteger();
			swap_byte(v, m_nSettingHash);
			swap_byte(m_nSettingHash, 0x3C87F3AAE9F8303BLL);
			m_oHash = m_oHash.toUInteger() ^ v;
			if (identifier != OV_UndefinedIdentifier)
			{
				v = identifier.toUInteger();
				swap_byte(v, 0x17185F7CDA63A9FA);
				m_oHash = m_oHash.toUInteger() ^ v;
			}
			return true;
		}

		bool addInputSupport(const CIdentifier& /*typeID*/) override { return true; }

		bool addOutputSupport(const CIdentifier& /*typeID*/) override { return true; }

		bool addFlag(const EBoxFlag eBoxFlag) override
		{
			switch (eBoxFlag)
			{
				case BoxFlag_CanAddInput:
					m_oHash = m_oHash.toUInteger() ^ CIdentifier(0x07507AC8, 0xEB643ACE).toUInteger();
					break;
				case BoxFlag_CanModifyInput:
					m_oHash = m_oHash.toUInteger() ^ CIdentifier(0x5C985376, 0x8D74CDB8).toUInteger();
					break;
				case BoxFlag_CanAddOutput:
					m_oHash = m_oHash.toUInteger() ^ CIdentifier(0x58DEA69B, 0x12411365).toUInteger();
					break;
				case BoxFlag_CanModifyOutput:
					m_oHash = m_oHash.toUInteger() ^ CIdentifier(0x6E162C01, 0xAC979F22).toUInteger();
					break;
				case BoxFlag_CanAddSetting:
					m_oHash = m_oHash.toUInteger() ^ CIdentifier(0xFA7A50DC, 0x2140C013).toUInteger();
					break;
				case BoxFlag_CanModifySetting:
					m_oHash = m_oHash.toUInteger() ^ CIdentifier(0x624D7661, 0xD8DDEA0A).toUInteger();
					break;
				case BoxFlag_IsDeprecated:
					m_isDeprecated = true;
					break;
				default: return false;
			}
			return true;
		}

		bool addFlag(const CIdentifier& cIdentifierFlag) override
		{
			const uint64_t flagValue = m_TypeManager.getEnumerationEntryValueFromName(OV_TypeId_BoxAlgorithmFlag, cIdentifierFlag.toString());
			return flagValue != OV_UndefinedIdentifier;
		}

		void swap_byte(uint64_t& v, const uint64_t s)
		{
			uint8_t V[sizeof(v)];
			uint8_t S[sizeof(s)];
			System::Memory::hostToLittleEndian(v, V);
			System::Memory::hostToLittleEndian(s, S);
			for (uint32_t i = 0; i < sizeof(s); i += 2)
			{
				const uint32_t j = S[i] % sizeof(v);
				const uint32_t k = S[i + 1] % sizeof(v);
				const uint8_t t  = V[j];
				V[j]             = V[k];
				V[k]             = t;
			}
			System::Memory::littleEndianToHost(V, &v);
		}

		_IsDerivedFromClass_Final_(IBoxProto, OV_UndefinedIdentifier)

		CIdentifier m_oHash;
		bool m_isDeprecated        = false;
		uint64_t m_nInputHash   = 0x64AC3CB54A35888CLL;
		uint64_t m_nOutputHash  = 0x21E0FAAFE5CAF1E1LL;
		uint64_t m_nSettingHash = 0x6BDFB15B54B09F63LL;
		ITypeManager& m_TypeManager;
	};
} // namespace

namespace
{
	extern "C" G_MODULE_EXPORT void open_url_mensia_cb(GtkWidget* /*widget*/, gpointer /*data*/)
	{
#if defined(TARGET_OS_Windows) && defined(MENSIA_DISTRIBUTION)
		system("start http://mensiatech.com");
#endif
	}

	guint __g_idle_add__(GSourceFunc fpCallback, gpointer data, gint /*iPriority*/  = G_PRIORITY_DEFAULT_IDLE)
	{
		GSource* l_pSource = g_idle_source_new();
		g_source_set_priority(l_pSource, G_PRIORITY_LOW);
		g_source_set_callback(l_pSource, fpCallback, data, nullptr);
		return g_source_attach(l_pSource, nullptr);
	}

	guint __g_timeout_add__(const guint uiInterval, GSourceFunc fpCallback, gpointer data, gint /*iPriority*/  = G_PRIORITY_DEFAULT)
	{
		GSource* l_pSource = g_timeout_source_new(uiInterval);
		g_source_set_priority(l_pSource, G_PRIORITY_LOW);
		g_source_set_callback(l_pSource, fpCallback, data, nullptr);
		return g_source_attach(l_pSource, nullptr);
	}

	void drag_data_get_cb(GtkWidget* widget, GdkDragContext* pDragContex, GtkSelectionData* pSelectionData, const guint uiInfo, const guint uiT,
						  const gpointer data) { static_cast<CApplication*>(data)->dragDataGetCB(widget, pDragContex, pSelectionData, uiInfo, uiT); }

	void menu_undo_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->undoCB(); }

	void menu_redo_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->redoCB(); }

	void menu_focus_search_cb(GtkMenuItem* /*pMenuItem*/, gpointer data)
	{
		gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));
	}

	void menu_copy_selection_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->copySelectionCB(); }

	void menu_cut_selection_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->cutSelectionCB(); }

	void menu_paste_selection_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->pasteSelectionCB(); }

	void menu_delete_selection_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->deleteSelectionCB(); }

	void menu_preferences_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->preferencesCB(); }

	void menu_new_scenario_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->newScenarioCB(); }

	void menu_open_scenario_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->openScenarioCB(); }

	void menu_open_recent_scenario_cb(GtkMenuItem* pMenuItem, gpointer data)
	{
		const gchar* fileName = gtk_menu_item_get_label(pMenuItem);
		static_cast<CApplication*>(data)->openScenario(fileName);
	}

	void menu_save_scenario_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioCB(); }

	void menu_save_scenario_as_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioAsCB(); }

	void menu_restore_default_scenarios_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->restoreDefaultScenariosCB(); }

	void menu_close_scenario_cb(GtkMenuItem* /*pMenuItem*/, gpointer data)
	{
		static_cast<CApplication*>(data)->closeScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
	}

	void menu_quit_application_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { if (static_cast<CApplication*>(data)->quitApplicationCB()) { gtk_main_quit(); } }

	void menu_about_scenario_cb(GtkMenuItem* /*pMenuItem*/, gpointer data)
	{
		static_cast<CApplication*>(data)->aboutScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
	}

	void menu_about_openvibe_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->aboutOpenViBECB(); }

#if defined(TARGET_OS_Windows)
	void menu_about_link_clicked_cb(GtkAboutDialog* /*pAboutDialog*/, const gchar* linkPtr, gpointer data)
	{
		static_cast<CApplication*>(data)->aboutLinkClickedCB(linkPtr);
	}
#endif

	void menu_browse_documentation_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->browseDocumentationCB(); }

#ifdef MENSIA_DISTRIBUTION
	void menu_register_license_cb(::GtkMenuItem* /*pMenuItem*/, gpointer data)
	{
		static_cast<CApplication*>(data)->registerLicenseCB();
	}
#endif

	void menu_report_issue_cb(GtkMenuItem* /*pMenuItem*/, gpointer data) { static_cast<CApplication*>(data)->reportIssueCB(); }

	void menu_display_changelog_cb(GtkMenuItem* /*pMenuItem*/, gpointer data)
	{
		if (!static_cast<CApplication*>(data)->displayChangelogWhenAvailable())
		{
			const std::string applicationVersion = static_cast<CApplication*>(data)
												   ->m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString();
			GtkWidget* infoDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
														   "No boxes were added or updated in version %s of " DESIGNER_NAME ".",
														   applicationVersion != "${Application_Version}" ? applicationVersion.c_str() : ProjectVersion);
			gtk_window_set_title(GTK_WINDOW(infoDialog), "No new boxes");
			gtk_dialog_run(GTK_DIALOG(infoDialog));
			gtk_widget_destroy(infoDialog);
		}
	}

	void button_new_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->newScenarioCB(); }

	void button_open_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->openScenarioCB(); }

	void button_save_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioCB(); }

	void button_save_scenario_as_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->saveScenarioAsCB(); }

	void button_close_scenario_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CApplication*>(data)->closeScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
	}

	void button_undo_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->undoCB(); }

	void button_redo_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->redoCB(); }

#ifdef MENSIA_DISTRIBUTION
	void button_toggle_neurort_engine_configuration_cb(::GtkMenuItem* pMenuItem, gpointer data)
	{
		auto l_pApplication = static_cast<CApplication*>(data);

		l_pApplication->m_pArchwayHandlerGUI->toggleNeuroRTEngineConfigurationDialog(bool(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pMenuItem)) == TRUE));
	}
#endif

	void delete_designer_visualisation_cb(gpointer data) { static_cast<CApplication*>(data)->deleteDesignerVisualizationCB(); }

	void button_toggle_window_manager_cb(GtkToggleToolButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->toggleDesignerVisualizationCB(); }

	void button_comment_cb(GtkButton* /*button*/, CApplication* pApplication) { pApplication->addCommentCB(pApplication->getCurrentInterfacedScenario()); }

	void button_about_scenario_cb(GtkButton* /*button*/, gpointer data)
	{
		static_cast<CApplication*>(data)->aboutScenarioCB(static_cast<CApplication*>(data)->getCurrentInterfacedScenario());
	}

	void stop_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->stopScenarioCB(); }

	void play_pause_scenario_cb(GtkButton* button, gpointer data)
	{
		if (std::string(gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(button))) == GTK_STOCK_MEDIA_PLAY) { static_cast<CApplication*>(data)->playScenarioCB(); }
		else { static_cast<CApplication*>(data)->pauseScenarioCB(); }
	}

	void next_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->nextScenarioCB(); }

	void forward_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->forwardScenarioCB(); }

	void button_configure_current_scenario_settings_cb(GtkButton* /*button*/, CApplication* pApplication)
	{
		pApplication->configureScenarioSettingsCB(pApplication->getCurrentInterfacedScenario());
	}

	gboolean button_quit_application_cb(GtkWidget* /*widget*/, GdkEvent* /*pEvent*/, gpointer data)
	{
		if (static_cast<CApplication*>(data)->quitApplicationCB())
		{
			gtk_main_quit();
			return FALSE;
		}
		return TRUE;
	}

	gboolean window_state_changed_cb(GtkWidget* /*widget*/, GdkEventWindowState* pEvent, gpointer data)
	{
		if (pEvent->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
		{
			// window has changed from maximized to not maximized or the other way around
			static_cast<CApplication*>(data)->windowStateChangedCB((pEvent->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0);
		}
		return TRUE;
	}

	void log_level_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->logLevelCB(); }

	void cpu_usage_cb(GtkToggleButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->CPUUsageCB(); }

	gboolean change_current_scenario_cb(GtkNotebook* /*notebook*/, GtkNotebookPage* /*notebookPage*/, const guint pageNumber, gpointer data)
	{
		static_cast<CApplication*>(data)->changeCurrentScenario(int(pageNumber));
		return TRUE;
	}

	gboolean reorder_scenario_cb(GtkNotebook* /*notebook*/, GtkNotebookPage* /*notebookPage*/, const guint pageNumber, gpointer data)
	{
		static_cast<CApplication*>(data)->reorderCurrentScenario(int(pageNumber));
		return TRUE;
	}

	void box_algorithm_title_button_expand_cb(GtkButton* /*button*/, gpointer data)
	{
		gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-box_algorithm_tree")));
		gtk_notebook_set_current_page(
			GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-resource_notebook")), 0);
	}

	void box_algorithm_title_button_collapse_cb(GtkButton* /*button*/, gpointer data)
	{
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-box_algorithm_tree")));
		gtk_notebook_set_current_page(
			GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-resource_notebook")), 0);
	}

	void algorithm_title_button_expand_cb(GtkButton* /*button*/, gpointer data)
	{
		gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-algorithm_tree")));
		gtk_notebook_set_current_page(
			GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}

	void algorithm_title_button_collapse_cb(GtkButton* /*button*/, gpointer data)
	{
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-algorithm_tree")));
		gtk_notebook_set_current_page(
			GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(data)->m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}

	void clear_messages_cb(GtkButton* /*button*/, gpointer data) { static_cast<CLogListenerDesigner*>(data)->clearMessages(); }

	void add_scenario_input_cb(GtkButton* /*button*/, CApplication* pApplication) { pApplication->getCurrentInterfacedScenario()->addScenarioInputCB(); }

	void add_scenario_output_cb(GtkButton* /*button*/, CApplication* pApplication) { pApplication->getCurrentInterfacedScenario()->addScenarioOutputCB(); }

	void add_scenario_setting_cb(GtkButton* /*button*/, CApplication* pApplication) { pApplication->getCurrentInterfacedScenario()->addScenarioSettingCB(); }

	string strtoupper(string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), std::ptr_fun<int, int>(std::toupper));
		return str;
	}

	gboolean box_algorithm_search_func(GtkTreeModel* model, GtkTreeIter* iter, gpointer data)
	{
		auto* l_pApplication = static_cast<CApplication*>(data);
		/* Visible if row is non-empty and first column is "HI" */


		gboolean l_bVisible = false;
		gchar* l_sHaystackName;
		gchar* l_sHaystackDescription;
		gboolean l_bHaystackUnstable;

		gtk_tree_model_get(model, iter, Resource_StringName, &l_sHaystackName, Resource_StringShortDescription, &l_sHaystackDescription,
						   Resource_BooleanIsUnstable, &l_bHaystackUnstable, -1);

		// consider only leaf nodes which match the search term
		if (l_sHaystackName != nullptr && l_sHaystackDescription != nullptr)
		{
			if (!l_bHaystackUnstable
				&& (string::npos != strtoupper(l_sHaystackName).find(strtoupper(l_pApplication->m_sSearchTerm))
					|| string::npos != strtoupper(l_sHaystackDescription).find(strtoupper(l_pApplication->m_sSearchTerm))
					|| gtk_tree_model_iter_has_child(model, iter)))
			{
				//std::cout << "value : " << l_pApplication->m_sSearchTerm << "\n";
				l_bVisible = true;
			}

			g_free(l_sHaystackName);
			g_free(l_sHaystackDescription);
		}
		else { l_bVisible = true; }

		return l_bVisible;
	}

	gboolean box_algorithm_prune_empty_folders(GtkTreeModel* model, GtkTreeIter* iter, gpointer /*data*/)
	{
		gboolean l_bIsPlugin;
		gtk_tree_model_get(model, iter, Resource_BooleanIsPlugin, &l_bIsPlugin, -1);

		if (gtk_tree_model_iter_has_child(model, iter) || l_bIsPlugin) { return true; }

		return false;
	}

	gboolean do_refilter(CApplication* pApplication)
	{
		/*
		if (0 == strcmp(pApplication->m_sSearchTerm, ""))
		{
			// reattach the old model
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(pApplication->m_BoxAlgorithmTreeModel));
		}
		else
		*/
		{
			pApplication->m_pBoxAlgorithmTreeModelFilter  = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_BoxAlgorithmTreeModel), nullptr);
			pApplication->m_pBoxAlgorithmTreeModelFilter2 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter), nullptr);
			pApplication->m_pBoxAlgorithmTreeModelFilter3 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter2), nullptr);
			pApplication->m_pBoxAlgorithmTreeModelFilter4 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter3), nullptr);
			// detach the normal model from the treeview
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, nullptr);

			// clear the model

			// add a filtering function to the model
			gtk_tree_model_filter_set_visible_func(
				GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter), box_algorithm_search_func, pApplication, nullptr);
			gtk_tree_model_filter_set_visible_func(
				GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter2), box_algorithm_prune_empty_folders, pApplication, nullptr);
			gtk_tree_model_filter_set_visible_func(
				GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter3), box_algorithm_prune_empty_folders, pApplication, nullptr);
			gtk_tree_model_filter_set_visible_func(
				GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter4), box_algorithm_prune_empty_folders, pApplication, nullptr);

			// attach the model to the treeview
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter4));

			if (0 == strcmp(pApplication->m_sSearchTerm, "")) { gtk_tree_view_collapse_all(pApplication->m_pBoxAlgorithmTreeView); }
			else { gtk_tree_view_expand_all(pApplication->m_pBoxAlgorithmTreeView); }
		}

		pApplication->m_giFilterTimeout = 0;

		return false;
	}

	void queue_refilter(CApplication* pApplication)
	{
		if (pApplication->m_giFilterTimeout) { g_source_remove(pApplication->m_giFilterTimeout); }

		pApplication->m_giFilterTimeout = g_timeout_add(300, GSourceFunc(do_refilter), pApplication);
	}

	void refresh_search_cb(GtkEntry* pTextfield, CApplication* pApplication)
	{
		pApplication->m_sSearchTerm = gtk_entry_get_text(pTextfield);

		queue_refilter(pApplication);
	}

	void refresh_search_no_data_cb(GtkToggleButton* /*pToggleButton*/, CApplication* pApplication)
	{
		pApplication->m_sSearchTerm = gtk_entry_get_text(
			GTK_ENTRY(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));

		queue_refilter(pApplication);
	}

	gboolean searchbox_select_all_cb(GtkWidget* widget, GdkEvent* /*pEvent*/, CApplication* /*pApplication*/)
	{
		// we select the current search
		gtk_widget_grab_focus(widget); // we must grab or selection wont work. It also triggers the other CBs.
		gtk_editable_select_region(GTK_EDITABLE(widget), 0, -1);
		return false;
	}

	gboolean searchbox_focus_in_cb(GtkWidget* /*widget*/, GdkEvent* /*pEvent*/, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), false);
		return false;
	}

	gboolean searchbox_focus_out_cb(GtkWidget* widget, GdkEvent* /*pEvent*/, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), true);
		gtk_editable_select_region(GTK_EDITABLE(widget), 0, 0);
		return false;
	}

#if defined TARGET_OS_Windows
	void about_newversion_button_display_changelog_cb(GtkButton* /*button*/, gpointer /*data*/)
	{
		System::WindowsUtilities::utf16CompliantShellExecute(nullptr, "open", (OVD_README_File).toASCIIString(), nullptr, nullptr, SHOW_OPENWINDOW);
	}
#endif

	gboolean idle_application_loop(gpointer data)
	{
		auto* l_pApplication = static_cast<CApplication*>(data);
#ifdef MENSIA_DISTRIBUTION
		if (l_pApplication->m_pArchwayHandler->isEngineStarted()) { l_pApplication->m_pArchwayHandler->loopEngine(); }
#endif

		CInterfacedScenario* currentInterfacedScenario = l_pApplication->getCurrentInterfacedScenario();
		if (currentInterfacedScenario)
		{
			if (l_pApplication->getPlayer() && currentInterfacedScenario->m_PlayerStatus != l_pApplication->getPlayer()->getStatus())
			{
				switch (l_pApplication->getPlayer()->getStatus())
				{
					case PlayerStatus_Stop:
						gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_stop")), "clicked");
						break;
					case PlayerStatus_Pause:
						while (currentInterfacedScenario->m_PlayerStatus != PlayerStatus_Pause) gtk_signal_emit_by_name(
							GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_play_pause")), "clicked");
						break;
					case PlayerStatus_Step:
						break;
					case PlayerStatus_Play:
						while (currentInterfacedScenario->m_PlayerStatus != PlayerStatus_Play) gtk_signal_emit_by_name(
							GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_play_pause")), "clicked");
						break;
					case PlayerStatus_Forward:
						gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_forward")), "clicked");
						break;
					default:
						std::cout << "unhandled player status : " << l_pApplication->getPlayer()->getStatus() << " :(\n";
						break;
				}
			}
			else
			{
				const double time = (currentInterfacedScenario->m_Player
										 ? TimeArithmetics::timeToSeconds(currentInterfacedScenario->m_Player->getCurrentSimulatedTime()) : 0);
				if (l_pApplication->m_lastTimeRefresh != time)
				{
					l_pApplication->m_lastTimeRefresh = uint64_t(time);

					const uint32_t milli   = (uint32_t(time * 1000) % 1000);
					const uint32_t seconds = uint32_t(time) % 60;
					const uint32_t minutes = (uint32_t(time) / 60) % 60;
					const uint32_t hours   = ((uint32_t(time) / 60) / 60);

					const double CPUUsage = (currentInterfacedScenario->m_Player ? currentInterfacedScenario->m_Player->getCPUUsage() : 0);

					char l_sTime[1024];
					if (hours) { sprintf(l_sTime, "Time : %02dh %02dm %02ds %03dms", hours, minutes, seconds, milli); }
					else if (minutes) { sprintf(l_sTime, "Time : %02dm %02ds %03dms", minutes, seconds, milli); }
					else if (seconds) { sprintf(l_sTime, "Time : %02ds %03dms", seconds, milli); }
					else { sprintf(l_sTime, "Time : %03dms", milli); }

					gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-label_current_time")), l_sTime);

					char l_sCPU[1024];
					sprintf(l_sCPU, "%3.01f%%", CPUUsage);

					gtk_progress_bar_set_fraction(
						GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), CPUUsage * .01);
					gtk_progress_bar_set_text(
						GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), l_sCPU);
					if (currentInterfacedScenario->m_Player && currentInterfacedScenario->m_DebugCPUUsage)
					{
						// redraws scenario
						currentInterfacedScenario->redraw();
					}
				}
			}
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-label_current_time")), "Time : 000ms");
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), "");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), 0);
		}

		if (!l_pApplication->hasRunningScenario()) { System::Time::sleep(50); }

		return TRUE;
	}

	gboolean idle_scenario_loop(gpointer data)
	{
		auto* interfacedScenario   = static_cast<CInterfacedScenario*>(data);
		const uint64_t currentTime = System::Time::zgetTime();
		if (interfacedScenario->m_LastLoopTime == uint64_t(-1)) { interfacedScenario->m_LastLoopTime = currentTime; }
		interfacedScenario->m_Player->setFastForwardMaximumFactor(gtk_spin_button_get_value(interfacedScenario->m_Application.m_pFastForwardFactor));
		if (!interfacedScenario->m_Player->loop(currentTime - interfacedScenario->m_LastLoopTime))
		{
			interfacedScenario->m_Application.stopInterfacedScenarioAndReleasePlayer(interfacedScenario);
		}
		interfacedScenario->m_LastLoopTime = currentTime;
		return TRUE;
	}

	gboolean timeout_application_loop(gpointer data)
	{
		auto* l_pApplication = static_cast<CApplication*>(data);
		if (!l_pApplication->hasRunningScenario() && l_pApplication->m_eCommandLineFlags & CommandLineFlag_NoGui)
		{
			l_pApplication->quitApplicationCB();
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
	gboolean receiveSecondInstanceMessage(gpointer data)
	{
		try
		{	// Open or create ensures that
			boost::interprocess::named_mutex mutex(boost::interprocess::open_or_create, MUTEX_NAME);
			{
				boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex);
				CApplication* l_pApplication = static_cast<CApplication*>(data);
				//Tries to open a message, if fails, go to catch
				boost::interprocess::message_queue message(boost::interprocess::open_only, MESSAGE_NAME);
				l_pApplication->m_kernelCtx.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- A message was detected \n";

				// Whatever contains the message the first instance should try to take the focus
				gtk_window_present(GTK_WINDOW(l_pApplication->m_pMainWindow));
				size_t recvd_size;
				uint32_t priority = 0;
				char buffer[2048];
				if (message.try_receive(&buffer, sizeof(buffer), recvd_size, priority))
				{
					boost::interprocess::message_queue::remove(MESSAGE_NAME);

					int l_iMode = 0;
					char l_sScenarioPath[2048];
					char* l_sMessage = strtok(buffer, ";");
					while (l_sMessage != nullptr)
					{
						sscanf(l_sMessage, "%1d : <%2048[^>]> ", &l_iMode, &l_sScenarioPath);
						switch (l_iMode)
						{
							case CommandLineFlag_Open:
								l_pApplication->openScenario(l_sScenarioPath);
								break;
							case CommandLineFlag_Play:
								if (l_pApplication->openScenario(l_sScenarioPath)) { l_pApplication->playScenarioCB(); }
								break;
							case CommandLineFlag_PlayFast:
								if (l_pApplication->openScenario(l_sScenarioPath)) { l_pApplication->forwardScenarioCB(); }
								break;
							default: break;
						}
						l_iMode    = 0;
						l_sMessage = strtok(nullptr, ";");
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

	void zoom_in_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->zoomInCB(); }
	void zoom_out_scenario_cb(GtkButton* /*button*/, gpointer data) { static_cast<CApplication*>(data)->zoomOutCB(); }

	void spinner_zoom_changed_cb(GtkSpinButton* button, gpointer data)
	{
		static_cast<CApplication*>(data)->spinnerZoomChangedCB(uint32_t(gtk_spin_button_get_value(button)));
	}
} // namespace

static GtkTargetEntry g_vTargetEntry[] = { { static_cast<gchar*>("STRING"), 0, 0 }, { static_cast<gchar*>("text/plain"), 0, 0 } };

CApplication::CApplication(const IKernelContext& ctx) : m_kernelCtx(ctx)
{
	m_pPluginManager   = &m_kernelCtx.getPluginManager();
	m_pScenarioManager = &m_kernelCtx.getScenarioManager();
	m_pScenarioManager->registerScenarioImporter(OVD_ScenarioImportContext_OpenScenario, ".xml", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	m_pScenarioManager->registerScenarioImporter(OVD_ScenarioImportContext_OpenScenario, ".mxs", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	m_pScenarioManager->registerScenarioImporter(OVD_ScenarioImportContext_OpenScenario, ".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	m_pScenarioManager->registerScenarioExporter(OVD_ScenarioExportContext_SaveScenario, ".xml", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
	m_pScenarioManager->registerScenarioExporter(OVD_ScenarioExportContext_SaveScenario, ".mxs", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
	m_pScenarioManager->registerScenarioExporter(OVD_ScenarioExportContext_SaveMetabox, ".xml", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
	m_pScenarioManager->registerScenarioExporter(OVD_ScenarioExportContext_SaveMetabox, ".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioExporter);

	m_pVisualizationManager = new CVisualizationManager(m_kernelCtx);
	m_VisualizationCtx  = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(m_kernelCtx.getPluginManager().createPluginObject(OVP_ClassId_Plugin_VisualizationCtx));
	m_VisualizationCtx->setManager(m_pVisualizationManager);
	m_pLogListenerDesigner = nullptr;

	m_kernelCtx.getConfigurationManager().createConfigurationToken("Player_ScenarioDirectory", "");
	m_kernelCtx.getConfigurationManager().createConfigurationToken("__volatile_ScenarioDir", "");

#ifdef MENSIA_DISTRIBUTION
	m_pArchwayHandler = new Mensia::CArchwayHandler(ctx);
	m_pArchwayHandlerGUI = new Mensia::CArchwayHandlerGUI(*m_pArchwayHandler, this);
#endif
}

CApplication::~CApplication()
{
	if (m_pBuilderInterface)
	{
		m_kernelCtx.getLogManager().removeListener(m_pLogListenerDesigner);
		// @FIXME this likely still does not deallocate the actual widgets allocated by add_from_file
		g_object_unref(G_OBJECT(m_pBuilderInterface));
		m_pBuilderInterface = nullptr;
	}

	m_kernelCtx.getPluginManager().releasePluginObject(m_VisualizationCtx);

#ifdef MENSIA_DISTRIBUTION
	delete m_pArchwayHandlerGUI;
	delete m_pArchwayHandler;
#endif
}

void CApplication::initialize(const ECommandLineFlag eCommandLineFlags)
{
	m_eCommandLineFlags = eCommandLineFlags;
	m_sSearchTerm       = "";

	// Load metaboxes from metabox path
	m_kernelCtx.getMetaboxManager().addMetaboxesFromFiles(m_kernelCtx.getConfigurationManager().expand("${Kernel_Metabox}"));

	// Copy recursively default scenario directory to the default working directory if not exists
	const CString defaultWorkingDirectory   = m_kernelCtx.getConfigurationManager().expand(OVD_WORKING_SCENARIOS_PATH);
	const CString defaultScenariosDirectory = m_kernelCtx.getConfigurationManager().expand(OVD_SCENARIOS_PATH);
	if (!FS::Files::directoryExists(defaultWorkingDirectory) && FS::Files::directoryExists(defaultScenariosDirectory))
	{
		if (!FS::Files::copyDirectory(defaultScenariosDirectory, defaultWorkingDirectory))
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Could not create " << defaultWorkingDirectory << " folder\n";
		}
	}




	// Prepares scenario clipboard
	CIdentifier l_oClipboardScenarioID;
	if (m_pScenarioManager->createScenario(l_oClipboardScenarioID))
	{
		m_pClipboardScenario = &m_pScenarioManager->getScenario(l_oClipboardScenarioID);
	}

	m_pBuilderInterface = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "openvibe", nullptr);
	gtk_builder_add_from_file(m_pBuilderInterface, OVD_GUI_File, nullptr);
	gtk_builder_connect_signals(m_pBuilderInterface, nullptr);

	std::string applicationVersion = m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString();
	if (applicationVersion == "${Application_Version}")
	{
		applicationVersion = m_kernelCtx.getConfigurationManager().expand("${ProjectVersion_Major}.${ProjectVersion_Minor}.${ProjectVersion_Patch}").toASCIIString();
	}
	const std::string defaultWindowTitle = BRAND_NAME " " DESIGNER_NAME " " + applicationVersion;

	m_pMainWindow = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe"));
	gtk_window_set_title(GTK_WINDOW(m_pMainWindow), defaultWindowTitle.c_str());
	gtk_menu_item_set_label(
		GTK_MENU_ITEM(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_display_changelog")),
		("What's new in " + applicationVersion + " version of " + BRAND_NAME " " DESIGNER_NAME).c_str());

	// Catch delete events when close button is clicked
	g_signal_connect(m_pMainWindow, "delete_event", G_CALLBACK(button_quit_application_cb), this);
	// be notified on maximize/minimize events
	g_signal_connect(m_pMainWindow, "window-state-event", G_CALLBACK(window_state_changed_cb), this);

	// Connects menu actions
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_undo")), "activate", G_CALLBACK(menu_undo_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_redo")), "activate", G_CALLBACK(menu_redo_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_focus_search")), "activate", G_CALLBACK(menu_focus_search_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_copy")), "activate", G_CALLBACK(menu_copy_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_cut")), "activate", G_CALLBACK(menu_cut_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_paste")), "activate", G_CALLBACK(menu_paste_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_delete")), "activate", G_CALLBACK(menu_delete_selection_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_preferences")), "activate", G_CALLBACK(menu_preferences_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_new")), "activate", G_CALLBACK(menu_new_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_open")), "activate", G_CALLBACK(menu_open_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save")), "activate", G_CALLBACK(menu_save_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save_as")), "activate", G_CALLBACK(menu_save_scenario_as_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_close")), "activate", G_CALLBACK(menu_close_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_quit")), "activate", G_CALLBACK(menu_quit_application_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_restore_default_scenarios")), "activate",
					 G_CALLBACK(menu_restore_default_scenarios_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_about")), "activate", G_CALLBACK(menu_about_openvibe_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_scenario_about")), "activate", G_CALLBACK(menu_about_scenario_cb),
					 this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_documentation")), "activate", G_CALLBACK(menu_browse_documentation_cb),
					 this);
#ifdef MENSIA_DISTRIBUTION
	if (FS::Files::fileExists(Directories::getBinDir() + "/mensia-flexnet-activation.exe"))
	{
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_register_license")), "activate", G_CALLBACK(menu_register_license_cb), this);
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET((gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_register_license"))));
	}
#else
	gtk_widget_hide(GTK_WIDGET((gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_register_license"))));
#endif

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_issue_report")), "activate", G_CALLBACK(menu_report_issue_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_display_changelog")), "activate",
					 G_CALLBACK(menu_display_changelog_cb), this);

	// g_signal_connect(G_OBJECT(gtk_builder_get_object(m_builderInterface, "openvibe-menu_test")),        "activate", G_CALLBACK(menu_test_cb),               this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_new")), "clicked", G_CALLBACK(button_new_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_open")), "clicked", G_CALLBACK(button_open_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save")), "clicked", G_CALLBACK(button_save_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save_as")), "clicked", G_CALLBACK(button_save_scenario_as_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_close")), "clicked", G_CALLBACK(button_close_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_undo")), "clicked", G_CALLBACK(button_undo_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_redo")), "clicked", G_CALLBACK(button_redo_cb), this);


	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_log_level")), "clicked", G_CALLBACK(log_level_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), "toggled",
					 G_CALLBACK(button_toggle_window_manager_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")), "clicked", G_CALLBACK(button_comment_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), "clicked", G_CALLBACK(button_about_scenario_cb),
					 this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), "clicked", G_CALLBACK(stop_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), "clicked", G_CALLBACK(play_pause_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), "clicked", G_CALLBACK(next_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), "clicked", G_CALLBACK(forward_scenario_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_title_button_expand")), "clicked",
					 G_CALLBACK(box_algorithm_title_button_expand_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_title_button_collapse")), "clicked",
					 G_CALLBACK(box_algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_button_expand")), "clicked",
					 G_CALLBACK(algorithm_title_button_expand_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_button_collapse")), "clicked",
					 G_CALLBACK(algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "icon-press",
					 G_CALLBACK(searchbox_select_all_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "changed", G_CALLBACK(refresh_search_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "focus-in-event",
					 G_CALLBACK(searchbox_focus_in_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "focus-out-event",
					 G_CALLBACK(searchbox_focus_out_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), "clicked",
					 G_CALLBACK(button_configure_current_scenario_settings_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_zoomin")), "clicked", G_CALLBACK(zoom_in_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_zoomout")), "clicked", G_CALLBACK(zoom_out_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner")), "value-changed", G_CALLBACK(spinner_zoom_changed_cb),
					 this);
#ifdef MENSIA_DISTRIBUTION
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration")), "clicked", G_CALLBACK(button_toggle_neurort_engine_configuration_cb), this);
	m_pArchwayHandlerGUI->m_ButtonOpenEngineConfigurationDialog = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration"));
#endif
	// Prepares fast forward feature
	const double fastForwardFactor = m_kernelCtx.getConfigurationManager().expandAsFloat("${Designer_FastForwardFactor}", -1);
	m_pFastForwardFactor           = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-spinbutton_fast-forward-factor"));
	if (fastForwardFactor == -1) { gtk_spin_button_set_value(m_pFastForwardFactor, 100); }
	else { gtk_spin_button_set_value(m_pFastForwardFactor, fastForwardFactor); }

#if defined(TARGET_OS_Windows)
#if GTK_CHECK_VERSION(2, 24, 0)
	// expect it to work */
#else
	gtk_about_dialog_set_url_hook((GtkAboutDialogActivateLinkFunc)menu_about_link_clicked_cb, this, nullptr);
#endif
#endif

	__g_idle_add__(idle_application_loop, this);
	__g_timeout_add__(1000, timeout_application_loop, this);
#ifdef NDEBUG
	__g_timeout_add__(100, receiveSecondInstanceMessage, this);
#endif

	// Prepares main notebooks
	m_pScenarioNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_notebook"));
	// optional behavior: vertically stacked scenarios (filename trimming mandatory in that case).
	if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ScenarioTabsVerticalStack}", false))
	{
		gtk_notebook_set_tab_pos(m_pScenarioNotebook, GTK_POS_LEFT);
	}

	g_signal_connect(G_OBJECT(m_pScenarioNotebook), "switch-page", G_CALLBACK(change_current_scenario_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioNotebook), "page-reordered", G_CALLBACK(reorder_scenario_cb), this);
	m_pResourceNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-resource_notebook"));

	// Creates an empty scnenario
	gtk_notebook_remove_page(m_pScenarioNotebook, 0);

	// Initialize menu open recent
	m_MenuOpenRecent = GTK_CONTAINER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_recent_content"));

	//newScenarioCB();
	{
		// Prepares box algorithm view
		m_pBoxAlgorithmTreeView                  = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_tree"));
		GtkTreeViewColumn* l_pTreeViewColumnName = gtk_tree_view_column_new();
		GtkTreeViewColumn* l_pTreeViewColumnDesc = gtk_tree_view_column_new();
		GtkCellRenderer* l_pCellRendererIcon     = gtk_cell_renderer_pixbuf_new();
		GtkCellRenderer* l_pCellRendererName     = gtk_cell_renderer_text_new();
		GtkCellRenderer* l_pCellRendererDesc     = gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Name");
		gtk_tree_view_column_set_title(l_pTreeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnDesc, l_pCellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", Resource_StringStockIcon, nullptr);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, "font",
											Resource_StringFont, "background", Resource_BackGroundColor, nullptr);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnDesc, l_pCellRendererDesc, "text", Resource_StringShortDescription, "foreground",
											Resource_StringColor, "background", Resource_BackGroundColor, nullptr);
		gtk_tree_view_column_set_sizing(l_pTreeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sizing(l_pTreeViewColumnDesc, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnName, FALSE);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnDesc, FALSE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnName, TRUE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnDesc, TRUE);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnName, 64);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnDesc, 64);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnName, 256);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnDesc, 512);
		gtk_tree_view_append_column(m_pBoxAlgorithmTreeView, l_pTreeViewColumnName);
		gtk_tree_view_append_column(m_pBoxAlgorithmTreeView, l_pTreeViewColumnDesc);

		// Prepares box algorithm model
		m_BoxAlgorithmTreeModel = gtk_tree_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
													  G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING);

		// Tree Storage for the searches
		gtk_tree_view_set_model(m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(m_BoxAlgorithmTreeModel));
	}

	{
		// Prepares algorithm view
		m_pAlgorithmTreeView                     = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_tree"));
		GtkTreeViewColumn* l_pTreeViewColumnName = gtk_tree_view_column_new();
		GtkTreeViewColumn* l_pTreeViewColumnDesc = gtk_tree_view_column_new();
		GtkCellRenderer* l_pCellRendererIcon     = gtk_cell_renderer_pixbuf_new();
		GtkCellRenderer* l_pCellRendererName     = gtk_cell_renderer_text_new();
		GtkCellRenderer* l_pCellRendererDesc     = gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Name");
		gtk_tree_view_column_set_title(l_pTreeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnDesc, l_pCellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", Resource_StringStockIcon, nullptr);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor,
											nullptr);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnDesc, l_pCellRendererDesc, "text", Resource_StringShortDescription, "foreground",
											Resource_StringColor, nullptr);

		gtk_tree_view_column_set_sizing(l_pTreeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sizing(l_pTreeViewColumnDesc, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnName, FALSE);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnDesc, FALSE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnName, TRUE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnDesc, TRUE);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnName, 64);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnDesc, 64);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnName, 256);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnDesc, 512);
		gtk_tree_view_append_column(m_pAlgorithmTreeView, l_pTreeViewColumnName);
		gtk_tree_view_append_column(m_pAlgorithmTreeView, l_pTreeViewColumnDesc);

		// Prepares algorithm model
		m_pAlgorithmTreeModel = gtk_tree_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN,
												   G_TYPE_BOOLEAN, G_TYPE_STRING);
		gtk_tree_view_set_model(m_pAlgorithmTreeView, GTK_TREE_MODEL(m_pAlgorithmTreeModel));
	}

	m_pConfigureSettingsAddSettingButton = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "dialog_scenario_configuration-button_add_setting"));
	g_signal_connect(G_OBJECT(m_pConfigureSettingsAddSettingButton), "clicked", G_CALLBACK(add_scenario_setting_cb), this);
	// Set up the UI for adding Inputs and Outputs to the scenario

	GtkWidget* l_pScenarioLinksVBox = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_links_vbox"));

	m_pTableInputs  = gtk_table_new(1, 3, FALSE);
	m_pTableOutputs = gtk_table_new(1, 3, FALSE);

	GtkWidget* l_pScrolledWindowInputs = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(l_pScrolledWindowInputs), GTK_WIDGET(m_pTableInputs));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l_pScrolledWindowInputs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* l_pScrolledWindowOutputs = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(l_pScrolledWindowOutputs), GTK_WIDGET(m_pTableOutputs));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l_pScrolledWindowOutputs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* l_pAddInputButton  = gtk_button_new_with_label("Add Input");
	GtkWidget* l_pAddOutputButton = gtk_button_new_with_label("Add Output");

	g_signal_connect(G_OBJECT(l_pAddInputButton), "clicked", G_CALLBACK(add_scenario_input_cb), this);
	g_signal_connect(G_OBJECT(l_pAddOutputButton), "clicked", G_CALLBACK(add_scenario_output_cb), this);

	gtk_box_pack_start(GTK_BOX(l_pScenarioLinksVBox), gtk_label_new("Inputs"), FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(l_pScenarioLinksVBox), l_pScrolledWindowInputs, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(l_pScenarioLinksVBox), l_pAddInputButton, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(l_pScenarioLinksVBox), gtk_label_new("Outputs"), FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(l_pScenarioLinksVBox), l_pScrolledWindowOutputs, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(l_pScenarioLinksVBox), l_pAddOutputButton, FALSE, FALSE, 4);

	gtk_widget_show_all(l_pScenarioLinksVBox);

	// Prepares drag& drop for box creation
	gtk_drag_source_set(GTK_WIDGET(m_pBoxAlgorithmTreeView), GDK_BUTTON1_MASK, g_vTargetEntry, sizeof(g_vTargetEntry) / sizeof(GtkTargetEntry),
						GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_pBoxAlgorithmTreeView), "drag_data_get", G_CALLBACK(drag_data_get_cb), this);

	// Shows main window
	gtk_builder_connect_signals(m_pBuilderInterface, nullptr);
	m_isMaximized = false;

	const int height = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${Designer_EditorSizeHeight}"));
	const int width  = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${Designer_EditorSizeWidth}"));
	if (height > 0 && width > 0) { gtk_window_resize(GTK_WINDOW(m_pMainWindow), width, height); }

	if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_FullscreenEditor}")) { gtk_window_maximize(GTK_WINDOW(m_pMainWindow)); }

	const int panedPosition = int(m_kernelCtx.getConfigurationManager().expandAsInteger("${Designer_EditorPanedPosition}"));
	if (panedPosition > 0) { gtk_paned_set_position(GTK_PANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container")), panedPosition); }

	GtkNotebook* sidebar = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-resource_notebook"));


	// List the notebook pages, cycle through them in reverse so we can remove pages without modifying indexes
	for (int notebookIdx = gtk_notebook_get_n_pages(sidebar) - 1; notebookIdx >= 0; notebookIdx--)
	{
		GtkWidget* tabWidget = gtk_notebook_get_nth_page(sidebar, notebookIdx);
		GtkWidget* tabLabel  = gtk_notebook_get_tab_label(sidebar, tabWidget);
		if (!m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ShowAlgorithms}"))
		{
			if (tabLabel == GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_container")))
			{
				gtk_notebook_remove_page(sidebar, notebookIdx);
			}
		}
	}

	// gtk_window_set_icon_name(GTK_WINDOW(m_pMainWindow), "ov-logo");
	gtk_window_set_icon_from_file(GTK_WINDOW(m_pMainWindow), Directories::getDataDir() + "/applications/designer/designer.ico", nullptr);
	gtk_window_set_default_icon_from_file(Directories::getDataDir() + "/applications/designer/designer.ico", nullptr);

	if (!(m_eCommandLineFlags & CommandLineFlag_NoManageSession))
	{
		CIdentifier l_oTokenID;
		char varName[1024];
		unsigned i = 0;
		do
		{
			sprintf(varName, "Designer_LastScenarioFilename_%03u", ++i);
			if ((l_oTokenID = m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier(varName)) != OV_UndefinedIdentifier)
			{
				CString fileName;
				fileName = m_kernelCtx.getConfigurationManager().getConfigurationTokenValue(l_oTokenID);
				fileName = m_kernelCtx.getConfigurationManager().expand(fileName);
				m_kernelCtx.getLogManager() << LogLevel_Trace << "Restoring scenario [" << fileName << "]\n";
				if (!this->openScenario(fileName.toASCIIString()))
				{
					m_kernelCtx.getLogManager() << LogLevel_ImportantWarning << "Failed to restore scenario [" << fileName << "]\n";
				}
			}
		} while (l_oTokenID != OV_UndefinedIdentifier);
	}

	CIdentifier l_oTokenID;
	char varName[1024];
	unsigned i = 0;
	do
	{
		sprintf(varName, "Designer_RecentScenario_%03u", ++i);
		if ((l_oTokenID = m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier(varName)) != OV_UndefinedIdentifier)
		{
			CString fileName;
			fileName = m_kernelCtx.getConfigurationManager().getConfigurationTokenValue(l_oTokenID);
			fileName = m_kernelCtx.getConfigurationManager().expand(fileName);

			GtkWidget* newRecentItem = gtk_image_menu_item_new_with_label(fileName.toASCIIString());
			g_signal_connect(G_OBJECT(newRecentItem), "activate", G_CALLBACK(menu_open_recent_scenario_cb), this);
			gtk_menu_shell_append(GTK_MENU_SHELL(m_MenuOpenRecent), newRecentItem);
			gtk_widget_show(newRecentItem);
			m_RecentScenarios.push_back(newRecentItem);
		}
	} while (l_oTokenID != OV_UndefinedIdentifier);

	refresh_search_no_data_cb(nullptr, this);
	// Add the designer log listener
	const CString logLevel = m_kernelCtx.getConfigurationManager().expand("${Kernel_ConsoleLogLevel}");
	string value(logLevel.toASCIIString());
	transform(value.begin(), value.end(), value.begin(), ::to_lower<std::string::value_type>);
	ELogLevel l_eLogLevel = LogLevel_Debug;
	if (value == "debug") { l_eLogLevel = LogLevel_Debug; }
	if (value == "benchmarking / profiling") { l_eLogLevel = LogLevel_Benchmark; }
	if (value == "trace") { l_eLogLevel = LogLevel_Trace; }
	if (value == "information") { l_eLogLevel = LogLevel_Info; }
	if (value == "warning") { l_eLogLevel = LogLevel_Warning; }
	if (value == "important warning") { l_eLogLevel = LogLevel_ImportantWarning; }
	if (value == "error") { l_eLogLevel = LogLevel_Error; }
	if (value == "fatal error") { l_eLogLevel = LogLevel_Fatal; }

	switch (l_eLogLevel)
	{
		case LogLevel_Debug:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")), true);
		case LogLevel_Benchmark:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")), true);
		case LogLevel_Trace:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")), true);
		case LogLevel_Info:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")), true);
		case LogLevel_Warning:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")), true);
		case LogLevel_ImportantWarning:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")), true);
		case LogLevel_Error:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")), true);
		case LogLevel_Fatal:
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")), true);
		default:
			break;
	}

	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")), m_kernelCtx.getLogManager().isActive(LogLevel_Debug));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")), m_kernelCtx.getLogManager().isActive(LogLevel_Benchmark));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")), m_kernelCtx.getLogManager().isActive(LogLevel_Trace));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")), m_kernelCtx.getLogManager().isActive(LogLevel_Info));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")), m_kernelCtx.getLogManager().isActive(LogLevel_Warning));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")),
		m_kernelCtx.getLogManager().isActive(LogLevel_ImportantWarning));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")), m_kernelCtx.getLogManager().isActive(LogLevel_Error));
	gtk_widget_set_sensitive(
		GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")), m_kernelCtx.getLogManager().isActive(LogLevel_Fatal));

	if (!(m_eCommandLineFlags & CommandLineFlag_NoGui))
	{
		m_pLogListenerDesigner                   = new CLogListenerDesigner(m_kernelCtx, m_pBuilderInterface);
		m_pLogListenerDesigner->m_CenterOnBoxFun = [this](CIdentifier& id) { this->getCurrentInterfacedScenario()->centerOnBox(id); };
		m_kernelCtx.getLogManager().addListener(m_pLogListenerDesigner);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_clear")), "clicked", G_CALLBACK(clear_messages_cb),
						 m_pLogListenerDesigner);
		gtk_widget_show(m_pMainWindow);
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
		|| (lastUsedVersionMajor == 0 && lastUsedVersionMinor == 0 && lastUsedVersionPatch == 0)) { m_isNewVersion = true; }

	std::string defaultURLBaseString = std::string(m_kernelCtx.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}"));
#ifdef MENSIA_DISTRIBUTION
	if (m_pArchwayHandler->initialize() == Mensia::EEngineInitialisationStatus::NotAvailable)
	{
		gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration")));
	}
#else
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration")));
#endif
}

bool CApplication::displayChangelogWhenAvailable()
{
	// If last version used is ulterior as current version, and at least one box was added/updated, show the list
	if (!m_NewBoxes.empty() || !m_UpdatedBoxes.empty())
	{
		GtkBuilder* builder = gtk_builder_new();
		gtk_builder_add_from_file(builder, OVD_GUI_File, nullptr);

		GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(builder, "aboutdialog-newversion"));
		gtk_window_set_title(GTK_WINDOW(l_pDialog), "Changelog");


		std::string projectVersion = m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString();
		projectVersion             = (projectVersion != "${Application_Version}") ? projectVersion
										 : m_kernelCtx.getConfigurationManager().expand(
															   "${ProjectVersion_Major}.${ProjectVersion_Minor}.${ProjectVersion_Patch}").
														   toASCIIString();

		gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(l_pDialog), projectVersion.c_str());

		std::string labelNewBoxesList = "<big><b>Changes in version " + projectVersion + " of the software:</b></big>";
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
		GtkLabel* l_pLabel = GTK_LABEL(gtk_builder_get_object(builder, "label-newversion"));
		gtk_label_set_markup(l_pLabel, labelNewBoxesList.c_str());
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		if (m_isNewVersion)
		{
			gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "(New)");
			gtk_window_set_focus(GTK_WINDOW(m_pMainWindow), GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));
		}

		gtk_widget_destroy(l_pDialog);
		g_object_unref(builder);
	}
	else { return false; }
	return true;
}

bool CApplication::openScenario(const char* sFileName)
{
	// Prevent opening twice the same scenario
	for (uint32_t i = 0; i < m_Scenarios.size(); ++i)
	{
		const auto interfacedScenario = m_Scenarios[i];
		if (interfacedScenario->m_Filename == std::string(sFileName))
		{
			gtk_notebook_set_current_page(m_pScenarioNotebook, i);
			return true;
		}
	}

	CIdentifier scenarioID;
	if (m_pScenarioManager->importScenarioFromFile(scenarioID, OVD_ScenarioImportContext_OpenScenario, sFileName))
	{
		// Closes first unnamed scenario
		if (m_Scenarios.size() == 1)
		{
			if (!m_Scenarios[0]->m_HasBeenModified && !m_Scenarios[0]->m_HasFileName)
			{
				const CIdentifier tmp = m_Scenarios[0]->m_ScenarioID;
				delete m_Scenarios[0];
				m_pScenarioManager->releaseScenario(tmp);
				m_Scenarios.clear();
			}
		}

		IScenario& scenario = m_pScenarioManager->getScenario(scenarioID);

		// Creates interfaced scenario
		CInterfacedScenario* interfacedScenario = new CInterfacedScenario(m_kernelCtx, *this, scenario, scenarioID, *m_pScenarioNotebook, OVD_GUI_File,
																		  OVD_GUI_Settings_File);

		// Deserialize the visualization tree from the scenario metadata, if it exists

		// Find the VisualizationTree metadata
		IMetadata* vizTreeMetadata = nullptr;
		CIdentifier metadataID     = OV_UndefinedIdentifier;
		while ((metadataID = scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
		{
			vizTreeMetadata = scenario.getMetadataDetails(metadataID);
			if (vizTreeMetadata && vizTreeMetadata->getType() == OVVIZ_MetadataIdentifier_VisualizationTree) { break; }
		}
		OpenViBEVisualizationToolkit::IVisualizationTree* vizTree = interfacedScenario->m_Tree;
		if (vizTreeMetadata && vizTree) { vizTree->deserialize(vizTreeMetadata->getData()); }

		CIdentifier l_oVisualizationWidgetID;

		// Ensure visualization widgets contained in the scenario (if any) appear in the window manager
		//  even when the VisualizationTree section of a scenario file is missing, erroneous or deprecated

		// no visualization widget was added to visualization tree : ensure there aren't any in scenario
		CIdentifier boxID;
		while ((boxID = scenario.getNextBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
		{
			if (!vizTree->getVisualizationWidgetFromBoxIdentifier(boxID))
			{
				const IBox* box                           = scenario.getBoxDetails(boxID);
				const IPluginObjectDesc* boxAlgorithmDesc = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(box->getAlgorithmClassIdentifier());
				if (boxAlgorithmDesc && boxAlgorithmDesc->hasFunctionality(OVD_Functionality_Visualization))
				{
					//a visualization widget was found in scenario : manually add it to visualization tree
					vizTree->addVisualizationWidget(l_oVisualizationWidgetID, box->getName(),
													OpenViBEVisualizationToolkit::VisualizationWidget_VisualizationBox, OV_UndefinedIdentifier,
													0, box->getIdentifier(), 0, OV_UndefinedIdentifier);
				}
			}
		}

		if (interfacedScenario->m_DesignerVisualization != nullptr)
		{
			interfacedScenario->m_DesignerVisualization->setDeleteEventCB(&delete_designer_visualisation_cb, this);
			interfacedScenario->m_DesignerVisualization->load();
		}
		//interfacedScenario->snapshotCB(); --> a snapshot is already created in CInterfacedScenario builder !
		interfacedScenario->m_Filename       = sFileName;
		interfacedScenario->m_HasFileName     = true;
		interfacedScenario->m_HasBeenModified = false;
		interfacedScenario->snapshotCB(false);

		m_Scenarios.push_back(interfacedScenario);

		interfacedScenario->redrawScenarioSettings();

		gtk_notebook_set_current_page(m_pScenarioNotebook, gtk_notebook_get_n_pages(m_pScenarioNotebook) - 1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);

		interfacedScenario->updateScenarioLabel();

		this->saveOpenedScenarios();
		return true;
	}
	m_kernelCtx.getLogManager() << LogLevel_Error << "Importing scenario from file [" << sFileName << "] failed... "
			<< " Current file either is corrupted or is not compatible with the selected scenario importer (ie not an OpenViBE scenario file)\n";

	if (!(m_eCommandLineFlags & CommandLineFlag_NoGui))
	{
		std::stringstream l_oStringStream;
		l_oStringStream << "The requested file: " << sFileName << "\n";
		l_oStringStream << "may either not be an OpenViBE scenario file, \n";
		l_oStringStream << "a " + std::string(BRAND_NAME) + " scenario file, \n";
		l_oStringStream << "be corrupted or not compatible with \n";
		l_oStringStream << "the selected scenario importer...";
		GtkWidget* l_pErrorDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
														   "Scenario importation process failed !");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pErrorDialog), "%s", l_oStringStream.str().c_str());
		gtk_dialog_run(GTK_DIALOG(l_pErrorDialog));
		gtk_widget_destroy(l_pErrorDialog);
	}
	return false;
}

CString CApplication::getWorkingDirectory()
{
	CString l_sWorkingDirectory = m_kernelCtx.getConfigurationManager().expand("${Designer_DefaultWorkingDirectory}/scenarios");

	CInterfacedScenario* l_pCurrentScenario = this->getCurrentInterfacedScenario();
	if (l_pCurrentScenario)
	{
		if (l_pCurrentScenario->m_HasFileName)
		{
			std::string l_sCurrentDirectory = std::string(g_path_get_dirname(l_pCurrentScenario->m_Filename.c_str()));
#if defined TARGET_OS_Windows
			std::replace(l_sCurrentDirectory.begin(), l_sCurrentDirectory.end(), '\\', '/');
#endif
			l_sWorkingDirectory = l_sCurrentDirectory.c_str();
		}
	}

	if (!g_path_is_absolute(l_sWorkingDirectory.toASCIIString()))
	{
		std::string l_sCurrentDirectory = g_get_current_dir();
#if defined TARGET_OS_Windows
		std::replace(l_sCurrentDirectory.begin(), l_sCurrentDirectory.end(), '\\', '/');
#endif
		l_sWorkingDirectory = l_sCurrentDirectory.c_str() + CString("/") + l_sWorkingDirectory;
	}

	return l_sWorkingDirectory;
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
	if (!(m_eCommandLineFlags & CommandLineFlag_NoManageSession))
	{
		const CString appConfigFile = m_kernelCtx.getConfigurationManager().expand("${Designer_CustomConfigurationFile}");

		FILE* l_pFile = FS::Files::open(appConfigFile.toASCIIString(), "wt");
		if (l_pFile)
		{
			uint32_t i = 1;
			fprintf(l_pFile, "# This file is generated\n");
			fprintf(l_pFile, "# Do not modify\n");
			fprintf(l_pFile, "\n");

			int width, height;
			gtk_window_get_size(GTK_WINDOW(m_pMainWindow), &width, &height);
			fprintf(l_pFile, "Designer_EditorSizeWidth = %i\n", width);
			fprintf(l_pFile, "Designer_EditorSizeHeight = %i\n", height);
			fprintf(l_pFile, "Designer_EditorPanedPosition = %i\n",
					gtk_paned_get_position(GTK_PANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container"))));
			fprintf(l_pFile, "Designer_FullscreenEditor = %s\n", m_isMaximized ? "True" : "False");

			fprintf(l_pFile, "# Last files opened in %s\n", std::string(DESIGNER_NAME).c_str());

			for (CInterfacedScenario* scenario : m_Scenarios)
			{
				if (!scenario->m_Filename.empty())
				{
					fprintf(l_pFile, "Designer_LastScenarioFilename_%03u = %s\n", i, scenario->m_Filename.c_str());
					i++;
				}
			}
			fprintf(l_pFile, "\n");

			const CString projectVersion = m_kernelCtx.getConfigurationManager().expand(
				"${ProjectVersion_Major}.${ProjectVersion_Minor}.${ProjectVersion_Patch}");
			const CString componentVersions = m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenValue("ProjectVersion_Components");
			fprintf(l_pFile, "# Last version of " DESIGNER_NAME " used:\n");
			fprintf(l_pFile, "Designer_LastVersionUsed = %s\n", projectVersion.toASCIIString());
			fprintf(l_pFile, "Designer_LastComponentVersionsUsed = %s\n", componentVersions.toASCIIString());
			fprintf(l_pFile, "\n");

			fprintf(l_pFile, "# Recently opened scenario\n");
			uint32_t scenarioID = 1;
			for (const GtkWidget* recentScenario : m_RecentScenarios)
			{
				const gchar* recentScenarioPath = gtk_menu_item_get_label(GTK_MENU_ITEM(recentScenario));
				fprintf(l_pFile, "Designer_RecentScenario_%03u = %s\n", scenarioID, recentScenarioPath);
				++scenarioID;
			}
			fprintf(l_pFile, "\n");

			fclose(l_pFile);
		}
		else { m_kernelCtx.getLogManager() << LogLevel_Error << "Error writing to '" << appConfigFile << "'\n"; }
	}
}

void CApplication::dragDataGetCB(GtkWidget* /*widget*/, GdkDragContext* /*pDragContex*/, GtkSelectionData* pSelectionData, guint /*uiInfo*/,
								 guint /*uiT*/) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "dragDataGetCB\n";

	GtkTreeView* l_pTreeView           = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_tree"));
	GtkTreeSelection* l_pTreeSelection = gtk_tree_view_get_selection(l_pTreeView);
	GtkTreeModel* l_pTreeModel         = nullptr;
	GtkTreeIter l_oTreeIter;
	if (gtk_tree_selection_get_selected(l_pTreeSelection, &l_pTreeModel, &l_oTreeIter))
	{
		const char* l_sBoxAlgorithmID = nullptr;
		gtk_tree_model_get(l_pTreeModel, &l_oTreeIter, Resource_StringIdentifier, &l_sBoxAlgorithmID, -1);
		if (l_sBoxAlgorithmID)
		{
			gtk_selection_data_set(pSelectionData, GDK_SELECTION_TYPE_STRING, 8, reinterpret_cast<const guchar*>(l_sBoxAlgorithmID),
								   gint(strlen(l_sBoxAlgorithmID) + 1));
		}
	}
}

void CApplication::undoCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "undoCB\n";

	CInterfacedScenario* currentInterfacedScenario = this->getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->undoCB(); }
}

void CApplication::redoCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "redoCB\n";

	CInterfacedScenario* currentInterfacedScenario = this->getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->redoCB(); }
}

void CApplication::copySelectionCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "copySelectionCB\n";
	CInterfacedScenario* currentInterfacedScenario = this->getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->copySelection(); }
}

void CApplication::cutSelectionCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "cutSelectionCB\n";
	CInterfacedScenario* currentInterfacedScenario = this->getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->cutSelection(); }
}

void CApplication::pasteSelectionCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "pasteSelectionCB\n";
	CInterfacedScenario* currentInterfacedScenario = this->getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->pasteSelection(); }
}

void CApplication::deleteSelectionCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "deleteSelectionCB\n";
	CInterfacedScenario* currentInterfacedScenario = this->getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->deleteSelection(); }
}

void CApplication::preferencesCB() const
{
	enum
	{
		Resource_TokenName,
		Resource_TokenValue,
		Resource_TokenExpand,
	};

	m_kernelCtx.getLogManager() << LogLevel_Debug << "preferencesCB\n";
	GtkBuilder* l_pBuilderInterface = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "configuration_manager", nullptr);
	gtk_builder_add_from_file(l_pBuilderInterface, OVD_GUI_File, nullptr);
	gtk_builder_connect_signals(l_pBuilderInterface, nullptr);

	GtkWidget* l_pConfigurationManager           = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "configuration_manager"));
	GtkTreeView* l_pConfigurationManagerTreeView = GTK_TREE_VIEW(gtk_builder_get_object(l_pBuilderInterface, "configuration_manager-treeview"));

	// Prepares tree view
	GtkTreeViewColumn* l_pTreeViewColumnTokenName   = gtk_tree_view_column_new();
	GtkTreeViewColumn* l_pTreeViewColumnTokenValue  = gtk_tree_view_column_new();
	GtkTreeViewColumn* l_pTreeViewColumnTokenExpand = gtk_tree_view_column_new();
	GtkCellRenderer* l_pCellRendererTokenName       = gtk_cell_renderer_text_new();
	GtkCellRenderer* l_pCellRendererTokenValue      = gtk_cell_renderer_text_new();
	GtkCellRenderer* l_pCellRendererTokenExpand     = gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenName, "Token name");
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenValue, "Token value");
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenExpand, "Expanded token value");
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, TRUE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenValue, l_pCellRendererTokenValue, TRUE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenExpand, l_pCellRendererTokenExpand, TRUE);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, "text", Resource_TokenName, nullptr);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenValue, l_pCellRendererTokenValue, "text", Resource_TokenValue, nullptr);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenExpand, l_pCellRendererTokenExpand, "text", Resource_TokenExpand, nullptr);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenName, Resource_TokenName);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenValue, Resource_TokenValue);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenExpand, Resource_TokenExpand);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenName, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenValue, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenExpand, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenValue, TRUE);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenExpand, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenValue, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenExpand, TRUE);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenName, 256);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenValue, 256);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenExpand, 256);
	gtk_tree_view_append_column(l_pConfigurationManagerTreeView, l_pTreeViewColumnTokenName);
	gtk_tree_view_append_column(l_pConfigurationManagerTreeView, l_pTreeViewColumnTokenValue);
	gtk_tree_view_append_column(l_pConfigurationManagerTreeView, l_pTreeViewColumnTokenExpand);
	gtk_tree_view_column_set_sort_indicator(l_pTreeViewColumnTokenName, TRUE);

	// Prepares tree model
	CIdentifier l_oTokenID;
	GtkTreeStore* l_pConfigurationManagerTreeModel = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	while ((l_oTokenID = m_kernelCtx.getConfigurationManager().getNextConfigurationTokenIdentifier(l_oTokenID)) != OV_UndefinedIdentifier)
	{
		GtkTreeIter l_oGtkIterChild;
		CString l_sTokenName   = m_kernelCtx.getConfigurationManager().getConfigurationTokenName(l_oTokenID);
		CString l_sTokenValue  = m_kernelCtx.getConfigurationManager().getConfigurationTokenValue(l_oTokenID);
		CString l_sTokenExpand = m_kernelCtx.getConfigurationManager().expand(l_sTokenValue);
		gtk_tree_store_append(l_pConfigurationManagerTreeModel, &l_oGtkIterChild, nullptr);
		gtk_tree_store_set(l_pConfigurationManagerTreeModel, &l_oGtkIterChild,
						   Resource_TokenName, l_sTokenName.toASCIIString(), Resource_TokenValue, l_sTokenValue.toASCIIString(),
						   Resource_TokenExpand, l_sTokenExpand.toASCIIString(), -1);
	}
	gtk_tree_view_set_model(l_pConfigurationManagerTreeView, GTK_TREE_MODEL(l_pConfigurationManagerTreeModel));
	g_signal_emit_by_name(l_pTreeViewColumnTokenName, "clicked");

	gtk_dialog_run(GTK_DIALOG(l_pConfigurationManager));
	gtk_widget_destroy(l_pConfigurationManager);

	g_object_unref(l_pConfigurationManagerTreeModel);
	g_object_unref(l_pBuilderInterface);
}

void CApplication::testCB() const { m_kernelCtx.getLogManager() << LogLevel_Debug << "testCB\n"; }

void CApplication::newScenarioCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "newScenarioCB\n";

	CIdentifier scenarioID;
	if (m_pScenarioManager->createScenario(scenarioID))
	{
		IScenario& scenario                     = m_pScenarioManager->getScenario(scenarioID);
		CInterfacedScenario* interfacedScenario = new CInterfacedScenario(m_kernelCtx, *this, scenario, scenarioID, *m_pScenarioNotebook, OVD_GUI_File,
																		  OVD_GUI_Settings_File);
		if (interfacedScenario->m_DesignerVisualization != nullptr)
		{
			interfacedScenario->m_DesignerVisualization->setDeleteEventCB(&delete_designer_visualisation_cb, this);
			interfacedScenario->m_DesignerVisualization->newVisualizationWindow("Default window");
		}
		interfacedScenario->updateScenarioLabel();
		m_Scenarios.push_back(interfacedScenario);
		gtk_notebook_set_current_page(m_pScenarioNotebook, gtk_notebook_get_n_pages(m_pScenarioNotebook) - 1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
	}
}

void CApplication::openScenarioCB()
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	GtkFileFilter* l_pFileFilterSpecific = gtk_file_filter_new();
	GtkFileFilter* l_pFileFilterAll      = gtk_file_filter_new();

	std::string allFileFormatsString = "All available formats (";
	CString fileNameExtension        = "";
	while ((fileNameExtension = m_kernelCtx.getScenarioManager().getNextScenarioImporter(OVD_ScenarioImportContext_OpenScenario, fileNameExtension)) !=
		   CString(""))
	{
		std::string currentFileFormatMask = "*" + std::string(fileNameExtension.toASCIIString());
		gtk_file_filter_add_pattern(l_pFileFilterSpecific, currentFileFormatMask.c_str());
		allFileFormatsString += "*" + std::string(fileNameExtension) + ", ";
	}

	allFileFormatsString.erase(allFileFormatsString.size() - 2); // because the loop adds one ", " too much
	allFileFormatsString += ")";

	gtk_file_filter_set_name(l_pFileFilterSpecific, allFileFormatsString.c_str());

	gtk_file_filter_set_name(l_pFileFilterAll, "All files");
	gtk_file_filter_add_pattern(l_pFileFilterAll, "*");

	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select scenario to open...", nullptr,
															  GTK_FILE_CHOOSER_ACTION_OPEN,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
															  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogOpen), l_pFileFilterAll);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(widgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(widgetDialogOpen), true);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(widgetDialogOpen), this->getWorkingDirectory().toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(widgetDialogOpen), true);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		//char* fileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		GSList* list;
		GSList* l_pFile = list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(widgetDialogOpen));
		while (l_pFile)
		{
			char* fileName = static_cast<char*>(l_pFile->data);
			char* l_pBackslash;
			while ((l_pBackslash = strchr(fileName, '\\')) != nullptr) { *l_pBackslash = '/'; }
			this->openScenario(fileName);
			g_free(l_pFile->data);
			l_pFile = l_pFile->next;
		}
		g_slist_free(list);
	}
	gtk_widget_destroy(widgetDialogOpen);
	//	g_object_unref(l_pFileFilterSpecific);
	//	g_object_unref(l_pFileFilterAll);
}

void CApplication::saveScenarioCB(CInterfacedScenario* interfacedScenario)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "saveScenarioCB\n";

	CInterfacedScenario* currentInterfacedScenario = interfacedScenario ? interfacedScenario : getCurrentInterfacedScenario();
	if (!currentInterfacedScenario) { return; }

	if (currentInterfacedScenario->m_Scenario.containsBoxWithDeprecatedInterfacors())
	{
		cannotSaveScenarioBeforeUpdate();
		return;
	}

	if (!currentInterfacedScenario->m_HasFileName) { saveScenarioAsCB(interfacedScenario); }
	else
	{
		// If the current scenario is a metabox, we will save its prototype hash into an attribute of the scenario
		// that way the standalone scheduler can check whether metaboxes included inside need updating.
		if (currentInterfacedScenario->m_Scenario.isMetabox())
		{
			SBoxProto metaboxProto(m_kernelCtx.getTypeManager());

			IScenario& scenario = currentInterfacedScenario->m_Scenario;
			for (uint32_t scenarioInputIdx = 0; scenarioInputIdx < scenario.getInputCount(); ++scenarioInputIdx)
			{
				CString inputName;
				CIdentifier inputTypeID;
				CIdentifier InputID;

				scenario.getInputType(scenarioInputIdx, inputTypeID);
				scenario.getInputName(scenarioInputIdx, inputName);
				scenario.getInterfacorIdentifier(Input, scenarioInputIdx, InputID);

				metaboxProto.addInput(inputName, inputTypeID, InputID, true);
			}

			for (uint32_t scenarioOutputIdx = 0; scenarioOutputIdx < scenario.getOutputCount(); ++scenarioOutputIdx)
			{
				CString outputName;
				CIdentifier OutputTypeID;
				CIdentifier OutputID;

				scenario.getOutputType(scenarioOutputIdx, OutputTypeID);
				scenario.getOutputName(scenarioOutputIdx, outputName);
				scenario.getInterfacorIdentifier(Output, scenarioOutputIdx, OutputID);

				metaboxProto.addOutput(outputName, OutputTypeID, OutputID, true);
			}

			for (uint32_t scenarioSettingIdx = 0; scenarioSettingIdx < scenario.getSettingCount(); ++scenarioSettingIdx)
			{
				CString l_sSettingName;
				CIdentifier l_oSettingTypeID;
				CString l_sSettingDefaultValue;

				scenario.getSettingName(scenarioSettingIdx, l_sSettingName);
				scenario.getSettingType(scenarioSettingIdx, l_oSettingTypeID);
				scenario.getSettingDefaultValue(scenarioSettingIdx, l_sSettingDefaultValue);

				metaboxProto.addSetting(l_sSettingName, l_oSettingTypeID, l_sSettingDefaultValue, false, OV_UndefinedIdentifier, true);
			}

			if (scenario.hasAttribute(OV_AttributeId_Scenario_MetaboxHash))
			{
				scenario.setAttributeValue(OV_AttributeId_Scenario_MetaboxHash, metaboxProto.m_oHash.toString());
			}
			else { scenario.addAttribute(OV_AttributeId_Scenario_MetaboxHash, metaboxProto.m_oHash.toString()); }

			if (!scenario.hasAttribute(OVP_AttributeId_Metabox_ID))
			{
				scenario.setAttributeValue(OVP_AttributeId_Metabox_ID, CIdentifier::random().str().c_str());
			}

			m_kernelCtx.getLogManager() << LogLevel_Trace << "This metaboxes Hash : " << metaboxProto.m_oHash << "\n";
		}

		const char* scenarioFileName = currentInterfacedScenario->m_Filename.c_str();

		// Remove attributes that were added to links and boxes by the designer and which are used only for interal functionality.
		// This way the scenarios do not change if, for example somebody opens them on a system with different font metrics.
		currentInterfacedScenario->m_Scenario.removeAttribute(OV_AttributeId_ScenarioFilename);

		CIdentifier linkID;
		while ((linkID = currentInterfacedScenario->m_Scenario.getNextLinkIdentifier(linkID)) != OV_UndefinedIdentifier)
		{
			auto link = currentInterfacedScenario->m_Scenario.getLinkDetails(linkID);
			link->removeAttribute(OV_AttributeId_Link_XSrc);
			link->removeAttribute(OV_AttributeId_Link_YSrc);
			link->removeAttribute(OV_AttributeId_Link_XDst);
			link->removeAttribute(OV_AttributeId_Link_YDst);
			link->removeAttribute(OV_ClassId_Selected);
		}

		CIdentifier boxID;
		while ((boxID = currentInterfacedScenario->m_Scenario.getNextBoxIdentifier(boxID)) != OV_UndefinedIdentifier)
		{
			auto box = currentInterfacedScenario->m_Scenario.getBoxDetails(boxID);
			box->removeAttribute(OV_AttributeId_Box_XSize);
			box->removeAttribute(OV_AttributeId_Box_YSize);
			box->removeAttribute(OV_ClassId_Selected);
		}

		CIdentifier commentID;
		while ((commentID = currentInterfacedScenario->m_Scenario.getNextCommentIdentifier(commentID)) != OV_UndefinedIdentifier)
		{
			auto comment = currentInterfacedScenario->m_Scenario.getCommentDetails(commentID);
			comment->removeAttribute(OV_ClassId_Selected);
		}

		// Remove all VisualizationTree type metadata
		// We save the last found identifier if there was one, this allows us to not modify it on subsequent saves
		CIdentifier metadataID              = OV_UndefinedIdentifier;
		CIdentifier lastFoundTreeIdentifier = OV_UndefinedIdentifier;
		while ((metadataID = currentInterfacedScenario->m_Scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
		{
			if (currentInterfacedScenario->m_Scenario.getMetadataDetails(metadataID)->getType() == OVVIZ_MetadataIdentifier_VisualizationTree)
			{
				currentInterfacedScenario->m_Scenario.removeMetadata(metadataID);
				lastFoundTreeIdentifier = metadataID;
				metadataID              = OV_UndefinedIdentifier;
			}
		}

		// Insert new metadata
		currentInterfacedScenario->m_Scenario.addMetadata(metadataID, lastFoundTreeIdentifier);
		currentInterfacedScenario->m_Scenario.getMetadataDetails(metadataID)->setType(OVVIZ_MetadataIdentifier_VisualizationTree);
		currentInterfacedScenario->m_Scenario.getMetadataDetails(metadataID)->setData(currentInterfacedScenario->m_Tree->serialize());

		CIdentifier scenarioExportContext = OVD_ScenarioExportContext_SaveScenario;
		if (currentInterfacedScenario->m_Scenario.isMetabox()) { scenarioExportContext = OVD_ScenarioExportContext_SaveMetabox; }

		m_kernelCtx.getErrorManager().releaseErrors();
		if (m_pScenarioManager->exportScenarioToFile(scenarioExportContext, scenarioFileName, currentInterfacedScenario->m_ScenarioID))
		{
			currentInterfacedScenario->snapshotCB();
			currentInterfacedScenario->m_HasFileName     = true;
			currentInterfacedScenario->m_HasBeenModified = false;
			currentInterfacedScenario->updateScenarioLabel();
			this->saveOpenedScenarios();
		}
		else
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Exporting scenario failed...\n";
			GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
			gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, nullptr);
			gtk_builder_connect_signals(l_pBuilder, nullptr);
			GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_error_popup_saving"));
			// Reset the labels
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pBuilder, "dialog_error_popup_saving-label1")), "");
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pBuilder, "dialog_error_popup_saving-label2")), "");
			if (m_kernelCtx.getErrorManager().hasError())
			{
				gtk_label_set_text(
					GTK_LABEL(gtk_builder_get_object(l_pBuilder, "dialog_error_popup_saving-label1")), m_kernelCtx.getErrorManager().getLastErrorString());
			}
			gtk_builder_connect_signals(l_pBuilder, nullptr);
			const gint l_iResponseId = gtk_dialog_run(GTK_DIALOG(l_pDialog));
			gtk_widget_destroy(l_pDialog);
			g_object_unref(l_pBuilder);

			switch (l_iResponseId)
			{
				case GTK_RESPONSE_OK:
					this->saveScenarioCB(interfacedScenario);
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

	GtkWidget* widgetDialogRestoreScenarios = gtk_message_dialog_new(nullptr, GTK_DIALOG_DESTROY_WITH_PARENT,
																	 GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
																	 "%s", message.toASCIIString());


	const CString backupFolder = defaultWorkingDirectory + m_kernelCtx.getConfigurationManager().expand("-$core{date}-$core{time}");

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogRestoreScenarios)) == GTK_RESPONSE_YES)
	{
		// to avoid to loose old data, make a backup
		FS::Files::removeAll(backupFolder);
		if (FS::Files::copyDirectory(defaultWorkingDirectory, backupFolder))
		{
			m_kernelCtx.getLogManager() << LogLevel_Info << "Old scenario folder backed up into "
					<< backupFolder << " folder\n";
			// make the copy
			FS::Files::removeAll(defaultWorkingDirectory);
			if (FS::Files::copyDirectory(defaultScenariosDirectory, defaultWorkingDirectory))
			{
				m_kernelCtx.getLogManager() << LogLevel_Info << "Default scenarios restored into " << defaultWorkingDirectory << " folder\n";
			}
			else { m_kernelCtx.getLogManager() << LogLevel_Error << "Could not copy " << defaultWorkingDirectory << " folder\n"; }
		}
		else { m_kernelCtx.getLogManager() << LogLevel_Error << "Could not back up " << defaultWorkingDirectory << " folder. Copy has aborted.\n"; }
	}

	gtk_widget_destroy(widgetDialogRestoreScenarios);
}

void CApplication::saveScenarioAsCB(CInterfacedScenario* interfacedScenario)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "saveScenarioAsCB\n";

	CInterfacedScenario* currentInterfacedScenario = interfacedScenario ? interfacedScenario : getCurrentInterfacedScenario();
	if (!currentInterfacedScenario) { return; }

	if (currentInterfacedScenario->m_Scenario.containsBoxWithDeprecatedInterfacors())
	{
		cannotSaveScenarioBeforeUpdate();
		return;
	}

	const bool isCurrentScenarioAMetabox = currentInterfacedScenario->m_Scenario.isMetabox();

	GtkFileFilter* l_pFileFilterAll = gtk_file_filter_new(); // All files
	gtk_file_filter_set_name(l_pFileFilterAll, "All files");
	gtk_file_filter_add_pattern(l_pFileFilterAll, "*");

	GtkFileFilter* allCompatibleFormatsFileFilter = gtk_file_filter_new(); // All compatible files

	std::map<GtkFileFilter*, std::string> fileFilters;


	std::set<std::string> compatibleExtensions;
	CString fileNameExtension;
	if (!isCurrentScenarioAMetabox)
	{
		while ((fileNameExtension = m_kernelCtx.getScenarioManager().getNextScenarioExporter(OVD_ScenarioExportContext_SaveScenario, fileNameExtension)) !=
			   CString("")) { compatibleExtensions.emplace(fileNameExtension); }
	}
	while ((fileNameExtension = m_kernelCtx.getScenarioManager().getNextScenarioExporter(OVD_ScenarioExportContext_SaveMetabox, fileNameExtension)) !=
		   CString("")) { compatibleExtensions.emplace(fileNameExtension); }

	std::string allCompatibleFormatsFilterName = "All compatible formats (";

	for (auto& extension : compatibleExtensions)
	{
		GtkFileFilter* fileFilter  = gtk_file_filter_new();
		std::string fileFilterName = m_kernelCtx.getConfigurationManager().expand(std::string("${ScenarioFileNameExtension" + extension + "}").c_str()).
													 toASCIIString() + std::string(" (*") + extension + ")";
		gtk_file_filter_set_name(fileFilter, fileFilterName.c_str());
		std::string fileFilterWildcard = "*" + extension;
		gtk_file_filter_add_pattern(fileFilter, fileFilterWildcard.c_str());
		fileFilters[fileFilter] = extension;

		allCompatibleFormatsFilterName += fileFilterWildcard + ", ";
		gtk_file_filter_add_pattern(allCompatibleFormatsFileFilter, fileFilterWildcard.c_str());
	}

	allCompatibleFormatsFilterName.erase(allCompatibleFormatsFilterName.size() - 2); // because the loop adds one ", " too much
	allCompatibleFormatsFilterName += ")";

	gtk_file_filter_set_name(allCompatibleFormatsFileFilter, allCompatibleFormatsFilterName.c_str());

	// gtk_file_filter_set_name(l_pFileFilterSVG, "SVG image");
	// gtk_file_filter_add_pattern(l_pFileFilterSVG, "*.svg");

	GtkWidget* widgetDialogSaveAs = gtk_file_chooser_dialog_new("Select scenario to save...", nullptr,
																GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
																GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, nullptr);

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), allCompatibleFormatsFileFilter);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), l_pFileFilterAll);

	for (const auto& fileFilter : fileFilters) { gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), fileFilter.first); }

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs), allCompatibleFormatsFileFilter);
	// gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_widgetDialogSaveAs), true);
	if (currentInterfacedScenario->m_HasFileName)
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogSaveAs), currentInterfacedScenario->m_Filename.c_str());
	}
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
		char* l_sTempFilename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogSaveAs));

		// replaces \ with / on windows
		char* l_pBackslash;
		while ((l_pBackslash = strchr(l_sTempFilename, '\\')) != nullptr) { *l_pBackslash = '/'; }

		// stores filename in a local variable
		char filename[1024];
		int filenameLength = sprintf(filename, "%s", l_sTempFilename);
		g_free(l_sTempFilename);

		GtkFileFilter* l_pFileFilter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(widgetDialogSaveAs));
		if (fileFilters.count(l_pFileFilter) != 0)
		{
			// User chose a specific filter
			const std::string l_sExpectedExtension = fileFilters[l_pFileFilter];
			if (_strcmpi(filename + filenameLength - 4, (std::string(".") + l_sExpectedExtension).c_str()) != 0)
			{
				// If filename already has an extension, remove it
				if (filename[filenameLength - 4] == '.')
				{
					filenameLength -= 4;
					filename[filenameLength] = '\0';
				}

				// When user did not put appropriate extension, append it
				strcat(filename, l_sExpectedExtension.c_str());
				//filenameLength += int(1 + l_sExpectedExtension.length());
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
		bool l_bIsSaveActionValid = true;
		FILE* l_pFile             = FS::Files::open(filename, "r");
		if (l_pFile)
		{
			fclose(l_pFile);
			GtkDialog* l_pConfirmationDialog = GTK_DIALOG(
				::gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "The file already exists"));
			gtk_message_dialog_format_secondary_text(
				GTK_MESSAGE_DIALOG(l_pConfirmationDialog),
				"%s\n\nThe file you are trying to save-as already exists, confirming this action will overwrite the existing file. Please confirm you want to overwrite the existing file.",
				filename);
			l_bIsSaveActionValid = (gtk_dialog_run(GTK_DIALOG(l_pConfirmationDialog)) == GTK_RESPONSE_OK);
			gtk_widget_destroy(GTK_WIDGET(l_pConfirmationDialog));
		}

		// Finally save the scenario
		if (l_bIsSaveActionValid)
		{
			currentInterfacedScenario->m_Filename       = filename;
			currentInterfacedScenario->m_HasFileName     = true;
			currentInterfacedScenario->m_HasBeenModified = false;
			currentInterfacedScenario->updateScenarioLabel();
			saveScenarioCB(currentInterfacedScenario);
		}
		else { m_kernelCtx.getLogManager() << LogLevel_Trace << "Canceled 'save-as' action for filename [" << CString(filename) << "]\n"; }
	}

	gtk_widget_destroy(widgetDialogSaveAs);
	//	g_object_unref(l_pFileFilterSpecific);
	//	g_object_unref(l_pFileFilterAll);
}

void CApplication::addRecentScenario(const std::string& scenarioPath)
{
	bool scenarioFound = false;
	// If scenario path is already in menu, remove it from menu shell and re-add it on top of list
	for (size_t i = 0; i < m_RecentScenarios.size(); ++i)
	{
		const gchar* fileName = gtk_menu_item_get_label(GTK_MENU_ITEM(m_RecentScenarios[i]));
		if (strcmp(fileName, scenarioPath.c_str()) == 0)
		{
			gtk_container_remove(m_MenuOpenRecent, GTK_WIDGET(m_RecentScenarios[i]));
			gtk_menu_shell_prepend(GTK_MENU_SHELL(m_MenuOpenRecent), GTK_WIDGET(m_RecentScenarios[i]));
			scenarioFound = true;
			m_RecentScenarios.insert(m_RecentScenarios.begin(), m_RecentScenarios[i]);
			m_RecentScenarios.erase(m_RecentScenarios.begin() + i + 1);
			break;
		}
	}
	// If scenario is not in menu, create new widget and add it to menu shell
	if (!scenarioFound)
	{
		GtkWidget* newRecentItem = gtk_image_menu_item_new_with_label(scenarioPath.c_str());

		g_signal_connect(G_OBJECT(newRecentItem), "activate", G_CALLBACK(menu_open_recent_scenario_cb), this);
		gtk_menu_shell_prepend(GTK_MENU_SHELL(m_MenuOpenRecent), newRecentItem);
		gtk_widget_show(newRecentItem);
		m_RecentScenarios.insert(m_RecentScenarios.begin(), newRecentItem);
	}

	if (m_RecentScenarios.size() > s_RecentFileNumber)
	{
		for (auto it = m_RecentScenarios.begin() + s_RecentFileNumber; it != m_RecentScenarios.end(); ++it)
		{
			gtk_container_remove(m_MenuOpenRecent, GTK_WIDGET(*it));
			gtk_widget_destroy(GTK_WIDGET(*it));
		}
		m_RecentScenarios.erase(m_RecentScenarios.begin() + s_RecentFileNumber, m_RecentScenarios.end());
	}
}

void CApplication::closeScenarioCB(CInterfacedScenario* interfacedScenario)
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "closeScenarioCB\n";

	if (!interfacedScenario) { return; }
	if (interfacedScenario->isLocked())
	{
		GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(l_pBuilder, nullptr);

		GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_running_scenario"));
		gtk_builder_connect_signals(l_pBuilder, nullptr);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);
		return;
	}
	if (interfacedScenario->m_HasBeenModified)
	{
		GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(l_pBuilder, nullptr);

		GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_unsaved_scenario"));
		gtk_builder_connect_signals(l_pBuilder, nullptr);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		const gint l_iResponseId = gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		switch (l_iResponseId)
		{
			case GTK_RESPONSE_OK:
				this->saveScenarioCB(interfacedScenario);
				if (interfacedScenario->m_HasBeenModified) { return; }
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				return;
			default:
				break;
		}
	}
	// Add scenario to recently opened:
	this->addRecentScenario(interfacedScenario->m_Filename);

	const auto it = std::find(m_Scenarios.begin(), m_Scenarios.end(), interfacedScenario);
	if (it != m_Scenarios.end())
	{
		// We need to erase the scenario from the list first, because deleting the scenario will launch a "switch-page"
		// callback accessing this array with the identifier of the deleted scenario (if its not the last one) -> boom.
		m_Scenarios.erase(it);
		const CIdentifier scenarioID = interfacedScenario->m_ScenarioID;
		delete interfacedScenario;
		m_pScenarioManager->releaseScenario(scenarioID);
		//when closing last open scenario, no "switch-page" event is triggered so we manually handle this case
		if (m_Scenarios.empty()) { newScenarioCB(); }
		else { changeCurrentScenario(gtk_notebook_get_current_page(m_pScenarioNotebook)); }
	}

	this->saveOpenedScenarios();
}

void CApplication::deleteDesignerVisualizationCB()
{
	//untoggle window manager button when its associated dialog is closed
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), FALSE);

	CInterfacedScenario* currentInterfacedScenario = getCurrentInterfacedScenario();
	if (currentInterfacedScenario) { currentInterfacedScenario->snapshotCB(); }
}

void CApplication::toggleDesignerVisualizationCB()
{
	CInterfacedScenario* currentInterfacedScenario = getCurrentInterfacedScenario();
	if (currentInterfacedScenario != nullptr && !currentInterfacedScenario->isLocked())
	{
		const auto index = size_t(gtk_notebook_get_current_page(m_pScenarioNotebook));
		if (index < m_Scenarios.size()) { m_Scenarios[index]->toggleDesignerVisualization(); }
	}
}

void CApplication::aboutOpenViBECB()

{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::aboutOpenViBECB\n";
	GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
	gtk_builder_add_from_file(l_pBuilder, OVD_GUI_AboutDialog_File, nullptr);
	gtk_builder_connect_signals(l_pBuilder, nullptr);
	//gtk_builder_connect_signals(l_pBuilder, nullptr);
	GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "about"));
	if (l_pDialog == nullptr)
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Dialog could not be opened\n";
		return;
	}

	if (m_kernelCtx.getConfigurationManager().expand("${Application_Name}").length() > 0)
	{
		gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(l_pDialog), m_kernelCtx.getConfigurationManager().expand("${Application_Name}").toASCIIString());
	}
	if (m_kernelCtx.getConfigurationManager().expand("${Application_Version}").length() > 0)
	{
		gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(l_pDialog), m_kernelCtx.getConfigurationManager().expand("${Application_Version}").toASCIIString());
	}

	gchar* strval;
	g_object_get(l_pDialog, "comments", &strval, nullptr);
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
			g_object_set(l_pDialog, "comments", update.c_str(), nullptr);
		}
	}
	g_free(strval);
	gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
	gtk_dialog_run(GTK_DIALOG(l_pDialog));
	gtk_widget_destroy(l_pDialog);
	g_object_unref(l_pBuilder);
}

void CApplication::aboutScenarioCB(CInterfacedScenario* pScenario) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::aboutScenarioCB\n";
	if (pScenario && !pScenario->isLocked()) { pScenario->contextMenuScenarioAboutCB(); }
}

void CApplication::aboutLinkClickedCB(const gchar* url) const
{
	if (!url) { return; }
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::aboutLinkClickedCB\n";
	const CString command = m_kernelCtx.getConfigurationManager().expand("${Designer_WebBrowserCommand} " + CString(url));
	const int result      = system(command.toASCIIString());
	if (result < 0) { m_kernelCtx.getLogManager() << LogLevel_Warning << "Could not launch command " << command << "\n"; }
}

void CApplication::browseDocumentationCB() const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::browseDocumentationCB\n";
	const CString command = m_kernelCtx.getConfigurationManager().expand(
		"${Designer_HelpBrowserCommand} \"${Designer_HelpBrowserDocumentationIndex}\" ${Designer_HelpBrowserCommandPostfix}");

	const int result = system(command.toASCIIString());
	OV_WARNING_UNLESS((result == 0), "Could not launch command " << command << "\n", m_kernelCtx.getLogManager());
}

void CApplication::registerLicenseCB() const
{
#if defined TARGET_OS_Windows && defined(MENSIA_DISTRIBUTION)
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::registerLicenseCB\n";
	std::string command = (Directories::getBinDir() + "/mensia-flexnet-activation.exe").toASCIIString();
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	GetStartupInfo(&startupInfo);
	if (!System::WindowsUtilities::utf16CompliantCreateProcess(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &startupInfo, &processInfo)) { exit(1); }
#elif defined TARGET_OS_Linux && defined(MENSIA_DISTRIBUTION)
	m_kernelCtx.getLogManager() << LogLevel_Info << "Register License application's GUI cannot run on Linux. In order to activate your license,"
		<< " you can use the tool 'mensia-flexnet-activation' in command line.\n";
#endif
}

void CApplication::reportIssueCB() const

{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::reportIssueCB\n";
	const CString command = m_kernelCtx.getConfigurationManager().expand(
		"${Designer_WebBrowserCommand} ${Designer_WebBrowserSupportURL} ${Designer_WebBrowserCommandPostfix}");
	const int result = system(command.toASCIIString());

	OV_WARNING_UNLESS((result == 0), "Could not launch command " << command << "\n", m_kernelCtx.getLogManager());
}

void CApplication::addCommentCB(CInterfacedScenario* pScenario) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::addCommentCB\n";
	if (pScenario && !pScenario->isLocked()) { pScenario->addCommentCB(); }
}

void CApplication::configureScenarioSettingsCB(CInterfacedScenario* pScenario) const
{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "CApplication::configureScenarioSettingsCB " << m_currentScenarioIdx << "\n";

	if (pScenario && !pScenario->isLocked()) { pScenario->configureScenarioSettingsCB(); }
}

IPlayer* CApplication::getPlayer()

{
	CInterfacedScenario* currentInterfacedScenario = getCurrentInterfacedScenario();
	return (currentInterfacedScenario ? currentInterfacedScenario->m_Player : nullptr);
}

bool CApplication::createPlayer()

{
	CInterfacedScenario* currentInterfacedScenario = getCurrentInterfacedScenario();
	if (currentInterfacedScenario && !currentInterfacedScenario->m_Player)
	{
		// create a snapshot so settings override does not modify the scenario !
		currentInterfacedScenario->snapshotCB(false);

		// set filename attribute to scenario so delayed configuration can be used
		if (currentInterfacedScenario->m_HasFileName)
		{
			if (currentInterfacedScenario->m_Scenario.hasAttribute(OV_AttributeId_ScenarioFilename))
			{
				currentInterfacedScenario->m_Scenario.setAttributeValue(OV_AttributeId_ScenarioFilename, currentInterfacedScenario->m_Filename.c_str());
			}
			else { currentInterfacedScenario->m_Scenario.addAttribute(OV_AttributeId_ScenarioFilename, currentInterfacedScenario->m_Filename.c_str()); }
		}

		m_kernelCtx.getPlayerManager().createPlayer(currentInterfacedScenario->m_PlayerID);
		const CIdentifier scenarioID         = currentInterfacedScenario->m_ScenarioID;
		const CIdentifier playerIdentifier   = currentInterfacedScenario->m_PlayerID;
		currentInterfacedScenario->m_Player = &m_kernelCtx.getPlayerManager().getPlayer(playerIdentifier);
		if (!currentInterfacedScenario->m_Player->setScenario(scenarioID))
		{
			currentInterfacedScenario->m_PlayerID = OV_UndefinedIdentifier;
			currentInterfacedScenario->m_Player           = nullptr;
			m_kernelCtx.getPlayerManager().releasePlayer(playerIdentifier);
			OV_ERROR_DRF("The current scenario could not be loaded by the player.\n", ErrorType::BadCall);
		}

		// The visualization manager needs to know the visualization tree in which the widgets should be inserted
		currentInterfacedScenario->m_Player->getRuntimeConfigurationManager().createConfigurationToken(
			"VisualizationContext_VisualizationTreeId", currentInterfacedScenario->m_TreeID.toString());

		// TODO_JL: This should be a copy of the tree containing visualizations from the metaboxes
		currentInterfacedScenario->createPlayerVisualization(currentInterfacedScenario->m_Tree);


		if (currentInterfacedScenario->m_Player->initialize() != PlayerReturnCode_Sucess)
		{
			currentInterfacedScenario->releasePlayerVisualization();
			m_kernelCtx.getLogManager() << LogLevel_Error << "The player could not be initialized.\n";
			currentInterfacedScenario->m_PlayerID = OV_UndefinedIdentifier;
			currentInterfacedScenario->m_Player           = nullptr;
			m_kernelCtx.getPlayerManager().releasePlayer(playerIdentifier);
			return false;
		}
		currentInterfacedScenario->m_LastLoopTime = uint64_t(-1);

		//set up idle function
		__g_idle_add__(idle_scenario_loop, currentInterfacedScenario);

		// redraws scenario
		currentInterfacedScenario->redraw();
	}
	return true;
}

void CApplication::stopInterfacedScenarioAndReleasePlayer(CInterfacedScenario* interfacedScenario)
{
	if (!(interfacedScenario && interfacedScenario->m_Player))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Trying to stop a non-started scenario" << "\n";
		return;
	}

	interfacedScenario->stopAndReleasePlayer();

	if (interfacedScenario == this->getCurrentInterfacedScenario())
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
	}
}

void CApplication::stopScenarioCB()

{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "stopScenarioCB\n";

	const EPlayerStatus currentState = this->getCurrentInterfacedScenario()->m_PlayerStatus;
	if (currentState == PlayerStatus_Play || currentState == PlayerStatus_Pause || currentState == PlayerStatus_Forward)
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());

		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-toggle_button_loop"))))
		{
			switch (currentState)
			{
				case PlayerStatus_Play:
					playScenarioCB();
					break;
				case PlayerStatus_Forward:
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
	m_kernelCtx.getLogManager() << LogLevel_Debug << "pauseScenarioCB\n";

	this->createPlayer();
	this->getPlayer()->pause();
	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::nextScenarioCB()

{
	m_kernelCtx.getLogManager() << LogLevel_Debug << "nextScenarioCB\n";

	this->createPlayer();
	auto player = this->getPlayer();
	OV_ERROR_UNLESS_DRV(player, "Player did not initialize correctly", ErrorType::BadCall);
	player->step();
	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::playScenarioCB()

{
	if (this->getCurrentInterfacedScenario() != nullptr)
	{
		IScenario& l_oCurrentScenario = this->getCurrentInterfacedScenario()->m_Scenario;
		m_kernelCtx.getLogManager() << LogLevel_Debug << "playScenarioCB\n";
		if (l_oCurrentScenario.hasOutdatedBox())
		{
			if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Kernel_AbortPlayerWhenBoxIsOutdated}", false))
			{
				std::string l_sOutdatedBoxesList = "You can not start the scenario because following boxes need to be updated: \n";
				CIdentifier l_oBoxID;
				while ((l_oBoxID = l_oCurrentScenario.getNextOutdatedBoxIdentifier(l_oBoxID)) != OV_UndefinedIdentifier)
				{
					const IBox* l_pBox = l_oCurrentScenario.getBoxDetails(l_oBoxID);
					l_sOutdatedBoxesList += "\t[" + l_pBox->getName() + "]\n";
				}
				l_sOutdatedBoxesList += "To update a box you need to delete it from scenario, and add it again.";
				GtkWidget* l_pDialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
															  GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", l_sOutdatedBoxesList.c_str());
				gtk_dialog_run(GTK_DIALOG(l_pDialog));
				gtk_widget_destroy(l_pDialog);
				return;
			}
			if (m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ThrowPopUpWhenBoxIsOutdated}", false))
			{
				std::string l_sOutdatedBoxesList = "The following boxes need to be updated: \n";
				CIdentifier l_oBoxID;
				while ((l_oBoxID = l_oCurrentScenario.getNextOutdatedBoxIdentifier(l_oBoxID)) != OV_UndefinedIdentifier)
				{
					const IBox* l_pBox = l_oCurrentScenario.getBoxDetails(l_oBoxID);
					l_sOutdatedBoxesList += "\t[" + l_pBox->getName() + "]\n";
				}
				l_sOutdatedBoxesList += "Do you still want to play the scenario ?";
				GtkWidget* l_pDialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
															  GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", l_sOutdatedBoxesList.c_str());
				const gint l_iResponse = gtk_dialog_run(GTK_DIALOG(l_pDialog));
				gtk_widget_destroy(l_pDialog);

				if (l_iResponse == GTK_RESPONSE_YES)
				{
					m_kernelCtx.getLogManager() << LogLevel_Trace << "CApplication::playScenarioCB - GTK_RESPONSE_YES: the scenario will be played. \n";
				}
				else
				{
					m_kernelCtx.getLogManager() << LogLevel_Trace << "CApplication::playScenarioCB - the scenario will not be played. \n";
					return;
				}
			}
		}
	}

	if (!this->createPlayer())
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "The initialization of player failed. Check the above log messages to get the issue.\n";
		return;
	}
	if (!this->getPlayer()->play())
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
		return;
	}
	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PAUSE);

	if (m_eCommandLineFlags & CommandLineFlag_NoVisualization) { for (auto& iScenario : m_Scenarios) { iScenario->hideCurrentVisualization(); } }
}

void CApplication::forwardScenarioCB()

{
	m_kernelCtx.getLogManager() << LogLevel_Trace << "forwardScenarioCB\n";

	if (!this->createPlayer())
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "CreatePlayer failed\n";
		return;
	}

	if (!this->getPlayer()->forward())
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
		return;
	}

	this->getCurrentInterfacedScenario()->m_PlayerStatus = this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

	if (m_eCommandLineFlags & CommandLineFlag_NoVisualization) { for (auto& iScenario : m_Scenarios) { iScenario->hideCurrentVisualization(); } }
}

bool CApplication::quitApplicationCB()

{
	CIdentifier identifier;
	m_kernelCtx.getLogManager() << LogLevel_Debug << "quitApplicationCB\n";

	// can't quit while scenarios are running
	if (this->hasRunningScenario())
	{
		GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(l_pBuilder, nullptr);

		GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_running_scenario_global"));
		gtk_builder_connect_signals(l_pBuilder, nullptr);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		// prevent Gtk from handling delete_event and killing app
		return false;
	}

	// can't quit while scenarios are unsaved
	if (this->hasUnsavedScenario())
	{
		GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", nullptr);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, nullptr);
		gtk_builder_connect_signals(l_pBuilder, nullptr);

		GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_unsaved_scenario_global"));
		gtk_builder_connect_signals(l_pBuilder, nullptr);
		const gint responseId = gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

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
	m_isQuitting = true;

	// Saves opened scenarios
	this->saveOpenedScenarios();

	// Clears all existing interfaced scenarios
	for (auto interfacedScenario : m_Scenarios) { delete interfacedScenario; }

	// Clears all existing scenarios
	vector<CIdentifier> l_vScenarioIdentifiers;
	while ((identifier = m_kernelCtx.getScenarioManager().getNextScenarioIdentifier(identifier)) != OV_UndefinedIdentifier)
	{
		l_vScenarioIdentifiers.push_back(identifier);
	}

	for (auto& scenario : l_vScenarioIdentifiers) { m_kernelCtx.getScenarioManager().releaseScenario(scenario); }

	// release the log manager and free the memory
	if (m_pLogListenerDesigner)
	{
		m_kernelCtx.getLogManager().removeListener(m_pLogListenerDesigner);
		delete m_pLogListenerDesigner;
		m_pLogListenerDesigner = nullptr;
	}

	// OK to kill app
	return true;
}

void CApplication::windowStateChangedCB(const bool bIsMaximized)
{
	if (m_isMaximized != bIsMaximized && !bIsMaximized) // we switched to not maximized
	{
		//gtk_paned_set_position(GTK_PANED(gtk_builder_get_object(m_builderInterface, "openvibe-horizontal_container")), 640);
		gtk_window_resize(GTK_WINDOW(m_pMainWindow), 1024, 768);
	}
	m_isMaximized = bIsMaximized;
}

void CApplication::logLevelCB() const
{
	// Loads log level dialog
	GtkBuilder* l_pBuilderInterface = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "loglevel", nullptr);
	gtk_builder_add_from_file(l_pBuilderInterface, OVD_GUI_File, nullptr);
	gtk_builder_connect_signals(l_pBuilderInterface, nullptr);

	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_fatal")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Fatal));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_error")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Error));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_important_warning")),
		m_kernelCtx.getLogManager().isActive(LogLevel_ImportantWarning));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_warning")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Warning));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_info")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Info));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_trace")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Trace));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_benchmark")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Benchmark));
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_debug")),
		m_kernelCtx.getLogManager().isActive(LogLevel_Debug));

	GtkDialog* l_pLogLevelDialog = GTK_DIALOG(gtk_builder_get_object(l_pBuilderInterface, "loglevel"));
	const gint result            = gtk_dialog_run(l_pLogLevelDialog);
	if (result == GTK_RESPONSE_APPLY)
	{
		m_kernelCtx.getLogManager().activate(
			LogLevel_Fatal, gtk_toggle_button_get_active(
								GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_fatal"))) != 0);
		m_kernelCtx.getLogManager().activate(
			LogLevel_Error, gtk_toggle_button_get_active(
								GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_error"))) != 0);
		m_kernelCtx.getLogManager().activate(LogLevel_ImportantWarning,
												 gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
													 gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_important_warning"))) != 0);
		m_kernelCtx.getLogManager().activate(LogLevel_Warning,
												 gtk_toggle_button_get_active(
													 GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_warning"))) !=
												 0);
		m_kernelCtx.getLogManager().activate(
			LogLevel_Info, gtk_toggle_button_get_active(
							   GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_info"))) != 0);
		m_kernelCtx.getLogManager().activate(
			LogLevel_Trace, gtk_toggle_button_get_active(
								GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_trace"))) != 0);
		m_kernelCtx.getLogManager().activate(LogLevel_Benchmark,
												 gtk_toggle_button_get_active(
													 GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_benchmark")))
												 != 0);
		m_kernelCtx.getLogManager().activate(
			LogLevel_Debug, gtk_toggle_button_get_active(
								GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_debug"))) != 0);

		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")), m_kernelCtx.getLogManager().isActive(LogLevel_Debug));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")),
			m_kernelCtx.getLogManager().isActive(LogLevel_Benchmark));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")), m_kernelCtx.getLogManager().isActive(LogLevel_Trace));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")), m_kernelCtx.getLogManager().isActive(LogLevel_Info));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")),
			m_kernelCtx.getLogManager().isActive(LogLevel_Warning));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")),
			m_kernelCtx.getLogManager().isActive(LogLevel_ImportantWarning));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")), m_kernelCtx.getLogManager().isActive(LogLevel_Error));
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")), m_kernelCtx.getLogManager().isActive(LogLevel_Fatal));
	}

	gtk_widget_destroy(GTK_WIDGET(l_pLogLevelDialog));
	g_object_unref(l_pBuilderInterface);
}

void CApplication::CPUUsageCB()

{
	CInterfacedScenario* currentInterfacedScenario = getCurrentInterfacedScenario();
	if (currentInterfacedScenario)
	{
		currentInterfacedScenario->m_DebugCPUUsage = (gtk_toggle_button_get_active(
														  GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage"))) !=
													  0);
		currentInterfacedScenario->redraw();
	}
}

void CApplication::changeCurrentScenario(const int pageIdx)
{
	if (m_isQuitting) { return; }

	//hide window manager of previously active scenario, if any
	const int i = gtk_notebook_get_current_page(m_pScenarioNotebook);
	if (i >= 0 && i < int(m_Scenarios.size())) { m_Scenarios[i]->hideCurrentVisualization(); }

	//closing last open scenario
	if (pageIdx == -1)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), false);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")),
											 G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* l_pWindowManagerButton = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		// toggle off and reset scenario settings
		GtkWidget* settingsVBox = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_vbox"));

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), false);


		GList* settingWidgets = gtk_container_get_children(
			GTK_CONTAINER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_vbox")));
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
		CInterfacedScenario* currentInterfacedScenario = m_Scenarios[pageIdx];
		const EPlayerStatus playerStatus               = (currentInterfacedScenario->m_Player ? currentInterfacedScenario->m_Player->getStatus()
															  : PlayerStatus_Stop);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), playerStatus != PlayerStatus_Stop);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), playerStatus != PlayerStatus_Forward);
		gtk_tool_button_set_stock_id(
			GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),
			(playerStatus == PlayerStatus_Stop || playerStatus == PlayerStatus_Pause) ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE);

		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_undo")), currentInterfacedScenario->m_StateStack->isUndoPossible());
		gtk_widget_set_sensitive(
			GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_redo")), currentInterfacedScenario->m_StateStack->isRedoPossible());

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")),
											 G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), playerStatus == PlayerStatus_Stop);
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), currentInterfacedScenario->m_DebugCPUUsage);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "openvibe-button_save")), currentInterfacedScenario->m_hasFileName && currentInterfacedScenario->m_hasBeenModified);
		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "openvibe-menu_save")),   currentInterfacedScenario->m_hasFileName && currentInterfacedScenario->m_hasBeenModified);

		//don't show window manager if in offline mode and it is toggled off
		if (playerStatus == PlayerStatus_Stop && !m_Scenarios[pageIdx]->isDesignerVisualizationToggled())
		{
			m_Scenarios[pageIdx]->hideCurrentVisualization();

			// we are in edition mode, updating internal configuration token
			std::string l_sPath = m_Scenarios[pageIdx]->m_Filename;
			l_sPath             = l_sPath.substr(0, l_sPath.rfind('/'));
			m_kernelCtx.getConfigurationManager().setConfigurationTokenValue(
				m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier("Player_ScenarioDirectory"), l_sPath.c_str());
			m_kernelCtx.getConfigurationManager().setConfigurationTokenValue(
				m_kernelCtx.getConfigurationManager().lookUpConfigurationTokenIdentifier("__volatile_ScenarioDir"), l_sPath.c_str());
		}
		else { m_Scenarios[pageIdx]->showCurrentVisualization(); }

		//update window manager button state
		GtkWidget* l_pWindowManagerButton = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), m_Scenarios[pageIdx]->isDesignerVisualizationToggled());
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), true);
		m_Scenarios[pageIdx]->redrawScenarioSettings();
		m_Scenarios[pageIdx]->redrawScenarioInputSettings();
		m_Scenarios[pageIdx]->redrawScenarioOutputSettings();


		// current scenario is the selected one
		m_currentScenarioIdx = pageIdx;
	}
		//first scenario is created (or a scenario is opened and replaces first unnamed unmodified scenario)
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")), true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_undo")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_redo")), false);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")),
											 G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* l_pWindowManagerButton = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), true);

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
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner")),
								  round(getCurrentInterfacedScenario()->getScale() * 100.0));
	}
	else { gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner")), 100); }
}

void CApplication::reorderCurrentScenario(const uint32_t newPageIdx)
{
	CInterfacedScenario* currentInterfacedScenario = m_Scenarios[m_currentScenarioIdx];
	m_Scenarios.erase(m_Scenarios.begin() + m_currentScenarioIdx);
	m_Scenarios.insert(m_Scenarios.begin() + newPageIdx, currentInterfacedScenario);

	this->changeCurrentScenario(newPageIdx);
}

//Increase the zoom of the current scenario
void CApplication::zoomInCB()
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner")),
							  round(getCurrentInterfacedScenario()->getScale() * 100.0) + 5);
}

//Decrease the zoom of the current scenario
void CApplication::zoomOutCB()
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner")),
							  round(getCurrentInterfacedScenario()->getScale() * 100.0) - 5);
}

void CApplication::spinnerZoomChangedCB(const uint32_t scaleDelta)
{
	if (getCurrentInterfacedScenario() != nullptr) { getCurrentInterfacedScenario()->setScale(double(scaleDelta) / 100.0); }
}

void CApplication::cannotSaveScenarioBeforeUpdate()

{
	const CString message = "Cannot save a scenario if deprecated I/O or Settings are still pending.\n"
			"Please handle or delete all pending deprecated I/O before saving scenario.";
	GtkWidget* l_pDialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
												  GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message.toASCIIString());
	gtk_dialog_run(GTK_DIALOG(l_pDialog));
	gtk_widget_destroy(l_pDialog);
}
