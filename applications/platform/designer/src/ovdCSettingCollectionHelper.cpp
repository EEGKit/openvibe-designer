#include "ovdCSettingCollectionHelper.h"

#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;

#include <visualization-toolkit/ovvizColorGradient.h>
// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

namespace
{
	void CollectWidgetCB(GtkWidget* widget, gpointer data) { static_cast<vector<GtkWidget*>*>(data)->push_back(widget); }
	void RemoveWidgetCB(GtkWidget* widget, gpointer data) { gtk_container_remove(GTK_CONTAINER(data), widget); }

	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	void OnEntrySettingBOOLEdited(GtkEntry* pEntry, gpointer /*data*/)
	{
		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(pEntry))), CollectWidgetCB, &widgets);
		GtkToggleButton* l_widget = GTK_TOGGLE_BUTTON(widgets[1]);

		const std::string l_sEntryValue = gtk_entry_get_text(pEntry);
		if (l_sEntryValue == "true")
		{
			gtk_toggle_button_set_active(l_widget, true);
			gtk_toggle_button_set_inconsistent(l_widget, false);
		}
		else if (l_sEntryValue == "false")
		{
			gtk_toggle_button_set_active(l_widget, false);
			gtk_toggle_button_set_inconsistent(l_widget, false);
		}
		else { gtk_toggle_button_set_inconsistent(l_widget, true); }
	}

	void OnCheckbuttonSettingBOOLPressed(GtkToggleButton* button, gpointer /*data*/)
	{
		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* l_widget = GTK_ENTRY(widgets[0]);

		if (gtk_toggle_button_get_active(button)) { gtk_entry_set_text(l_widget, "true"); }
		else { gtk_entry_set_text(l_widget, "false"); }
		gtk_toggle_button_set_inconsistent(button, false);
	}

	void OnButtonSettingIntegerPressed(GtkButton* button, gpointer data, const gint iOffset)
	{
		const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_kernelCtx;

		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* widget = GTK_ENTRY(widgets[0]);

		const int64_t value = ctx.getConfigurationManager().expandAsInteger(gtk_entry_get_text(widget), 0) + iOffset;
		const char* res     = std::to_string(value).c_str();
		gtk_entry_set_text(widget, res);
	}

	void OnButtonSettingIntegerUpPressed(GtkButton* button, gpointer data) { OnButtonSettingIntegerPressed(button, data, 1); }

	void OnButtonSettingIntegerDownPressed(GtkButton* button, gpointer data) { OnButtonSettingIntegerPressed(button, data, -1); }

	void OnButtonSettingFloatPressed(GtkButton* button, gpointer data, const gdouble offset)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelCtx;

		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* l_widget = GTK_ENTRY(widgets[0]);

		char l_sValue[1024];
		double l_f64lValue = l_rKernelContext.getConfigurationManager().expandAsFloat(gtk_entry_get_text(l_widget), 0);
		l_f64lValue += offset;
		sprintf(l_sValue, "%lf", l_f64lValue);
		gtk_entry_set_text(l_widget, l_sValue);
	}

	void OnButtonSettingFloatUpPressed(GtkButton* button, gpointer data) { OnButtonSettingFloatPressed(button, data, 1); }

	void OnButtonSettingFloatDownPressed(GtkButton* button, gpointer data) { OnButtonSettingFloatPressed(button, data, -1); }

	void OnButtonSettingFilenameBrowsePressed(GtkButton* button, gpointer data)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelCtx;

		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* l_widget = GTK_ENTRY(widgets[0]);

		GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to open...", nullptr,
																  GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
																  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

		const CString initialFileName = l_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(l_widget));
		if (g_path_is_absolute(initialFileName.toASCIIString())) { gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString()); }
		else
		{
			char* fullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
			g_free(fullPath);
		}

		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

		if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
		{
			char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
			char* l_pBackslash;
			while ((l_pBackslash = strchr(fileName, '\\')) != nullptr) { *l_pBackslash = '/'; }
			gtk_entry_set_text(l_widget, fileName);
			g_free(fileName);
		}
		gtk_widget_destroy(widgetDialogOpen);
	}

	void OnButtonSettingFoldernameBrowsePressed(GtkButton* button, gpointer data)
	{
		const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_kernelCtx;

		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* widget = GTK_ENTRY(widgets[0]);

		GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select folder to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
																  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

		const CString initialFileName = ctx.getConfigurationManager().expand(gtk_entry_get_text(widget));
		if (g_path_is_absolute(initialFileName.toASCIIString())) { gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString()); }
		else
		{
			char* fullPath = g_build_filename(g_get_current_dir(), initialFileName.toASCIIString(), nullptr);
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
			g_free(fullPath);
		}

		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

		if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
		{
			char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
			char* backslash;
			while ((backslash = strchr(fileName, '\\')) != nullptr) { *backslash = '/'; }
			gtk_entry_set_text(widget, fileName);
			g_free(fileName);
		}
		gtk_widget_destroy(widgetDialogOpen);
	}
	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	void OnButtonSettingScriptEditPressed(GtkButton* button, gpointer data)
	{
		const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_kernelCtx;

		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* widget = GTK_ENTRY(widgets[0]);

		const CString fileName      = ctx.getConfigurationManager().expand(gtk_entry_get_text(widget));
		const CString editorCommand = ctx.getConfigurationManager().expand("${Designer_ScriptEditorCommand}");

		if (editorCommand != CString(""))
		{
			CString fullCommand = editorCommand + CString(" \"") + fileName + CString("\"");
#if defined TARGET_OS_Windows
			fullCommand = "START " + fullCommand;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			fullCommand = fullCommand + " &";
#else
#endif
			if (system(fullCommand.toASCIIString()) < 0) { ctx.getLogManager() << LogLevel_Warning << "Could not run command " << fullCommand << "\n"; }
		}
	}

	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	void OnButtonSettingColorChoosePressed(GtkColorButton* button, gpointer /*data*/)
	{
		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* widget = GTK_ENTRY(widgets[0]);
		GdkColor color;
		gtk_color_button_get_color(button, &color);

		const char* buffer = (std::to_string((color.red * 100) / 65535) + "," + std::to_string((color.green * 100) / 65535) + "," + std::to_string((color.blue * 100) / 65535)).c_str();
		gtk_entry_set_text(widget, buffer);
	}

	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	typedef struct
	{
		double fPercent;
		GdkColor oColor;
		GtkColorButton* pColorButton;
		GtkSpinButton* pSpinButton;
	} SColorGradientDataNode;

	typedef struct
	{
		string sGUIFilename;
		GtkWidget* pDialog;
		GtkWidget* pContainer;
		GtkWidget* pDrawingArea;
		vector<SColorGradientDataNode> vColorGradient;
		map<GtkColorButton*, uint32_t> vColorButtonMap;
		map<GtkSpinButton*, uint32_t> vSpinButtonMap;
	} SColorGradientData;

	void OnGTKWidgetDestroy(GtkWidget* widget, gpointer /*data*/) { gtk_widget_destroy(widget); }

	void OnInitializeColorGradient(GtkWidget* widget, gpointer data);

	void OnRefreshColorGradient(GtkWidget* /*widget*/, GdkEventExpose* /*pEvent*/, gpointer data)
	{
		auto* userData = static_cast<SColorGradientData*>(data);

		uint32_t i;
		const uint32_t ui32Steps = 100;
		gint sizex               = 0;
		gint sizey               = 0;
		gdk_drawable_get_size(userData->pDrawingArea->window, &sizex, &sizey);

		CMatrix l_oGradientMatrix;
		l_oGradientMatrix.setDimensionCount(2);
		l_oGradientMatrix.setDimensionSize(0, 4);
		l_oGradientMatrix.setDimensionSize(1, userData->vColorGradient.size());
		for (i = 0; i < userData->vColorGradient.size(); ++i)
		{
			l_oGradientMatrix[i * 4]     = userData->vColorGradient[i].fPercent;
			l_oGradientMatrix[i * 4 + 1] = userData->vColorGradient[i].oColor.red * 100. / 65535.;
			l_oGradientMatrix[i * 4 + 2] = userData->vColorGradient[i].oColor.green * 100. / 65535.;
			l_oGradientMatrix[i * 4 + 3] = userData->vColorGradient[i].oColor.blue * 100. / 65535.;
		}

		CMatrix l_oInterpolatedMatrix;
		OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(l_oInterpolatedMatrix, l_oGradientMatrix, ui32Steps);

		GdkGC* l_pGC = gdk_gc_new(userData->pDrawingArea->window);
		GdkColor color;

		for (i = 0; i < ui32Steps; ++i)
		{
			color.red   = guint(l_oInterpolatedMatrix[i * 4 + 1] * 65535 * .01);
			color.green = guint(l_oInterpolatedMatrix[i * 4 + 2] * 65535 * .01);
			color.blue  = guint(l_oInterpolatedMatrix[i * 4 + 3] * 65535 * .01);
			gdk_gc_set_rgb_fg_color(l_pGC, &color);
			gdk_draw_rectangle(userData->pDrawingArea->window, l_pGC, TRUE, (sizex * i) / ui32Steps, 0, (sizex * (i + 1)) / ui32Steps, sizey);
		}
		g_object_unref(l_pGC);
	}

	void OnColorGradientSpinButtonValueChanged(GtkSpinButton* button, gpointer data)
	{
		auto* userData = static_cast<SColorGradientData*>(data);

		gtk_spin_button_update(button);

		const uint32_t i                 = userData->vSpinButtonMap[button];
		GtkSpinButton* l_pPrevSpinButton = (i > 0 ? userData->vColorGradient[i - 1].pSpinButton : nullptr);
		GtkSpinButton* l_pNextSpinButton = (i < userData->vColorGradient.size() - 1 ? userData->vColorGradient[i + 1].pSpinButton : nullptr);
		if (!l_pPrevSpinButton) { gtk_spin_button_set_value(button, 0); }
		if (!l_pNextSpinButton) { gtk_spin_button_set_value(button, 100); }
		if (l_pPrevSpinButton && gtk_spin_button_get_value(button) < gtk_spin_button_get_value(l_pPrevSpinButton)) { gtk_spin_button_set_value(button, gtk_spin_button_get_value(l_pPrevSpinButton)); }
		if (l_pNextSpinButton && gtk_spin_button_get_value(button) > gtk_spin_button_get_value(l_pNextSpinButton)) { gtk_spin_button_set_value(button, gtk_spin_button_get_value(l_pNextSpinButton)); }

		userData->vColorGradient[i].fPercent = gtk_spin_button_get_value(button);

		OnRefreshColorGradient(nullptr, nullptr, data);
	}

	void OnColorGradientColorButtonPressed(GtkColorButton* button, gpointer data)
	{
		auto* userData = static_cast<SColorGradientData*>(data);

		GdkColor color;
		gtk_color_button_get_color(button, &color);

		userData->vColorGradient[userData->vColorButtonMap[button]].oColor = color;

		OnRefreshColorGradient(nullptr, nullptr, data);
	}

	void OnInitializeColorGradient(GtkWidget* /*widget*/, gpointer data)
	{
		auto* userData = static_cast<SColorGradientData*>(data);

		gtk_widget_hide(userData->pContainer);

		gtk_container_foreach(GTK_CONTAINER(userData->pContainer), OnGTKWidgetDestroy, nullptr);

		uint32_t i         = 0;
		const size_t count = userData->vColorGradient.size();
		userData->vColorButtonMap.clear();
		userData->vSpinButtonMap.clear();
		for (auto it = userData->vColorGradient.begin(); it != userData->vColorGradient.end(); ++it, ++i)
		{
			GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(userData->sGUIFilename.c_str(), "setting_editor-color_gradient-hbox", nullptr);
			gtk_builder_add_from_file(builder, userData->sGUIFilename.c_str(), nullptr);
			gtk_builder_connect_signals(builder, nullptr);

			GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-hbox"));

			it->pColorButton = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "setting_editor-color_gradient-colorbutton"));
			it->pSpinButton  = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "setting_editor-color_gradient-spinbutton"));

			gtk_color_button_set_color(it->pColorButton, &it->oColor);
			gtk_spin_button_set_value(it->pSpinButton, it->fPercent);

			g_signal_connect(G_OBJECT(it->pColorButton), "color-set", G_CALLBACK(OnColorGradientColorButtonPressed), userData);
			g_signal_connect(G_OBJECT(it->pSpinButton), "value-changed", G_CALLBACK(OnColorGradientSpinButtonValueChanged), userData);

			g_object_ref(widget);
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widget)), widget);
			gtk_container_add(GTK_CONTAINER(userData->pContainer), widget);
			g_object_unref(widget);

			g_object_unref(builder);

			userData->vColorButtonMap[it->pColorButton] = i;
			userData->vSpinButtonMap[it->pSpinButton]   = i;
		}

		gtk_spin_button_set_value(userData->vColorGradient[0].pSpinButton, 0);
		gtk_spin_button_set_value(userData->vColorGradient[count - 1].pSpinButton, 100);

		gtk_widget_show(userData->pContainer);
	}

	void OnButtonColorGradientAddPressed(GtkButton* /*button*/, gpointer data)
	{
		auto* userData = static_cast<SColorGradientData*>(data);
		userData->vColorGradient.resize(userData->vColorGradient.size() + 1);
		userData->vColorGradient[userData->vColorGradient.size() - 1].fPercent = 100;
		OnInitializeColorGradient(nullptr, data);
		OnRefreshColorGradient(nullptr, nullptr, data);
	}

	void OnButtonColorGradientRemovePressed(GtkButton* /*button*/, gpointer data)
	{
		auto* userData = static_cast<SColorGradientData*>(data);
		if (userData->vColorGradient.size() > 2)
		{
			userData->vColorGradient.resize(userData->vColorGradient.size() - 1);
			userData->vColorGradient[userData->vColorGradient.size() - 1].fPercent = 100;
			OnInitializeColorGradient(nullptr, data);
			OnRefreshColorGradient(nullptr, nullptr, data);
		}
	}

	void OnButtonSettingColorGradientConfigurePressed(GtkButton* button, gpointer data)
	{
		SColorGradientData userData;

		userData.sGUIFilename = static_cast<CSettingCollectionHelper*>(data)->m_sGUIFilename.toASCIIString();

		vector<GtkWidget*> widgets;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
		GtkEntry* widget = GTK_ENTRY(widgets[0]);

		GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(l_oUserData.sGUIFilename.c_str(), "setting_editor-color_gradient-dialog", nullptr);
		gtk_builder_add_from_file(builder, userData.sGUIFilename.c_str(), nullptr);
		gtk_builder_connect_signals(builder, nullptr);

		userData.pDialog = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-dialog"));

		const CString sInitialGradient = static_cast<CSettingCollectionHelper*>(data)->m_kernelCtx.getConfigurationManager().expand(gtk_entry_get_text(widget));
		CMatrix initialGradient;

		OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(initialGradient, sInitialGradient);

		userData.vColorGradient.resize(initialGradient.getDimensionSize(1) > 2 ? initialGradient.getDimensionSize(1) : 2);
		for (uint32_t i = 0; i < initialGradient.getDimensionSize(1); ++i)
		{
			userData.vColorGradient[i].fPercent     = initialGradient[i * 4];
			userData.vColorGradient[i].oColor.red   = guint(initialGradient[i * 4 + 1] * .01 * 65535.);
			userData.vColorGradient[i].oColor.green = guint(initialGradient[i * 4 + 2] * .01 * 65535.);
			userData.vColorGradient[i].oColor.blue  = guint(initialGradient[i * 4 + 3] * .01 * 65535.);
		}

		userData.pContainer   = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-vbox"));
		userData.pDrawingArea = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-drawingarea"));

		g_signal_connect(G_OBJECT(userData.pDialog), "show", G_CALLBACK(OnInitializeColorGradient), &userData);
		g_signal_connect(G_OBJECT(userData.pDrawingArea), "expose_event", G_CALLBACK(OnRefreshColorGradient), &userData);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "setting_editor-color_gradient-add_button")), "pressed",
						 G_CALLBACK(OnButtonColorGradientAddPressed), &userData);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "setting_editor-color_gradient-remove_button")), "pressed",
						 G_CALLBACK(OnButtonColorGradientRemovePressed), &userData);

		if (gtk_dialog_run(GTK_DIALOG(userData.pDialog)) == GTK_RESPONSE_APPLY)
		{
			CString sFinalGradient;
			CMatrix finalGradient;
			finalGradient.setDimensionCount(2);
			finalGradient.setDimensionSize(0, 4);
			finalGradient.setDimensionSize(1, userData.vColorGradient.size());
			for (uint32_t i = 0; i < userData.vColorGradient.size(); ++i)
			{
				finalGradient[i * 4]     = userData.vColorGradient[i].fPercent;
				finalGradient[i * 4 + 1] = userData.vColorGradient[i].oColor.red * 100. / 65535.;
				finalGradient[i * 4 + 2] = userData.vColorGradient[i].oColor.green * 100. / 65535.;
				finalGradient[i * 4 + 3] = userData.vColorGradient[i].oColor.blue * 100. / 65535.;
			}
			OpenViBEVisualizationToolkit::Tools::ColorGradient::format(sFinalGradient, finalGradient);
			gtk_entry_set_text(widget, sFinalGradient.toASCIIString());
		}

		gtk_widget_destroy(userData.pDialog);
		g_object_unref(builder);
	}
} // namespace

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

