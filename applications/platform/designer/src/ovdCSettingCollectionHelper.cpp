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

#include <iostream>
#include <visualization-toolkit/ovvizColorGradient.h>
// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

namespace
{
	void collect_widget_cb(GtkWidget* widget, gpointer data)
	{
		static_cast<vector<GtkWidget*>*>(data)->push_back(widget);
	}

	void remove_widget_cb(GtkWidget* widget, gpointer data)
	{
		gtk_container_remove(GTK_CONTAINER(data), widget);
	}

	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	void on_entry_setting_bool_edited(GtkEntry* pEntry, gpointer /*data*/)
	{
		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(pEntry))), collect_widget_cb, &l_vWidget);
		GtkToggleButton* l_widget = GTK_TOGGLE_BUTTON(l_vWidget[1]);

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

	void on_checkbutton_setting_bool_pressed(GtkToggleButton* button, gpointer /*data*/)
	{
		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		if (gtk_toggle_button_get_active(button)) { gtk_entry_set_text(l_widget, "true"); }
		else { gtk_entry_set_text(l_widget, "false"); }
		gtk_toggle_button_set_inconsistent(button, false);
	}

	void on_button_setting_integer_pressed(GtkButton* button, gpointer data, const gint iOffset)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelContext;

		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		char l_sValue[1024];
		int64_t l_i64lValue = l_rKernelContext.getConfigurationManager().expandAsInteger(gtk_entry_get_text(l_widget), 0);
		l_i64lValue += iOffset;
		sprintf(l_sValue, "%lli", l_i64lValue);
		gtk_entry_set_text(l_widget, l_sValue);
	}

	void on_button_setting_integer_up_pressed(GtkButton* button, gpointer data) { on_button_setting_integer_pressed(button, data, 1); }

	void on_button_setting_integer_down_pressed(GtkButton* button, gpointer data) { on_button_setting_integer_pressed(button, data, -1); }

	void on_button_setting_float_pressed(GtkButton* button, gpointer data, const gdouble offset)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelContext;

		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		char l_sValue[1024];
		double l_f64lValue = l_rKernelContext.getConfigurationManager().expandAsFloat(gtk_entry_get_text(l_widget), 0);
		l_f64lValue += offset;
		sprintf(l_sValue, "%lf", l_f64lValue);
		gtk_entry_set_text(l_widget, l_sValue);
	}

	void on_button_setting_float_up_pressed(GtkButton* button, gpointer data) { on_button_setting_float_pressed(button, data, 1); }

	void on_button_setting_float_down_pressed(GtkButton* button, gpointer data) { on_button_setting_float_pressed(button, data, -1); }

	void on_button_setting_filename_browse_pressed(GtkButton* button, gpointer data)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelContext;

		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to open...", nullptr,
																  GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
																  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

		const CString initialFileName = l_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(l_widget));
		if (g_path_is_absolute(initialFileName.toASCIIString()))
		{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
		}
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
			while ((l_pBackslash = strchr(fileName, '\\')) != nullptr)
			{
				*l_pBackslash = '/';
			}
			gtk_entry_set_text(l_widget, fileName);
			g_free(fileName);
		}
		gtk_widget_destroy(widgetDialogOpen);
	}

	void on_button_setting_foldername_browse_pressed(GtkButton* button, gpointer data)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelContext;

		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select folder to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
																  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

		const CString initialFileName = l_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(l_widget));
		if (g_path_is_absolute(initialFileName.toASCIIString()))
		{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), initialFileName.toASCIIString());
		}
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
			while ((l_pBackslash = strchr(fileName, '\\')) != nullptr)
			{
				*l_pBackslash = '/';
			}
			gtk_entry_set_text(l_widget, fileName);
			g_free(fileName);
		}
		gtk_widget_destroy(widgetDialogOpen);
	}
	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	void on_button_setting_script_edit_pressed(GtkButton* button, gpointer data)
	{
		const IKernelContext& l_rKernelContext = static_cast<CSettingCollectionHelper*>(data)->m_kernelContext;

		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		const CString fileName         = l_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(l_widget));
		const CString l_sEditorCommand = l_rKernelContext.getConfigurationManager().expand("${Designer_ScriptEditorCommand}");

		if (l_sEditorCommand != CString(""))
		{
			CString fullCommand = l_sEditorCommand + CString(" \"") + fileName + CString("\"");
#if defined TARGET_OS_Windows
			fullCommand = "START " + fullCommand;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			fullCommand = fullCommand + " &";
#else
#endif
			if (system(fullCommand.toASCIIString()) < 0)
			{
				l_rKernelContext.getLogManager() << LogLevel_Warning << "Could not run command " << fullCommand << "\n";
			}
		}
	}

	// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

	void on_button_setting_color_choose_pressed(GtkColorButton* button, gpointer /*data*/)
	{
		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
		GdkColor l_oColor;
		gtk_color_button_get_color(button, &l_oColor);

		char l_sBuffer[1024];
		sprintf(l_sBuffer, "%i,%i,%i", (l_oColor.red * 100) / 65535, (l_oColor.green * 100) / 65535, (l_oColor.blue * 100) / 65535);

		gtk_entry_set_text(l_widget, l_sBuffer);
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

	void on_gtk_widget_destroy_cb(GtkWidget* widget, gpointer /*data*/) { gtk_widget_destroy(widget); }

	void on_initialize_color_gradient(GtkWidget* widget, gpointer data);

	void on_refresh_color_gradient(GtkWidget* /*widget*/, GdkEventExpose* /*pEvent*/, gpointer data)
	{
		auto* l_pUserData = static_cast<SColorGradientData*>(data);

		uint32_t i;
		const uint32_t ui32Steps = 100;
		gint sizex               = 0;
		gint sizey               = 0;
		gdk_drawable_get_size(l_pUserData->pDrawingArea->window, &sizex, &sizey);

		CMatrix l_oGradientMatrix;
		l_oGradientMatrix.setDimensionCount(2);
		l_oGradientMatrix.setDimensionSize(0, 4);
		l_oGradientMatrix.setDimensionSize(1, uint32_t(l_pUserData->vColorGradient.size()));
		for (i = 0; i < l_pUserData->vColorGradient.size(); ++i)
		{
			l_oGradientMatrix[i * 4]     = l_pUserData->vColorGradient[i].fPercent;
			l_oGradientMatrix[i * 4 + 1] = l_pUserData->vColorGradient[i].oColor.red * 100. / 65535.;
			l_oGradientMatrix[i * 4 + 2] = l_pUserData->vColorGradient[i].oColor.green * 100. / 65535.;
			l_oGradientMatrix[i * 4 + 3] = l_pUserData->vColorGradient[i].oColor.blue * 100. / 65535.;
		}

		CMatrix l_oInterpolatedMatrix;
		OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(l_oInterpolatedMatrix, l_oGradientMatrix, ui32Steps);

		GdkGC* l_pGC = gdk_gc_new(l_pUserData->pDrawingArea->window);
		GdkColor l_oColor;

		for (i = 0; i < ui32Steps; ++i)
		{
			l_oColor.red   = guint(l_oInterpolatedMatrix[i * 4 + 1] * 65535 * .01);
			l_oColor.green = guint(l_oInterpolatedMatrix[i * 4 + 2] * 65535 * .01);
			l_oColor.blue  = guint(l_oInterpolatedMatrix[i * 4 + 3] * 65535 * .01);
			gdk_gc_set_rgb_fg_color(l_pGC, &l_oColor);
			gdk_draw_rectangle(l_pUserData->pDrawingArea->window, l_pGC, TRUE, (sizex * i) / ui32Steps, 0, (sizex * (i + 1)) / ui32Steps, sizey);
		}
		g_object_unref(l_pGC);
	}

	void on_color_gradient_spin_button_value_changed(GtkSpinButton* button, gpointer data)
	{
		auto* l_pUserData = static_cast<SColorGradientData*>(data);

		gtk_spin_button_update(button);

		const uint32_t i                 = l_pUserData->vSpinButtonMap[button];
		GtkSpinButton* l_pPrevSpinButton = (i > 0 ? l_pUserData->vColorGradient[i - 1].pSpinButton : nullptr);
		GtkSpinButton* l_pNextSpinButton = (i < l_pUserData->vColorGradient.size() - 1 ? l_pUserData->vColorGradient[i + 1].pSpinButton : nullptr);
		if (!l_pPrevSpinButton) { gtk_spin_button_set_value(button, 0); }
		if (!l_pNextSpinButton) { gtk_spin_button_set_value(button, 100); }
		if (l_pPrevSpinButton && gtk_spin_button_get_value(button) < gtk_spin_button_get_value(l_pPrevSpinButton))
		{
			gtk_spin_button_set_value(button, gtk_spin_button_get_value(l_pPrevSpinButton));
		}
		if (l_pNextSpinButton && gtk_spin_button_get_value(button) > gtk_spin_button_get_value(l_pNextSpinButton))
		{
			gtk_spin_button_set_value(button, gtk_spin_button_get_value(l_pNextSpinButton));
		}

		l_pUserData->vColorGradient[i].fPercent = gtk_spin_button_get_value(button);

		on_refresh_color_gradient(nullptr, nullptr, data);
	}

	void on_color_gradient_color_button_pressed(GtkColorButton* button, gpointer data)
	{
		auto* l_pUserData = static_cast<SColorGradientData*>(data);

		GdkColor l_oColor;
		gtk_color_button_get_color(button, &l_oColor);

		l_pUserData->vColorGradient[l_pUserData->vColorButtonMap[button]].oColor = l_oColor;

		on_refresh_color_gradient(nullptr, nullptr, data);
	}

	void on_initialize_color_gradient(GtkWidget* /*widget*/, gpointer data)
	{
		auto* l_pUserData = static_cast<SColorGradientData*>(data);

		gtk_widget_hide(l_pUserData->pContainer);

		gtk_container_foreach(GTK_CONTAINER(l_pUserData->pContainer), on_gtk_widget_destroy_cb, nullptr);

		uint32_t i         = 0;
		const size_t count = l_pUserData->vColorGradient.size();
		l_pUserData->vColorButtonMap.clear();
		l_pUserData->vSpinButtonMap.clear();
		for (vector<SColorGradientDataNode>::iterator it = l_pUserData->vColorGradient.begin(); it != l_pUserData->vColorGradient.end(); ++it, ++i)
		{
			GtkBuilder* l_pBuilderInterface = gtk_builder_new(); // glade_xml_new(l_pUserData->sGUIFilename.c_str(), "setting_editor-color_gradient-hbox", nullptr);
			gtk_builder_add_from_file(l_pBuilderInterface, l_pUserData->sGUIFilename.c_str(), nullptr);
			gtk_builder_connect_signals(l_pBuilderInterface, nullptr);

			GtkWidget* l_widget = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-hbox"));

			it->pColorButton = GTK_COLOR_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-colorbutton"));
			it->pSpinButton  = GTK_SPIN_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-spinbutton"));

			gtk_color_button_set_color(it->pColorButton, &it->oColor);
			gtk_spin_button_set_value(it->pSpinButton, it->fPercent);

			g_signal_connect(G_OBJECT(it->pColorButton), "color-set", G_CALLBACK(on_color_gradient_color_button_pressed), l_pUserData);
			g_signal_connect(G_OBJECT(it->pSpinButton), "value-changed", G_CALLBACK(on_color_gradient_spin_button_value_changed), l_pUserData);

			g_object_ref(l_widget);
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_widget)), l_widget);
			gtk_container_add(GTK_CONTAINER(l_pUserData->pContainer), l_widget);
			g_object_unref(l_widget);

			g_object_unref(l_pBuilderInterface);

			l_pUserData->vColorButtonMap[it->pColorButton] = i;
			l_pUserData->vSpinButtonMap[it->pSpinButton]   = i;
		}

		gtk_spin_button_set_value(l_pUserData->vColorGradient[0].pSpinButton, 0);
		gtk_spin_button_set_value(l_pUserData->vColorGradient[count - 1].pSpinButton, 100);

		gtk_widget_show(l_pUserData->pContainer);
	}

	void on_button_color_gradient_add_pressed(GtkButton* /*button*/, gpointer data)
	{
		auto* l_pUserData = static_cast<SColorGradientData*>(data);
		l_pUserData->vColorGradient.resize(l_pUserData->vColorGradient.size() + 1);
		l_pUserData->vColorGradient[l_pUserData->vColorGradient.size() - 1].fPercent = 100;
		on_initialize_color_gradient(nullptr, data);
		on_refresh_color_gradient(nullptr, nullptr, data);
	}

	void on_button_color_gradient_remove_pressed(GtkButton* /*button*/, gpointer data)
	{
		auto* l_pUserData = static_cast<SColorGradientData*>(data);
		if (l_pUserData->vColorGradient.size() > 2)
		{
			l_pUserData->vColorGradient.resize(l_pUserData->vColorGradient.size() - 1);
			l_pUserData->vColorGradient[l_pUserData->vColorGradient.size() - 1].fPercent = 100;
			on_initialize_color_gradient(nullptr, data);
			on_refresh_color_gradient(nullptr, nullptr, data);
		}
	}

	void on_button_setting_color_gradient_configure_pressed(GtkButton* button, gpointer data)
	{
		SColorGradientData l_oUserData;

		l_oUserData.sGUIFilename = static_cast<CSettingCollectionHelper*>(data)->m_sGUIFilename.toASCIIString();

		vector<GtkWidget*> l_vWidget;
		gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), collect_widget_cb, &l_vWidget);
		GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

		GtkBuilder* l_pBuilderInterface = gtk_builder_new(); // glade_xml_new(l_oUserData.sGUIFilename.c_str(), "setting_editor-color_gradient-dialog", nullptr);
		gtk_builder_add_from_file(l_pBuilderInterface, l_oUserData.sGUIFilename.c_str(), nullptr);
		gtk_builder_connect_signals(l_pBuilderInterface, nullptr);

		l_oUserData.pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-dialog"));

		const CString l_sInitialGradient = static_cast<CSettingCollectionHelper*>(data)->m_kernelContext.getConfigurationManager().expand(gtk_entry_get_text(l_widget));
		CMatrix l_oInitialGradient;

		OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(l_oInitialGradient, l_sInitialGradient);

		l_oUserData.vColorGradient.resize(l_oInitialGradient.getDimensionSize(1) > 2 ? l_oInitialGradient.getDimensionSize(1) : 2);
		for (uint32_t i = 0; i < l_oInitialGradient.getDimensionSize(1); ++i)
		{
			l_oUserData.vColorGradient[i].fPercent     = l_oInitialGradient[i * 4];
			l_oUserData.vColorGradient[i].oColor.red   = guint(l_oInitialGradient[i * 4 + 1] * .01 * 65535.);
			l_oUserData.vColorGradient[i].oColor.green = guint(l_oInitialGradient[i * 4 + 2] * .01 * 65535.);
			l_oUserData.vColorGradient[i].oColor.blue  = guint(l_oInitialGradient[i * 4 + 3] * .01 * 65535.);
		}

		l_oUserData.pContainer   = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-vbox"));
		l_oUserData.pDrawingArea = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-drawingarea"));

		g_signal_connect(G_OBJECT(l_oUserData.pDialog), "show", G_CALLBACK(on_initialize_color_gradient), &l_oUserData);
		g_signal_connect(G_OBJECT(l_oUserData.pDrawingArea), "expose_event", G_CALLBACK(on_refresh_color_gradient), &l_oUserData);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-add_button")), "pressed", G_CALLBACK(on_button_color_gradient_add_pressed), &l_oUserData);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-remove_button")), "pressed", G_CALLBACK(on_button_color_gradient_remove_pressed), &l_oUserData);

		if (gtk_dialog_run(GTK_DIALOG(l_oUserData.pDialog)) == GTK_RESPONSE_APPLY)
		{
			CString l_sFinalGradient;
			CMatrix l_oFinalGradient;
			l_oFinalGradient.setDimensionCount(2);
			l_oFinalGradient.setDimensionSize(0, 4);
			l_oFinalGradient.setDimensionSize(1, uint32_t(l_oUserData.vColorGradient.size()));
			for (uint32_t i = 0; i < l_oUserData.vColorGradient.size(); ++i)
			{
				l_oFinalGradient[i * 4]     = l_oUserData.vColorGradient[i].fPercent;
				l_oFinalGradient[i * 4 + 1] = l_oUserData.vColorGradient[i].oColor.red * 100. / 65535.;
				l_oFinalGradient[i * 4 + 2] = l_oUserData.vColorGradient[i].oColor.green * 100. / 65535.;
				l_oFinalGradient[i * 4 + 3] = l_oUserData.vColorGradient[i].oColor.blue * 100. / 65535.;
			}
			OpenViBEVisualizationToolkit::Tools::ColorGradient::format(l_sFinalGradient, l_oFinalGradient);
			gtk_entry_set_text(l_widget, l_sFinalGradient.toASCIIString());
		}

		gtk_widget_destroy(l_oUserData.pDialog);
		g_object_unref(l_pBuilderInterface);
	}
} // namespace

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

