#include "ovd_base.h"

#include <boost/filesystem.hpp>

#include <system/ovCTime.h>
#include <system/ovCMemory.h>
#include <stack>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <cstring>
#include <cstdlib>
#include <algorithm>

#if defined TARGET_OS_Windows
#include "system/WindowsUtilities.h"
#include "windows.h"
#endif

#include <openvibe/ovITimeArithmetics.h>
#include <visualization-toolkit/ovviz_defines.h>
#include <visualization-toolkit/ovvizIVisualizationContext.h>
#include <ovp_global_defines.h>

#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
#include <strings.h>
#define _strcmpi strcasecmp
#endif

#define OVD_GUI_File          OpenViBE::Directories::getDataDir() + "/applications/designer/interface.ui"
#define OVD_GUI_AboutDialog_File OpenViBE::Directories::getDataDir() + "/applications/designer/about-dialog.ui"
#define OVD_GUI_Settings_File OpenViBE::Directories::getDataDir() + "/applications/designer/interface-settings.ui"
#define OVD_AttributeId_ScenarioFilename OpenViBE::CIdentifier(0x4C536D0A, 0xB23DC545)
#define OVD_README_File                  OpenViBE::Directories::getDistRootDir() + "/ReadMe.txt"
static const unsigned int s_RecentFileNumber = 10;

#include "ovdCDesignerVisualization.h"
#include "ovdCPlayerVisualization.h"
#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"
#include "ovdCLogListenerDesigner.h"
#include <metabox-loader/mCMetaboxLoader.h>

#include "visualization/ovdCVisualizationManager.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	// because std::tolower has multiple signatures,
	// it can not be easily used in std::transform
	// this workaround is taken from http://www.gcek.net/ref/books/sw/cpp/ticppv2/
	template <class charT>
	charT to_lower(charT c)
	{
		return tolower(c);
	}

};

namespace
{
	struct SBoxProto : public IBoxProto
	{
	public:

		SBoxProto(void)
			:m_bIsDeprecated(false)
			,m_ui64InputCountHash  (0x64AC3CB54A35888CLL)
			,m_ui64OutputCountHash (0x21E0FAAFE5CAF1E1LL)
			,m_ui64SettingCountHash(0x6BDFB15B54B09F63LL)
		{
		}
		virtual OpenViBE::boolean addInput(const CString& sName, const CIdentifier& rTypeIdentifier)
		{
			uint64 v=rTypeIdentifier.toUInteger();
			swap_byte(v, m_ui64InputCountHash);
			swap_byte(m_ui64InputCountHash, 0x7936A0F3BD12D936LL);
			m_oHash=m_oHash.toUInteger()^v;
			return true;
		}
		//
		virtual OpenViBE::boolean addOutput(const CString& sName, const CIdentifier& rTypeIdentifier)
		{
			uint64 v=rTypeIdentifier.toUInteger();
			swap_byte(v, m_ui64OutputCountHash);
			swap_byte(m_ui64OutputCountHash, 0xCBB66A5B893AA4E9LL);
			m_oHash=m_oHash.toUInteger()^v;
			return true;
		}
		virtual OpenViBE::boolean addSetting(const CString& sName, const CIdentifier& rTypeIdentifier, const CString& sDefaultValue, const bool bModifiable)
		{
			uint64 v=rTypeIdentifier.toUInteger();
			swap_byte(v, m_ui64SettingCountHash);
			swap_byte(m_ui64SettingCountHash, 0x3C87F3AAE9F8303BLL);
			m_oHash=m_oHash.toUInteger()^v;
			return true;
		}
		virtual OpenViBE::boolean addInputSupport(const OpenViBE::CIdentifier &rTypeIdentifier)
		{
			return true;
		}

		virtual OpenViBE::boolean addOutputSupport(const OpenViBE::CIdentifier &rTypeIdentifier)
		{
			return true;
		}

		virtual OpenViBE::boolean addFlag(const EBoxFlag eBoxFlag)
		{
			switch(eBoxFlag)
			{
				case BoxFlag_CanAddInput:       m_oHash=m_oHash.toUInteger()^OpenViBE::CIdentifier(0x07507AC8, 0xEB643ACE).toUInteger(); break;
				case BoxFlag_CanModifyInput:    m_oHash=m_oHash.toUInteger()^OpenViBE::CIdentifier(0x5C985376, 0x8D74CDB8).toUInteger(); break;
				case BoxFlag_CanAddOutput:      m_oHash=m_oHash.toUInteger()^OpenViBE::CIdentifier(0x58DEA69B, 0x12411365).toUInteger(); break;
				case BoxFlag_CanModifyOutput:   m_oHash=m_oHash.toUInteger()^OpenViBE::CIdentifier(0x6E162C01, 0xAC979F22).toUInteger(); break;
				case BoxFlag_CanAddSetting:     m_oHash=m_oHash.toUInteger()^OpenViBE::CIdentifier(0xFA7A50DC, 0x2140C013).toUInteger(); break;
				case BoxFlag_CanModifySetting:  m_oHash=m_oHash.toUInteger()^OpenViBE::CIdentifier(0x624D7661, 0xD8DDEA0A).toUInteger(); break;
				case BoxFlag_IsDeprecated:      m_bIsDeprecated=true; break;
				default:
					return false;
					break;
			}
			return true;
		}
		void swap_byte(uint64& v, const uint64 s)
		{
			uint8 t;
			uint8 V[sizeof(v)];
			uint8 S[sizeof(s)];
			System::Memory::hostToLittleEndian(v, V);
			System::Memory::hostToLittleEndian(s, S);
			for(uint32 i=0; i<sizeof(s); i+=2)
			{
				uint32 j=S[i  ]%sizeof(v);
				uint32 k=S[i+1]%sizeof(v);
				t=V[j];
				V[j]=V[k];
				V[k]=t;
			}
			System::Memory::littleEndianToHost(V, &v);
		}

		_IsDerivedFromClass_Final_(IBoxProto, OV_UndefinedIdentifier)

		CIdentifier m_oHash;
		OpenViBE::boolean m_bIsDeprecated;
		uint64 m_ui64InputCountHash;
		uint64 m_ui64OutputCountHash;
		uint64 m_ui64SettingCountHash;
	};
}

namespace
{
	extern "C" G_MODULE_EXPORT void open_url_mensia_cb(GtkWidget* pWidget, gpointer data)
	{
	#ifdef TARGET_OS_Windows
		system("start http://mensiatech.com");
	#endif
	}

	::guint __g_idle_add__(::GSourceFunc fpCallback, ::gpointer pUserData, ::gint iPriority=G_PRIORITY_DEFAULT_IDLE)
	{
		::GSource* l_pSource=g_idle_source_new();
		g_source_set_priority(l_pSource, G_PRIORITY_LOW);
		g_source_set_callback(l_pSource, fpCallback, pUserData, NULL);
		return g_source_attach(l_pSource, NULL);
	}

	::guint __g_timeout_add__(::guint uiInterval, ::GSourceFunc fpCallback, ::gpointer pUserData, ::gint iPriority=G_PRIORITY_DEFAULT)
	{
		::GSource* l_pSource=g_timeout_source_new(uiInterval);
		g_source_set_priority(l_pSource, G_PRIORITY_LOW);
		g_source_set_callback(l_pSource, fpCallback, pUserData, NULL);
		return g_source_attach(l_pSource, NULL);
	}

	void drag_data_get_cb(::GtkWidget* pWidget, ::GdkDragContext* pDragContex, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->dragDataGetCB(pWidget, pDragContex, pSelectionData, uiInfo, uiT);
	}
	void menu_undo_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->undoCB();
	}
	void menu_redo_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->redoCB();
	}
	void menu_focus_search_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));
	}
	void menu_copy_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->copySelectionCB();
	}
	void menu_cut_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->cutSelectionCB();
	}
	void menu_paste_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->pasteSelectionCB();
	}
	void menu_delete_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->deleteSelectionCB();
	}
	void menu_preferences_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->preferencesCB();
	}

	void menu_test_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->testCB();
	}
	void menu_new_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->newScenarioCB();
	}
	void menu_open_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->openScenarioCB();
	}
	void menu_open_recent_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		const gchar* fileName = gtk_menu_item_get_label(pMenuItem);
		static_cast<CApplication*>(pUserData)->openScenario(fileName);
	}
	void menu_save_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioCB();
	}
	void menu_save_scenario_as_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioAsCB();
	}
	void menu_close_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->closeScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}
	void menu_quit_application_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		if(static_cast<CApplication*>(pUserData)->quitApplicationCB())
		{
			gtk_main_quit();
		}
	}

	void menu_about_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}
	void menu_about_openvibe_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutOpenViBECB();
	}
	void menu_about_link_clicked_cb(::GtkAboutDialog* pAboutDialog, const gchar *linkPtr, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutLinkClickedCB(linkPtr);
	}

	void menu_browse_documentation_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->browseDocumentationCB();
	}
	
	void menu_report_issue_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->reportIssueCB();
	}

	void menu_display_changelog_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		if(!static_cast<CApplication*>(pUserData)->displayChangelogWhenAvailable())
		{
			::GtkWidget* l_pInfoDialog = gtk_message_dialog_new(
				NULL,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				"No boxes were added or updated in version %s of Studio.",
				ProjectVersion
				);
			gtk_window_set_title(GTK_WINDOW(l_pInfoDialog), "No new boxes");
			gtk_dialog_run(GTK_DIALOG(l_pInfoDialog));
			gtk_widget_destroy(l_pInfoDialog);
		}
	}

	void button_new_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->newScenarioCB();
	}
	void button_open_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->openScenarioCB();
	}
	void button_save_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioCB();
	}
	void button_save_scenario_as_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioAsCB();
	}
	void button_close_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->closeScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}

	void button_undo_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->undoCB();
	}

	void button_redo_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->redoCB();
	}

#if defined TARGET_HAS_LibArchway
	void button_toggle_neurort_engine_configuration_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		auto l_pApplication = static_cast<CApplication*>(pUserData);

		l_pApplication->m_oArchwayHandlerGUI.toggleNeuroRTEngineConfigurationDialog(static_cast<bool>(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pMenuItem))));
	}
