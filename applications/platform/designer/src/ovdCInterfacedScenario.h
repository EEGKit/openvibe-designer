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

		CInterfacedScenario(const OpenViBE::Kernel::IKernelContext& rKernelContext, CApplication& rApplication, OpenViBE::Kernel::IScenario& rScenario, OpenViBE::CIdentifier& scenarioID,
							GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename);
		virtual ~CInterfacedScenario();

		virtual bool isLocked() const { return m_pPlayer != nullptr; }
		virtual void redraw();
		virtual void redraw(OpenViBE::Kernel::IBox& box);
		virtual void redraw(OpenViBE::Kernel::IComment& rComment);
		virtual void redraw(OpenViBE::Kernel::ILink& rLink);
		virtual void updateScenarioLabel();
		uint32_t pickInterfacedObject(int x, int y) const;
		bool pickInterfacedObject(int x, int y, int iSizeX, int iSizeY);

		void undoCB(bool bManageModifiedStatusFlag = true);
		void redoCB(bool bManageModifiedStatusFlag = true);
		void snapshotCB(bool bManageModifiedStatusFlag = true);
		void addCommentCB(int x = -1, int y = -1);

		// Utility functions for scenario settings, inputs and outputs

		void addScenarioSettingCB();
		void editScenarioSettingCB(unsigned int settingIndex);
		void swapScenarioSettings(unsigned int settingAIndex, unsigned int settingBIndex);

		void addScenarioInputCB();
		void editScenarioInputCB(unsigned int index);
		void swapScenarioInputs(unsigned int inputAIndex, unsigned int inputBIndex);
		void addScenarioOutputCB();
		void editScenarioOutputCB(unsigned int outputIdx);
		void swapScenarioOutputs(unsigned int outputAIndex, unsigned int outputBIndex);


		// Utility functions for scenario settings, inputs and outputs
		void configureScenarioSettingsCB();

		// Drawing functions for scenario settings, inputs and outputs
		void redrawConfigureScenarioSettingsDialog();
		void redrawScenarioSettings();
		void redrawScenarioInputSettings();
		void redrawScenarioOutputSettings();

		void scenarioDrawingAreaExposeCB(GdkEventExpose* event);
		void scenarioDrawingAreaDragDataReceivedCB(GdkDragContext* pDragContext, gint iX, gint iY, GtkSelectionData* pSelectionData, guint info, guint t);
		void scenarioDrawingAreaMotionNotifyCB(GtkWidget* widget, GdkEventMotion* event);
		void scenarioDrawingAreaButtonPressedCB(GtkWidget* widget, GdkEventButton* event);
		void scenarioDrawingAreaButtonReleasedCB(GtkWidget* widget, GdkEventButton* event);
		void scenarioDrawingAreaKeyPressEventCB(GtkWidget* widget, GdkEventKey* event);
		void scenarioDrawingAreaKeyReleaseEventCB(GtkWidget* widget, GdkEventKey* event);

		void copySelection();
		void cutSelection();
		void pasteSelection();
		void deleteSelection();

		void deleteBox(const OpenViBE::CIdentifier& boxID); // Utility method to remove box from scenario and visualization
		void contextMenuBoxUpdateCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxRemoveDeprecatedInterfacorsCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxRenameCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxRenameAllCB();
		void contextMenuBoxToggleEnableAllCB();
		void contextMenuBoxAddInputCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxEditInputCB(OpenViBE::Kernel::IBox& box, const uint32_t index);
		void contextMenuBoxRemoveInputCB(OpenViBE::Kernel::IBox& box, const uint32_t index);
		void contextMenuBoxAddOutputCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxEditOutputCB(OpenViBE::Kernel::IBox& box, const uint32_t index);
		void contextMenuBoxRemoveOutputCB(OpenViBE::Kernel::IBox& box, const uint32_t index);

		void contextMenuBoxConnectScenarioInputCB(OpenViBE::Kernel::IBox& box, uint32_t boxInputIdx, uint32_t scenarioInputIdx);
		void contextMenuBoxConnectScenarioOutputCB(OpenViBE::Kernel::IBox& box, uint32_t boxOutputIdx, uint32_t scenarioOutputIdx);
		void contextMenuBoxDisconnectScenarioInputCB(OpenViBE::Kernel::IBox& box, uint32_t boxInputIdx, uint32_t scenarioInputIdx);
		void contextMenuBoxDisconnectScenarioOutputCB(OpenViBE::Kernel::IBox& box, uint32_t boxOutputIdx, uint32_t scenarioOutputIdx);

		void contextMenuBoxAddSettingCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxEditSettingCB(OpenViBE::Kernel::IBox& box, const uint32_t index);
		void contextMenuBoxRemoveSettingCB(OpenViBE::Kernel::IBox& box, const uint32_t index);
		void contextMenuBoxConfigureCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxAboutCB(OpenViBE::Kernel::IBox& box) const;
		void contextMenuBoxEditMetaboxCB(OpenViBE::Kernel::IBox& box) const;

		void contextMenuBoxEnableCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxDisableCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxEnableAllCB();
		void contextMenuBoxDisableAllCB();

		void contextMenuBoxDocumentationCB(OpenViBE::Kernel::IBox& box) const;

		void contextMenuScenarioAddCommentCB();
		void contextMenuScenarioAboutCB();

		bool browseURL(const OpenViBE::CString& url, const OpenViBE::CString& browserPrefix, const OpenViBE::CString& browserPostfix) const;
		bool browseBoxDocumentation(const OpenViBE::CIdentifier& oBoxId) const;

		void toggleDesignerVisualization();
		bool isDesignerVisualizationToggled() const { return m_designerVisualizationToggled; }

		void showCurrentVisualization() const;
		void hideCurrentVisualization() const;

		void createPlayerVisualization(OpenViBEVisualizationToolkit::IVisualizationTree* pVisualizationTree = nullptr);
		void releasePlayerVisualization();


		void stopAndReleasePlayer();
		bool setModifiableSettingsWidgets();
		bool hasSelection() const { return !m_SelectedObjects.empty(); }
		bool centerOnBox(const OpenViBE::CIdentifier& rIdentifier);
		void setScale(double scale);
		double getScale() const { return m_currentScale; }

		/*
			private:
		
				void generateDisplayPluginName(OpenViBE::Kernel::IBox* pDisplayBox, OpenViBE::CString& rDisplayBoxName);*/

		OpenViBE::Kernel::EPlayerStatus m_ePlayerStatus;
		OpenViBE::CIdentifier m_oScenarioIdentifier = OV_UndefinedIdentifier;
		OpenViBE::CIdentifier m_oPlayerIdentifier = OV_UndefinedIdentifier;
		OpenViBE::CIdentifier m_oVisualizationTreeIdentifier = OV_UndefinedIdentifier;
		CApplication& m_rApplication;
		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::Kernel::IScenario& m_rScenario;
		OpenViBE::Kernel::IPlayer* m_pPlayer = nullptr;
		uint64_t m_lastLoopTime = 0;
		GtkNotebook& m_rNotebook;
		OpenViBEVisualizationToolkit::IVisualizationTree* m_pVisualizationTree = nullptr;
		bool m_designerVisualizationToggled = false;
		CDesignerVisualization* m_pDesignerVisualization = nullptr;
		CPlayerVisualization* m_pPlayerVisualization = nullptr;
		GtkBuilder* m_pGUIBuilder = nullptr;
		GtkWidget* m_pNotebookPageTitle = nullptr;
		GtkWidget* m_pNotebookPageContent = nullptr;
		GtkViewport* m_pScenarioViewport = nullptr;
		GtkDrawingArea* m_pScenarioDrawingArea = nullptr;
		GdkPixmap* m_pStencilBuffer = nullptr;
		GdkPixbuf* m_pMensiaLogoPixbuf = nullptr;
		bool m_hasFileName = false;
		bool m_hasBeenModified = false;
		bool m_buttonPressed = false;
		bool m_shiftPressed = false;
		bool m_controlPressed = false;
		bool m_altPressed = false;
		bool m_aPressed = false;
		bool m_wPressed = false;
		bool m_debugCPUUsage = false;
		std::string m_sFileName;
		std::string m_sGUIFilename;
		std::string m_sGUISettingsFilename;
		double m_pressMouseX = 0;
		double m_pressMouseY = 0;
		double m_releaseMouseX = 0;
		double m_releaseMouseY = 0;
		double m_currentMouseX = 0;
		double m_currentMouseY = 0;
		int m_viewOffsetX = 0;
		int m_viewOffsetY = 0;
		uint32_t m_currentMode = 0;

		uint32_t m_boxCount = 0;
		uint32_t m_commentCount = 0;
		uint32_t m_linkCount = 0;

		uint32_t m_interfacedObjectId = 0;
		std::map<uint32_t, CInterfacedObject> m_vInterfacedObject;
		std::set<OpenViBE::CIdentifier> m_SelectedObjects;
		CInterfacedObject m_oCurrentObject;

		double m_panInitialPositionH = 0;
		double m_panInitialPositionV = 0;

		typedef struct _BoxContextMenuCB
		{
			uint32_t command;
			uint32_t index;
			uint32_t secondaryIndex; // Used for connecting two streams
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
		GtkImageMenuItem* gtk_menu_add_new_image_menu_item_with_cb_generic(GtkMenu* menu, const char* icon, const char* label, 
																		   menu_callback_function cb, OpenViBE::Kernel::IBox* box, 
																		   uint32_t command, const uint32_t index, const uint32_t index2);

		GtkImageMenuItem* gtk_menu_add_new_image_menu_item_with_cb(GtkMenu* menu, const char* icon, const char* label, 
																   menu_callback_function cb, OpenViBE::Kernel::IBox* box, 
																   uint32_t command, const uint32_t index) { return gtk_menu_add_new_image_menu_item_with_cb_generic(menu, icon, label, cb, box, command, index, 0); }

		void redrawScenarioLinkSettings(GtkWidget* pLinkTable, bool bIsInput, std::vector<SLinkCallbackData>& vLinkCallbackData,
										uint32_t (OpenViBE::Kernel::IScenario::*pfGetLinkCount)() const,
										bool (OpenViBE::Kernel::IScenario::*pfGetLinkName)(uint32_t, OpenViBE::CString&) const,
										bool (OpenViBE::Kernel::IScenario::*pfGetLinkType)(uint32_t, OpenViBE::CIdentifier&) const);

		double m_currentScale = 1;
		gint m_normalFontSize = 0;

		std::vector<CBoxConfigurationDialog*> m_vBoxConfigurationDialog;
	};
}
