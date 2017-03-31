#ifndef __OpenViBEDesigner_CApplication_H__
#define __OpenViBEDesigner_CApplication_H__

#include <visualization-toolkit/ovvizIVisualizationManager.h>
#include "ovd_base.h"
#if defined TARGET_HAS_LibArchway
#include "ovdCArchwayHandler.h"
#include "ovdCArchwayHandlerGUI.h"
#endif

#define OVD_ScenarioImportContext_OpenScenario OpenViBE::CIdentifier(0xA180DB91, 0x19235AEE)
#define OVD_ScenarioExportContext_SaveScenario OpenViBE::CIdentifier(0xC98C47AD, 0xCBD952B2)
#define OVD_ScenarioExportContext_SaveMetabox  OpenViBE::CIdentifier(0x529494F1, 0x6C2527D9)

#include <vector>
namespace Mensia
{
	class CMetaboxLoader;

	namespace ActivationTool
	{
		class CActivationTool;
	};
};

namespace OpenViBEDesigner
{

	class CInterfacedScenario;

	class CLogListenerDesigner;

	class CApplication
	{
	public:
		CApplication(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		~CApplication(void);

		void initialize(OpenViBEDesigner::ECommandLineFlag eCommandLineFlags);

		bool openScenario(const char* sFileName);

		/** \name Drag and drop management */
		//@{

		void dragDataGetCB(
			::GtkWidget* pWidget,
			::GdkDragContext* pDragContex,
			::GtkSelectionData* pSelectionData,
			guint uiInfo,
			guint uiT);

		//@}

		/** \name Selection management */
		//@{

		void undoCB(void);
		void redoCB(void);

		void copySelectionCB(void);
		void cutSelectionCB(void);
		void pasteSelectionCB(void);
		void deleteSelectionCB(void);
		void preferencesCB(void);

		//@}

		/** \name Scenario management */
		//@{

		OpenViBE::CString getWorkingDirectory(void);

		bool hasRunningScenario(void);
		bool hasUnsavedScenario(void);

		OpenViBEDesigner::CInterfacedScenario* getCurrentInterfacedScenario(void);
		void saveOpenedScenarios(void);

		void testCB(void);
		void newScenarioCB(void);
		void openScenarioCB(void);
		void saveScenarioCB(OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario=NULL); // defaults to current scenario if NULL
		void saveScenarioAsCB(OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario=NULL); // defaults to current scenario if NULL
		void closeScenarioCB(
			OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario);

		void stopScenarioCB(void);
		void pauseScenarioCB(void);
		void nextScenarioCB(void);
		void playScenarioCB(void);
		void forwardScenarioCB(void);

		void configureScenarioSettingsCB(OpenViBEDesigner::CInterfacedScenario* pScenario);

		void addCommentCB(
			OpenViBEDesigner::CInterfacedScenario* pScenario);

		void changeCurrentScenario(
			OpenViBE::int32 i32PageIndex);
		void reorderCurrentScenario(
			OpenViBE::uint32 i32NewPageIndex);

		void addRecentScenario(const std::string& scenarioPath);

		//@}

		/** \name Designer visualization management */
		//@{

		void deleteDesignerVisualizationCB();

		void toggleDesignerVisualizationCB();

		//@}

		/** \name Player management */
		//@{

		OpenViBE::Kernel::IPlayer* getPlayer(void);

		bool createPlayer(void);

		void stopInterfacedScenarioAndReleasePlayer(OpenViBEDesigner::CInterfacedScenario* interfacedScenario);

		//@}

		/** \name Application management */
		//@{

		bool quitApplicationCB(void);
		void aboutOpenViBECB(void);
		void aboutScenarioCB(OpenViBEDesigner::CInterfacedScenario* pScenario);
		void aboutLinkClickedCB(const gchar *url);

		void browseDocumentationCB(void);
		void reportIssueCB(void);
		void windowStateChangedCB(bool bIsMaximized);
		bool displayChangelogWhenAvailable();

		//@}

		/** \name Log management */
		//@{

		void logLevelCB(void);
		void logLevelMessagesCB(void);

		//@}

		/** \name CPU usage */
		//@{

		void CPUUsageCB(void);

		//@}
		void zoomInCB(void);//Call when a zoom in is required
		void zoomOutCB(void);//Call when a zoom out is required
		void spinnerZoomChangedCB(uint32_t scaleDelta);

	public:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IPluginManager* m_pPluginManager;
		OpenViBE::Kernel::IScenarioManager* m_pScenarioManager;
		OpenViBEVisualizationToolkit::IVisualizationManager* m_pVisualizationManager;
		OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext;
		OpenViBE::Kernel::IScenario* m_pClipboardScenario;

		OpenViBEDesigner::CLogListenerDesigner* m_pLogListenerDesigner;

		OpenViBEDesigner::ECommandLineFlag m_eCommandLineFlags;

		::GtkBuilder* m_pBuilderInterface;
		::GtkWidget* m_pMainWindow;
		::GtkWidget* m_pSplashScreen;
		::GtkNotebook* m_pScenarioNotebook;
		::GtkNotebook* m_pResourceNotebook;
		::GtkTreeStore* m_pBoxAlgorithmTreeModel;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter2;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter3;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter4;
		::GtkTreeView* m_pBoxAlgorithmTreeView;
		::GtkTreeStore* m_pAlgorithmTreeModel;
		::GtkTreeView* m_pAlgorithmTreeView;
		::GtkSpinButton* m_pFastForwardFactor;
		::GtkWidget* m_pConfigureSettingsAddSettingButton;
		::GtkContainer* m_MenuOpenRecent;
		std::vector <const GtkWidget*> m_RecentScenarios;

		// UI for adding inputs and outputs to a scenario
		GtkWidget* m_pTableInputs;
		GtkWidget* m_pTableOutputs;

		gint m_giFilterTimeout;

		OpenViBE::boolean m_bIsMaximized;

		const gchar* m_sSearchTerm;

		OpenViBE::uint64 m_ui64LastTimeRefresh;
		bool m_bIsQuitting;
		bool m_bIsNewVersion;

		std::vector < OpenViBEDesigner::CInterfacedScenario* > m_vInterfacedScenario;
		OpenViBE::uint32 m_ui32CurrentInterfacedScenarioIndex;
		Mensia::CMetaboxLoader* m_pMetaboxLoader;
		Mensia::ActivationTool::CActivationTool* m_pActivationTool;
		std::vector <const OpenViBE::Plugins::IPluginObjectDesc*> m_vNewBoxes;
		std::vector <const OpenViBE::Plugins::IPluginObjectDesc*> m_vUpdatedBoxes;
		std::vector <std::string> m_vDocumentedBoxes;
#if defined TARGET_HAS_LibArchway
		Mensia::CArchwayHandler m_oArchwayHandler;
		Mensia::CArchwayHandlerGUI m_oArchwayHandlerGUI;
#endif
	};
}

#endif // __OpenViBEDesigner_CApplication_H__