#endif

	void delete_designer_visualisation_cb(gpointer user_data)
	{
		static_cast<CApplication*>(user_data)->deleteDesignerVisualizationCB();
	}
	void button_toggle_window_manager_cb(::GtkToggleToolButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->toggleDesignerVisualizationCB();
	}

	void button_comment_cb(::GtkButton* pButton, CApplication* pApplication)
	{
		pApplication->addCommentCB(pApplication->getCurrentInterfacedScenario());
	}
	void button_about_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}

	void stop_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->stopScenarioCB();
	}
	void play_pause_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		if(std::string(gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(pButton)))==GTK_STOCK_MEDIA_PLAY)
		{
			static_cast<CApplication*>(pUserData)->playScenarioCB();
		}
		else
		{
			static_cast<CApplication*>(pUserData)->pauseScenarioCB();
		}
	}
	void next_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->nextScenarioCB();
	}
	void forward_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->forwardScenarioCB();
	}

	void button_configure_current_scenario_settings_cb(::GtkButton* pButton, CApplication* pApplication)
	{
		pApplication->configureScenarioSettingsCB(pApplication->getCurrentInterfacedScenario());
	}

	gboolean button_quit_application_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, gpointer pUserData)
	{
		if(static_cast<CApplication*>(pUserData)->quitApplicationCB())
		{
			gtk_main_quit();
			return FALSE;
		}
		return TRUE;
	}

	gboolean window_state_changed_cb(GtkWidget* pWidget, GdkEventWindowState* pEvent, gpointer pUserData)
	{
		if(pEvent->changed_mask && GDK_WINDOW_STATE_MAXIMIZED)
		{
			// window has changed from maximized to not maximized or the other way around
			static_cast<CApplication*>(pUserData)->windowStateChangedCB(pEvent->new_window_state && GDK_WINDOW_STATE_MAXIMIZED);
		}
		return TRUE;
	}

	void log_level_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->logLevelCB();
	}

	void cpu_usage_cb(::GtkToggleButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->CPUUsageCB();
	}

	gboolean change_current_scenario_cb(::GtkNotebook* pNotebook, ::GtkNotebookPage* pNotebookPage, guint uiPageNumber, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->changeCurrentScenario((int32)uiPageNumber);
		return TRUE;
	}

	gboolean reorder_scenario_cb(::GtkNotebook* pNotebook, ::GtkNotebookPage* pNotebookPage, guint uiPageNumber, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->reorderCurrentScenario((int32)uiPageNumber);
		return TRUE;
	}

	void box_algorithm_title_button_expand_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-box_algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 0);
	}
	void box_algorithm_title_button_collapse_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-box_algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 0);
	}

	void algorithm_title_button_expand_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}
	void algorithm_title_button_collapse_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}

	void clear_messages_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CLogListenerDesigner*>(pUserData)->clearMessages();
	}

	void add_scenario_input_cb(GtkButton*, CApplication* pApplication)
	{
		pApplication->getCurrentInterfacedScenario()->addScenarioInputCB();
	}
	void add_scenario_output_cb(GtkButton*, CApplication* pApplication)
	{
		pApplication->getCurrentInterfacedScenario()->addScenarioOutputCB();
	}
	void add_scenario_setting_cb(GtkButton*, CApplication* pApplication)
	{
		pApplication->getCurrentInterfacedScenario()->addScenarioSettingCB();
	}

	string strtoupper(string str)
	{
		int leng=str.length();
		for(int i=0; i<leng; i++)
			if (97<=str[i]&&str[i]<=122)//a-z
				str[i]-=32;
		return str;
	}
	static gboolean box_algorithm_search_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
		/* Visible if row is non-empty and first column is "HI" */


		gboolean l_bVisible = false;
		gboolean l_bShowUnstable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-show_unstable")));

		gchar* l_sHaystackName;
		gchar* l_sHaystackDescription;
		gboolean l_bHaystackUnstable;

		gtk_tree_model_get(model, iter, Resource_StringName, &l_sHaystackName, Resource_StringShortDescription, &l_sHaystackDescription, Resource_BooleanIsUnstable, &l_bHaystackUnstable, -1);

		// consider only leaf nodes which match the search term
		if (l_sHaystackName!=NULL && l_sHaystackDescription!=NULL)
		{
			if ((l_bShowUnstable || !l_bHaystackUnstable) && (string::npos != strtoupper(l_sHaystackName).find(strtoupper(l_pApplication->m_sSearchTerm)) || string::npos != strtoupper(l_sHaystackDescription).find(strtoupper(l_pApplication->m_sSearchTerm)) || gtk_tree_model_iter_has_child(model, iter)))
			{
				//std::cout << "value : " << l_pApplication->m_sSearchTerm << "\n";
				l_bVisible = true;
			}

			g_free(l_sHaystackName);
			g_free(l_sHaystackDescription);
		}
		else
		{
			l_bVisible = true;
		}

		return l_bVisible;
	}

	static gboolean box_algorithm_prune_empty_folders(GtkTreeModel *model, GtkTreeIter *iter, gpointer pUserData)
	{
		gboolean l_bIsPlugin;
		gtk_tree_model_get(model, iter, Resource_BooleanIsPlugin, &l_bIsPlugin, -1);

		if (gtk_tree_model_iter_has_child(model, iter) || l_bIsPlugin)
		{
			return true;
		}

		return false;
	}

	static gboolean	do_refilter( CApplication *pApplication )
	{
		/*
		if (0 == strcmp(pApplication->m_sSearchTerm, ""))
		{
			// reattach the old model
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModel));
		}
		else
		*/
		{
			pApplication->m_pBoxAlgorithmTreeModelFilter = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModel), NULL);
			pApplication->m_pBoxAlgorithmTreeModelFilter2 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter), NULL);
			pApplication->m_pBoxAlgorithmTreeModelFilter3 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter2), NULL);
			pApplication->m_pBoxAlgorithmTreeModelFilter4 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter3), NULL);
			// detach the normal model from the treeview
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, NULL);

			// clear the model

			// add a filtering function to the model
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter), box_algorithm_search_func, pApplication, NULL );
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter2), box_algorithm_prune_empty_folders, pApplication, NULL );
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter3), box_algorithm_prune_empty_folders, pApplication, NULL );
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter4), box_algorithm_prune_empty_folders, pApplication, NULL );

			// attach the model to the treeview
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter4));

			if (0 == strcmp(pApplication->m_sSearchTerm, ""))
			{
				gtk_tree_view_collapse_all(pApplication->m_pBoxAlgorithmTreeView);
			}
			else
			{
				gtk_tree_view_expand_all(pApplication->m_pBoxAlgorithmTreeView);
			}
		}

		pApplication->m_giFilterTimeout = 0;

		return false;
	}

	static void	queue_refilter( CApplication* pApplication )
	{
		if( pApplication->m_giFilterTimeout )
			g_source_remove( pApplication->m_giFilterTimeout );

		pApplication->m_giFilterTimeout = g_timeout_add( 300, (GSourceFunc)do_refilter, pApplication );
	}

	void refresh_search_cb(::GtkEntry* pTextfield, CApplication* pApplication)
	{
		pApplication->m_sSearchTerm = gtk_entry_get_text(pTextfield);

		queue_refilter(pApplication);
	}

	void refresh_search_no_data_cb(::GtkToggleButton* pToggleButton, CApplication* pApplication)
	{
		pApplication->m_sSearchTerm = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));

		queue_refilter(pApplication);
	}

	static gboolean searchbox_select_all_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, CApplication* pApplication)
	{
		// we select the current search
		gtk_widget_grab_focus(pWidget); // we must grab or selection wont work. It also triggers the other CBs.
		gtk_editable_select_region(GTK_EDITABLE(pWidget), 0, -1);
		return false;
	}

	static gboolean searchbox_focus_in_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), false);
		return false;
	}

	static gboolean searchbox_focus_out_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), true);
		gtk_editable_select_region(GTK_EDITABLE(pWidget), 0, 0);
		return false;
	}
	
	static void about_newversion_button_display_changelog_cb(::GtkButton* pButton, gpointer pUserData)
	{
#if defined TARGET_OS_Windows
		System::WindowsUtilities::utf16CompliantShellExecute(NULL, "open", (OVD_README_File).toASCIIString(), NULL, NULL, SHOW_OPENWINDOW);
#endif
	}

	gboolean idle_application_loop(gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
#if defined TARGET_HAS_LibArchway
		if (l_pApplication->m_oArchwayHandler.isEngineStarted())
		{
			l_pApplication->m_oArchwayHandler.loopEngine();
		}
#endif

		CInterfacedScenario* l_pCurrentInterfacedScenario=l_pApplication->getCurrentInterfacedScenario();
		if(l_pCurrentInterfacedScenario)
		{
			if(l_pApplication->getPlayer() && l_pCurrentInterfacedScenario->m_ePlayerStatus != l_pApplication->getPlayer()->getStatus())
			{
				switch(l_pApplication->getPlayer()->getStatus())
				{
					case PlayerStatus_Stop:    gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_stop")), "clicked"); break;
					case PlayerStatus_Pause:   while(l_pCurrentInterfacedScenario->m_ePlayerStatus != PlayerStatus_Pause) gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_play_pause")), "clicked"); break;
					case PlayerStatus_Step:          break;
					case PlayerStatus_Play:    while(l_pCurrentInterfacedScenario->m_ePlayerStatus != PlayerStatus_Play)  gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_play_pause")), "clicked"); break;
					case PlayerStatus_Forward: gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_forward")), "clicked"); break;
					default: std::cout << "unhandled player status : " << l_pApplication->getPlayer()->getStatus() << " :(\n"; break;
				}
			}
			else
			{
				float64 l_f64Time=(l_pCurrentInterfacedScenario->m_pPlayer? ITimeArithmetics::timeToSeconds(l_pCurrentInterfacedScenario->m_pPlayer->getCurrentSimulatedTime()) : 0);
				if(l_pApplication->m_ui64LastTimeRefresh!=l_f64Time)
				{
					l_pApplication->m_ui64LastTimeRefresh=static_cast<uint64>(l_f64Time);

					uint32 l_ui32Milli  = ((uint32)(l_f64Time*1000)%1000);
					uint32 l_ui32Seconds=  ((uint32)l_f64Time)%60;
					uint32 l_ui32Minutes= (((uint32)l_f64Time)/60)%60;
					uint32 l_ui32Hours  =((((uint32)l_f64Time)/60)/60);

					float64 l_f64CPUUsage=(l_pCurrentInterfacedScenario->m_pPlayer?l_pCurrentInterfacedScenario->m_pPlayer->getCPUUsage():0);

					char l_sTime[1024];
					if(l_ui32Hours)				sprintf(l_sTime, "Time : %02dh %02dm %02ds %03dms", l_ui32Hours, l_ui32Minutes, l_ui32Seconds, l_ui32Milli);
					else if(l_ui32Minutes)		sprintf(l_sTime, "Time : %02dm %02ds %03dms", l_ui32Minutes, l_ui32Seconds, l_ui32Milli);
					else if(l_ui32Seconds)		sprintf(l_sTime, "Time : %02ds %03dms", l_ui32Seconds, l_ui32Milli);
					else						sprintf(l_sTime, "Time : %03dms", l_ui32Milli);

					gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-label_current_time")), l_sTime);

					char l_sCPU[1024];
					sprintf(l_sCPU, "%3.01f%%", l_f64CPUUsage);

					gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), l_f64CPUUsage*.01);
					gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), l_sCPU);
					if(l_pCurrentInterfacedScenario->m_pPlayer&&l_pCurrentInterfacedScenario->m_bDebugCPUUsage)
					{
						// redraws scenario
						l_pCurrentInterfacedScenario->redraw();
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

		if(!l_pApplication->hasRunningScenario())
		{
			System::Time::sleep(50);
		}

		return TRUE;
	}

	gboolean idle_scenario_loop(gpointer pUserData)
	{
		CInterfacedScenario* l_pInterfacedScenario=static_cast<CInterfacedScenario*>(pUserData);
		uint64 l_ui64CurrentTime=System::Time::zgetTime();
		if(l_pInterfacedScenario->m_ui64LastLoopTime == uint64(-1))
		{
			l_pInterfacedScenario->m_ui64LastLoopTime = l_ui64CurrentTime;
		}
		l_pInterfacedScenario->m_pPlayer->setFastForwardMaximumFactor(::gtk_spin_button_get_value(l_pInterfacedScenario->m_rApplication.m_pFastForwardFactor));
		if (!l_pInterfacedScenario->m_pPlayer->loop(l_ui64CurrentTime-l_pInterfacedScenario->m_ui64LastLoopTime))
		{
			l_pInterfacedScenario->m_rApplication.stopInterfacedScenarioAndReleasePlayer(l_pInterfacedScenario);
		}
		l_pInterfacedScenario->m_ui64LastLoopTime=l_ui64CurrentTime;
		return TRUE;
	}

	gboolean timeout_application_loop(gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
		if(!l_pApplication->hasRunningScenario() && l_pApplication->m_eCommandLineFlags&CommandLineFlag_NoGui)
		{
			l_pApplication->quitApplicationCB();
			gtk_main_quit();
			return FALSE;
		}
		return TRUE;
	}

	/**
	* Function called in gtk loop: to check each 0.1second if a message was sent by a second instance of Studio
	* (Meaning that someone tried to reopen Studio and this instance has to do something)
	**/
	gboolean receiveSecondInstanceMessage(gpointer pUserData)
	{
		try
		{	// Open or create ensures that 
			boost::interprocess::named_mutex l_oMutex(boost::interprocess::open_or_create, MUTEX_NAME);
			{
				boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(l_oMutex);
				CApplication* l_pApplication = static_cast<CApplication*>(pUserData);
				//Tries to open a message, if fails, go to catch
				boost::interprocess::message_queue l_oMessage(boost::interprocess::open_only, MESSAGE_NAME);
				l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- A message was detected \n";

				// Whatever contains the message the first instance should try to take the focus
				gtk_window_present(GTK_WINDOW(l_pApplication->m_pMainWindow));
				size_t recvd_size;
				unsigned int priority = 0;
				char l_pBuffer[2048];
				if ( l_oMessage.try_receive(&l_pBuffer, sizeof(l_pBuffer), recvd_size, priority))
				{
					l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- Remove message: \n";
					boost::interprocess::message_queue::remove(MESSAGE_NAME);

					int32 l_iMode = 0;
					char l_sScenarioPath[1024];
					char* l_sMessage = strtok(l_pBuffer, ";");
					while(l_sMessage != NULL)
					{
						sscanf(l_sMessage, "%1d : <%1024[^>]> ", &l_iMode, &l_sScenarioPath);
						switch(l_iMode)
						{
						case MessageType_OpenScenario:
							l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- Open scenario: " << l_sScenarioPath << "\n";
							l_pApplication->openScenario(l_sScenarioPath);
							break;
						case MessageType_PlayScenario:
							l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- Play scenario: " << l_sScenarioPath << "\n";
							if(l_pApplication->openScenario(l_sScenarioPath))
							{
								l_pApplication->playScenarioCB();
							}
							break;
						case MessageType_PlayFastScenario:
							l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- Play fast scenario: " << l_sScenarioPath << "\n";
							if(l_pApplication->openScenario(l_sScenarioPath))
							{
								l_pApplication->forwardScenarioCB();
							}
							break;
						}
						l_iMode = 0;
						l_sMessage = strtok(NULL, ";");
					}
				}
				else
				{
					l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "ovdCApplication::receiveSecondInstanceMessage- Remove message: \n";
					boost::interprocess::message_queue::remove(MESSAGE_NAME);
				}
			}
		}
		// An interprocess_exception is throwed if no messages could be found, in this case we do nothing
		catch(boost::interprocess::interprocess_exception) {}
		return TRUE;
	}
}

static ::GtkTargetEntry g_vTargetEntry[]= {
	{ (gchar*)"STRING", 0, 0 },
	{ (gchar*)"text/plain", 0, 0 } };

CApplication::CApplication(const IKernelContext& rKernelContext)
	:m_rKernelContext(rKernelContext)
	,m_pPluginManager(NULL)
	,m_pScenarioManager(NULL)
	,m_pVisualizationManager(NULL)
	,m_pClipboardScenario(NULL)
	,m_eCommandLineFlags(CommandLineFlag_None)
	,m_pBuilderInterface(NULL)
	,m_pMainWindow(NULL)
	,m_pSplashScreen(NULL)
	,m_pScenarioNotebook(NULL)
	,m_pResourceNotebook(NULL)
	,m_pBoxAlgorithmTreeModel(NULL)
	,m_pBoxAlgorithmTreeModelFilter(NULL)
	,m_pBoxAlgorithmTreeModelFilter2(NULL)
	,m_pBoxAlgorithmTreeModelFilter3(NULL)
	,m_pBoxAlgorithmTreeModelFilter4(NULL)
	,m_pBoxAlgorithmTreeView(NULL)
	,m_pAlgorithmTreeModel(NULL)
	,m_pAlgorithmTreeView(NULL)
	,m_pFastForwardFactor(NULL)
	,m_pConfigureSettingsAddSettingButton(NULL)
	,m_MenuOpenRecent(NULL)
	,m_pTableInputs(NULL)
	,m_pTableOutputs(NULL)
	,m_giFilterTimeout(0)
	,m_bIsMaximized(false)
	,m_ui64LastTimeRefresh(0)
	,m_bIsQuitting(false)
	,m_bIsNewVersion(false)
	,m_ui32CurrentInterfacedScenarioIndex(0)
    ,m_pMetaboxLoader(NULL)
