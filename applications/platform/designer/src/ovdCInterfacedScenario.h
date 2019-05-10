#pragma once

#include <set>
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

		CInterfacedScenario(const OpenViBE::Kernel::IKernelContext& rKernelContext, CApplication& rApplication, OpenViBE::Kernel::IScenario& rScenario, OpenViBE::CIdentifier& rScenarioIdentifier,
							GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename);
		virtual ~CInterfacedScenario();

		virtual bool isLocked() const;
		virtual void redraw();
		virtual void redraw(OpenViBE::Kernel::IBox& rBox);
		virtual void redraw(OpenViBE::Kernel::IComment& rComment);
		virtual void redraw(OpenViBE::Kernel::ILink& rLink);
		virtual void updateScenarioLabel();
		uint32_t pickInterfacedObject(int x, int y);
		bool pickInterfacedObject(int x, int y, int iSizeX, int iSizeY);

		void undoCB(bool bManageModifiedStatusFlag = true);
		void redoCB(bool bManageModifiedStatusFlag = true);
		void snapshotCB(bool bManageModifiedStatusFlag = true);
		void addCommentCB(int x = -1, int y = -1);

		// Utility functions for scenario settings, inputs and outputs

		void addScenarioSettingCB();
		void editScenarioSettingCB(unsigned int l_ui32SettingIndex);
		void swapScenarioSettings(unsigned int uiSettingAIndex, unsigned int uiSettingBIndex);

		void addScenarioInputCB();
		void editScenarioInputCB(unsigned int l_ui32InputIndex);
		void swapScenarioInputs(unsigned int ui32InputAIndex, unsigned int ui32InputBIndex);
		void addScenarioOutputCB();
		void editScenarioOutputCB(unsigned int l_ui32OutputIndex);
		void swapScenarioOutputs(unsigned int ui32OutputAIndex, unsigned int ui32OutputBIndex);


		// Utility functions for scenario settings, inputs and outputs
		void configureScenarioSettingsCB();

		// Drawing functions for scenario settings, inputs and outputs
		void redrawConfigureScenarioSettingsDialog();
		void redrawScenarioSettings();
		void redrawScenarioInputSettings();
		void redrawScenarioOutputSettings();

		void scenarioDrawingAreaExposeCB(GdkEventExpose* pEvent);
		void scenarioDrawingAreaDragDataReceivedCB(GdkDragContext* pDragContext, gint iX, gint iY, GtkSelectionData* pSelectionData, guint uiInfo, guint uiT);
		void scenarioDrawingAreaMotionNotifyCB(GtkWidget* pWidget, GdkEventMotion* pEvent);
		void scenarioDrawingAreaButtonPressedCB(GtkWidget* pWidget, GdkEventButton* pEvent);
		void scenarioDrawingAreaButtonReleasedCB(GtkWidget* pWidget, GdkEventButton* pEvent);
		void scenarioDrawingAreaKeyPressEventCB(GtkWidget* pWidget, GdkEventKey* pEvent);
		void scenarioDrawingAreaKeyReleaseEventCB(GtkWidget* pWidget, GdkEventKey* pEvent);

		void copySelection();
		void cutSelection();
		void pasteSelection();
		void deleteSelection();

		void deleteBox(const OpenViBE::CIdentifier& rBoxIdentifier); // Utility method to remove box from scenario and visualization
		void contextMenuBoxUpdateCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRemoveDeprecatedInterfacorsCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRenameCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRenameAllCB();
		void contextMenuBoxToggleEnableAllCB();
		void contextMenuBoxAddInputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditInputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index);
		void contextMenuBoxRemoveInputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index);
		void contextMenuBoxAddOutputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditOutputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index);
		void contextMenuBoxRemoveOutputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index);

		void contextMenuBoxConnectScenarioInputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32BoxInputIndex, uint32_t ui32ScenarioInputIndex);
		void contextMenuBoxConnectScenarioOutputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32BoxOutputIndex, uint32_t ui32ScenarioOutputIndex);
		void contextMenuBoxDisconnectScenarioInputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32BoxInputIndex, uint32_t ui32ScenarioInputIndex);
		void contextMenuBoxDisconnectScenarioOutputCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32BoxOutputIndex, uint32_t ui32ScenarioOutputIndex);

		void contextMenuBoxAddSettingCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditSettingCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index);
		void contextMenuBoxRemoveSettingCB(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index);
		void contextMenuBoxConfigureCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxAboutCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditMetaboxCB(OpenViBE::Kernel::IBox& rBox);

		void contextMenuBoxEnableCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxDisableCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEnableAllCB();
		void contextMenuBoxDisableAllCB();

		void contextMenuBoxDocumentationCB(OpenViBE::Kernel::IBox& rBox);

		void contextMenuScenarioAddCommentCB();
		void contextMenuScenarioAboutCB();

		bool browseURL(const OpenViBE::CString& url, const OpenViBE::CString& browserPrefix, const OpenViBE::CString& browserPostfix);
		bool browseBoxDocumentation(const OpenViBE::CIdentifier& oBoxId);

		void toggleDesignerVisualization();
		bool isDesignerVisualizationToggled();

		void showCurrentVisualization();
		void hideCurrentVisualization();

		void createPlayerVisualization(OpenViBEVisualizationToolkit::IVisualizationTree* pVisualizationTree = nullptr);
		void releasePlayerVisualization();


		void stopAndReleasePlayer();
		bool setModifiableSettingsWidgets();
		bool hasSelection();
		bool centerOnBox(const OpenViBE::CIdentifier& rIdentifier);
		void setScale(double scale);
		double getScale();

		/*
			private:
		
				void generateDisplayPluginName(OpenViBE::Kernel::IBox* pDisplayBox, OpenViBE::CString& rDisplayBoxName);*/

		OpenViBE::Kernel::EPlayerStatus m_ePlayerStatus;
		OpenViBE::CIdentifier m_oScenarioIdentifier;
		OpenViBE::CIdentifier m_oPlayerIdentifier;
		OpenViBE::CIdentifier m_oVisualizationTreeIdentifier;
		CApplication& m_rApplication;
		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IScenario& m_rScenario;
		OpenViBE::Kernel::IPlayer* m_pPlayer = nullptr;
		OpenViBE::uint64 m_ui64LastLoopTime = 0;
		GtkNotebook& m_rNotebook;
		OpenViBEVisualizationToolkit::IVisualizationTree* m_pVisualizationTree = nullptr;
		bool m_bDesignerVisualizationToggled = false;
		CDesignerVisualization* m_pDesignerVisualization = nullptr;
		CPlayerVisualization* m_pPlayerVisualization = nullptr;
		GtkBuilder* m_pGUIBuilder = nullptr;
		GtkWidget* m_pNotebookPageTitle = nullptr;
		GtkWidget* m_pNotebookPageContent = nullptr;
		GtkViewport* m_pScenarioViewport = nullptr;
		GtkDrawingArea* m_pScenarioDrawingArea = nullptr;
		GdkPixmap* m_pStencilBuffer = nullptr;
		GdkPixbuf* m_pMensiaLogoPixbuf = nullptr;
		bool m_bHasFileName = false;
		bool m_bHasBeenModified = false;
		bool m_bButtonPressed = false;
		bool m_bShiftPressed = false;
		bool m_bControlPressed = false;
		bool m_bAltPressed = false;
		bool m_bAPressed = false;
		bool m_bWPressed = false;
		bool m_bDebugCPUUsage = false;
		std::string m_sFileName;
		std::string m_sGUIFilename;
		std::string m_sGUISettingsFilename;
		double m_f64PressMouseX = 0;
		double m_f64PressMouseY = 0;
		double m_f64ReleaseMouseX = 0;
		double m_f64ReleaseMouseY = 0;
		double m_f64CurrentMouseX = 0;
		double m_f64CurrentMouseY = 0;
		int32_t m_i32ViewOffsetX = 0;
		int32_t m_i32ViewOffsetY = 0;
		uint32_t m_ui32CurrentMode = 0;

		uint32_t m_ui32BoxCount = 0;
		uint32_t m_ui32CommentCount = 0;
		uint32_t m_ui32LinkCount = 0;

		uint32_t m_ui32InterfacedObjectId = 0;
		std::map<uint32_t, CInterfacedObject> m_vInterfacedObject;
		std::set<OpenViBE::CIdentifier> m_SelectedObjects;
		CInterfacedObject m_oCurrentObject;

		double m_f64HPanInitialPosition = 0;
		double m_f64VPanInitialPosition = 0;

		typedef struct _BoxContextMenuCB
		{
			uint32_t ui32Command;
			uint32_t ui32Index;
			uint32_t ui32SecondaryIndex; // Used for connecting two streams
			OpenViBE::Kernel::IBox* pBox;
			CInterfacedScenario* pInterfacedScenario;
		} BoxContextMenuCB;

		std::map<uint32_t, BoxContextMenuCB> m_vBoxContextMenuCB;

		std::unique_ptr<CScenarioStateStack> m_oStateStack;

		// Objects necessary for holding settings GUI
		std::map<std::string, OpenViBE::CIdentifier> m_vSettingType;
		CSettingCollectionHelper* m_pSettingHelper = nullptr;
		std::string m_sSerializedSettingGUIXML;

		GtkWidget* m_pConfigureSettingsDialog = nullptr;
		GtkWidget* m_pSettingsVBox = nullptr;
		GtkWidget* m_pNoHelpDialog = nullptr;
		GtkWidget* m_pErrorPendingDeprecatedInterfacorsDialog = nullptr;

		// This struct is used for both settings inside the scenario and inside
		// the settings configurator
		typedef struct _SSettingCallbackData
		{
			CInterfacedScenario* interfacedScenario;
			uint32_t settingIndex;
			GtkWidget* widgetValue;
			GtkWidget* widgetEntryValue;
			GtkWidget* container;
		} SSettingCallbackData;


		std::vector<SSettingCallbackData> m_vSettingConfigurationCallbackData;
		std::vector<SSettingCallbackData> m_vSettingCallbackData;

		// Object necessary for holding scenario inputs/outputs

		std::map<std::string, OpenViBE::CIdentifier> m_mStreamType;

		// This struct is used for both inputs and outputs of the scenario

		typedef struct _SLinkCallbackData
		{
			CInterfacedScenario* m_pInterfacedScenario;
			uint32_t m_uiLinkIndex;
			bool m_bIsInput;
		} SLinkCallbackData;

		std::vector<SLinkCallbackData> m_vScenarioInputCallbackData;
		std::vector<SLinkCallbackData> m_vScenarioOutputCallbackData;

	private:
		typedef void (*menu_callback_function)(GtkMenuItem*, BoxContextMenuCB*);
		GtkImageMenuItem* gtk_menu_add_new_image_menu_item_with_cb_generic(GtkMenu* menu, const char* icon, const char* label, menu_callback_function cb, OpenViBE::Kernel::IBox* cb_box, uint32_t cb_command, uint32_t cb_index, uint32_t cb_index2);

		GtkImageMenuItem* gtk_menu_add_new_image_menu_item_with_cb(GtkMenu* menu, const char* icon, const char* label, menu_callback_function cb, OpenViBE::Kernel::IBox* cb_box, uint32_t cb_command, uint32_t cb_index)
		{
			return gtk_menu_add_new_image_menu_item_with_cb_generic(menu, icon, label, cb, cb_box, cb_command, cb_index, 0);
		}

		void redrawScenarioLinkSettings(GtkWidget* pLinkTable, bool bIsInput, std::vector<SLinkCallbackData>& vLinkCallbackData,
										uint32_t (OpenViBE::Kernel::IScenario::*pfGetLinkCount)() const,
										bool (OpenViBE::Kernel::IScenario::*pfGetLinkName)(uint32_t, OpenViBE::CString&) const,
										bool (OpenViBE::Kernel::IScenario::*pfGetLinkType)(uint32_t, OpenViBE::CIdentifier&) const);

		double m_f64CurrentScale = 1;
		gint m_ui32NormalFontSize = 0;

		std::vector<CBoxConfigurationDialog*> m_vBoxConfigurationDialog;
	};
}