CSettingCollectionHelper::CSettingCollectionHelper(const IKernelContext& ctx, const char* guiFilename)
	: m_kernelCtx(ctx), m_sGUIFilename(guiFilename) { }

CSettingCollectionHelper::~CSettingCollectionHelper() = default;

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

CString CSettingCollectionHelper::getSettingWidgetName(const CIdentifier& typeID) const
{
	if (typeID == OV_TypeId_Boolean) { return "settings_collection-hbox_setting_bool"; }
	if (typeID == OV_TypeId_Integer) { return "settings_collection-hbox_setting_integer"; }
	if (typeID == OV_TypeId_Float) { return "settings_collection-hbox_setting_float"; }
	if (typeID == OV_TypeId_String) { return "settings_collection-entry_setting_string"; }
	if (typeID == OV_TypeId_Filename) { return "settings_collection-hbox_setting_filename"; }
	if (typeID == OV_TypeId_Foldername) { return "settings_collection-hbox_setting_foldername"; }
	if (typeID == OV_TypeId_Script) { return "settings_collection-hbox_setting_script"; }
	if (typeID == OV_TypeId_Color) { return "settings_collection-hbox_setting_color"; }
	if (typeID == OV_TypeId_ColorGradient) { return "settings_collection-hbox_setting_color_gradient"; }
	if (m_kernelCtx.getTypeManager().isEnumeration(typeID)) { return "settings_collection-comboboxentry_setting_enumeration"; }
	if (m_kernelCtx.getTypeManager().isBitMask(typeID)) { return "settings_collection-table_setting_bitmask"; }
	return "settings_collection-entry_setting_string";
}

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