#if defined TARGET_HAS_LibArchway
    ,m_oArchwayHandler(rKernelContext)
    ,m_oArchwayHandlerGUI(m_oArchwayHandler)
#endif
{
	m_pPluginManager=&m_rKernelContext.getPluginManager();
	m_pScenarioManager=&m_rKernelContext.getScenarioManager();
	m_pVisualizationManager = new CVisualizationManager(m_rKernelContext);
	m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(m_rKernelContext.getPluginManager().createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
	m_visualizationContext->setManager(m_pVisualizationManager);
	m_pLogListenerDesigner = NULL;
	m_pMetaboxLoader = new Mensia::CMetaboxLoader(m_rKernelContext);

	m_rKernelContext.getConfigurationManager().createConfigurationToken("Player_ScenarioDirectory", "");
	m_rKernelContext.getConfigurationManager().createConfigurationToken("__volatile_ScenarioDir", "");
}

CApplication::~CApplication(void)
{
	if(m_pBuilderInterface)
	{
		m_rKernelContext.getLogManager().removeListener(m_pLogListenerDesigner);
		// @FIXME this likely still does not deallocate the actual widgets allocated by add_from_file
		g_object_unref(G_OBJECT(m_pBuilderInterface));
		m_pBuilderInterface = NULL;
	}

	delete m_pMetaboxLoader;
}

void CApplication::initialize(ECommandLineFlag eCommandLineFlags)
{
	m_eCommandLineFlags=eCommandLineFlags;
	m_sSearchTerm = "";

	// Prepares scenario clipboard
	CIdentifier l_oClipboardScenarioIdentifier;
	if(m_pScenarioManager->createScenario(l_oClipboardScenarioIdentifier))
	{
		m_pClipboardScenario=&m_pScenarioManager->getScenario(l_oClipboardScenarioIdentifier);
	}

	m_pBuilderInterface=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "openvibe", NULL);
	gtk_builder_add_from_file(m_pBuilderInterface, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(m_pBuilderInterface, NULL);

	m_pMainWindow=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe"));

	// Catch delete events when close button is clicked
	g_signal_connect(m_pMainWindow, "delete_event", G_CALLBACK(button_quit_application_cb), this);
	// be notified on maximize/minimize events
	g_signal_connect(m_pMainWindow, "window-state-event", G_CALLBACK(window_state_changed_cb), this);

	// Connects menu actions
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_undo")),        "activate", G_CALLBACK(menu_undo_cb),               this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_redo")),        "activate", G_CALLBACK(menu_redo_cb),               this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_focus_search")),"activate", G_CALLBACK(menu_focus_search_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_copy")),        "activate", G_CALLBACK(menu_copy_selection_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_cut")),         "activate", G_CALLBACK(menu_cut_selection_cb),      this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_paste")),       "activate", G_CALLBACK(menu_paste_selection_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_delete")),      "activate", G_CALLBACK(menu_delete_selection_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_preferences")), "activate", G_CALLBACK(menu_preferences_cb),        this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_new")),         "activate", G_CALLBACK(menu_new_scenario_cb),       this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_open")),        "activate", G_CALLBACK(menu_open_scenario_cb),      this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save")),        "activate", G_CALLBACK(menu_save_scenario_cb),      this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save_as")),     "activate", G_CALLBACK(menu_save_scenario_as_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_close")),       "activate", G_CALLBACK(menu_close_scenario_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_quit")),        "activate", G_CALLBACK(menu_quit_application_cb),   this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_about")),          "activate", G_CALLBACK(menu_about_openvibe_cb),  this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_scenario_about")), "activate", G_CALLBACK(menu_about_scenario_cb),  this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_documentation")),  "activate", G_CALLBACK(menu_browse_documentation_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_issue_report")),  "activate", G_CALLBACK(menu_report_issue_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_display_changelog")),  "activate", G_CALLBACK(menu_display_changelog_cb),   this);

	// g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_test")),        "activate", G_CALLBACK(menu_test_cb),               this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_new")),       "clicked",  G_CALLBACK(button_new_scenario_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_open")),      "clicked",  G_CALLBACK(button_open_scenario_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save")),      "clicked",  G_CALLBACK(button_save_scenario_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save_as")),   "clicked",  G_CALLBACK(button_save_scenario_as_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_close")), "clicked", G_CALLBACK(button_close_scenario_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_undo")), "clicked", G_CALLBACK(button_undo_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_redo")), "clicked", G_CALLBACK(button_redo_cb), this);

	
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_log_level")),     "clicked",  G_CALLBACK(log_level_cb),                    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), "toggled",  G_CALLBACK(button_toggle_window_manager_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       "clicked", G_CALLBACK(button_comment_cb),        this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), "clicked", G_CALLBACK(button_about_scenario_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       "clicked",  G_CALLBACK(stop_scenario_cb),          this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), "clicked",  G_CALLBACK(play_pause_scenario_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       "clicked",  G_CALLBACK(next_scenario_cb),          this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    "clicked",  G_CALLBACK(forward_scenario_cb),       this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_title_button_expand")),   "clicked", G_CALLBACK(box_algorithm_title_button_expand_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_title_button_collapse")), "clicked", G_CALLBACK(box_algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_button_expand")),   "clicked", G_CALLBACK(algorithm_title_button_expand_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_button_collapse")), "clicked", G_CALLBACK(algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "icon-press", G_CALLBACK(searchbox_select_all_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "changed", G_CALLBACK(refresh_search_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "focus-in-event", G_CALLBACK(searchbox_focus_in_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "focus-out-event", G_CALLBACK(searchbox_focus_out_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")), "toggled", G_CALLBACK(refresh_search_no_data_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), "clicked", G_CALLBACK(button_configure_current_scenario_settings_cb), this);
#if defined TARGET_HAS_LibArchway
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration")),       "clicked", G_CALLBACK(button_toggle_neurort_engine_configuration_cb),   this);
	m_oArchwayHandlerGUI.m_pButtonOpenEngineConfigurationDialog = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration"));
#endif

#if !defined(TARGET_OS_Windows) && !defined(TARGET_OS_Linux)
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_issue_report")));
#endif
	// Prepares fast forward feature
	float64 l_f64FastForwardFactor=m_rKernelContext.getConfigurationManager().expandAsFloat("${Designer_FastForwardFactor}", -1);
	m_pFastForwardFactor=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-spinbutton_fast-forward-factor"));
	if(l_f64FastForwardFactor==-1)
	{
		::gtk_spin_button_set_value(m_pFastForwardFactor, 100);
	}
	else
	{
		::gtk_spin_button_set_value(m_pFastForwardFactor, l_f64FastForwardFactor);
	}

#if defined(TARGET_OS_Windows)
#if GTK_CHECK_VERSION(2,24,0)
	// expect it to work */
#else
	gtk_about_dialog_set_url_hook ((GtkAboutDialogActivateLinkFunc)menu_about_link_clicked_cb, this, NULL);
#endif
#endif

	__g_idle_add__(idle_application_loop, this);
	__g_timeout_add__(1000, timeout_application_loop, this);
#if _NDEBUG
	__g_timeout_add__(100, receiveSecondInstanceMessage, this);
