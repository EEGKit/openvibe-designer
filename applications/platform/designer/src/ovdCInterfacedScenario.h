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

namespace OpenViBE {
namespace Designer {
class CApplication;
class CDesignerVisualization;
class CPlayerVisualization;

class CInterfacedScenario
{
public:

	CInterfacedScenario(const Kernel::IKernelContext& ctx, CApplication& application, Kernel::IScenario& scenario,
						CIdentifier& scenarioID, GtkNotebook& notebook, const char* guiFilename, const char* guiSettingsFilename);
	virtual ~CInterfacedScenario();

	virtual bool isLocked() const { return m_Player != nullptr; }
	virtual void redraw();
	virtual void redraw(Kernel::IBox& box);
	virtual void redraw(Kernel::IComment& comment);
	virtual void redraw(Kernel::ILink& link);
	virtual void updateScenarioLabel();
	size_t pickInterfacedObject(int x, int y);
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

	void scenarioDrawingAreaDrawCB(cairo_t* cr);
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

	void deleteBox(const CIdentifier& boxID); // Utility method to remove box from scenario and visualization
	void contextMenuBoxUpdateCB(Kernel::IBox& box);
	void contextMenuBoxRemoveDeprecatedInterfacorsCB(Kernel::IBox& box);
	void contextMenuBoxRenameCB(Kernel::IBox& box);
	void contextMenuBoxRenameAllCB();
	void contextMenuBoxToggleEnableAllCB();
	void contextMenuBoxAddInputCB(Kernel::IBox& box);
	void contextMenuBoxEditInputCB(Kernel::IBox& box, const size_t index);
	void contextMenuBoxRemoveInputCB(Kernel::IBox& box, const size_t index);
	void contextMenuBoxAddOutputCB(Kernel::IBox& box);
	void contextMenuBoxEditOutputCB(Kernel::IBox& box, const size_t index);
	void contextMenuBoxRemoveOutputCB(Kernel::IBox& box, const size_t index);

	void contextMenuBoxConnectScenarioInputCB(Kernel::IBox& box, size_t boxInputIdx, size_t scenarioInputIdx);
	void contextMenuBoxConnectScenarioOutputCB(Kernel::IBox& box, size_t boxOutputIdx, size_t scenarioOutputIdx);
	void contextMenuBoxDisconnectScenarioInputCB(Kernel::IBox& box, size_t boxInputIdx, size_t scenarioInputIdx);
	void contextMenuBoxDisconnectScenarioOutputCB(Kernel::IBox& box, size_t boxOutputIdx, size_t scenarioOutputIdx);

	void contextMenuBoxAddSettingCB(Kernel::IBox& box);
	void contextMenuBoxEditSettingCB(Kernel::IBox& box, const size_t index);
	void contextMenuBoxRemoveSettingCB(Kernel::IBox& box, const size_t index);
	void contextMenuBoxConfigureCB(Kernel::IBox& box);
	void contextMenuBoxAboutCB(Kernel::IBox& box) const;
	void contextMenuBoxEditMetaboxCB(Kernel::IBox& box) const;

	void contextMenuBoxEnableCB(Kernel::IBox& box);
	void contextMenuBoxDisableCB(Kernel::IBox& box);
	void contextMenuBoxEnableAllCB();
	void contextMenuBoxDisableAllCB();

	void contextMenuBoxDocumentationCB(Kernel::IBox& box) const;

	void contextMenuScenarioAddCommentCB();
	void contextMenuScenarioAboutCB();

	bool browseURL(const CString& url, const CString& browserPrefix, const CString& browserPostfix) const;
	bool browseBoxDocumentation(const CIdentifier& boxID) const;

	void toggleDesignerVisualization();
	bool isDesignerVisualizationToggled() const { return m_designerVisualizationToggled; }

	void showCurrentVisualization() const;
	void hideCurrentVisualization() const;

	void createPlayerVisualization(VisualizationToolkit::IVisualizationTree* tree = nullptr);
	void releasePlayerVisualization();


	void stopAndReleasePlayer();
	bool setModifiableSettingsWidgets();
	bool hasSelection() const { return !m_SelectedObjects.empty(); }
	bool centerOnBox(const CIdentifier& identifier);
	void setScale(double scale);
	double getScale() const { return m_currentScale; }

	//-----------------------------
	//---------- typedef ----------
	//-----------------------------
	typedef struct SBoxContextMenuCB
	{
		EContextMenu command;
		size_t index;
		size_t secondaryIndex; // Used for connecting two streams
		Kernel::IBox* box;
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
	Kernel::EPlayerStatus m_PlayerStatus;
	CIdentifier m_ScenarioID = OV_UndefinedIdentifier;
	CIdentifier m_PlayerID   = OV_UndefinedIdentifier;
	CIdentifier m_TreeID     = OV_UndefinedIdentifier;

	CApplication& m_Application;
	Kernel::IScenario& m_Scenario;
	Kernel::IPlayer* m_Player                        = nullptr;
	VisualizationToolkit::IVisualizationTree* m_Tree = nullptr;
	CDesignerVisualization* m_DesignerVisualization  = nullptr;

	uint64_t m_LastLoopTime = 0;
	bool m_HasFileName      = false;
	bool m_HasBeenModified  = false;
	bool m_DebugCPUUsage    = false;

	std::string m_Filename;

	std::set<CIdentifier> m_SelectedObjects;

	std::unique_ptr<CScenarioStateStack> m_StateStack;

	// Objects necessary for holding settings GUI
	std::map<std::string, CIdentifier> m_SettingTypes;
	CSettingCollectionHelper* m_SettingHelper = nullptr;
	std::string m_SerializedSettingGUIXML;

private:

    struct GtkCairoContext {
        GtkCairoContext(GtkWidget* widget) {
            window = gtk_widget_get_window(widget);
            region = cairo_region_create();
            gdc = gdk_window_begin_draw_frame(window,region);
            cr = gdk_drawing_context_get_cairo_context(gdc);
        }

        ~GtkCairoContext() {
            gdk_window_end_draw_frame(window,gdc);
            cairo_region_destroy(region);
        }

        operator cairo_t*() { return cr; }

        GdkWindow*         window;
        cairo_region_t*    region;
        GdkDrawingContext* gdc;
        cairo_t*           cr;
    };

    struct CairoStencil {
        CairoStencil() { }

        ~CairoStencil() { destroy(); }

        operator cairo_surface_t*() { return surface; }
        operator cairo_t*()         { return cr;      }

        void reset_index() { ind = 0; }

        bool is_valid() const { return surface!=nullptr; }

        void create(const int x,const int y) {
            destroy();
            surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, x, y);
            cr = cairo_create(surface);
            cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE); // Disable antialiasing
        }