CString CSettingCollectionHelper::getSettingEntryWidgetName(const CIdentifier& typeID) const
{
	if (typeID == OV_TypeId_Boolean) { return "settings_collection-entry_setting_bool"; }
	if (typeID == OV_TypeId_Integer) { return "settings_collection-entry_setting_integer_string"; }
	if (typeID == OV_TypeId_Float) { return "settings_collection-entry_setting_float_string"; }
	if (typeID == OV_TypeId_String) { return "settings_collection-entry_setting_string"; }
	if (typeID == OV_TypeId_Filename) { return "settings_collection-entry_setting_filename_string"; }
	if (typeID == OV_TypeId_Foldername) { return "settings_collection-entry_setting_foldername_string"; }
	if (typeID == OV_TypeId_Script) { return "settings_collection-entry_setting_script_string"; }
	if (typeID == OV_TypeId_Color) { return "settings_collection-hbox_setting_color_string"; }
	if (typeID == OV_TypeId_ColorGradient) { return "settings_collection-hbox_setting_color_gradient_string"; }
	if (m_kernelCtx.getTypeManager().isEnumeration(typeID)) { return "settings_collection-comboboxentry_setting_enumeration"; }
	if (m_kernelCtx.getTypeManager().isBitMask(typeID)) { return "settings_collection-table_setting_bitmask"; }
	return "settings_collection-entry_setting_string";
}

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