#endif

	// Prepares main notebooks
	m_pScenarioNotebook=GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_notebook"));
	// optional behavior: vertically stacked scenarios (filename trimming mandatory in that case).
	if(m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ScenarioTabsVerticalStack}", false))
	{
		gtk_notebook_set_tab_pos(m_pScenarioNotebook, GTK_POS_LEFT);
	}

	g_signal_connect(G_OBJECT(m_pScenarioNotebook), "switch-page", G_CALLBACK(change_current_scenario_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioNotebook), "page-reordered", G_CALLBACK(reorder_scenario_cb), this);
	m_pResourceNotebook=GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-resource_notebook"));

	// Creates an empty scnenario
	gtk_notebook_remove_page(m_pScenarioNotebook, 0);

	// Initialize menu open recent
	m_MenuOpenRecent = GTK_CONTAINER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_recent_content"));

	//newScenarioCB();
	{
		// Prepares box algorithm view
		m_pBoxAlgorithmTreeView=GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_tree"));
		::GtkTreeViewColumn* l_pTreeViewColumnName=gtk_tree_view_column_new();
		::GtkTreeViewColumn* l_pTreeViewColumnDesc=gtk_tree_view_column_new();
		::GtkCellRenderer* l_pCellRendererIcon=gtk_cell_renderer_pixbuf_new();
		::GtkCellRenderer* l_pCellRendererName=gtk_cell_renderer_text_new();
		::GtkCellRenderer* l_pCellRendererDesc=gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Name");
		gtk_tree_view_column_set_title(l_pTreeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnDesc, l_pCellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", Resource_StringStockIcon, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, "font", Resource_StringFont, "background", Resource_BackGroundColor, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnDesc, l_pCellRendererDesc, "text", Resource_StringShortDescription, "foreground", Resource_StringColor, "background", Resource_BackGroundColor, NULL);
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
		m_pBoxAlgorithmTreeModel=gtk_tree_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING);

		// Tree Storage for the searches
		gtk_tree_view_set_model(m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(m_pBoxAlgorithmTreeModel) );
	}

	{
		// Prepares algorithm view
		m_pAlgorithmTreeView=GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_tree"));
		::GtkTreeViewColumn* l_pTreeViewColumnName=gtk_tree_view_column_new();
		::GtkTreeViewColumn* l_pTreeViewColumnDesc=gtk_tree_view_column_new();
		::GtkCellRenderer* l_pCellRendererIcon=gtk_cell_renderer_pixbuf_new();
		::GtkCellRenderer* l_pCellRendererName=gtk_cell_renderer_text_new();
		::GtkCellRenderer* l_pCellRendererDesc=gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Name");
		gtk_tree_view_column_set_title(l_pTreeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnDesc, l_pCellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", Resource_StringStockIcon, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnDesc, l_pCellRendererDesc, "text", Resource_StringShortDescription, "foreground", Resource_StringColor, NULL);

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
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")), m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowUnstable}"));

		// Prepares algorithm model
		m_pAlgorithmTreeModel=gtk_tree_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING);
		gtk_tree_view_set_model(m_pAlgorithmTreeView, GTK_TREE_MODEL(m_pAlgorithmTreeModel));


	}

	m_pConfigureSettingsAddSettingButton = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "dialog_scenario_configuration-button_add_setting"));
	g_signal_connect(G_OBJECT(m_pConfigureSettingsAddSettingButton), "clicked", G_CALLBACK(add_scenario_setting_cb), this);
	// Set up the UI for adding Inputs and Outputs to the scenario

	GtkWidget* l_pScenarioLinksVBox = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_links_vbox"));

	m_pTableInputs = gtk_table_new(1, 3, FALSE);
	m_pTableOutputs = gtk_table_new(1, 3, FALSE);

	GtkWidget* l_pScrolledWindowInputs = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(l_pScrolledWindowInputs), GTK_WIDGET(m_pTableInputs));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l_pScrolledWindowInputs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* l_pScrolledWindowOutputs = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(l_pScrolledWindowOutputs), GTK_WIDGET(m_pTableOutputs));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l_pScrolledWindowOutputs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* l_pAddInputButton = gtk_button_new_with_label("Add Input");
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

	// Prepares drag & drop for box creation
	gtk_drag_source_set(GTK_WIDGET(m_pBoxAlgorithmTreeView), GDK_BUTTON1_MASK, g_vTargetEntry, sizeof(g_vTargetEntry)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(
		G_OBJECT(m_pBoxAlgorithmTreeView),
		"drag_data_get",
		G_CALLBACK(drag_data_get_cb),
		this);

	// Shows main window
	gtk_builder_connect_signals(m_pBuilderInterface, NULL);
	m_bIsMaximized = false;

	int l_iHeight = static_cast<int>(m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_EditorSizeHeight}"));
	int l_iWidth = static_cast<int>(m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_EditorSizeWidth}"));
	if(l_iHeight > 0 && l_iWidth > 0)
	{
		gtk_window_resize(GTK_WINDOW(m_pMainWindow), l_iWidth, l_iHeight);
	}

	if(m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_FullscreenEditor}"))
	{
		gtk_window_maximize(GTK_WINDOW(m_pMainWindow));
	}

	int l_iPanedPosition = static_cast<int>(m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_EditorPanedPosition}"));
	if(l_iPanedPosition > 0)
	{
		gtk_paned_set_position(GTK_PANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container")), l_iPanedPosition);
	}

	GtkNotebook* l_pSidebar = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-resource_notebook"));


	// List the notebook pages, cycle through them in reverse so we can remove pages without modifying indexes
	for (int l_iNotebookIndex = gtk_notebook_get_n_pages(l_pSidebar) - 1; l_iNotebookIndex >= 0 ; l_iNotebookIndex--)
	{
		GtkWidget* l_pTabWidget = gtk_notebook_get_nth_page(l_pSidebar, l_iNotebookIndex);
		GtkWidget* l_sTabLabel = gtk_notebook_get_tab_label(l_pSidebar, l_pTabWidget);
		if(!m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowAlgorithms}"))
		{
			if (l_sTabLabel == GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_container"))) {
				gtk_notebook_remove_page(l_pSidebar, l_iNotebookIndex);
			}
		}

		if (m_pMetaboxLoader == NULL)
		{
			if (l_sTabLabel == GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_links_title_container"))) {
				gtk_notebook_remove_page(l_pSidebar, l_iNotebookIndex);
			}
		}
	}

	// gtk_window_set_icon_name(GTK_WINDOW(m_pMainWindow), "ov-logo");
	gtk_window_set_icon_from_file(GTK_WINDOW(m_pMainWindow), Directories::getDataDir() + "/applications/designer/designer.ico", NULL);
	gtk_window_set_default_icon_from_file(Directories::getDataDir() + "/applications/designer/designer.ico", NULL);

	if(!(m_eCommandLineFlags&CommandLineFlag_NoManageSession))
	{
		CIdentifier l_oTokenIdentifier;
		char l_sVarName[1024];
		unsigned i=0;
		do
		{
			::sprintf(l_sVarName, "Designer_LastScenarioFilename_%03u", ++i);
			if((l_oTokenIdentifier=m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier(l_sVarName))!=OV_UndefinedIdentifier)
			{
				CString l_sFilename;
				l_sFilename=m_rKernelContext.getConfigurationManager().getConfigurationTokenValue(l_oTokenIdentifier);
				l_sFilename=m_rKernelContext.getConfigurationManager().expand(l_sFilename);
				m_rKernelContext.getLogManager() << LogLevel_Trace << "Restoring scenario [" << l_sFilename << "]\n";
				if(!this->openScenario(l_sFilename.toASCIIString()))
				{
					m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Failed to restore scenario [" << l_sFilename << "]\n";
				}
			}
		}
		while(l_oTokenIdentifier!=OV_UndefinedIdentifier);
	}

	CIdentifier l_oTokenIdentifier;
	char l_sVarName[1024];
	unsigned i = 0;
	do
	{
		::sprintf(l_sVarName, "Designer_RecentScenario_%03u", ++i);
		if ((l_oTokenIdentifier = m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier(l_sVarName)) != OV_UndefinedIdentifier)
		{
			CString l_sFilename;
			l_sFilename = m_rKernelContext.getConfigurationManager().getConfigurationTokenValue(l_oTokenIdentifier);
			l_sFilename = m_rKernelContext.getConfigurationManager().expand(l_sFilename);

			GtkWidget* newRecentItem = gtk_image_menu_item_new_with_label(l_sFilename.toASCIIString());
			g_signal_connect(G_OBJECT(newRecentItem), "activate", G_CALLBACK(menu_open_recent_scenario_cb), this);
			gtk_menu_shell_append(GTK_MENU_SHELL(m_MenuOpenRecent), newRecentItem);
			gtk_widget_show(newRecentItem);
			m_RecentScenarios.push_back(newRecentItem);
		}
	} while (l_oTokenIdentifier != OV_UndefinedIdentifier);

	refresh_search_no_data_cb(NULL, this);
	// Add the designer log listener
	CString l_sLogLevel = m_rKernelContext.getConfigurationManager().expand("${Kernel_ConsoleLogLevel}");
	string l_sValue(l_sLogLevel.toASCIIString());
	transform(l_sValue.begin(), l_sValue.end(), l_sValue.begin(), ::to_lower<std::string::value_type>);
	ELogLevel l_eLogLevel = LogLevel_Debug;
	if(l_sValue=="debug")                    l_eLogLevel = LogLevel_Debug;
	if(l_sValue=="benchmarking / profiling") l_eLogLevel = LogLevel_Benchmark;
	if(l_sValue=="trace")                    l_eLogLevel = LogLevel_Trace;
	if(l_sValue=="information")              l_eLogLevel = LogLevel_Info;
	if(l_sValue=="warning")                  l_eLogLevel = LogLevel_Warning;
	if(l_sValue=="important warning")        l_eLogLevel = LogLevel_ImportantWarning;
	if(l_sValue=="error")                    l_eLogLevel = LogLevel_Error;
	if(l_sValue=="fatal error")              l_eLogLevel = LogLevel_Fatal;

	switch(l_eLogLevel)
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

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")), m_rKernelContext.getLogManager().isActive(LogLevel_Debug));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")), m_rKernelContext.getLogManager().isActive(LogLevel_Benchmark));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")), m_rKernelContext.getLogManager().isActive(LogLevel_Trace));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")), m_rKernelContext.getLogManager().isActive(LogLevel_Info));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")), m_rKernelContext.getLogManager().isActive(LogLevel_Warning));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")), m_rKernelContext.getLogManager().isActive(LogLevel_ImportantWarning));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")), m_rKernelContext.getLogManager().isActive(LogLevel_Error));
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")), m_rKernelContext.getLogManager().isActive(LogLevel_Fatal));

	if(!(m_eCommandLineFlags&CommandLineFlag_NoGui))
	{
		m_pLogListenerDesigner = new CLogListenerDesigner(m_rKernelContext, m_pBuilderInterface);
		m_rKernelContext.getLogManager().addListener(m_pLogListenerDesigner);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_clear")),       "clicked",  G_CALLBACK(clear_messages_cb), m_pLogListenerDesigner);

		gtk_widget_show(m_pMainWindow);
	}
	// If last version of Studio used is anterior or null, then consider it as a new version
	CString l_sLastUsedVersion = m_rKernelContext.getConfigurationManager().expand("${Designer_LastVersionUsed}");
	uint32 l_uiMajor = 0;
	uint32 l_uiMinor = 0;
	sscanf(l_sLastUsedVersion.toASCIIString(), "%3u.%3u.%*u.%*u", &l_uiMajor, &l_uiMinor);
	if(l_uiMajor < M_VERSION_MAJOR
		|| (l_uiMajor == M_VERSION_MAJOR && l_uiMinor < M_VERSION_MINOR)
		|| (l_uiMajor == 0 && l_uiMinor == 0))
	{
		m_bIsNewVersion = true;
	}

	std::string l_sDefaultURLBaseString=std::string(m_rKernelContext.getConfigurationManager().expand("${Designer_HelpBrowserURLBase}"));
	// Search txt file that contains the list of boxes
	if(l_sDefaultURLBaseString.substr(l_sDefaultURLBaseString.find_last_of(".") + 1) == "chm::")
	{
		std::string l_sCHMFile = l_sDefaultURLBaseString.substr(0, l_sDefaultURLBaseString.length() - 2);
		if(FS::Files::fileExists(l_sCHMFile.c_str()))
		{
			std::string l_sTXTFile = l_sDefaultURLBaseString.substr(0, l_sDefaultURLBaseString.length() - 5) + "txt";
			if( FS::Files::fileExists(l_sTXTFile.c_str()))
			{
				std::ifstream l_pDocumentationStream;
				FS::Files::openIFStream(l_pDocumentationStream, l_sTXTFile.c_str());
				std::string l_sLine;
				// Check if current box's documentation is listed (thus available in default documentation)
				while(!l_pDocumentationStream.eof())
				{
					l_pDocumentationStream >> l_sLine;
					m_vDocumentedBoxes.push_back(l_sLine);
				}
				// Sort vector to ease search of elements
				 std::sort(m_vDocumentedBoxes.begin(), m_vDocumentedBoxes.end());
			}
			else
			{
				// Should not happen unless the user removes it
				m_rKernelContext.getLogManager() << LogLevel_Error << "The list of documented boxes could not be found at ["<< l_sTXTFile.c_str() <<"]. "<<
					"Either the configuration token ${Designer_HelpBrowserURLBase} was modified to an incorrect value, or it is missing.\n";
			}
		}
		else
		{
			// Should not happen unless the user removed the documentation 
			m_rKernelContext.getLogManager() << LogLevel_Error << "The box documentation could not be found at ["<< l_sCHMFile.c_str() <<"]. "<<
				"Either the configuration token ${Designer_HelpBrowserURLBase} was modified to an incorrect value, or the documentation is missing.\n";
		}
	}
	else
	{
		// Should not happen unless the user modified the token by hand
		m_rKernelContext.getLogManager() << LogLevel_Error << "The configuration token ${Designer_HelpBrowserURLBase} seems to be set to an incorrect value.\n";
	}

#if defined TARGET_HAS_LibArchway
	if (m_oArchwayHandler.initialize() == Mensia::EngineInitialisationStatus::NotAvailable)
	{
		gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "neurort-toggle_engine_configuration")));
	}
#endif
}

OpenViBE::boolean CApplication::displayChangelogWhenAvailable()
{	
	// If last version used is ulterior as current version, and at least one box was added/updated, show the list
	if(!m_vNewBoxes.empty() || !m_vUpdatedBoxes.empty())
	{
		::GtkBuilder* l_pInterface = gtk_builder_new();
		gtk_builder_add_from_file(l_pInterface, OVD_GUI_File, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "aboutdialog-newversion"));
		gtk_window_set_title(GTK_WINDOW(l_pDialog), "Changelog");

		std::string l_sLabelNewBoxesList = "<big><b>Changes in version "+std::string(ProjectVersion) + " of the software:</b></big>";
		OpenViBE::boolean l_bUnstableBoxes = false;
		if(!m_vNewBoxes.empty())
		{
			l_sLabelNewBoxesList += "\n<big>The following boxes were added:</big>\n";
			for(auto pNewBoxDesc : m_vNewBoxes)
			{
				l_sLabelNewBoxesList += "    <b>" + pNewBoxDesc->getName() + ":</b> " + pNewBoxDesc->getShortDescription()+"\n";
			}
		}
		if(!m_vUpdatedBoxes.empty())
		{
			l_sLabelNewBoxesList += "\n<big>The following boxes were updated:</big>\n";
			for(auto pUpdatedBoxDesc : m_vUpdatedBoxes)
			{
				l_sLabelNewBoxesList += "    <b>" + pUpdatedBoxDesc->getName() + ":</b> " + pUpdatedBoxDesc->getShortDescription()+"\n";
			}
		}
		if(m_bIsNewVersion && l_bUnstableBoxes)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")), true);
		}
#if defined TARGET_OS_Windows // This function makes Windows calls only, hide button on other OSs
		g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pInterface, "button-display_changelog")), "clicked", G_CALLBACK(about_newversion_button_display_changelog_cb),  this);
#else
		gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(l_pInterface, "button-display_changelog")));
#endif
		if(!FS::Files::fileExists((OVD_README_File).toASCIIString()))
		{
			gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(l_pInterface, "button-display_changelog")));
		}
		GtkLabel* l_pLabel = GTK_LABEL(gtk_builder_get_object(l_pInterface, "label-newversion"));
		gtk_label_set_markup(l_pLabel, l_sLabelNewBoxesList.c_str());
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		if(m_bIsNewVersion)
		{
			gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "(New)");
			gtk_window_set_focus(GTK_WINDOW(m_pMainWindow), GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));
		}

		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pInterface);
	}
	else
	{
		return false;
	}
	return true;
}

