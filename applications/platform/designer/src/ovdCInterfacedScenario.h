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

		CInterfacedScenario(const OpenViBE::Kernel::IKernelContext& ctx, CApplication& application, OpenViBE::Kernel::IScenario& scenario,
							OpenViBE::CIdentifier& scenarioID,
							GtkNotebook& notebook, const char* guiFilename, const char* guiSettingsFilename);
		virtual ~CInterfacedScenario();

		virtual bool isLocked() const { return m_Player != nullptr; }
		virtual void redraw();
		virtual void redraw(OpenViBE::Kernel::IBox& box);
		virtual void redraw(OpenViBE::Kernel::IComment& comment);
		virtual void redraw(OpenViBE::Kernel::ILink& link);
		virtual void updateScenarioLabel();
		size_t pickInterfacedObject(int x, int y) const;
		bool pickInterfacedObject(int x, int y, int sizeX, int sizeY);

		void undoCB(bool manageModifiedStatusFlag = true);
		void redoCB(bool manageModifiedStatusFlag = true);
		void snapshotCB(bool manageModifiedStatusFlag = true);
		void addCommentCB(int x = -1, int y = -1);

		// Utility functions for scenario settings, inputs and outputs

		void addScenarioSettingCB();
		//void editScenarioSettingCB(size_t index);
		void swapScenarioSettings(size_t indexA, size_t indexB);

		void addScenarioInputCB();
		void editScenarioInputCB(size_t index);
		void swapScenarioInputs(size_t indexA, size_t indexB);

		void addScenarioOutputCB();
		void editScenarioOutputCB(size_t index);
		void swapScenarioOutputs(size_t indexA, size_t indexB);


		// Utility functions for scenario settings, inputs and outputs
		void configureScenarioSettingsCB();

		// Drawing functions for scenario settings, inputs and outputs
		void redrawConfigureScenarioSettingsDialog();
		void redrawScenarioSettings();
		void redrawScenarioInputSettings();
		void redrawScenarioOutputSettings();

		void scenarioDrawingAreaExposeCB(GdkEventExpose* event);
		void scenarioDrawingAreaDragDataReceivedCB(GdkDragContext* dc, gint x, gint y, GtkSelectionData* selectionData, guint info, guint t);
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
		void contextMenuBoxEditInputCB(OpenViBE::Kernel::IBox& box, const size_t index);
		void contextMenuBoxRemoveInputCB(OpenViBE::Kernel::IBox& box, const size_t index);
		void contextMenuBoxAddOutputCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxEditOutputCB(OpenViBE::Kernel::IBox& box, const size_t index);
		void contextMenuBoxRemoveOutputCB(OpenViBE::Kernel::IBox& box, const size_t index);

		void contextMenuBoxConnectScenarioInputCB(OpenViBE::Kernel::IBox& box, size_t boxInputIdx, size_t scenarioInputIdx);
		void contextMenuBoxConnectScenarioOutputCB(OpenViBE::Kernel::IBox& box, size_t boxOutputIdx, size_t scenarioOutputIdx);
		void contextMenuBoxDisconnectScenarioInputCB(OpenViBE::Kernel::IBox& box, size_t boxInputIdx, size_t scenarioInputIdx);
		void contextMenuBoxDisconnectScenarioOutputCB(OpenViBE::Kernel::IBox& box, size_t boxOutputIdx, size_t scenarioOutputIdx);

		void contextMenuBoxAddSettingCB(OpenViBE::Kernel::IBox& box);
		void contextMenuBoxEditSettingCB(OpenViBE::Kernel::IBox& box, const size_t index);
		void contextMenuBoxRemoveSettingCB(OpenViBE::Kernel::IBox& box, const size_t index);
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
		bool browseBoxDocumentation(const OpenViBE::CIdentifier& boxID) const;

		void toggleDesignerVisualization();
		bool isDesignerVisualizationToggled() const { return m_designerVisualizationToggled; }

		void showCurrentVisualization() const;
		void hideCurrentVisualization() const;

		void createPlayerVisualization(OpenViBEVisualizationToolkit::IVisualizationTree* tree = nullptr);
		void releasePlayerVisualization();


		void stopAndReleasePlayer();
		bool setModifiableSettingsWidgets();
		bool hasSelection() const { return !m_SelectedObjects.empty(); }
		bool centerOnBox(const OpenViBE::CIdentifier& identifier);
		void setScale(double scale);
		double getScale() const { return m_currentScale; }

		//-----------------------------
		//---------- typedef ----------
		//-----------------------------
		typedef struct SBoxContextMenuCB
		{
			size_t command;
			size_t index;
			size_t secondaryIndex; // Used for connecting two streams
			OpenViBE::Kernel::IBox* box;
			CInterfacedScenario* scenario;
		} box_ctx_menu_cb_t;
		
		// This struct is used for both settings inside the scenario and inside
		// the settings configurator
		typedef struct SSettingCallbackData
		{
			CInterfacedScenario* scenario;
			size_t index;
			GtkWidget* widgetValue;
			GtkWidget* widgetEntryValue;
			GtkWidget* container;
		} setting_cb_data_t;

		// This struct is used for both inputs and outputs of the scenario
		typedef struct SLinkCallbackData
		{
			CInterfacedScenario* scenario;
			size_t index;
			bool input;
		} link_cb_data_t;

		//-------------------------------
		//---------- variables ----------
		//-------------------------------
		OpenViBE::Kernel::EPlayerStatus m_PlayerStatus;
		OpenViBE::CIdentifier m_ScenarioID = OV_UndefinedIdentifier;
		OpenViBE::CIdentifier m_PlayerID   = OV_UndefinedIdentifier;
		OpenViBE::CIdentifier m_TreeID     = OV_UndefinedIdentifier;

		CApplication& m_Application;
		OpenViBE::Kernel::IScenario& m_Scenario;
		OpenViBE::Kernel::IPlayer* m_Player                      = nullptr;
		OpenViBEVisualizationToolkit::IVisualizationTree* m_Tree = nullptr;
		CDesignerVisualization* m_DesignerVisualization          = nullptr;

		uint64_t m_LastLoopTime = 0;
		bool m_HasFileName      = false;
		bool m_HasBeenModified  = false;
		bool m_DebugCPUUsage    = false;

		std::string m_Filename;

		std::set<OpenViBE::CIdentifier> m_SelectedObjects;

		std::unique_ptr<CScenarioStateStack> m_StateStack;

		// Objects necessary for holding settings GUI
		std::map<std::string, OpenViBE::CIdentifier> m_SettingTypes;
		CSettingCollectionHelper* m_SettingHelper = nullptr;
		std::string m_SerializedSettingGUIXML;

	private:
		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		GtkNotebook& m_notebook;
		bool m_designerVisualizationToggled   = false;
		CPlayerVisualization* m_playerVisu    = nullptr;
		GtkBuilder* m_guiBuilder              = nullptr;
		GtkWidget* m_notebookPageTitle        = nullptr;
		GtkWidget* m_notebookPageContent      = nullptr;
		GtkViewport* m_scenarioViewport       = nullptr;
		GtkDrawingArea* m_scenarioDrawingArea = nullptr;
		GdkPixmap* m_stencilBuffer            = nullptr;
		GdkPixbuf* m_mensiaLogoPixbuf         = nullptr;

		bool m_buttonPressed  = false;
		bool m_shiftPressed   = false;
		bool m_controlPressed = false;
		bool m_altPressed     = false;
		bool m_aPressed       = false;
		bool m_wPressed       = false;
		std::string m_guiFilename;
		std::string m_guiSettingsFilename;

		double m_pressMouseX   = 0;
		double m_pressMouseY   = 0;
		double m_releaseMouseX = 0;
		double m_releaseMouseY = 0;
		double m_currentMouseX = 0;
		double m_currentMouseY = 0;
		int m_viewOffsetX      = 0;
		int m_viewOffsetY      = 0;
		size_t m_currentMode   = 0;

		size_t m_nBox     = 0;
		size_t m_nComment = 0;
		size_t m_nLink    = 0;

		size_t m_interfacedObjectId = 0;
		std::map<size_t, CInterfacedObject> m_interfacedObjects;
		CInterfacedObject m_currentObject;

		double m_panInitialPositionH = 0;
		double m_panInitialPositionV = 0;

		GtkWidget* m_configureSettingsDialog                 = nullptr;
		GtkWidget* m_settingsVBox                            = nullptr;
		GtkWidget* m_noHelpDialog                            = nullptr;
		GtkWidget* m_errorPendingDeprecatedInterfacorsDialog = nullptr;

		std::map<std::string, OpenViBE::CIdentifier> m_streamTypes;

		std::map<size_t, box_ctx_menu_cb_t> m_boxCtxMenuCBs;
		std::vector<setting_cb_data_t> m_settingConfigCBDatas;
		std::vector<setting_cb_data_t> m_settingCBDatas;
		std::vector<link_cb_data_t> m_scenarioInputCBDatas;
		std::vector<link_cb_data_t> m_scenarioOutputCBDatas;

		double m_currentScale = 1;
		gint m_normalFontSize = 0;

		std::vector<CBoxConfigurationDialog*> m_boxConfigDialogs;

		//void generateDisplayPluginName(OpenViBE::Kernel::IBox* pDisplayBox, OpenViBE::CString& rDisplayBoxName);

		typedef void (*menu_cb_function_t)(GtkMenuItem*, box_ctx_menu_cb_t*);
		GtkImageMenuItem* addNewImageMenuItemWithCBGeneric(GtkMenu* menu, const char* icon, const char* label, menu_cb_function_t cb,
														   OpenViBE::Kernel::IBox* box, size_t command, const size_t index, const size_t index2);

		GtkImageMenuItem* addNewImageMenuItemWithCB(GtkMenu* menu, const char* icon, const char* label, const menu_cb_function_t cb,
													OpenViBE::Kernel::IBox* box, const size_t command, const size_t index)
		{
			return addNewImageMenuItemWithCBGeneric(menu, icon, label, cb, box, command, index, 0);
		}

		void redrawScenarioLinkSettings(GtkWidget* links, bool isInput, std::vector<link_cb_data_t>& linkCBDatas,
										size_t (OpenViBE::Kernel::IScenario::* getNLink)() const,
										bool (OpenViBE::Kernel::IScenario::* getLinkName)(size_t, OpenViBE::CString&) const,
										bool (OpenViBE::Kernel::IScenario::* getLinkType)(size_t, OpenViBE::CIdentifier&) const);
	};
} // namespace OpenViBEDesigner