CSettingCollectionHelper::CSettingCollectionHelper(const IKernelContext& rKernelContext, const char* sGUIFilename)
	: m_kernelContext(rKernelContext), m_sGUIFilename(sGUIFilename) { }

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
	if (m_kernelContext.getTypeManager().isEnumeration(typeID)) { return "settings_collection-comboboxentry_setting_enumeration"; }
	if (m_kernelContext.getTypeManager().isBitMask(typeID)) { return "settings_collection-table_setting_bitmask"; }
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
	if (m_kernelContext.getTypeManager().isEnumeration(typeID)) { return "settings_collection-comboboxentry_setting_enumeration"; }
	if (m_kernelContext.getTypeManager().isBitMask(typeID)) { return "settings_collection-table_setting_bitmask"; }
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
	if (m_kernelContext.getTypeManager().isEnumeration(typeID)) { return getValueEnumeration(typeID, widget); }
	if (m_kernelContext.getTypeManager().isBitMask(typeID)) { return getValueBitMask(typeID, widget); }
	return getValueString(widget);
}

CString CSettingCollectionHelper::getValueBoolean(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return "false"; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[1])) { return "false"; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[1]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueInteger(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return "0"; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return "O"; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueFloat(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return "0"; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return "O"; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueString(GtkWidget* widget)
{
	if (!GTK_IS_ENTRY(widget)) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(widget);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueFilename(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueFoldername(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueScript(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueColor(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueColorGradient(GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	if (!GTK_IS_ENTRY(l_vWidget[0])) { return ""; }
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);
	return CString(gtk_entry_get_text(l_widget));
}

CString CSettingCollectionHelper::getValueEnumeration(const CIdentifier& /*typeID*/, GtkWidget* widget)
{
	if (!GTK_IS_COMBO_BOX(widget)) { return ""; }
	GtkComboBox* l_widget = GTK_COMBO_BOX(widget);
	return CString(gtk_combo_box_get_active_text(l_widget));
}

CString CSettingCollectionHelper::getValueBitMask(const CIdentifier& /*typeID*/, GtkWidget* widget)
{
	vector<GtkWidget*> l_vWidget;
	if (!GTK_IS_CONTAINER(widget)) { return ""; }
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	string l_sResult;

	for (auto& window : l_vWidget)
	{
		if (!GTK_IS_TOGGLE_BUTTON(window)) { return ""; }
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window)))
		{
			if (!l_sResult.empty())
			{
				l_sResult += string(1, OV_Value_EnumeratedStringSeparator);
			}
			l_sResult += gtk_button_get_label(GTK_BUTTON(window));
		}
	}
	return CString(l_sResult.c_str());
}

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

void CSettingCollectionHelper::setValue(const CIdentifier& typeID, GtkWidget* widget, const CString& rValue)
{
	if (!widget) { return; }
	if (typeID == OV_TypeId_Boolean) { return setValueBoolean(widget, rValue); }
	if (typeID == OV_TypeId_Integer) { return setValueInteger(widget, rValue); }
	if (typeID == OV_TypeId_Float) { return setValueFloat(widget, rValue); }
	if (typeID == OV_TypeId_String) { return setValueString(widget, rValue); }
	if (typeID == OV_TypeId_Filename) { return setValueFilename(widget, rValue); }
	if (typeID == OV_TypeId_Foldername) { return setValueFoldername(widget, rValue); }
	if (typeID == OV_TypeId_Script) { return setValueScript(widget, rValue); }
	if (typeID == OV_TypeId_Color) { return setValueColor(widget, rValue); }
	if (typeID == OV_TypeId_ColorGradient) { return setValueColorGradient(widget, rValue); }
	if (m_kernelContext.getTypeManager().isEnumeration(typeID)) { return setValueEnumeration(typeID, widget, rValue); }
	if (m_kernelContext.getTypeManager().isBitMask(typeID)) { return setValueBitMask(typeID, widget, rValue); }
	return setValueString(widget, rValue);
}

void CSettingCollectionHelper::setValueBoolean(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_pEntryWidget               = GTK_ENTRY(l_vWidget[0]);
	GtkToggleButton* l_pToggleButtonWidget = GTK_TOGGLE_BUTTON(l_vWidget[1]);

	if (rValue == CString("true"))
	{
		gtk_toggle_button_set_active(l_pToggleButtonWidget, true);
	}
	else if (rValue == CString("false"))
	{
		gtk_toggle_button_set_active(l_pToggleButtonWidget, false);
	}
	else
	{
		gtk_toggle_button_set_inconsistent(l_pToggleButtonWidget, true);
	}

	gtk_entry_set_text(l_pEntryWidget, rValue);

	g_signal_connect(G_OBJECT(l_pToggleButtonWidget), "toggled", G_CALLBACK(on_checkbutton_setting_bool_pressed), this);
	g_signal_connect(G_OBJECT(l_pEntryWidget), "changed", G_CALLBACK(on_entry_setting_bool_edited), this);
}

void CSettingCollectionHelper::setValueInteger(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_integer_up_pressed), this);
	g_signal_connect(G_OBJECT(l_vWidget[2]), "clicked", G_CALLBACK(on_button_setting_integer_down_pressed), this);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueFloat(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_float_up_pressed), this);
	g_signal_connect(G_OBJECT(l_vWidget[2]), "clicked", G_CALLBACK(on_button_setting_float_down_pressed), this);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueString(GtkWidget* widget, const CString& rValue)
{
	GtkEntry* l_widget = GTK_ENTRY(widget);
	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueFilename(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_filename_browse_pressed), this);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueFoldername(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_foldername_browse_pressed), this);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueScript(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_script_edit_pressed), this);
	g_signal_connect(G_OBJECT(l_vWidget[2]), "clicked", G_CALLBACK(on_button_setting_filename_browse_pressed), this);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueColor(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "color-set", G_CALLBACK(on_button_setting_color_choose_pressed), this);

	int r = 0, g = 0, b = 0;
	sscanf(m_kernelContext.getConfigurationManager().expand(rValue).toASCIIString(), "%i,%i,%i", &r, &g, &b);

	GdkColor l_oColor;
	l_oColor.red   = (r * 65535) / 100;
	l_oColor.green = (g * 65535) / 100;
	l_oColor.blue  = (b * 65535) / 100;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(l_vWidget[1]), &l_oColor);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueColorGradient(GtkWidget* widget, const CString& rValue)
{
	vector<GtkWidget*> l_vWidget;
	gtk_container_foreach(GTK_CONTAINER(widget), collect_widget_cb, &l_vWidget);
	GtkEntry* l_widget = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_color_gradient_configure_pressed), this);

	gtk_entry_set_text(l_widget, rValue);
}