OpenViBE::boolean CApplication::openScenario(const char* sFileName)
{
	// Prevent opening twice the same scenario
	for(uint32 i = 0; i < m_vInterfacedScenario.size(); i++)
	{
		auto l_vInterfacedScenario = m_vInterfacedScenario[i];
		if(l_vInterfacedScenario->m_sFileName == std::string(sFileName))
		{
			gtk_notebook_set_current_page(m_pScenarioNotebook, i);
			return true;
		}
	}

	std::string scenarioFilenameExtension = boost::filesystem::extension(sFileName);
	if (!CFileFormats::filenameExtensionImporters.count(scenarioFilenameExtension))
	{
		// TODO: Report error
		return false;
	}

	CIdentifier scenarioImporterIdentifier = CFileFormats::filenameExtensionImporters.at(scenarioFilenameExtension);

	CIdentifier l_oScenarioIdentifier;
	if (m_pScenarioManager->importScenarioFromFile(l_oScenarioIdentifier, sFileName, scenarioImporterIdentifier))
	{


		// Closes first unnamed scenario
		if(m_vInterfacedScenario.size()==1)
		{
			if(m_vInterfacedScenario[0]->m_bHasBeenModified==false && !m_vInterfacedScenario[0]->m_bHasFileName)
			{
				CIdentifier l_oScenarioIdentifierTmp=m_vInterfacedScenario[0]->m_oScenarioIdentifier;
				delete m_vInterfacedScenario[0];
				m_pScenarioManager->releaseScenario(l_oScenarioIdentifierTmp);
				m_vInterfacedScenario.clear();
			}
		}

		IScenario& l_rScenario = m_pScenarioManager->getScenario(l_oScenarioIdentifier);

		// Creates interfaced scenario
		CInterfacedScenario* l_pInterfacedScenario=new CInterfacedScenario(m_rKernelContext, *this, l_rScenario, l_oScenarioIdentifier, *m_pScenarioNotebook, OVD_GUI_File, OVD_GUI_Settings_File);

		// Deserialize the visualization tree from the scenario metadata, if it exists

		// Find the VisualizationTree metadata
		IMetadata* visualizationTreeMetadata = nullptr;
		CIdentifier metadataIdentifier = OV_UndefinedIdentifier;
		while ((metadataIdentifier = l_rScenario.getNextMetadataIdentifier(metadataIdentifier)) != OV_UndefinedIdentifier)
		{
			visualizationTreeMetadata = l_rScenario.getMetadataDetails(metadataIdentifier);
			if (visualizationTreeMetadata && visualizationTreeMetadata->getType() == OVVIZ_MetadataIdentifier_VisualizationTree)
			{
				break;
			}
		}
		OpenViBEVisualizationToolkit::IVisualizationTree* visualizationTree = l_pInterfacedScenario->m_pVisualizationTree;
		if (visualizationTreeMetadata && visualizationTree)
		{
			visualizationTree->deserialize(visualizationTreeMetadata->getData());
		}

		CIdentifier l_oVisualizationWidgetIdentifier;

		// Ensure visualization widgets contained in the scenario (if any) appear in the window manager
		//  even when the VisualizationTree section of a scenario file is missing, erroneous or deprecated

		// no visualization widget was added to visualization tree : ensure there aren't any in scenario
		CIdentifier boxIdentifier;
		while ((boxIdentifier = l_rScenario.getNextBoxIdentifier(boxIdentifier)) != OV_UndefinedIdentifier)
		{
			if (!visualizationTree->getVisualizationWidgetFromBoxIdentifier(boxIdentifier))
			{
				const IBox* box = l_rScenario.getBoxDetails(boxIdentifier);
				const IPluginObjectDesc* boxAlgorithmDesc = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(box->getAlgorithmClassIdentifier());
				if (boxAlgorithmDesc && boxAlgorithmDesc->hasFunctionality(OVD_Functionality_Visualization))
				{
					//a visualization widget was found in scenario : manually add it to visualization tree
					visualizationTree->addVisualizationWidget(
					            l_oVisualizationWidgetIdentifier,
					            box->getName(),
					            OpenViBEVisualizationToolkit::EVisualizationWidget_VisualizationBox,
					            OV_UndefinedIdentifier,
					            0,
					            box->getIdentifier(),
					            0,
					            OV_UndefinedIdentifier);
				}
			}
		}

		if(l_pInterfacedScenario->m_pDesignerVisualization != NULL)
		{
			l_pInterfacedScenario->m_pDesignerVisualization->setDeleteEventCB(&::delete_designer_visualisation_cb, this);
			l_pInterfacedScenario->m_pDesignerVisualization->load();
		}
		//l_pInterfacedScenario->snapshotCB(); --> a snapshot is already created in CInterfacedScenario builder !
		l_pInterfacedScenario->m_sFileName=sFileName;
		l_pInterfacedScenario->m_bHasFileName=true;
		l_pInterfacedScenario->m_bHasBeenModified=false;
		l_pInterfacedScenario->snapshotCB(false);

		m_vInterfacedScenario.push_back(l_pInterfacedScenario);

		l_pInterfacedScenario->redrawScenarioSettings();

		gtk_notebook_set_current_page(m_pScenarioNotebook, gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);

		l_pInterfacedScenario->updateScenarioLabel();

		this->saveOpenedScenarios();
		return true;
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Importing scenario failed...\n";

		std::stringstream l_oStringStream;
		l_oStringStream << "The requested file: " << sFileName << "\n";
		l_oStringStream << "may either not be an OpenViBE scenario file, \n";
		l_oStringStream << "a " + std::string(BRAND_NAME) + " scenario file, \n";
		l_oStringStream << "be corrupted or not compatible with \n";
		l_oStringStream << "the selected scenario importer...";

		::GtkWidget* l_pErrorDialog=gtk_message_dialog_new(
		            NULL,
		            GTK_DIALOG_MODAL,
		            GTK_MESSAGE_WARNING,
		            GTK_BUTTONS_OK,
		            "Scenario importation process failed !");
		gtk_message_dialog_format_secondary_text(
		            GTK_MESSAGE_DIALOG(l_pErrorDialog), "%s", l_oStringStream.str().c_str());
		gtk_dialog_run(GTK_DIALOG(l_pErrorDialog));
		gtk_widget_destroy(l_pErrorDialog);
	}
	return false;
}

CString CApplication::getWorkingDirectory(void)
{
	CString l_sWorkingDirectory=m_rKernelContext.getConfigurationManager().expand("${Designer_DefaultWorkingDirectory}");

	CInterfacedScenario* l_pCurrentScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentScenario)
	{
		if(l_pCurrentScenario->m_bHasFileName)
		{
			std::string l_sCurrentDirectory=std::string(g_path_get_dirname(l_pCurrentScenario->m_sFileName.c_str()));
#if defined TARGET_OS_Windows
			std::replace(l_sCurrentDirectory.begin(), l_sCurrentDirectory.end(), '\\', '/');
#endif
			l_sWorkingDirectory=l_sCurrentDirectory.c_str();
		}
	}

	if(!g_path_is_absolute(l_sWorkingDirectory.toASCIIString()))
	{
		std::string l_sCurrentDirectory=g_get_current_dir();
#if defined TARGET_OS_Windows
		std::replace(l_sCurrentDirectory.begin(), l_sCurrentDirectory.end(), '\\', '/');
#endif
		l_sWorkingDirectory=l_sCurrentDirectory.c_str()+CString("/")+l_sWorkingDirectory;
	}

	return l_sWorkingDirectory;
}

OpenViBE::boolean CApplication::hasRunningScenario(void)
{
	for(CInterfacedScenario* pInterfacedScenario : m_vInterfacedScenario)
	{
		if(pInterfacedScenario->m_pPlayer)
		{
			return true;
		}
	}
	return false;
}

OpenViBE::boolean CApplication::hasUnsavedScenario(void)
{
	for(CInterfacedScenario* pInterfacedScenario : m_vInterfacedScenario)
	{
		if(pInterfacedScenario->m_bHasBeenModified)
		{
			return true;
		}
	}
	return false;
}

CInterfacedScenario* CApplication::getCurrentInterfacedScenario(void)
{
	
	if(m_ui32CurrentInterfacedScenarioIndex<m_vInterfacedScenario.size())
	{
		return m_vInterfacedScenario[m_ui32CurrentInterfacedScenarioIndex];
	}
	return NULL;
}

void CApplication::saveOpenedScenarios(void)
{
	// Saves opened scenarios
	if(!(m_eCommandLineFlags&CommandLineFlag_NoManageSession))
	{
		OpenViBE::CString l_sAppConfigFile = m_rKernelContext.getConfigurationManager().expand("${Designer_CustomConfigurationFile}");

		FILE* l_pFile = FS::Files::open(l_sAppConfigFile.toASCIIString(), "wt");
		if(l_pFile)
		{
			unsigned int i=1;
			::fprintf(l_pFile, "# This file is generated\n");
			::fprintf(l_pFile, "# Do not modify\n");
			::fprintf(l_pFile, "\n");
			::fprintf(l_pFile, "Designer_ShowUnstable = %s\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")))?"True":"False");

			int l_iWidth, l_iHeight;
			gtk_window_get_size(GTK_WINDOW(m_pMainWindow), &l_iWidth, & l_iHeight);
			::fprintf(l_pFile, "Designer_EditorSizeWidth = %i\n", l_iWidth);
			::fprintf(l_pFile, "Designer_EditorSizeHeight = %i\n", l_iHeight);
			::fprintf(l_pFile, "Designer_EditorPanedPosition = %i\n", gtk_paned_get_position(GTK_PANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container"))));
			::fprintf(l_pFile, "Designer_FullscreenEditor = %s\n", m_bIsMaximized ? "True":"False");

			::fprintf(l_pFile, "# Last files opened in %s\n", std::string(STUDIO_NAME).c_str());

			for (CInterfacedScenario* scenario : m_vInterfacedScenario)
			{
				if (scenario->m_sFileName != "")
				{
					::fprintf(l_pFile, "Designer_LastScenarioFilename_%03u = %s\n", i, scenario->m_sFileName.c_str());
					i++;
				}
			}
			::fprintf(l_pFile, "\n");

			::fprintf(l_pFile, "# Last version of Studio used:\n");
			::fprintf(l_pFile, "Designer_LastVersionUsed = %s\n", ProjectVersion);
			::fprintf(l_pFile, "\n");

			::fprintf(l_pFile, "# Recently opened scenario\n");
			unsigned int scenarioID = 1;
			for (const GtkWidget* recentScenario : m_RecentScenarios)
			{
				const gchar* recentScenarioPath = gtk_menu_item_get_label(GTK_MENU_ITEM(recentScenario));
				::fprintf(l_pFile, "Designer_RecentScenario_%03u = %s\n", scenarioID, recentScenarioPath);
				++scenarioID;
			}
			::fprintf(l_pFile, "\n");
			
			::fclose(l_pFile);
		}
		else
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "Error writing to '" << l_sAppConfigFile << "'\n";
		}
	}
}

void CApplication::dragDataGetCB(::GtkWidget* pWidget, ::GdkDragContext* pDragContex, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "dragDataGetCB\n";

	::GtkTreeView* l_pTreeView=GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_tree"));
	::GtkTreeSelection* l_pTreeSelection=gtk_tree_view_get_selection(l_pTreeView);
	::GtkTreeModel* l_pTreeModel=NULL;
	::GtkTreeIter l_oTreeIter;
	if(gtk_tree_selection_get_selected(l_pTreeSelection, &l_pTreeModel, &l_oTreeIter))
	{
		const char* l_sBoxAlgorithmIdentifier=NULL;
		gtk_tree_model_get(
				l_pTreeModel, &l_oTreeIter,
				Resource_StringIdentifier, &l_sBoxAlgorithmIdentifier,
				-1);
		if(l_sBoxAlgorithmIdentifier)
		{
			gtk_selection_data_set(
					pSelectionData,
					GDK_SELECTION_TYPE_STRING,
					8,
					(const guchar*)l_sBoxAlgorithmIdentifier,
					strlen(l_sBoxAlgorithmIdentifier)+1);
		}
	}
}

void CApplication::undoCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "undoCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->undoCB();
	}
}

void CApplication::redoCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "redoCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->redoCB();
	}
}

void CApplication::copySelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "copySelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->copySelection();
	}
}

void CApplication::cutSelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "cutSelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->cutSelection();
	}
}

void CApplication::pasteSelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "pasteSelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->pasteSelection();
	}
}

void CApplication::deleteSelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "deleteSelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->deleteSelection();
	}
}

void CApplication::preferencesCB(void)
{
	enum
	{
		Resource_TokenName,
		Resource_TokenValue,
		Resource_TokenExpand,
	};

	m_rKernelContext.getLogManager() << LogLevel_Debug << "preferencesCB\n";
	::GtkBuilder* l_pBuilderInterface=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "configuration_manager", NULL);
	gtk_builder_add_from_file(l_pBuilderInterface, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(l_pBuilderInterface, NULL);

	::GtkWidget* l_pConfigurationManager=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "configuration_manager"));
	::GtkTreeView* l_pConfigurationManagerTreeView=GTK_TREE_VIEW(gtk_builder_get_object(l_pBuilderInterface, "configuration_manager-treeview"));

	// Prepares tree view
	::GtkTreeViewColumn* l_pTreeViewColumnTokenName=gtk_tree_view_column_new();
	::GtkTreeViewColumn* l_pTreeViewColumnTokenValue=gtk_tree_view_column_new();
	::GtkTreeViewColumn* l_pTreeViewColumnTokenExpand=gtk_tree_view_column_new();
	::GtkCellRenderer* l_pCellRendererTokenName=gtk_cell_renderer_text_new();
	::GtkCellRenderer* l_pCellRendererTokenValue=gtk_cell_renderer_text_new();
	::GtkCellRenderer* l_pCellRendererTokenExpand=gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenName, "Token name");
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenValue, "Token value");
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenExpand, "Expanded token value");
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, TRUE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenValue, l_pCellRendererTokenValue, TRUE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenExpand, l_pCellRendererTokenExpand, TRUE);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, "text", Resource_TokenName, NULL);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenValue, l_pCellRendererTokenValue, "text", Resource_TokenValue, NULL);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenExpand, l_pCellRendererTokenExpand, "text", Resource_TokenExpand, NULL);
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
	CIdentifier l_oTokenIdentifier;
	::GtkTreeStore* l_pConfigurationManagerTreeModel=gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	while((l_oTokenIdentifier=m_rKernelContext.getConfigurationManager().getNextConfigurationTokenIdentifier(l_oTokenIdentifier))!=OV_UndefinedIdentifier)
	{
		::GtkTreeIter l_oGtkIterChild;
		CString l_sTokenName=m_rKernelContext.getConfigurationManager().getConfigurationTokenName(l_oTokenIdentifier);
		CString l_sTokenValue=m_rKernelContext.getConfigurationManager().getConfigurationTokenValue(l_oTokenIdentifier);
		CString l_sTokenExpand=m_rKernelContext.getConfigurationManager().expand(l_sTokenValue);
		gtk_tree_store_append(
				l_pConfigurationManagerTreeModel,
				&l_oGtkIterChild,
				NULL);
		gtk_tree_store_set(
				l_pConfigurationManagerTreeModel,
				&l_oGtkIterChild,
				Resource_TokenName, l_sTokenName.toASCIIString(),
				Resource_TokenValue, l_sTokenValue.toASCIIString(),
				Resource_TokenExpand, l_sTokenExpand.toASCIIString(),
				-1);
	}
	gtk_tree_view_set_model(l_pConfigurationManagerTreeView, GTK_TREE_MODEL(l_pConfigurationManagerTreeModel));
	g_signal_emit_by_name(l_pTreeViewColumnTokenName, "clicked");

	gtk_dialog_run(GTK_DIALOG(l_pConfigurationManager));
	gtk_widget_destroy(l_pConfigurationManager);

	g_object_unref(l_pConfigurationManagerTreeModel);
	g_object_unref(l_pBuilderInterface);
}

void CApplication::testCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "testCB\n";
}