CString CSettingCollectionHelper::getValue(const CIdentifier& typeID, GtkWidget* widget) const
{
	if (!widget) { return ""; }
	if (typeID == OV_TypeId_Boolean) { return getValueBoolean(widget); }
	if (typeID == OV_TypeId_Integer) { return getValueInteger(widget); }
	if (typeID == OV_TypeId_Float) { return getValueFloat(widget); }
	if (typeID == OV_TypeId_String) { return getValueString(widget); }
	if (typeID == OV_TypeId_Filename) { return getValueFilename(widget); }
	if (typeID == OV_TypeId_Foldername) { return getValueFoldername(widget); }
	if (typeID == OV_TypeId_Script) { return getValueScript(widget); }
	if (typeID == OV_TypeId_Color) { return getValueColor(widget); }
	if (typeID == OV_TypeId_ColorGradient) { return getValueColorGradient(widget); }
	if (m_kernelCtx.getTypeManager().isEnumeration(typeID)) { return getValueEnumeration(typeID, widget); }
	if (m_kernelCtx.getTypeManager().isBitMask(typeID)) { return getValueBitMask(typeID, widget); }
	return getValueString(widget);
}

CString CSettingCollectionHelper::getValueBoolean(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return "false"; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[1])) { return "false"; }
	GtkEntry* l_widget = GTK_ENTRY(widgets[1]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueInteger(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return "0"; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return "O"; }
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueFloat(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return "0"; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return "O"; }
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueString(GtkWidget* widget)
{
	if (!GTK_IS_ENTRY(widget)) { return ""; }
	GtkEntry* entry = GTK_ENTRY(widget);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueFilename(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return ""; }
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueFoldername(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return ""; }
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueScript(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return ""; }
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueColor(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return ""; }
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
}

CString CSettingCollectionHelper::getValueColorGradient(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[0])) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueEnumeration(const CIdentifier& /*typeID*/, GtkWidget* widget)
{
	if (!GTK_IS_COMBO_BOX(widget)) { return ""; }
	GtkComboBox* comboBox = GTK_COMBO_BOX(widget);
	return CString(gtk_combo_box_get_active_text(comboBox));
}

