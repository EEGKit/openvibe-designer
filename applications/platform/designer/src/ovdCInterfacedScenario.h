#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>

#include "ovd_base.h"

#include "ovdCInterfacedObject.h"
#include "ovdCScenarioStateStack.h"
#include "ovdCSettingCollectionHelper.h"
#include "ovdCBoxConfigurationDialog.h"
#include <visualization-toolkit/ovvizIVisualizationTree.h>

namespace OpenViBEDesigner
{
	class CApplication;
	class CDesignerVisualization;
	class CPlayerVisualization;

	class CInterfacedScenario
	{
	public:

		CInterfacedScenario(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBEDesigner::CApplication& rApplication, OpenViBE::Kernel::IScenario& rScenario, OpenViBE::CIdentifier& rScenarioIdentifier,
			::GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename);
		virtual ~CInterfacedScenario(void);

		virtual OpenViBE::boolean isLocked(void) const;
		virtual void redraw(void);
		virtual void redraw(OpenViBE::Kernel::IBox& rBox);
		virtual void redraw(OpenViBE::Kernel::IComment& rComment);
		virtual void redraw(OpenViBE::Kernel::ILink& rLink);
		virtual void updateScenarioLabel(void);
		OpenViBE::uint32 pickInterfacedObject(int x, int y);
		OpenViBE::boolean pickInterfacedObject(int x, int y, int iSizeX, int iSizeY);

		void undoCB(OpenViBE::boolean bManageModifiedStatusFlag=true);
		void redoCB(OpenViBE::boolean bManageModifiedStatusFlag=true);
		void snapshotCB(OpenViBE::boolean bManageModifiedStatusFlag=true);
		void addCommentCB(int x=-1, int y=-1);

		void addScenarioSettingCB(void);
		void addScenarioInputCB(void);
		void addScenarioOutputCB(void);

		// Utility functions for scenario settings, inputs and outputs
		void configureScenarioSettingsCB(void);
		void swapScenarioSettings(unsigned int uiSettingAIndex, unsigned int uiSettingBIndex);

		// Drawing functions for scenario settings, inputs and outputs
		void redrawConfigureScenarioSettingsDialog();
		void redrawScenarioSettings();
		void redrawScenarioInputSettings();
		void redrawScenarioOutputSettings();

		void scenarioDrawingAreaExposeCB(::GdkEventExpose* pEvent);
		void scenarioDrawingAreaDragDataReceivedCB(::GdkDragContext* pDragContext, gint iX, gint iY, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT);
		void scenarioDrawingAreaMotionNotifyCB(::GtkWidget* pWidget, ::GdkEventMotion* pEvent);
		void scenarioDrawingAreaButtonPressedCB(::GtkWidget* pWidget, ::GdkEventButton* pEvent);
		void scenarioDrawingAreaButtonReleasedCB(::GtkWidget* pWidget, ::GdkEventButton* pEvent);
		void scenarioDrawingAreaKeyPressEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent);
		void scenarioDrawingAreaKeyReleaseEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent);

		void copySelection(void);
		void cutSelection(void);
		void pasteSelection(void);
		void deleteSelection(void);

		void contextMenuBoxRenameCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRenameAllCB();
		void contextMenuBoxToggleEnableAllCB(void);
		void contextMenuBoxDeleteCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxAddInputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxRemoveInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxAddOutputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxRemoveOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);

		void contextMenuBoxConnectScenarioInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32BoxInputIndex, OpenViBE::uint32 ui32ScenarioInputIndex);
		void contextMenuBoxConnectScenarioOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32BoxOutputIndex, OpenViBE::uint32 ui32ScenarioOutputIndex);
		void contextMenuBoxDisconnectScenarioInputCB(OpenViBE::Kernel::IBox&rBox, OpenViBE::uint32 ui32BoxInputIndex, OpenViBE::uint32 ui32ScenarioInputIndex);
		void contextMenuBoxDisconnectScenarioOutputCB(OpenViBE::Kernel::IBox&rBox, OpenViBE::uint32 ui32BoxOutputIndex, OpenViBE::uint32 ui32ScenarioOutputIndex);

		void contextMenuBoxAddSettingCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditSettingCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxRemoveSettingCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxConfigureCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxAboutCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditMetaboxCB(OpenViBE::Kernel::IBox& rBox);

		void contextMenuBoxEnableCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxDisableCB(OpenViBE::Kernel::IBox& rBox);

		void contextMenuBoxDocumentationCB(OpenViBE::Kernel::IBox& rBox);

		void contextMenuScenarioAddCommentCB(void);
		void contextMenuScenarioAboutCB(void);

		bool browseURL(OpenViBE::CString sURL, OpenViBE::CString sBrowser = "");
		bool browseBoxDocumentation(OpenViBE::CIdentifier oBoxId);

		void toggleDesignerVisualization();
		OpenViBE::boolean isDesignerVisualizationToggled();

		void showCurrentVisualization();
		void hideCurrentVisualization();

		void createPlayerVisualization(OpenViBEVisualizationToolkit::IVisualizationTree* pVisualizationTree = NULL);
		void releasePlayerVisualization(void);


		void stopAndReleasePlayer(void);
		OpenViBE::boolean setModifiableSettingsWidgets(void);
		OpenViBE::boolean hasSelection(void);
		bool centerOnBox(OpenViBE::CIdentifier rIdentifier);