void CApplication::newScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "newScenarioCB\n";

	CIdentifier l_oScenarioIdentifier;
	if(m_pScenarioManager->createScenario(l_oScenarioIdentifier))
	{
		IScenario& l_rScenario=m_pScenarioManager->getScenario(l_oScenarioIdentifier);
		CInterfacedScenario* l_pInterfacedScenario=new CInterfacedScenario(m_rKernelContext, *this, l_rScenario, l_oScenarioIdentifier, *m_pScenarioNotebook, OVD_GUI_File, OVD_GUI_Settings_File);
		if(l_pInterfacedScenario->m_pDesignerVisualization != NULL)
		{
			l_pInterfacedScenario->m_pDesignerVisualization->setDeleteEventCB(&::delete_designer_visualisation_cb, this);
			l_pInterfacedScenario->m_pDesignerVisualization->newVisualizationWindow("Default window");
		}
		l_pInterfacedScenario->updateScenarioLabel();
		m_vInterfacedScenario.push_back(l_pInterfacedScenario);
		gtk_notebook_set_current_page(m_pScenarioNotebook, gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
	}
}

void CApplication::openScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterSpecific=gtk_file_filter_new();
	::GtkFileFilter* l_pFileFilterAll=gtk_file_filter_new();

	std::string allFileFormatsString = "All available formats (";
	for (auto const& fileFormat : CFileFormats::filenameExtensionImporters)
	{
		std::string currentFileFormatMask = "*" + fileFormat.first;
		gtk_file_filter_add_pattern(l_pFileFilterSpecific, currentFileFormatMask.c_str());
		allFileFormatsString += "*" + fileFormat.first + ", ";
	}
	allFileFormatsString.erase(allFileFormatsString.size() - 2); // because the loop adds one ", " too much
	allFileFormatsString += ")";

	gtk_file_filter_set_name(l_pFileFilterSpecific, allFileFormatsString.c_str());

	gtk_file_filter_set_name(l_pFileFilterAll, "All files");
	gtk_file_filter_add_pattern(l_pFileFilterAll, "*");

	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
			"Select scenario to open...",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterAll);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),true);

	gtk_file_chooser_set_current_folder(
			GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
			this->getWorkingDirectory().toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),true);

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		//char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while(l_pFile)
		{
			char* l_sFileName = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while((l_pBackslash = ::strchr(l_sFileName, '\\'))!=NULL)
			{
				*l_pBackslash = '/';
			}
			this->openScenario(l_sFileName);
			g_free(l_pFile->data);
			l_pFile=l_pFile->next;
		}
		g_slist_free(l_pList);
	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
//	g_object_unref(l_pFileFilterSpecific);
//	g_object_unref(l_pFileFilterAll);
}

void CApplication::saveScenarioCB(CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "saveScenarioCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=pScenario?pScenario:getCurrentInterfacedScenario();
	if(!l_pCurrentInterfacedScenario)
	{
		return;
	}
	if(!l_pCurrentInterfacedScenario->m_bHasFileName)
	{
		saveScenarioAsCB(pScenario);
	}
	else
	{
		// If the current scenario is a metabox, we will save its prototype hash into an attribute of the scenario
		// that way the standalone scheduler can check whether metaboxes included inside need updating.
		if (l_pCurrentInterfacedScenario->m_rScenario.isMetabox())
		{
			SBoxProto l_oMetaboxProto;

			IScenario& l_rScenario = l_pCurrentInterfacedScenario->m_rScenario;
			for (uint32 l_ui32ScenarioInputIndex = 0; l_ui32ScenarioInputIndex < l_rScenario.getInputCount(); l_ui32ScenarioInputIndex++)
			{
				CString l_sInputName;
				CIdentifier l_oInputTypeIdentifier;

				l_rScenario.getInputType(l_ui32ScenarioInputIndex, l_oInputTypeIdentifier);
				l_rScenario.getInputName(l_ui32ScenarioInputIndex, l_sInputName);

				l_oMetaboxProto.addInput(l_sInputName, l_oInputTypeIdentifier);
			}

			for (uint32 l_ui32ScenarioOutputIndex = 0; l_ui32ScenarioOutputIndex < l_rScenario.getOutputCount(); l_ui32ScenarioOutputIndex++)
			{
				CString l_sOutputName;
				CIdentifier l_oOutputTypeIdentifier;

				l_rScenario.getOutputType(l_ui32ScenarioOutputIndex, l_oOutputTypeIdentifier);
				l_rScenario.getOutputName(l_ui32ScenarioOutputIndex, l_sOutputName);

				l_oMetaboxProto.addOutput(l_sOutputName, l_oOutputTypeIdentifier);
			}

			for (uint32 l_ui32ScenarioSettingIndex = 0; l_ui32ScenarioSettingIndex < l_rScenario.getSettingCount(); l_ui32ScenarioSettingIndex++)
			{
				CString l_sSettingName;
				CIdentifier l_oSettingTypeIdentifier;
				CString l_sSettingDefaultValue;

				l_rScenario.getSettingName(l_ui32ScenarioSettingIndex, l_sSettingName);
				l_rScenario.getSettingType(l_ui32ScenarioSettingIndex, l_oSettingTypeIdentifier);
				l_rScenario.getSettingDefaultValue(l_ui32ScenarioSettingIndex, l_sSettingDefaultValue);

				l_oMetaboxProto.addSetting(l_sSettingName, l_oSettingTypeIdentifier, l_sSettingDefaultValue, false);
			}

			if (l_rScenario.hasAttribute(OV_AttributeId_Scenario_MetaboxHash))
			{
				l_rScenario.setAttributeValue(OV_AttributeId_Scenario_MetaboxHash, l_oMetaboxProto.m_oHash.toString());
			}
			else
			{
				l_rScenario.addAttribute(OV_AttributeId_Scenario_MetaboxHash, l_oMetaboxProto.m_oHash.toString());
			}

			m_rKernelContext.getLogManager() << LogLevel_Trace << "This metaboxes Hash : " << l_oMetaboxProto.m_oHash << "\n";
		}

		const char* l_sScenarioFileName = l_pCurrentInterfacedScenario->m_sFileName.c_str();

		std::string scenarioFilenameExtension = boost::filesystem::extension(l_sScenarioFileName);
		if (!CFileFormats::filenameExtensionImporters.count(scenarioFilenameExtension))
		{
			// TODO: Report error
			return;
		}

		// Remove all VisualizationTree type metadata
		CIdentifier metadataIdentifier = OV_UndefinedIdentifier;
		while ((metadataIdentifier = l_pCurrentInterfacedScenario->m_rScenario.getNextMetadataIdentifier(metadataIdentifier)) != OV_UndefinedIdentifier)
		{
			if (l_pCurrentInterfacedScenario->m_rScenario.getMetadataDetails(metadataIdentifier)->getType() == OVVIZ_MetadataIdentifier_VisualizationTree)
			{
				l_pCurrentInterfacedScenario->m_rScenario.removeMetadata(metadataIdentifier);
				metadataIdentifier = OV_UndefinedIdentifier;
			}
		}

		// Insert new metadata
		l_pCurrentInterfacedScenario->m_rScenario.addMetadata(metadataIdentifier, OV_UndefinedIdentifier);
		l_pCurrentInterfacedScenario->m_rScenario.getMetadataDetails(metadataIdentifier)->setType(OVVIZ_MetadataIdentifier_VisualizationTree);
		l_pCurrentInterfacedScenario->m_rScenario.getMetadataDetails(metadataIdentifier)->setData(l_pCurrentInterfacedScenario->m_pVisualizationTree->serialize());

		CIdentifier scenarioExporterIdentifier = CFileFormats::filenameExtensionExporters.at(scenarioFilenameExtension);

		if (m_pScenarioManager->exportScenarioToFile(l_sScenarioFileName, l_pCurrentInterfacedScenario->m_oScenarioIdentifier, scenarioExporterIdentifier))
		{
				l_pCurrentInterfacedScenario->snapshotCB();
				l_pCurrentInterfacedScenario->m_bHasFileName=true;
				l_pCurrentInterfacedScenario->m_bHasBeenModified=false;
				l_pCurrentInterfacedScenario->updateScenarioLabel();
					this->saveOpenedScenarios();
				}
		else
		{
			m_rKernelContext.getLogManager() << LogLevel_Warning << "Exporting scenario failed...\n";
			gint l_iResponseId;
			::GtkBuilder* l_pBuilder = gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
			gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
			gtk_builder_connect_signals(l_pBuilder, NULL);
			::GtkWidget* l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_error_popup_saving"));
			gtk_builder_connect_signals(l_pBuilder, NULL);
			l_iResponseId = gtk_dialog_run(GTK_DIALOG(l_pDialog));
			gtk_widget_destroy(l_pDialog);
			g_object_unref(l_pBuilder);

			switch (l_iResponseId)
			{
			case GTK_RESPONSE_OK:
				this->saveScenarioCB(pScenario);
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				return;
			default:
				break;
			}
		}
	}
}

void CApplication::saveScenarioAsCB(CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "saveScenarioAsCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=pScenario?pScenario:getCurrentInterfacedScenario();
	if(!l_pCurrentInterfacedScenario)
	{
		return;
	}

	bool l_bIsCurrentScenarioAMetabox = l_pCurrentInterfacedScenario->m_rScenario.isMetabox();

	::GtkFileFilter* l_pFileFilterAll = gtk_file_filter_new(); // All files
	gtk_file_filter_set_name(l_pFileFilterAll, "All files");
	gtk_file_filter_add_pattern(l_pFileFilterAll, "*");

	::GtkFileFilter* allCompatibleFormatsFileFilter = gtk_file_filter_new(); // All compatible files

	std::map<GtkFileFilter*, std::string> fileFilters;

	std::string allCompatibleFormatsFilterName = "All compatible formats (";
	for (auto const& fileFormat : CFileFormats::filenameExtensionExporters)
	{
		if (!l_bIsCurrentScenarioAMetabox || CFileFormats::filenameExtensionDescriptions.at(fileFormat.first).second == CFileFormats::FileFormatType_Metabox)
		{
			GtkFileFilter* fileFilter = gtk_file_filter_new();
			std::string fileFilterName = CFileFormats::filenameExtensionDescriptions.at(fileFormat.first).first + " (*" + fileFormat.first + ")";
			gtk_file_filter_set_name(fileFilter, fileFilterName.c_str());
			std::string fileFilterWildcard = "*" + fileFormat.first;
			gtk_file_filter_add_pattern(fileFilter, fileFilterWildcard.c_str());
			fileFilters[fileFilter] = fileFormat.first;

			allCompatibleFormatsFilterName += fileFilterWildcard + ", ";
			gtk_file_filter_add_pattern(allCompatibleFormatsFileFilter, fileFilterWildcard.c_str());
	}
	}
	allCompatibleFormatsFilterName.erase(allCompatibleFormatsFilterName.size() - 2); // because the loop adds one ", " too much
	allCompatibleFormatsFilterName += ")";

	gtk_file_filter_set_name(allCompatibleFormatsFileFilter, allCompatibleFormatsFilterName.c_str());

	// gtk_file_filter_set_name(l_pFileFilterSVG, "SVG image");
	// gtk_file_filter_add_pattern(l_pFileFilterSVG, "*.svg");

	::GtkWidget* l_pWidgetDialogSaveAs=gtk_file_chooser_dialog_new(
		"Select scenario to save...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), allCompatibleFormatsFileFilter);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), l_pFileFilterAll);

	for (auto fileFilter : fileFilters)
	{
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), fileFilter.first);
	}

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), allCompatibleFormatsFileFilter);
	// gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), true);
	if(l_pCurrentInterfacedScenario->m_bHasFileName)
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), l_pCurrentInterfacedScenario->m_sFileName.c_str());
	}
	else
	{
		// Put metaboxes to the User metabox folder by default
		if (l_bIsCurrentScenarioAMetabox)
		{
			gtk_file_chooser_set_current_folder(
				GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs),
				m_rKernelContext.getConfigurationManager().expand("${Path_UserData}/metaboxes").toASCIIString());
		}
		else
		{
			gtk_file_chooser_set_current_folder(
				GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs),
				this->getWorkingDirectory().toASCIIString());
		}
	}

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogSaveAs))==GTK_RESPONSE_ACCEPT)
	{
		char* l_sTempFilename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs));

		// replaces \ with / on windows
		char* l_pBackslash = NULL;
		while((l_pBackslash = ::strchr(l_sTempFilename, '\\'))!=NULL)
		{
			*l_pBackslash = '/';
		}

		// stores filename in a local variable
		char l_sFilename[1024];
		int l_iFilenameLength=::sprintf(l_sFilename, "%s", l_sTempFilename);
		g_free(l_sTempFilename);

		::GtkFileFilter* l_pFileFilter=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs));
		if (fileFilters.count(l_pFileFilter) != 0)
		{
			// User chose a specific filter
			std::string l_sExpectedExtension = fileFilters[l_pFileFilter];
			if(_strcmpi(l_sFilename + l_iFilenameLength - 4, (std::string(".") + l_sExpectedExtension).c_str()) != 0)
			{
				// If filename already has an extension, remove it
				if(l_sFilename[l_iFilenameLength-4] == '.')
				{
					l_iFilenameLength-=4;
					l_sFilename[l_iFilenameLength] = '\0';
				}

				// When user did not put appropriate extension, append it
				strcat(l_sFilename, ".");
				strcat(l_sFilename, l_sExpectedExtension.c_str());
				l_iFilenameLength += 1+l_sExpectedExtension.length();
			}
		}

		// Finaly decide export strategy based on extensions
		std::string scenarioFilenameExtension = boost::filesystem::extension(l_sFilename);
		if (CFileFormats::filenameExtensionExporters.count(scenarioFilenameExtension) == 0)
		{
			if(l_bIsCurrentScenarioAMetabox)
			{
				strcat(l_sFilename, ".mxb");
				l_iFilenameLength+=4;
			}
			else
			{
				strcat(l_sFilename, ".mxs");
				l_iFilenameLength+=4;
			}
		}

		// We ensure the file does not exist
		OpenViBE::boolean l_bIsSaveActionValid=true;
		FILE* l_pFile=FS::Files::open(l_sFilename, "r");
		if(l_pFile)
		{
			::fclose(l_pFile);
			::GtkDialog* l_pConfirmationDialog=GTK_DIALOG(::gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "The file already exists"));
			::gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pConfirmationDialog), "%s\n\nThe file you are trying to save-as already exists, confirming this action will overwrite the existing file. Please confirm you want to overwrite the existing file.", l_sFilename);
			l_bIsSaveActionValid=(::gtk_dialog_run(GTK_DIALOG(l_pConfirmationDialog)) == GTK_RESPONSE_OK);
			::gtk_widget_destroy(GTK_WIDGET(l_pConfirmationDialog));
		}

		// Finally save the scenario
		if(l_bIsSaveActionValid)
		{
			l_pCurrentInterfacedScenario->m_sFileName=l_sFilename;
			l_pCurrentInterfacedScenario->m_bHasFileName=true;
			l_pCurrentInterfacedScenario->m_bHasBeenModified=false;
			l_pCurrentInterfacedScenario->updateScenarioLabel();
			saveScenarioCB(l_pCurrentInterfacedScenario);
		}
		else
		{
			m_rKernelContext.getLogManager() << LogLevel_Trace << "Canceled 'save-as' action for filename [" << CString(l_sFilename) << "]\n";
		}
	}

	gtk_widget_destroy(l_pWidgetDialogSaveAs);
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
			m_RecentScenarios.erase(m_RecentScenarios.begin() + i+1);
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
		for (size_t i = s_RecentFileNumber; i < m_RecentScenarios.size(); i++)
		{
			gtk_container_remove(m_MenuOpenRecent, GTK_WIDGET(m_RecentScenarios[i]));
			gtk_widget_destroy(GTK_WIDGET(m_RecentScenarios[i]));
		}
		m_RecentScenarios.erase(m_RecentScenarios.begin() + s_RecentFileNumber, m_RecentScenarios.end());
	}
}