CString CSettingCollectionHelper::getValueBitMask(const CIdentifier& /*typeID*/, GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	string res;

	for (auto& window : widgets)
	{
		if (!GTK_IS_TOGGLE_BUTTON(window)) { return ""; }
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window)))
		{
			if (!res.empty()) { res += string(1, OV_Value_EnumeratedStringSeparator); }
			res += gtk_button_get_label(GTK_BUTTON(window));
		}
	}
	return CString(res.c_str());
}

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

void CSettingCollectionHelper::setValue(const CIdentifier& typeID, GtkWidget* widget, const CString& value)
{
	if (!widget) { return; }
	if (typeID == OV_TypeId_Boolean) { return setValueBoolean(widget, value); }
	if (typeID == OV_TypeId_Integer) { return setValueInteger(widget, value); }
	if (typeID == OV_TypeId_Float) { return setValueFloat(widget, value); }
	if (typeID == OV_TypeId_String) { return setValueString(widget, value); }
	if (typeID == OV_TypeId_Filename) { return setValueFilename(widget, value); }
	if (typeID == OV_TypeId_Foldername) { return setValueFoldername(widget, value); }
	if (typeID == OV_TypeId_Script) { return setValueScript(widget, value); }
	if (typeID == OV_TypeId_Color) { return setValueColor(widget, value); }
	if (typeID == OV_TypeId_ColorGradient) { return setValueColorGradient(widget, value); }
	if (m_kernelCtx.getTypeManager().isEnumeration(typeID)) { return setValueEnumeration(typeID, widget, value); }
	if (m_kernelCtx.getTypeManager().isBitMask(typeID)) { return setValueBitMask(typeID, widget, value); }
	return setValueString(widget, value);
}

