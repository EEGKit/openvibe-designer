#pragma once

#include <visualization-toolkit/ovvizIVisualizationManager.h>
#include "ovd_base.h"

#include "archway/ovdCArchwayHandler.h"
#include "archway/ovdCArchwayHandlerGUI.h"

#define OVD_ScenarioImportContext_OpenScenario OpenViBE::CIdentifier(0xA180DB91, 0x19235AEE)
#define OVD_ScenarioExportContext_SaveScenario OpenViBE::CIdentifier(0xC98C47AD, 0xCBD952B2)
#define OVD_ScenarioExportContext_SaveMetabox  OpenViBE::CIdentifier(0x529494F1, 0x6C2527D9)

#include <vector>

namespace OpenViBEDesigner
{
	class CInterfacedScenario;

	class CLogListenerDesigner;

	class CApplication
	{
	public:
		CApplication(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		~CApplication();

		void initialize(ECommandLineFlag eCommandLineFlags);

		bool openScenario(const char* sFileName);

		/** \name Drag and drop management */
		//@{

		void dragDataGetCB(GtkWidget* pWidget, GdkDragContext* pDragContex, GtkSelectionData* pSelectionData, guint uiInfo, guint uiT);

		//@}

		/** \name Selection management */
		//@{

		void undoCB();
		void redoCB();

		void copySelectionCB();
		void cutSelectionCB();
		void pasteSelectionCB();
		void deleteSelectionCB();
		void preferencesCB();

		//@}

		/** \name Scenario management */
		//@{

		OpenViBE::CString getWorkingDirectory();

		bool hasRunningScenario();
		bool hasUnsavedScenario();

		CInterfacedScenario* getCurrentInterfacedScenario();
		void saveOpenedScenarios();

		void testCB();
		void newScenarioCB();
		void openScenarioCB();
		void saveScenarioCB(CInterfacedScenario* interfacedScenario = nullptr); // defaults to current scenario if NULL
		void saveScenarioAsCB(CInterfacedScenario* interfacedScenario = nullptr); // defaults to current scenario if NULL
		void closeScenarioCB(CInterfacedScenario* interfacedScenario);
		void restoreDefaultScenariosCB();

		void stopScenarioCB();
		void pauseScenarioCB();
		void nextScenarioCB();
		void playScenarioCB();
		void forwardScenarioCB();

		void configureScenarioSettingsCB(CInterfacedScenario* pScenario);

		void addCommentCB(CInterfacedScenario* pScenario);

		void changeCurrentScenario(int i32PageIndex);
		void reorderCurrentScenario(uint32_t i32NewPageIndex);

		void addRecentScenario(const std::string& scenarioPath);

		void cannotSaveScenarioBeforeUpdate();

		//@}

		/** \name Designer visualization management */
		//@{

		void deleteDesignerVisualizationCB();

		void toggleDesignerVisualizationCB();

		//@}

		/** \name Player management */
		//@{

		OpenViBE::Kernel::IPlayer* getPlayer();

		bool createPlayer();

		void stopInterfacedScenarioAndReleasePlayer(CInterfacedScenario* interfacedScenario);

		//@}

		/** \name Application management */
		//@{

		bool quitApplicationCB();
		void aboutOpenViBECB();
		void aboutScenarioCB(CInterfacedScenario* pScenario);
		void aboutLinkClickedCB(const gchar* url);

		void browseDocumentationCB();
		void registerLicenseCB();
		void reportIssueCB();
		void windowStateChangedCB(bool bIsMaximized);
		bool displayChangelogWhenAvailable();

		//@}

		/** \name Log management */
		//@{

		void logLevelCB();
		void logLevelMessagesCB();

		//@}

		/** \name CPU usage */
		//@{

		void CPUUsageCB();

		//@}
		void zoomInCB();//Call when a zoom in is required
		void zoomOutCB();//Call when a zoom out is required
		void spinnerZoomChangedCB(uint32_t scaleDelta);

		const OpenViBE::Kernel::IKernelContext& m_KernelContext;
		OpenViBE::Kernel::IPluginManager* m_pPluginManager = nullptr;
		OpenViBE::Kernel::IScenarioManager* m_pScenarioManager = nullptr;
		OpenViBEVisualizationToolkit::IVisualizationManager* m_pVisualizationManager = nullptr;
		OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext = nullptr;
		OpenViBE::Kernel::IScenario* m_pClipboardScenario = nullptr;

		CLogListenerDesigner* m_pLogListenerDesigner = nullptr;

		ECommandLineFlag m_eCommandLineFlags = CommandLineFlag_None;

		GtkBuilder* m_pBuilderInterface = nullptr;
		GtkWidget* m_pMainWindow = nullptr;
		GtkWidget* m_pSplashScreen = nullptr;
		GtkNotebook* m_pScenarioNotebook = nullptr;
		GtkNotebook* m_pResourceNotebook = nullptr;
		GtkTreeStore* m_pBoxAlgorithmTreeModel = nullptr;
		GtkTreeModel* m_pBoxAlgorithmTreeModelFilter = nullptr;
		GtkTreeModel* m_pBoxAlgorithmTreeModelFilter2 = nullptr;
		GtkTreeModel* m_pBoxAlgorithmTreeModelFilter3 = nullptr;
		GtkTreeModel* m_pBoxAlgorithmTreeModelFilter4 = nullptr;
		GtkTreeView* m_pBoxAlgorithmTreeView = nullptr;
		GtkTreeStore* m_pAlgorithmTreeModel = nullptr;
		GtkTreeView* m_pAlgorithmTreeView = nullptr;
		GtkSpinButton* m_pFastForwardFactor = nullptr;
		GtkWidget* m_pConfigureSettingsAddSettingButton = nullptr;
		GtkContainer* m_MenuOpenRecent = nullptr;
		std::vector<const GtkWidget*> m_RecentScenarios;

		// UI for adding inputs and outputs to a scenario
		GtkWidget* m_pTableInputs = nullptr;
		GtkWidget* m_pTableOutputs = nullptr;

		gint m_giFilterTimeout = 0;

		bool m_bIsMaximized = false;

		const gchar* m_sSearchTerm = nullptr;

		uint64_t m_ui64LastTimeRefresh = 0;
		bool m_bIsQuitting = false;
		bool m_bIsNewVersion = false;

		std::vector<CInterfacedScenario*> m_vInterfacedScenario;
		uint32_t m_ui32CurrentInterfacedScenarioIndex = 0;
		std::vector<const OpenViBE::Plugins::IPluginObjectDesc*> m_vNewBoxes;
		std::vector<const OpenViBE::Plugins::IPluginObjectDesc*> m_vUpdatedBoxes;
		std::vector<std::string> m_vDocumentedBoxes;
#ifdef MENSIA_DISTRIBUTION
		Mensia::CArchwayHandler* m_pArchwayHandler;
		Mensia::CArchwayHandlerGUI* m_pArchwayHandlerGUI;
#endif
	};
}