void CApplication::closeScenarioCB(CInterfacedScenario* pInterfacedScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "closeScenarioCB\n";

	if(!pInterfacedScenario)
	{
		return;
	}
	if(pInterfacedScenario->isLocked())
	{
		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_running_scenario"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);
		return;
	}
	if(pInterfacedScenario->m_bHasBeenModified)
	{
		gint l_iResponseId;

		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_unsaved_scenario"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		l_iResponseId=gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		switch(l_iResponseId)
		{
			case GTK_RESPONSE_OK:
				this->saveScenarioCB(pInterfacedScenario);
				if(pInterfacedScenario->m_bHasBeenModified)
				{
					return;
				}
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				return;
			default:
				break;
		}
	}
	// Add scenario to recently opened:
	this->addRecentScenario(pInterfacedScenario->m_sFileName.c_str());

	vector<CInterfacedScenario*>::iterator i=m_vInterfacedScenario.begin();
	while(i!=m_vInterfacedScenario.end() && *i!=pInterfacedScenario)
	{
		i++;
	}

	if(i!=m_vInterfacedScenario.end())
	{
		// We need to erase the scenario from the list first, because deleting the scenario will launch a "switch-page"
		// callback accessing this array with the identifier of the deleted scenario (if its not the last one) -> boom.
		m_vInterfacedScenario.erase(i);
		CIdentifier l_oScenarioIdentifier=pInterfacedScenario->m_oScenarioIdentifier;
		delete pInterfacedScenario;
		m_pScenarioManager->releaseScenario(l_oScenarioIdentifier);
		//when closing last open scenario, no "switch-page" event is triggered so we manually handle this case
		if(m_vInterfacedScenario.empty() == true)
		{
			newScenarioCB();
		}
		else
		{
			changeCurrentScenario(gtk_notebook_get_current_page(m_pScenarioNotebook));
		}
	}

	this->saveOpenedScenarios();
}

void CApplication::deleteDesignerVisualizationCB()
{
	//untoggle window manager button when its associated dialog is closed
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), FALSE);

	CInterfacedScenario* l_pCurrentInterfacedScenario = getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->snapshotCB();
	}
}

void CApplication::toggleDesignerVisualizationCB()
{
	CInterfacedScenario* l_pCurrentInterfacedScenario = getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario != NULL && l_pCurrentInterfacedScenario->isLocked() == false)
	{
		uint32 l_ui32Index=(uint32)gtk_notebook_get_current_page(m_pScenarioNotebook);
		if(l_ui32Index<m_vInterfacedScenario.size())
		{
			m_vInterfacedScenario[l_ui32Index]->toggleDesignerVisualization();
		}
	}
}

void CApplication::aboutOpenViBECB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::aboutOpenViBECB\n";
	::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
	gtk_builder_add_from_file(l_pBuilder, OVD_GUI_AboutDialog_File, NULL);
	gtk_builder_connect_signals(l_pBuilder, NULL);
	//gtk_builder_connect_signals(l_pBuilder, NULL);
	::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "about"));
	if (l_pDialog == NULL)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Dialog could not be opened\n";
		return;

	}
	gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
	gtk_dialog_run(GTK_DIALOG(l_pDialog));
	gtk_widget_destroy(l_pDialog);
	g_object_unref(l_pBuilder);
}

void CApplication::aboutScenarioCB(CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::aboutScenarioCB\n";
	if(pScenario && !pScenario->isLocked())
	{
		pScenario->contextMenuScenarioAboutCB();
	}
}

void CApplication::aboutLinkClickedCB(const gchar *url)
{
	if(!url)
	{
		return;
	}
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::aboutLinkClickedCB\n";
	CString l_sCommand = m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommand} " + OpenViBE::CString(url));
	int l_iResult = system(l_sCommand.toASCIIString());
	if(l_iResult<0)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Could not launch command " << l_sCommand << "\n";
	}
}

void CApplication::browseDocumentationCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::browseDocumentationCB\n";
	CString l_sCommand = m_rKernelContext.getConfigurationManager().expand("${Designer_HelpBrowserCommand} \"${Designer_HelpBrowserURLBase}\" ${Designer_HelpBrowserCommandPostfix}");
	int l_iResult = system(l_sCommand.toASCIIString());

	if(l_iResult<0)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Could not launch command " << l_sCommand << "\n";
	}
}

void CApplication::reportIssueCB(void)
{
#if defined TARGET_OS_Windows
	//On windows, call the issue reporter tool
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::reportIssueCB\n";
	std::string l_sCommand = Directories::getBinDir() + "/neurort-issue_reporter.exe";
	STARTUPINFO l_oStartupInfo;
	PROCESS_INFORMATION lpProcessInfo;
	GetStartupInfo(&l_oStartupInfo);
	if(!System::WindowsUtilities::utf16CompliantCreateProcess(NULL,const_cast<char*>(l_sCommand.c_str()), NULL, NULL, NULL, NULL, NULL, NULL, &l_oStartupInfo, &lpProcessInfo))
	{
		exit(1);
	}
#elif defined TARGET_OS_Linux
	//On other os, open Zendesk home page
	std::string l_sCommand = "x-www-browser https://mensiatech.zendesk.com/&";
	system(l_sCommand.c_str());
#endif

}

void CApplication::addCommentCB(
		CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::addCommentCB\n";
	if(pScenario && !pScenario->isLocked())
	{
		pScenario->addCommentCB();
	}
}

void CApplication::configureScenarioSettingsCB(CInterfacedScenario* pScenario)
{

	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::configureScenarioSettingsCB " << m_ui32CurrentInterfacedScenarioIndex << "\n";

	if(pScenario && !pScenario->isLocked())
	{
		pScenario->configureScenarioSettingsCB();
	}
}

IPlayer* CApplication::getPlayer(void)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	return (l_pCurrentInterfacedScenario?l_pCurrentInterfacedScenario->m_pPlayer:NULL);
}

OpenViBE::boolean CApplication::createPlayer(void)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario = getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario && !l_pCurrentInterfacedScenario->m_pPlayer)
	{
		// create a snapshot so settings override does not modify the scenario !
		l_pCurrentInterfacedScenario->snapshotCB(false);

		// set filename attribute to scenario so delayed configuration can be used
		if(l_pCurrentInterfacedScenario->m_bHasFileName)
		{
			if(l_pCurrentInterfacedScenario->m_rScenario.hasAttribute(OV_AttributeId_ScenarioFilename))
			{
				l_pCurrentInterfacedScenario->m_rScenario.setAttributeValue(OV_AttributeId_ScenarioFilename, l_pCurrentInterfacedScenario->m_sFileName.c_str());
			}
			else
			{
				l_pCurrentInterfacedScenario->m_rScenario.addAttribute(OV_AttributeId_ScenarioFilename, l_pCurrentInterfacedScenario->m_sFileName.c_str());
			}
		}

		m_rKernelContext.getPlayerManager().createPlayer(l_pCurrentInterfacedScenario->m_oPlayerIdentifier);
		CIdentifier l_oScenarioIdentifier=l_pCurrentInterfacedScenario->m_oScenarioIdentifier;
		CIdentifier l_oPlayerIdentifier=l_pCurrentInterfacedScenario->m_oPlayerIdentifier;
		l_pCurrentInterfacedScenario->m_pPlayer=&m_rKernelContext.getPlayerManager().getPlayer(l_oPlayerIdentifier);
		if(!l_pCurrentInterfacedScenario->m_pPlayer->setScenario(l_oScenarioIdentifier))
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "The current scenario could not be loaded by the player.\n";
			l_pCurrentInterfacedScenario->m_oPlayerIdentifier = OV_UndefinedIdentifier;
			l_pCurrentInterfacedScenario->m_pPlayer=NULL;
			m_rKernelContext.getPlayerManager().releasePlayer(l_oPlayerIdentifier);
			return false;
		}

		// The visualization manager needs to know the visualization tree in which the widgets should be inserted
		l_pCurrentInterfacedScenario->m_pPlayer->getRuntimeConfigurationManager().createConfigurationToken("VisualizationContext_VisualizationTreeId", l_pCurrentInterfacedScenario->m_oVisualizationTreeIdentifier.toString());

		// TODO_JL: This should be a copy of the tree containing visualizations from the metaboxes
		l_pCurrentInterfacedScenario->createPlayerVisualization(l_pCurrentInterfacedScenario->m_pVisualizationTree);
		if(l_pCurrentInterfacedScenario->m_pPlayer->initialize() != EPlayerReturnCode::PlayerReturnCode_Sucess)
		{
			l_pCurrentInterfacedScenario->releasePlayerVisualization();
			m_rKernelContext.getLogManager() << LogLevel_Error << "The player could not be initialized.\n";
			l_pCurrentInterfacedScenario->m_oPlayerIdentifier = OV_UndefinedIdentifier;
			l_pCurrentInterfacedScenario->m_pPlayer=NULL;
			m_rKernelContext.getPlayerManager().releasePlayer(l_oPlayerIdentifier);
			return false;
		}
		l_pCurrentInterfacedScenario->m_ui64LastLoopTime = uint64(-1);

		//set up idle function
		__g_idle_add__(idle_scenario_loop, l_pCurrentInterfacedScenario);

		// redraws scenario
		l_pCurrentInterfacedScenario->redraw();
	}
	return true;
}

void CApplication::stopInterfacedScenarioAndReleasePlayer(CInterfacedScenario* interfacedScenario)
{
	if (!(interfacedScenario && interfacedScenario->m_pPlayer))
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Trying to stop a non-started scenario" << "\n";
		return;
	}

	interfacedScenario->m_rKernelContext.getErrorManager().releaseErrors();
	interfacedScenario->m_pPlayer->stop();
	interfacedScenario->m_ePlayerStatus = interfacedScenario->m_pPlayer->getStatus();
	// removes idle function
	g_idle_remove_by_data(interfacedScenario);

	if (!interfacedScenario->m_pPlayer->uninitialize())
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to uninitialize the player" << "\n";
	}

	if (!interfacedScenario->m_rKernelContext.getPlayerManager().releasePlayer(interfacedScenario->m_oPlayerIdentifier))
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to release the player" << "\n";
	}

	interfacedScenario->m_oPlayerIdentifier = OV_UndefinedIdentifier;
	interfacedScenario->m_pPlayer = NULL;

	// restore the snapshot so settings override does not modify the scenario !
	interfacedScenario->undoCB(false);

	// destroy player windows
	interfacedScenario->releasePlayerVisualization();

	// redraws scenario
	interfacedScenario->redraw();
	if (interfacedScenario == this->getCurrentInterfacedScenario())
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
	}
}

void CApplication::stopScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "stopScenarioCB\n";

	if (this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Play || this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Pause || this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Forward)
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
	}
}

