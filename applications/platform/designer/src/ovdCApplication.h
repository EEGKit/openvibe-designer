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
		CApplication(const OpenViBE::Kernel::IKernelContext& ctx);
		~CApplication();

		void initialize(ECommandLineFlag eCommandLineFlags);

		bool openScenario(const char* sFileName);

		/** \name Drag and drop management */
		//@{

		void dragDataGetCB(GtkWidget* pWidget, GdkDragContext* pDragContex, GtkSelectionData* pSelectionData, guint uiInfo, guint uiT) const;

		//@}

		/** \name Selection management */
		//@{

		void undoCB();
		void redoCB();

		void copySelectionCB();
		void cutSelectionCB();
		void pasteSelectionCB();
		void deleteSelectionCB();
		void preferencesCB() const;

		//@}

		/** \name Scenario management */
		//@{

		OpenViBE::CString getWorkingDirectory();

		bool hasRunningScenario();
		bool hasUnsavedScenario();

		CInterfacedScenario* getCurrentInterfacedScenario();
		void saveOpenedScenarios();

		void testCB() const;
		void newScenarioCB();
		void openScenarioCB();
		void saveScenarioCB(CInterfacedScenario* interfacedScenario = nullptr); // defaults to current scenario if nullptr
		void saveScenarioAsCB(CInterfacedScenario* interfacedScenario = nullptr); // defaults to current scenario if nullptr
		void closeScenarioCB(CInterfacedScenario* interfacedScenario);
		void restoreDefaultScenariosCB() const;

		void stopScenarioCB();
		void pauseScenarioCB();
		void nextScenarioCB();
		void playScenarioCB();
		void forwardScenarioCB();

		void configureScenarioSettingsCB(CInterfacedScenario* pScenario) const;

		void addCommentCB(CInterfacedScenario* pScenario) const;

		void changeCurrentScenario(int pageIdx);
		void reorderCurrentScenario(uint32_t newPageIdx);

		void addRecentScenario(const std::string& scenarioPath);

		static void cannotSaveScenarioBeforeUpdate();

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
		void aboutScenarioCB(CInterfacedScenario* pScenario) const;
		void aboutLinkClickedCB(const gchar* url) const;

		void browseDocumentationCB() const;
		void registerLicenseCB() const;
		void reportIssueCB() const;
		void windowStateChangedCB(bool bIsMaximized);
		bool displayChangelogWhenAvailable();

		//@}

		/** \name Log management */
		//@{

		void logLevelCB() const;
		void logLevelMessagesCB();

		//@}

		/** \name CPU usage */
		//@{

		void CPUUsageCB();

		//@}
		void zoomInCB();//Call when a zoom in is required
		void zoomOutCB();//Call when a zoom out is required
		void spinnerZoomChangedCB(uint32_t scaleDelta);

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IPluginManager* m_pPluginManager = nullptr;
		OpenViBE::Kernel::IScenarioManager* m_pScenarioManager = nullptr;
		OpenViBEVisualizationToolkit::IVisualizationManager* m_pVisualizationManager = nullptr;
		OpenViBEVisualizationToolkit::IVisualizationContext* m_VisualizationCtx = nullptr;
		OpenViBE::Kernel::IScenario* m_pClipboardScenario = nullptr;

		CLogListenerDesigner* m_pLogListenerDesigner = nullptr;

		ECommandLineFlag m_eCommandLineFlags = CommandLineFlag_None;

		GtkBuilder* m_pBuilderInterface = nullptr;
		GtkWidget* m_pMainWindow = nullptr;
		GtkWidget* m_pSplashScreen = nullptr;
		GtkNotebook* m_pScenarioNotebook = nullptr;
		GtkNotebook* m_pResourceNotebook = nullptr;
		GtkTreeStore* m_BoxAlgorithmTreeModel = nullptr;
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

		bool m_isMaximized = false;

		const gchar* m_sSearchTerm = nullptr;

		uint64_t m_lastTimeRefresh = 0;
		bool m_isQuitting = false;
		bool m_isNewVersion = false;

		std::vector<CInterfacedScenario*> m_Scenarios;
		uint32_t m_currentScenarioIdx = 0;
		std::vector<const OpenViBE::Plugins::IPluginObjectDesc*> m_NewBoxes;
		std::vector<const OpenViBE::Plugins::IPluginObjectDesc*> m_UpdatedBoxes;
		std::vector<std::string> m_vDocumentedBoxes;
#ifdef MENSIA_DISTRIBUTION
		Mensia::CArchwayHandler* m_pArchwayHandler = nullptr;
		Mensia::CArchwayHandlerGUI* m_pArchwayHandlerGUI = nullptr;
#endif
	};
}
