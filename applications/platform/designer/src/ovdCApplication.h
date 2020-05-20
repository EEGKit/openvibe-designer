#pragma once

#include <visualization-toolkit/ovvizIVisualizationManager.h>
#include "ovd_base.h"

#include "archway/ovdCArchwayHandler.h"
#include "archway/ovdCArchwayHandlerGUI.h"

#define OVD_ScenarioImportContext_OpenScenario	OpenViBE::CIdentifier(0xA180DB91, 0x19235AEE)
#define OVD_ScenarioExportContext_SaveScenario	OpenViBE::CIdentifier(0xC98C47AD, 0xCBD952B2)
#define OVD_ScenarioExportContext_SaveMetabox	OpenViBE::CIdentifier(0x529494F1, 0x6C2527D9)

#include <vector>

namespace OpenViBE {
namespace Designer {

class CInterfacedScenario;

class CLogListenerDesigner;

class CApplication
{
public:
	explicit CApplication(const Kernel::IKernelContext& ctx);
	~CApplication();

	void initialize(ECommandLineFlag cmdLineFlags);
	bool openScenario(const char* filename);

	/** \name Drag and drop management */
	//@{

	void dragDataGetCB(GtkWidget* widget, GdkDragContext* dragCtx, GtkSelectionData* selectionData, guint info, guint time) const;

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

	CString getWorkingDirectory();

	bool hasRunningScenario();
	bool hasUnsavedScenario();

	CInterfacedScenario* getCurrentInterfacedScenario();
	void saveOpenedScenarios();

	void testCB() const;
	void newScenarioCB();
	void openScenarioCB();
	void saveScenarioCB(CInterfacedScenario* scenario = nullptr); // defaults to current scenario if nullptr
	void saveScenarioAsCB(CInterfacedScenario* scenario = nullptr); // defaults to current scenario if nullptr
	void closeScenarioCB(CInterfacedScenario* scenario);
	void restoreDefaultScenariosCB() const;

	void stopScenarioCB();
	void pauseScenarioCB();
	void nextScenarioCB();
	void playScenarioCB();
	void forwardScenarioCB();

	void configureScenarioSettingsCB(CInterfacedScenario* scenario) const;

	void addCommentCB(CInterfacedScenario* scenario) const;

	void changeCurrentScenario(int pageIdx);
	void reorderCurrentScenario(size_t newPageIdx);
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

	Kernel::IPlayer* getPlayer();
	bool createPlayer();
	void stopInterfacedScenarioAndReleasePlayer(CInterfacedScenario* scenario);

	//@}

	/** \name Application management */
	//@{

	bool quitApplicationCB();
	void aboutOpenViBECB() const;
	void aboutScenarioCB(CInterfacedScenario* scenario) const;
	void aboutLinkClickedCB(const gchar* url) const;

	void browseDocumentationCB() const;
	static void registerLicenseCB();
	void reportIssueCB() const;
	void windowStateChangedCB(bool isMaximized);
	bool displayChangelogWhenAvailable();

	//@}

	void logLevelCB() const;
	//void logLevelMessagesCB();
	void cpuUsageCB();
	void zoomInCB();	//Call when a zoom in is required
	void zoomOutCB();	//Call when a zoom out is required
	void spinnerZoomChangedCB(const size_t scaleDelta);

	const Kernel::IKernelContext& m_kernelCtx;
	Kernel::IPluginManager* m_PluginMgr                             = nullptr;
	Kernel::IScenarioManager* m_ScenarioMgr                         = nullptr;
	VisualizationToolkit::IVisualizationManager* m_VisualizationMgr = nullptr;
	Kernel::IScenario* m_ClipboardScenario                          = nullptr;

	ECommandLineFlag m_CmdLineFlags = CommandLineFlag_None;

	GtkBuilder* m_Builder   = nullptr;
	GtkWidget* m_MainWindow = nullptr;

	GtkTreeStore* m_BoxAlgorithmTreeModel        = nullptr;
	GtkTreeModel* m_BoxAlgorithmTreeModelFilter  = nullptr;
	GtkTreeModel* m_BoxAlgorithmTreeModelFilter2 = nullptr;
	GtkTreeModel* m_BoxAlgorithmTreeModelFilter3 = nullptr;
	GtkTreeModel* m_BoxAlgorithmTreeModelFilter4 = nullptr;
	GtkTreeView* m_BoxAlgorithmTreeView          = nullptr;
	GtkTreeStore* m_AlgorithmTreeModel           = nullptr;

	GtkSpinButton* m_FastForwardFactor = nullptr;

	// UI for adding inputs and outputs to a scenario
	GtkWidget* m_Inputs  = nullptr;
	GtkWidget* m_Outputs = nullptr;

	gint m_FilterTimeout      = 0;
	const gchar* m_SearchTerm = nullptr;

	uint64_t m_LastTimeRefresh = 0;
	bool m_IsQuitting          = false;
	bool m_IsNewVersion        = false;

	std::vector<CInterfacedScenario*> m_Scenarios;
	std::vector<const Plugins::IPluginObjectDesc*> m_NewBoxes;
	std::vector<const Plugins::IPluginObjectDesc*> m_UpdatedBoxes;

#ifdef MENSIA_DISTRIBUTION
		Mensia::CArchwayHandler* m_ArchwayHandler       = nullptr;
		Mensia::CArchwayHandlerGUI* m_ArchwayHandlerGUI = nullptr;
#endif

protected:
	VisualizationToolkit::IVisualizationContext* m_visualizationCtx = nullptr;

	CLogListenerDesigner* m_logListener = nullptr;

	GtkWidget* m_splashScreen                      = nullptr;
	GtkNotebook* m_scenarioNotebook                = nullptr;
	GtkNotebook* m_resourceNotebook                = nullptr;
	GtkTreeView* m_algorithmTreeView               = nullptr;
	GtkWidget* m_configureSettingsAddSettingButton = nullptr;
	GtkContainer* m_menuOpenRecent                 = nullptr;
	std::vector<const GtkWidget*> m_recentScenarios;

	bool m_isMaximized = false;

	size_t m_currentScenarioIdx = 0;
	std::vector<std::string> m_documentedBoxes;
};

}  //namespace Designer
}  //namespace OpenViBE