        void destroy() {
            if (is_valid()) {
                cairo_destroy(cr);
                cairo_surface_destroy(surface);
                surface = nullptr;
                cr      = nullptr;
            }
        }

        // Create a new unique index and map it to a unique color.

        size_t new_index() {
            ++ind;
            const double red   = static_cast<double>((ind&0xff0000)>>16)/255;
            const double green = static_cast<double>((ind&0x00ff00)>>8)/255;
            const double blue  = static_cast<double>(ind&0x0000ff)/255;
            cairo_set_source_rgb(cr, red, green, blue);
            return ind;
        }

        size_t get_index(const int x,const int y) {
            if (!is_valid()) 
                return invalid;

            const int maxX = cairo_image_surface_get_width(surface);
            const int maxY = cairo_image_surface_get_height(surface);

            if (x < 0 || y < 0 || x >= maxX || y >= maxY)
                return invalid;

            cairo_surface_flush(surface);
            const unsigned stride = cairo_image_surface_get_stride(surface);
            const unsigned char* data = cairo_image_surface_get_data(surface);
            if (data==nullptr)
                return invalid;

            const unsigned char* pixel = data+y*stride + x*4;
            return (pixel[0]) | (pixel[1] << 8) | (pixel[2] << 16);
        }

        template <size_t N>
        void draw_polygon(const std::array<GdkPoint, N>& points,const bool fill=true) {
            cairo_new_sub_path(cr);
            cairo_move_to(cr, points[0].x, points[0].y);
            for (unsigned i=1; i<N; ++i)
                cairo_line_to(cr, points[i].x, points[i].y);
            cairo_close_path(cr);
            if (fill)
                cairo_fill(cr);
        }

        static constexpr size_t invalid = 0xffffffff;

        size_t           ind     = 0;
        cairo_surface_t* surface = nullptr;
        cairo_t*         cr      = nullptr;
    };

	const Kernel::IKernelContext& m_kernelCtx;
	GtkNotebook& m_notebook;
	bool m_designerVisualizationToggled         = false;
	CPlayerVisualization* m_playerVisualization = nullptr;
	GtkBuilder* m_guiBuilder                    = nullptr;
	GtkWidget* m_notebookPageTitle              = nullptr;
	GtkWidget* m_notebookPageContent            = nullptr;
	GtkViewport* m_scenarioViewport             = nullptr;
	GtkDrawingArea* m_scenarioDrawingArea       = nullptr;
	CairoStencil m_stencil_cr;
	GdkPixbuf* m_mensiaLogoPixbuf               = nullptr;

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

	std::map<size_t, CInterfacedObject> m_interfacedObjects;
	CInterfacedObject m_currentObject;

	GtkWidget* m_configureSettingsDialog                 = nullptr;
	GtkWidget* m_settingsVBox                            = nullptr;
	GtkWidget* m_noHelpDialog                            = nullptr;
	GtkWidget* m_errorPendingDeprecatedInterfacorsDialog = nullptr;

	std::map<std::string, CIdentifier> m_streamTypes;

	std::map<size_t, box_ctx_menu_cb_t> m_boxCtxMenuCBs;
	std::vector<setting_cb_data_t> m_settingConfigCBDatas;
	std::vector<setting_cb_data_t> m_settingCBDatas;
	std::vector<link_cb_data_t> m_scenarioInputCBDatas;
	std::vector<link_cb_data_t> m_scenarioOutputCBDatas;

	double m_currentScale = 1;
	gint m_normalFontSize = 0;

	std::vector<CBoxConfigurationDialog*> m_boxConfigDialogs;

	typedef void (*menu_cb_function_t)(GtkMenuItem*, box_ctx_menu_cb_t*);
	GtkWidget* addNewImageMenuItemWithCBGeneric(GtkMenu* menu, const char* icon, const char* label, menu_cb_function_t cb,
											    Kernel::IBox* box, EContextMenu command, const size_t index, const size_t index2);

	GtkWidget* addNewImageMenuItemWithCB(GtkMenu* menu, const char* icon, const char* label, const menu_cb_function_t cb,
												Kernel::IBox* box, const EContextMenu command, const size_t index)
	{
		return addNewImageMenuItemWithCBGeneric(menu, icon, label, cb, box, command, index, 0);
	}

	void redrawScenarioLinkSettings(GtkWidget* links, bool isInput, std::vector<link_cb_data_t>& linkCBDatas,
									size_t (Kernel::IScenario::* getNLink)() const,
									bool (Kernel::IScenario::* getLinkName)(size_t, CString&) const,
									bool (Kernel::IScenario::* getLinkType)(size_t, CIdentifier&) const);
};
}  // namespace Designer
}  // namespace OpenViBE