void CApplication::pauseScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "pauseScenarioCB\n";

	this->createPlayer();
	this->getPlayer()->pause();
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::nextScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "nextScenarioCB\n";

	this->createPlayer();
	this->getPlayer()->step();
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::playScenarioCB(void)
{
	if(this->getCurrentInterfacedScenario() != NULL)
	{
		OpenViBE::Kernel::IScenario& l_oCurrentScenario = this->getCurrentInterfacedScenario()->m_rScenario;
		m_rKernelContext.getLogManager() << LogLevel_Debug << "playScenarioCB\n";
		if (l_oCurrentScenario.checkNeedsUpdateBox())
		{
			if(m_rKernelContext.getConfigurationManager().expandAsBoolean("${Kernel_AbortPlayerWhenBoxNeedsUpdate}", false))
			{
				std::string l_sNeedsUpdatesBoxesList = "You can not start the scenario because following boxes need to be updated: \n";
				CIdentifier l_oBoxIdentifier;
				while((l_oBoxIdentifier = l_oCurrentScenario.getNextNeedsUpdateBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
				{
					const IBox* l_pBox = l_oCurrentScenario.getBoxDetails(l_oBoxIdentifier);
					l_sNeedsUpdatesBoxesList += "\t[" + l_pBox->getName() + "]\n";
				}
				l_sNeedsUpdatesBoxesList += "To update a box you need to delete it from scenario, and add it again.";
				::GtkWidget* l_pDialog=::gtk_message_dialog_new(
					NULL,
					::GtkDialogFlags(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
					GtkMessageType::GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					l_sNeedsUpdatesBoxesList.c_str(),
					"Box needs update.");
				gint l_iResponse = ::gtk_dialog_run(GTK_DIALOG(l_pDialog));
				::gtk_widget_destroy(l_pDialog);
				return;
			}
			else if(m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ThrowPopUpWhenBoxNeedsUpdate}", false))
			{
				std::string l_sNeedsUpdatesBoxesList = "The following boxes need to be updated: \n";
				CIdentifier l_oBoxIdentifier;
				while((l_oBoxIdentifier = l_oCurrentScenario.getNextNeedsUpdateBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
				{
					const IBox* l_pBox = l_oCurrentScenario.getBoxDetails(l_oBoxIdentifier);
					l_sNeedsUpdatesBoxesList += "\t[" + l_pBox->getName() + "]\n";
				}
				l_sNeedsUpdatesBoxesList += "Do you still want to play the scenario ?";
				::GtkWidget* l_pDialog=::gtk_message_dialog_new(
					NULL,
					::GtkDialogFlags(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
					GtkMessageType::GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					l_sNeedsUpdatesBoxesList.c_str(),
					"Box needs update.");
				gint l_iResponse = ::gtk_dialog_run(GTK_DIALOG(l_pDialog));
				::gtk_widget_destroy(l_pDialog);
		
				if(l_iResponse == GTK_RESPONSE_YES)
				{
					m_rKernelContext.getLogManager() << LogLevel_Trace << "CApplication::playScenarioCB - GTK_RESPONSE_YES: the scenario will be played. \n";
				}
				else
				{
					m_rKernelContext.getLogManager() << LogLevel_Trace << "CApplication::playScenarioCB - the scenario will not be played. \n";
					return;
				}
			}
		}
	}

	if(!this->createPlayer())
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "The initialization of player failed. Check the above log messages to get the issue.\n";
		return;
	}
	if (!this->getPlayer()->play())
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
		return;
	}
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PAUSE);
}

void CApplication::forwardScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "forwardScenarioCB\n";

	if(!this->createPlayer())
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "CreatePlayer failed\n";
		return;
	}

	if (!this->getPlayer()->forward())
	{
		this->stopInterfacedScenarioAndReleasePlayer(this->getCurrentInterfacedScenario());
		return;
	}

	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

OpenViBE::boolean CApplication::quitApplicationCB(void)
{
	std::vector < CInterfacedScenario* >::iterator it;

	CIdentifier l_oIdentifier;
	m_rKernelContext.getLogManager() << LogLevel_Debug << "quitApplicationCB\n";

	// can't quit while scenarios are running
	if(this->hasRunningScenario() == true)
	{
		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_running_scenario_global"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		// prevent Gtk from handling delete_event and killing app
		return false;
	}

	// can't quit while scenarios are unsaved
	if(this->hasUnsavedScenario() == true)
	{
		gint l_iResponseId;

		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_unsaved_scenario_global"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		l_iResponseId=gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		switch(l_iResponseId)
		{
			case GTK_RESPONSE_OK:
				for(std::vector < CInterfacedScenario* >::iterator i=m_vInterfacedScenario.begin(); i!=m_vInterfacedScenario.end(); i++)
				{
					this->saveScenarioCB(*i);
				}
				if(this->hasUnsavedScenario())
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
	m_bIsQuitting=true;

	// Saves opened scenarios
	this->saveOpenedScenarios();

	// Clears all existing interfaced scenarios
	for(it=m_vInterfacedScenario.begin(); it!=m_vInterfacedScenario.end(); it++)
	{
		delete *it;
	}

	// Clears all existing scenarios
	vector < CIdentifier > l_vScenarioIdentifiers;
	while((l_oIdentifier=m_rKernelContext.getScenarioManager().getNextScenarioIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
	{
		l_vScenarioIdentifiers.push_back(l_oIdentifier);
	}
	for(vector < CIdentifier > ::iterator i=l_vScenarioIdentifiers.begin(); i!=l_vScenarioIdentifiers.end(); i++)
	{
		m_rKernelContext.getScenarioManager().releaseScenario(*i);
	}

	// release the log manager and free the memory
	if(m_pLogListenerDesigner)
	{
		m_rKernelContext.getLogManager().removeListener( m_pLogListenerDesigner );
		delete m_pLogListenerDesigner;
		m_pLogListenerDesigner = NULL;
	}

	// OK to kill app
	return true;
}

void CApplication::windowStateChangedCB(OpenViBE::boolean bIsMaximized)
{
	if(m_bIsMaximized != bIsMaximized && !bIsMaximized) // we switched to not maximized
	{
		//gtk_paned_set_position(GTK_PANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container")), 640);
		gtk_window_resize(GTK_WINDOW(m_pMainWindow), 1024, 768);
	}
	m_bIsMaximized = bIsMaximized;
}

void CApplication::logLevelCB(void)
{
	// Loads log level dialog
	::GtkBuilder* l_pBuilderInterface=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "loglevel", NULL);
	gtk_builder_add_from_file(l_pBuilderInterface, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(l_pBuilderInterface, NULL);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_fatal")),             m_rKernelContext.getLogManager().isActive(LogLevel_Fatal));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_error")),             m_rKernelContext.getLogManager().isActive(LogLevel_Error));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_important_warning")), m_rKernelContext.getLogManager().isActive(LogLevel_ImportantWarning));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_warning")),           m_rKernelContext.getLogManager().isActive(LogLevel_Warning));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_info")),              m_rKernelContext.getLogManager().isActive(LogLevel_Info));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_trace")),             m_rKernelContext.getLogManager().isActive(LogLevel_Trace));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_benchmark")),         m_rKernelContext.getLogManager().isActive(LogLevel_Benchmark));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_debug")),             m_rKernelContext.getLogManager().isActive(LogLevel_Debug));

	::GtkDialog* l_pLogLevelDialog=GTK_DIALOG(gtk_builder_get_object(l_pBuilderInterface, "loglevel"));
	gint l_iResult=gtk_dialog_run(l_pLogLevelDialog);
	if(l_iResult==GTK_RESPONSE_APPLY)
	{
		m_rKernelContext.getLogManager().activate(LogLevel_Fatal,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_fatal")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Error,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_error")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_ImportantWarning, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_important_warning")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Warning,          gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_warning")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Info,             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_info")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Trace,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_trace")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Benchmark,        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_benchmark")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Debug,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_debug")))?true:false);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")), m_rKernelContext.getLogManager().isActive(LogLevel_Debug));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")), m_rKernelContext.getLogManager().isActive(LogLevel_Benchmark));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")), m_rKernelContext.getLogManager().isActive(LogLevel_Trace));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")), m_rKernelContext.getLogManager().isActive(LogLevel_Info));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")), m_rKernelContext.getLogManager().isActive(LogLevel_Warning));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")), m_rKernelContext.getLogManager().isActive(LogLevel_ImportantWarning));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")), m_rKernelContext.getLogManager().isActive(LogLevel_Error));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")), m_rKernelContext.getLogManager().isActive(LogLevel_Fatal));
	}

	gtk_widget_destroy(GTK_WIDGET(l_pLogLevelDialog));
	g_object_unref(l_pBuilderInterface);
}

void CApplication::CPUUsageCB(void)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->m_bDebugCPUUsage=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")))?true:false);
		l_pCurrentInterfacedScenario->redraw();
	}
}

void CApplication::changeCurrentScenario(int32 i32PageIndex)
{
	if(m_bIsQuitting) return;

	//hide window manager of previously active scenario, if any
	int i = gtk_notebook_get_current_page(m_pScenarioNotebook);
	if(i >= 0 && i < (int)m_vInterfacedScenario.size())
	{
		m_vInterfacedScenario[i]->hideCurrentVisualization();
	}

	//closing last open scenario
	if(i32PageIndex == -1)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    false);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* l_pWindowManagerButton=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);
		
		// toggle off and reset scenario settings
		GtkWidget* l_pSettingsVBox = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_vbox"));

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), false);


		GList* l_pSettingWidgets;
		GList* l_pSettingIterator;

		l_pSettingWidgets = gtk_container_get_children(GTK_CONTAINER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_vbox")));
		for (l_pSettingIterator = l_pSettingWidgets; l_pSettingIterator != NULL; l_pSettingIterator = g_list_next(l_pSettingIterator))
		{
			gtk_widget_destroy(GTK_WIDGET(l_pSettingIterator->data));
		}
		g_list_free(l_pSettingWidgets);

		GtkWidget* l_pSettingPlaceholderLabel = gtk_label_new("This scenario has no settings");
		gtk_box_pack_end_defaults(GTK_BOX(l_pSettingsVBox), l_pSettingPlaceholderLabel);
		gtk_widget_show_all(l_pSettingsVBox);


		// current scenario is the current notebook page.
		m_ui32CurrentInterfacedScenarioIndex = i;
	}
	//switching to an existing scenario
	else if(i32PageIndex<(int32)m_vInterfacedScenario.size())
	{
		CInterfacedScenario* l_pCurrentInterfacedScenario=m_vInterfacedScenario[i32PageIndex];
		EPlayerStatus l_ePlayerStatus=(l_pCurrentInterfacedScenario->m_pPlayer?l_pCurrentInterfacedScenario->m_pPlayer->getStatus():PlayerStatus_Stop);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       l_ePlayerStatus!=PlayerStatus_Stop);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    l_ePlayerStatus!=PlayerStatus_Forward);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), (l_ePlayerStatus==PlayerStatus_Stop || l_ePlayerStatus==PlayerStatus_Pause) ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_undo")), l_pCurrentInterfacedScenario->m_oStateStack->isUndoPossible());
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_redo")), l_pCurrentInterfacedScenario->m_oStateStack->isRedoPossible());

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), l_ePlayerStatus==PlayerStatus_Stop);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), l_pCurrentInterfacedScenario->m_bDebugCPUUsage);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save")), l_pCurrentInterfacedScenario->m_bHasFileName && l_pCurrentInterfacedScenario->m_bHasBeenModified);
		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save")),   l_pCurrentInterfacedScenario->m_bHasFileName && l_pCurrentInterfacedScenario->m_bHasBeenModified);

		//don't show window manager if in offline mode and it is toggled off
		if(l_ePlayerStatus==PlayerStatus_Stop && m_vInterfacedScenario[i32PageIndex]->isDesignerVisualizationToggled() == false)
		{
			m_vInterfacedScenario[i32PageIndex]->hideCurrentVisualization();
			
			// we are in edition mode, updating internal configuration token
			std::string l_sPath = m_vInterfacedScenario[i32PageIndex]->m_sFileName;
			l_sPath = l_sPath.substr(0, l_sPath.rfind('/'));
			m_rKernelContext.getConfigurationManager().setConfigurationTokenValue(m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier("Player_ScenarioDirectory"), l_sPath.c_str());
			m_rKernelContext.getConfigurationManager().setConfigurationTokenValue(m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier("__volatile_ScenarioDir"), l_sPath.c_str());
		}
		else
		{
			m_vInterfacedScenario[i32PageIndex]->showCurrentVisualization();
		}

		//update window manager button state
		GtkWidget* l_pWindowManagerButton=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), m_vInterfacedScenario[i32PageIndex]->isDesignerVisualizationToggled() ? true : false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), true);
		m_vInterfacedScenario[i32PageIndex]->redrawScenarioSettings();
		m_vInterfacedScenario[i32PageIndex]->redrawScenarioInputSettings();
		m_vInterfacedScenario[i32PageIndex]->redrawScenarioOutputSettings();

	
		// current scenario is the selected one
		m_ui32CurrentInterfacedScenarioIndex = i32PageIndex;
	}
	//first scenario is created (or a scenario is opened and replaces first unnamed unmodified scenario)
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_undo")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_redo")), false);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* l_pWindowManagerButton=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_configuration_button_configure")), true);

		// we have a new notebook page
		m_ui32CurrentInterfacedScenarioIndex = i32PageIndex;

		// we are in edition mode, updating internal configuration token
		m_rKernelContext.getConfigurationManager().setConfigurationTokenValue(m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier("Player_ScenarioDirectory"), "");
		m_rKernelContext.getConfigurationManager().setConfigurationTokenValue(m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier("__volatile_ScenarioDir"), "");
	}

	// updates the trimming if need be
	for(uint32 i = 0; i < (uint32)m_vInterfacedScenario.size(); i++)
	{
		m_vInterfacedScenario[i]->updateScenarioLabel();
	}
}

void CApplication::reorderCurrentScenario(OpenViBE::uint32 i32NewPageIndex)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario = m_vInterfacedScenario[m_ui32CurrentInterfacedScenarioIndex];
	m_vInterfacedScenario.erase(m_vInterfacedScenario.begin() + m_ui32CurrentInterfacedScenarioIndex);
	m_vInterfacedScenario.insert(m_vInterfacedScenario.begin() + i32NewPageIndex, l_pCurrentInterfacedScenario);

	this->changeCurrentScenario(i32NewPageIndex);
}