void CSettingCollectionHelper::setValueBoolean(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry               = GTK_ENTRY(widgets[0]);
	GtkToggleButton* toggleButton = GTK_TOGGLE_BUTTON(widgets[1]);

	if (value == CString("true")) { gtk_toggle_button_set_active(toggleButton, true); }
	else if (value == CString("false")) { gtk_toggle_button_set_active(toggleButton, false); }
	else { gtk_toggle_button_set_inconsistent(toggleButton, true); }

	gtk_entry_set_text(entry, value);

	g_signal_connect(G_OBJECT(toggleButton), "toggled", G_CALLBACK(OnCheckbuttonSettingBOOLPressed), this);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(OnEntrySettingBOOLEdited), this);
}

void CSettingCollectionHelper::setValueInteger(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* l_widget = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingIntegerUpPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingIntegerDownPressed), this);

	gtk_entry_set_text(l_widget, value);
}

void CSettingCollectionHelper::setValueFloat(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingFloatUpPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingFloatDownPressed), this);

	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueString(GtkWidget* widget, const CString& value)
{
	GtkEntry* entry = GTK_ENTRY(widget);
	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueFilename(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingFilenameBrowsePressed), this);

	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueFoldername(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingFoldernameBrowsePressed), this);

	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueScript(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingScriptEditPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingFilenameBrowsePressed), this);

	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueColor(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "color-set", G_CALLBACK(OnButtonSettingColorChoosePressed), this);

	int r = 0, g = 0, b = 0;
	sscanf(m_kernelCtx.getConfigurationManager().expand(value).toASCIIString(), "%i,%i,%i", &r, &g, &b);

	GdkColor color;
	color.red   = (r * 65535) / 100;
	color.green = (g * 65535) / 100;
	color.blue  = (b * 65535) / 100;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widgets[1]), &color);

	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueColorGradient(GtkWidget* widget, const CString& value)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingColorGradientConfigurePressed), this);

	gtk_entry_set_text(entry, value);
}