void CSettingCollectionHelper::setValueEnumeration(const CIdentifier& typeID, GtkWidget* widget, const CString& rValue) const
{
	GtkTreeIter l_oListIter;
	GtkComboBox* l_widget      = GTK_COMBO_BOX(widget);
	GtkListStore* l_pList      = GTK_LIST_STORE(gtk_combo_box_get_model(l_widget));
	const uint64_t l_ui64Value = m_kernelContext.getTypeManager().getEnumerationEntryValueFromName(typeID, rValue);
	uint64_t i;

#if 0
	if (typeID == OV_TypeId_Stimulation)
	{
#endif
	std::map<CString, uint64_t> m_vListEntries;
	std::map<CString, uint64_t>::const_iterator it;

	for (i = 0; i < m_kernelContext.getTypeManager().getEnumerationEntryCount(typeID); ++i)
	{
		CString l_sEntryName;
		uint64_t l_ui64EntryValue;
		if (m_kernelContext.getTypeManager().getEnumerationEntry(typeID, i, l_sEntryName, l_ui64EntryValue))
		{
			m_vListEntries[l_sEntryName] = l_ui64EntryValue;
		}
	}

	gtk_combo_box_set_wrap_width(l_widget, 0);
	gtk_list_store_clear(l_pList);
	for (i = 0, it = m_vListEntries.begin(); it != m_vListEntries.end(); ++it, i++)
	{
		gtk_list_store_append(l_pList, &l_oListIter);
		gtk_list_store_set(l_pList, &l_oListIter, 0, it->first.toASCIIString(), -1);

		if (l_ui64Value == it->second) { gtk_combo_box_set_active(l_widget, gint(i)); }
	}
#if 0
	}
	else
	{
		gtk_list_store_clear(l_pList);
		for (i = 0; i < m_kernelContext.getTypeManager().getEnumerationEntryCount(typeID); ++i)
		{
			CString l_sEntryName;
			uint64_t l_ui64EntryValue;
			if (m_kernelContext.getTypeManager().getEnumerationEntry(typeID, i, l_sEntryName, l_ui64EntryValue))
			{
				gtk_list_store_append(l_pList, &l_oListIter);
				gtk_list_store_set(l_pList, &l_oListIter, 0, l_sEntryName.toASCIIString(), -1);

				if (l_ui64Value == l_ui64EntryValue) { gtk_combo_box_set_active(l_widget, gint(i)); }
			}
		}
	}
#endif
	if (gtk_combo_box_get_active(l_widget) == -1)
	{
		gtk_list_store_append(l_pList, &l_oListIter);
		gtk_list_store_set(l_pList, &l_oListIter, 0, rValue.toASCIIString(), -1);
		gtk_combo_box_set_active(l_widget, gint(i)); // $$$ i should be ok :)
	}
}