/*
	private:

		void generateDisplayPluginName(OpenViBE::Kernel::IBox* pDisplayBox, OpenViBE::CString& rDisplayBoxName);*/

	public:

		OpenViBE::Kernel::EPlayerStatus m_ePlayerStatus;
		OpenViBE::CIdentifier m_oScenarioIdentifier;
		OpenViBE::CIdentifier m_oPlayerIdentifier;
		OpenViBE::CIdentifier m_oVisualizationTreeIdentifier;
		OpenViBEDesigner::CApplication& m_rApplication;
		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IScenario& m_rScenario;
		OpenViBE::Kernel::IPlayer* m_pPlayer;
		OpenViBE::uint64 m_ui64LastLoopTime;
		::GtkNotebook& m_rNotebook;
		OpenViBEVisualizationToolkit::IVisualizationTree* m_pVisualizationTree;
		OpenViBE::boolean m_bDesignerVisualizationToggled;
		OpenViBEDesigner::CDesignerVisualization* m_pDesignerVisualization;
		OpenViBEDesigner::CPlayerVisualization* m_pPlayerVisualization;
		::GtkBuilder* m_pGUIBuilder;
//		::GtkBuilder* m_pSettingsGUIBuilder;
/*		::GtkBuilder* m_pBuilder;
		::GtkBuilder* m_pBuilder;*/
		::GtkWidget* m_pNotebookPageTitle;
		::GtkWidget* m_pNotebookPageContent;
		::GtkViewport* m_pScenarioViewport;
		::GtkDrawingArea* m_pScenarioDrawingArea;
		::GdkPixmap* m_pStencilBuffer;
		::GdkPixbuf* m_pMensiaLogoPixbuf;
		OpenViBE::boolean m_bHasFileName;
		OpenViBE::boolean m_bHasBeenModified;
		OpenViBE::boolean m_bButtonPressed;
		OpenViBE::boolean m_bShiftPressed;
		OpenViBE::boolean m_bControlPressed;
		OpenViBE::boolean m_bAltPressed;
		OpenViBE::boolean m_bAPressed;
		OpenViBE::boolean m_bWPressed;
		OpenViBE::boolean m_bDebugCPUUsage;
		std::string m_sFileName;
		std::string m_sGUIFilename;
		std::string m_sGUISettingsFilename;
		OpenViBE::float64 m_f64PressMouseX;
		OpenViBE::float64 m_f64PressMouseY;
		OpenViBE::float64 m_f64ReleaseMouseX;
		OpenViBE::float64 m_f64ReleaseMouseY;
		OpenViBE::float64 m_f64CurrentMouseX;
		OpenViBE::float64 m_f64CurrentMouseY;
		OpenViBE::int32 m_i32ViewOffsetX;
		OpenViBE::int32 m_i32ViewOffsetY;
		OpenViBE::uint32 m_ui32CurrentMode;

		OpenViBE::uint32 m_ui32BoxCount;
		OpenViBE::uint32 m_ui32CommentCount;
		OpenViBE::uint32 m_ui32LinkCount;

		OpenViBE::uint32 m_ui32InterfacedObjectId;
		std::map<OpenViBE::uint32, OpenViBEDesigner::CInterfacedObject> m_vInterfacedObject;
		std::map<OpenViBE::CIdentifier, OpenViBE::boolean> m_vCurrentObject;
		OpenViBEDesigner::CInterfacedObject m_oCurrentObject;

		OpenViBE::float64 m_f64HPanInitialPosition;
		OpenViBE::float64 m_f64VPanInitialPosition;

		typedef struct _BoxContextMenuCB
		{
			OpenViBE::uint32 ui32Command;
			OpenViBE::uint32 ui32Index;
			OpenViBE::uint32 ui32SecondaryIndex; // Used for connecting two streams
			OpenViBE::Kernel::IBox* pBox;
			OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario;
		} BoxContextMenuCB;
		std::map < OpenViBE::uint32, BoxContextMenuCB > m_vBoxContextMenuCB;

		std::unique_ptr<OpenViBEDesigner::CScenarioStateStack> m_oStateStack;

		// Objects necessary for holding settings GUI
		std::map<std::string, OpenViBE::CIdentifier> m_vSettingType;
		OpenViBEDesigner::CSettingCollectionHelper* m_pSettingHelper;
		std::string m_sSerializedSettingGUIXML;

		GtkWidget* m_pConfigureSettingsDialog;

		GtkWidget* m_pSettingsVBox;

		GtkWidget* m_pNoHelpDialog;

		// This struct is used for both settings inside the scenario and inside
		// the settings configurator
		typedef struct _SSettingCallbackData
		{
			CInterfacedScenario* m_pInterfacedScenario;
			int m_iSettingIndex;
			GtkWidget* m_pWidgetValue;
			GtkWidget* m_pWidgetEntryValue;
			GtkWidget* m_pContainer;

		} SSettingCallbackData;


		std::vector<SSettingCallbackData> m_vSettingConfigurationCallbackData;
		std::vector<SSettingCallbackData> m_vSettingCallbackData;

		// Object necessary for holding scenario inputs/outputs

		std::map<std::string, OpenViBE::CIdentifier> m_mStreamType;

		// This struct is used for both inputs and outputs of the scenario

		typedef struct _SLinkCallbackData
		{
			CInterfacedScenario* m_pInterfacedScenario;
			int m_iLinkIndex;
			bool m_bIsInput;

		} SLinkCallbackData;

		std::vector<SLinkCallbackData> m_vScenarioInputCallbackData;
		std::vector<SLinkCallbackData> m_vScenarioOutputCallbackData;

	private:
		void redrawScenarioLinkSettings(
		        GtkWidget* pLinkTable,
		        OpenViBE::boolean bIsInput,
		        std::vector<SLinkCallbackData>& vLinkCallbackData,
		        OpenViBE::uint32 (OpenViBE::Kernel::IScenario::*pfGetLinkCount)() const,
		        OpenViBE::boolean (OpenViBE::Kernel::IScenario::*pfGetLinkName)(OpenViBE::uint32, OpenViBE::CString&) const,
		        OpenViBE::boolean (OpenViBE::Kernel::IScenario::*pfGetLinkType)(OpenViBE::uint32, OpenViBE::CIdentifier&) const
		        );


		std::vector<CBoxConfigurationDialog*> m_vBoxConfigurationDialog;
	};
}