void CSettingCollectionHelper::setValueEnumeration(const CIdentifier& typeID, GtkWidget* widget, const CString& value) const
{
	GtkTreeIter listIter;
	GtkComboBox* comboBox = GTK_COMBO_BOX(widget);
	GtkListStore* list    = GTK_LIST_STORE(gtk_combo_box_get_model(comboBox));
	const uint64_t v      = m_kernelCtx.getTypeManager().getEnumerationEntryValueFromName(typeID, value);
	uint64_t i;

#if 0
	if (typeID == OV_TypeId_Stimulation)
	{
#endif

	std::map<CString, uint64_t> listEntries;
	for (i = 0; i < m_kernelCtx.getTypeManager().getEnumerationEntryCount(typeID); ++i)
	{
		CString entryName;
		uint64_t entryValue;
		if (m_kernelCtx.getTypeManager().getEnumerationEntry(typeID, i, entryName, entryValue)) { listEntries[entryName] = entryValue; }
	}

	gtk_combo_box_set_wrap_width(comboBox, 0);
	gtk_list_store_clear(list);
	i = 0;
	for (auto it = listEntries.begin(); it != listEntries.end(); ++it, i++)
	{
		gtk_list_store_append(list, &listIter);
		gtk_list_store_set(list, &listIter, 0, it->first.toASCIIString(), -1);

		if (v == it->second) { gtk_combo_box_set_active(comboBox, gint(i)); }
	}
#if 0
	}
	else
	{
		gtk_list_store_clear(list);
		for (i = 0; i < m_kernelCtx.getTypeManager().getEnumerationEntryCount(typeID); ++i)
		{
			CString l_sEntryName;
			uint64_t l_ui64EntryValue;
			if (m_kernelCtx.getTypeManager().getEnumerationEntry(typeID, i, l_sEntryName, l_ui64EntryValue))
			{
				gtk_list_store_append(list, &listIter);
				gtk_list_store_set(list, &listIter, 0, l_sEntryName.toASCIIString(), -1);

				if (l_ui64Value == l_ui64EntryValue) { gtk_combo_box_set_active(l_widget, gint(i)); }
			}
		}
	}
#endif
	if (gtk_combo_box_get_active(comboBox) == -1)
	{
		gtk_list_store_append(list, &listIter);
		gtk_list_store_set(list, &listIter, 0, value.toASCIIString(), -1);
		gtk_combo_box_set_active(comboBox, gint(i)); // $$$ i should be ok :)
	}
}

void CSettingCollectionHelper::setValueBitMask(const CIdentifier& typeID, GtkWidget* widget, const CString& value) const
{
	gtk_container_foreach(GTK_CONTAINER(widget), RemoveWidgetCB, widget);

	const string sValue(value);

	const gint size        = guint((m_kernelCtx.getTypeManager().getBitMaskEntryCount(typeID) + 1) >> 1);
	GtkTable* bitMaskTable = GTK_TABLE(widget);
	gtk_table_resize(bitMaskTable, 2, size);

	for (uint64_t i = 0; i < m_kernelCtx.getTypeManager().getBitMaskEntryCount(typeID); ++i)
	{
		CString entryName;
		uint64_t entryValue;
		if (m_kernelCtx.getTypeManager().getBitMaskEntry(typeID, i, entryName, entryValue))
		{
			GtkWidget* settingButton = gtk_check_button_new();
			gtk_table_attach_defaults(bitMaskTable, settingButton, guint(i & 1), guint((i & 1) + 1), guint(i >> 1), guint((i >> 1) + 1));
			gtk_button_set_label(GTK_BUTTON(settingButton), static_cast<const char*>(entryName));

			if (sValue.find(static_cast<const char*>(entryName)) != string::npos) { gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(settingButton), true); }
		}
	}

	/*
	 * TODO - Add an entry text somewhere to manage
	 * configuration through configuration manager !
	 */

	gtk_widget_show_all(GTK_WIDGET(bitMaskTable));
}