void CSettingCollectionHelper::setValueBitMask(const CIdentifier& typeID, GtkWidget* widget, const CString& rValue) const
{
	gtk_container_foreach(GTK_CONTAINER(widget), remove_widget_cb, widget);

	const string l_sValue(rValue);

	const gint l_iTableSize   = guint((m_kernelContext.getTypeManager().getBitMaskEntryCount(typeID) + 1) >> 1);
	GtkTable* l_pBitMaskTable = GTK_TABLE(widget);
	gtk_table_resize(l_pBitMaskTable, 2, l_iTableSize);

	for (uint64_t i = 0; i < m_kernelContext.getTypeManager().getBitMaskEntryCount(typeID); ++i)
	{
		CString l_sEntryName;
		uint64_t l_ui64EntryValue;
		if (m_kernelContext.getTypeManager().getBitMaskEntry(typeID, i, l_sEntryName, l_ui64EntryValue))
		{
			GtkWidget* l_pSettingButton = gtk_check_button_new();
			gtk_table_attach_defaults(l_pBitMaskTable, l_pSettingButton, guint(i & 1), guint((i & 1) + 1), guint(i >> 1), guint((i >> 1) + 1));
			gtk_button_set_label(GTK_BUTTON(l_pSettingButton), static_cast<const char*>(l_sEntryName));

			if (l_sValue.find(static_cast<const char*>(l_sEntryName)) != string::npos)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pSettingButton), true);
			}
		}
	}

	/*
	 * TODO - Add an entry text somewhere to manage
	 * configuration through configuration manager !
	 */

	gtk_widget_show_all(GTK_WIDGET(l_pBitMaskTable));
}
