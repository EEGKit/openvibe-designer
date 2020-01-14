#include "ovdCSettingCollectionHelper.h"

#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;
using namespace OpenViBEDesigner;
using namespace std;

#include <visualization-toolkit/ovvizColorGradient.h>
// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

typedef struct
{
	double percent;
	GdkColor color;
	GtkColorButton* colorButton;
	GtkSpinButton* spinButton;
} color_gradient_node_t;

typedef struct
{
	string guiFilename;
	GtkWidget* dialog;
	GtkWidget* container;
	GtkWidget* drawingArea;
	vector<color_gradient_node_t> colorGradient;
	map<GtkColorButton*, uint32_t> colorButtons;
	map<GtkSpinButton*, uint32_t> spinButtons;
} color_gradient_t;

static void gradients2Matrix(const vector<color_gradient_node_t>& in, CMatrix& out)
{
	out.setDimensionCount(2);
	out.setDimensionSize(0, 4);
	out.setDimensionSize(1, in.size());
	size_t i = 0;
	for (const auto& color : in)
	{
		out[i++] = color.percent;
		out[i++] = color.color.red / 655.35;	// * 100.0 / 65535.0;
		out[i++] = color.color.green / 655.35;	// * 100.0 / 65535.0;
		out[i++] = color.color.blue / 655.35;	// * 100.0 / 65535.0;
	}
}

static void CollectWidgetCB(GtkWidget* widget, gpointer data) { static_cast<vector<GtkWidget*>*>(data)->push_back(widget); }
static void RemoveWidgetCB(GtkWidget* widget, gpointer data) { gtk_container_remove(GTK_CONTAINER(data), widget); }

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

static void OnEntrySettingBOOLEdited(GtkEntry* entry, gpointer /*data*/)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(entry))), CollectWidgetCB, &widgets);
	GtkToggleButton* widget = GTK_TOGGLE_BUTTON(widgets[1]);

	const std::string value = gtk_entry_get_text(entry);
	if (value == "true")
	{
		gtk_toggle_button_set_active(widget, true);
		gtk_toggle_button_set_inconsistent(widget, false);
	}
	else if (value == "false")
	{
		gtk_toggle_button_set_active(widget, false);
		gtk_toggle_button_set_inconsistent(widget, false);
	}
	else { gtk_toggle_button_set_inconsistent(widget, true); }
}

static void OnCheckbuttonSettingBOOLPressed(GtkToggleButton* button, gpointer /*data*/)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	if (gtk_toggle_button_get_active(button)) { gtk_entry_set_text(widget, "true"); }
	else { gtk_entry_set_text(widget, "false"); }
	gtk_toggle_button_set_inconsistent(button, false);
}

static void OnButtonSettingIntegerPressed(GtkButton* button, gpointer data, const gint iOffset)
{
	const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_KernelCtx;

	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	const int64_t value   = ctx.getConfigurationManager().expandAsInteger(gtk_entry_get_text(widget), 0) + iOffset;
	const std::string res = std::to_string(value);
	gtk_entry_set_text(widget, res.c_str());
}

static void OnButtonSettingIntegerUpPressed(GtkButton* button, gpointer data) { OnButtonSettingIntegerPressed(button, data, 1); }

static void OnButtonSettingIntegerDownPressed(GtkButton* button, gpointer data) { OnButtonSettingIntegerPressed(button, data, -1); }

static void OnButtonSettingFloatPressed(GtkButton* button, gpointer data, const gdouble offset)
{
	const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_KernelCtx;

	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	double value = ctx.getConfigurationManager().expandAsFloat(gtk_entry_get_text(widget), 0);
	value += offset;
	const std::string res = std::to_string(value);
	gtk_entry_set_text(widget, res.c_str());
}

static void OnButtonSettingFloatUpPressed(GtkButton* button, gpointer data) { OnButtonSettingFloatPressed(button, data, 1); }

static void OnButtonSettingFloatDownPressed(GtkButton* button, gpointer data) { OnButtonSettingFloatPressed(button, data, -1); }

static void OnButtonSettingFilenameBrowsePressed(GtkButton* button, gpointer data)
{
	const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_KernelCtx;

	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select file to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
															  GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

	const CString filename = ctx.getConfigurationManager().expand(gtk_entry_get_text(widget));
	if (g_path_is_absolute(filename.toASCIIString())) { gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), filename.toASCIIString()); }
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), filename.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		char* backslash;
		while ((backslash = strchr(name, '\\')) != nullptr) { *backslash = '/'; }
		gtk_entry_set_text(widget, name);
		g_free(name);
	}
	gtk_widget_destroy(widgetDialogOpen);
}

static void OnButtonSettingFoldernameBrowsePressed(GtkButton* button, gpointer data)
{
	const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_KernelCtx;

	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	GtkWidget* widgetDialogOpen = gtk_file_chooser_dialog_new("Select folder to open...", nullptr, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, nullptr);

	const CString filename = ctx.getConfigurationManager().expand(gtk_entry_get_text(widget));
	if (g_path_is_absolute(filename.toASCIIString())) { gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), filename.toASCIIString()); }
	else
	{
		char* fullPath = g_build_filename(g_get_current_dir(), filename.toASCIIString(), nullptr);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widgetDialogOpen), fullPath);
		g_free(fullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widgetDialogOpen), false);

	if (gtk_dialog_run(GTK_DIALOG(widgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		char* name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widgetDialogOpen));
		char* backslash;
		while ((backslash = strchr(name, '\\')) != nullptr) { *backslash = '/'; }
		gtk_entry_set_text(widget, name);
		g_free(name);
	}
	gtk_widget_destroy(widgetDialogOpen);
}
// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

static void OnButtonSettingScriptEditPressed(GtkButton* button, gpointer data)
{
	const IKernelContext& ctx = static_cast<CSettingCollectionHelper*>(data)->m_KernelCtx;

	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	const CString name = ctx.getConfigurationManager().expand(gtk_entry_get_text(widget));
	const CString cmd  = ctx.getConfigurationManager().expand("${Designer_ScriptEditorCommand}");

	if (cmd != CString(""))
	{
		CString fullCmd = cmd + CString(" \"") + name + CString("\"");
#if defined TARGET_OS_Windows
		fullCmd = "START " + fullCmd;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			fullCmd = fullCmd + " &";
#else
#endif
		if (system(fullCmd.toASCIIString()) < 0) { ctx.getLogManager() << LogLevel_Warning << "Could not run command " << fullCmd << "\n"; }
	}
}

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

static void OnButtonSettingColorChoosePressed(GtkColorButton* button, gpointer /*data*/)
{
	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);
	GdkColor color;
	gtk_color_button_get_color(button, &color);

	const std::string res = std::to_string((color.red * 100) / 65535) + "," + std::to_string((color.green * 100) / 65535) + ","
							+ std::to_string((color.blue * 100) / 65535);
	gtk_entry_set_text(widget, res.c_str());
}

// ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- -----------

static void OnGTKWidgetDestroy(GtkWidget* widget, gpointer /*data*/) { gtk_widget_destroy(widget); }

static void OnInitializeColorGradient(GtkWidget* widget, gpointer data);

static void OnRefreshColorGradient(GtkWidget* /*widget*/, GdkEventExpose* /*event*/, gpointer data)
{
	auto* userData = static_cast<color_gradient_t*>(data);

	const size_t steps = 100;
	gint sizex         = 0;
	gint sizey         = 0;
	gdk_drawable_get_size(userData->drawingArea->window, &sizex, &sizey);

	CMatrix gradient;
	gradients2Matrix(userData->colorGradient, gradient);

	CMatrix interpolated;
	OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(interpolated, gradient, steps);

	GdkGC* gc = gdk_gc_new(userData->drawingArea->window);
	GdkColor color;

	for (size_t i = 0; i < steps; ++i)
	{
		color.red   = guint(interpolated[i * 4 + 1] * 655.35);
		color.green = guint(interpolated[i * 4 + 2] * 655.35);
		color.blue  = guint(interpolated[i * 4 + 3] * 655.35);
		gdk_gc_set_rgb_fg_color(gc, &color);
		gdk_draw_rectangle(userData->drawingArea->window, gc, TRUE, (sizex * i) / steps, 0, (sizex * (i + 1)) / steps, sizey);
	}
	g_object_unref(gc);
}

static void OnColorGradientSpinButtonValueChanged(GtkSpinButton* button, gpointer data)
{
	auto* userData = static_cast<color_gradient_t*>(data);

	gtk_spin_button_update(button);

	const size_t i          = userData->spinButtons[button];
	GtkSpinButton* prevSpin = (i > 0 ? userData->colorGradient[i - 1].spinButton : nullptr);
	GtkSpinButton* nextSpin = (i < userData->colorGradient.size() - 1 ? userData->colorGradient[i + 1].spinButton : nullptr);
	if (!prevSpin) { gtk_spin_button_set_value(button, 0); }
	if (!nextSpin) { gtk_spin_button_set_value(button, 100); }
	if (prevSpin && gtk_spin_button_get_value(button) < gtk_spin_button_get_value(prevSpin))
	{
		gtk_spin_button_set_value(button, gtk_spin_button_get_value(prevSpin));
	}
	if (nextSpin && gtk_spin_button_get_value(button) > gtk_spin_button_get_value(nextSpin))
	{
		gtk_spin_button_set_value(button, gtk_spin_button_get_value(nextSpin));
	}

	userData->colorGradient[i].percent = gtk_spin_button_get_value(button);

	OnRefreshColorGradient(nullptr, nullptr, data);
}

static void OnColorGradientColorButtonPressed(GtkColorButton* button, gpointer data)
{
	auto* userData = static_cast<color_gradient_t*>(data);

	GdkColor color;
	gtk_color_button_get_color(button, &color);

	userData->colorGradient[userData->colorButtons[button]].color = color;

	OnRefreshColorGradient(nullptr, nullptr, data);
}

static void OnInitializeColorGradient(GtkWidget* /*widget*/, gpointer data)
{
	auto* userData = static_cast<color_gradient_t*>(data);

	gtk_widget_hide(userData->container);

	gtk_container_foreach(GTK_CONTAINER(userData->container), OnGTKWidgetDestroy, nullptr);

	const size_t count = userData->colorGradient.size();
	userData->colorButtons.clear();
	userData->spinButtons.clear();
	size_t i = 0;
	for (auto& it : userData->colorGradient)
	{
		GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(userData->guiFilename.c_str(), "setting_editor-color_gradient-hbox", nullptr);
		gtk_builder_add_from_file(builder, userData->guiFilename.c_str(), nullptr);
		gtk_builder_connect_signals(builder, nullptr);

		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-hbox"));

		it.colorButton = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "setting_editor-color_gradient-colorbutton"));
		it.spinButton  = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "setting_editor-color_gradient-spinbutton"));

		gtk_color_button_set_color(it.colorButton, &it.color);
		gtk_spin_button_set_value(it.spinButton, it.percent);

		g_signal_connect(G_OBJECT(it.colorButton), "color-set", G_CALLBACK(OnColorGradientColorButtonPressed), userData);
		g_signal_connect(G_OBJECT(it.spinButton), "value-changed", G_CALLBACK(OnColorGradientSpinButtonValueChanged), userData);

		g_object_ref(widget);
		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widget)), widget);
		gtk_container_add(GTK_CONTAINER(userData->container), widget);
		g_object_unref(widget);

		g_object_unref(builder);

		userData->colorButtons[it.colorButton] = i;
		userData->spinButtons[it.spinButton]   = i;
		i++;
	}

	gtk_spin_button_set_value(userData->colorGradient[0].spinButton, 0);
	gtk_spin_button_set_value(userData->colorGradient[count - 1].spinButton, 100);

	gtk_widget_show(userData->container);
}

static void OnButtonColorGradientAddPressed(GtkButton* /*button*/, gpointer data)
{
	auto* userData = static_cast<color_gradient_t*>(data);
	userData->colorGradient.resize(userData->colorGradient.size() + 1);
	userData->colorGradient[userData->colorGradient.size() - 1].percent = 100;
	OnInitializeColorGradient(nullptr, data);
	OnRefreshColorGradient(nullptr, nullptr, data);
}

static void OnButtonColorGradientRemovePressed(GtkButton* /*button*/, gpointer data)
{
	auto* userData = static_cast<color_gradient_t*>(data);
	if (userData->colorGradient.size() > 2)
	{
		userData->colorGradient.resize(userData->colorGradient.size() - 1);
		userData->colorGradient[userData->colorGradient.size() - 1].percent = 100;
		OnInitializeColorGradient(nullptr, data);
		OnRefreshColorGradient(nullptr, nullptr, data);
	}
}

static void OnButtonSettingColorGradientConfigurePressed(GtkButton* button, gpointer data)
{
	color_gradient_t userData;

	userData.guiFilename = static_cast<CSettingCollectionHelper*>(data)->m_GUIFilename.toASCIIString();

	vector<GtkWidget*> widgets;
	gtk_container_foreach(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(button))), CollectWidgetCB, &widgets);
	GtkEntry* widget = GTK_ENTRY(widgets[0]);

	GtkBuilder* builder = gtk_builder_new(); // glade_xml_new(l_oUserData.guiFilename.c_str(), "setting_editor-color_gradient-dialog", nullptr);
	gtk_builder_add_from_file(builder, userData.guiFilename.c_str(), nullptr);
	gtk_builder_connect_signals(builder, nullptr);

	userData.dialog = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-dialog"));

	const CString sInitialGradient = static_cast<CSettingCollectionHelper*>(data)->m_KernelCtx.getConfigurationManager().expand(gtk_entry_get_text(widget));
	CMatrix initialGradient;

	OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(initialGradient, sInitialGradient);

	userData.colorGradient.resize(initialGradient.getDimensionSize(1) > 2 ? initialGradient.getDimensionSize(1) : 2);
	for (size_t i = 0; i < initialGradient.getDimensionSize(1); ++i)
	{
		userData.colorGradient[i].percent     = initialGradient[i * 4];
		userData.colorGradient[i].color.red   = guint(initialGradient[i * 4 + 1] * 655.35);
		userData.colorGradient[i].color.green = guint(initialGradient[i * 4 + 2] * 655.35);
		userData.colorGradient[i].color.blue  = guint(initialGradient[i * 4 + 3] * 655.35);
	}

	userData.container   = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-vbox"));
	userData.drawingArea = GTK_WIDGET(gtk_builder_get_object(builder, "setting_editor-color_gradient-drawingarea"));

	g_signal_connect(G_OBJECT(userData.dialog), "show", G_CALLBACK(OnInitializeColorGradient), &userData);
	g_signal_connect(G_OBJECT(userData.drawingArea), "expose_event", G_CALLBACK(OnRefreshColorGradient), &userData);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "setting_editor-color_gradient-add_button")), "pressed",
					 G_CALLBACK(OnButtonColorGradientAddPressed), &userData);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "setting_editor-color_gradient-remove_button")), "pressed",
					 G_CALLBACK(OnButtonColorGradientRemovePressed), &userData);

	if (gtk_dialog_run(GTK_DIALOG(userData.dialog)) == GTK_RESPONSE_APPLY)
	{
		CString str;
		CMatrix gradient;
		gradients2Matrix(userData.colorGradient, gradient);

		OpenViBEVisualizationToolkit::Tools::ColorGradient::format(str, gradient);
		gtk_entry_set_text(widget, str.toASCIIString());
	}

	gtk_widget_destroy(userData.dialog);
	g_object_unref(builder);
}

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
	if (m_KernelCtx.getTypeManager().isEnumeration(typeID)) { return "settings_collection-comboboxentry_setting_enumeration"; }
	if (m_KernelCtx.getTypeManager().isBitMask(typeID)) { return "settings_collection-table_setting_bitmask"; }
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
	if (m_KernelCtx.getTypeManager().isEnumeration(typeID)) { return "settings_collection-comboboxentry_setting_enumeration"; }
	if (m_KernelCtx.getTypeManager().isBitMask(typeID)) { return "settings_collection-table_setting_bitmask"; }
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
	if (m_KernelCtx.getTypeManager().isEnumeration(typeID)) { return getValueEnumeration(typeID, widget); }
	if (m_KernelCtx.getTypeManager().isBitMask(typeID)) { return getValueBitMask(typeID, widget); }
	return getValueString(widget);
}

CString CSettingCollectionHelper::getValueBoolean(GtkWidget* widget)
{
	vector<GtkWidget*> widgets;
	if (!GTK_IS_CONTAINER(widget)) { return "false"; }
	gtk_container_foreach(GTK_CONTAINER(widget), CollectWidgetCB, &widgets);
	if (!GTK_IS_ENTRY(widgets[1])) { return "false"; }
	GtkEntry* entry = GTK_ENTRY(widgets[1]);
	return CString(gtk_entry_get_text(entry));
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
	GtkEntry* entry = GTK_ENTRY(widgets[0]);
	return CString(gtk_entry_get_text(entry));
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
	return res.c_str();
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
	if (m_KernelCtx.getTypeManager().isEnumeration(typeID)) { return setValueEnumeration(typeID, widget, value); }
	if (m_KernelCtx.getTypeManager().isBitMask(typeID)) { return setValueBitMask(typeID, widget, value); }
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
	GtkEntry* entry = GTK_ENTRY(widgets[0]);

	g_signal_connect(G_OBJECT(widgets[1]), "clicked", G_CALLBACK(OnButtonSettingIntegerUpPressed), this);
	g_signal_connect(G_OBJECT(widgets[2]), "clicked", G_CALLBACK(OnButtonSettingIntegerDownPressed), this);

	gtk_entry_set_text(entry, value);
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
	sscanf(m_KernelCtx.getConfigurationManager().expand(value).toASCIIString(), "%i,%i,%i", &r, &g, &b);

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
	const uint64_t v      = m_KernelCtx.getTypeManager().getEnumerationEntryValueFromName(typeID, value);
	uint64_t i;

#if 0
	if (typeID == OV_TypeId_Stimulation)
	{
#endif

	std::map<CString, uint64_t> listEntries;
	for (i = 0; i < m_KernelCtx.getTypeManager().getEnumerationEntryCount(typeID); ++i)
	{
		CString entryName;
		uint64_t entryValue;
		if (m_KernelCtx.getTypeManager().getEnumerationEntry(typeID, i, entryName, entryValue)) { listEntries[entryName] = entryValue; }
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
			CString entryName;
			uint64_t entryValue;
			if (m_kernelCtx.getTypeManager().getEnumerationEntry(typeID, i, entryName, entryValue))
			{
				gtk_list_store_append(list, &listIter);
				gtk_list_store_set(list, &listIter, 0, entryName.toASCIIString(), -1);

				if (v == entryValue) { gtk_combo_box_set_active(l_widget, gint(i)); }
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

	const string str(value);

	const gint size        = guint((m_KernelCtx.getTypeManager().getBitMaskEntryCount(typeID) + 1) >> 1);
	GtkTable* bitMaskTable = GTK_TABLE(widget);
	gtk_table_resize(bitMaskTable, 2, size);

	for (uint64_t i = 0; i < m_KernelCtx.getTypeManager().getBitMaskEntryCount(typeID); ++i)
	{
		CString entryName;
		uint64_t entryValue;
		if (m_KernelCtx.getTypeManager().getBitMaskEntry(typeID, i, entryName, entryValue))
		{
			GtkWidget* settingButton = gtk_check_button_new();
			gtk_table_attach_defaults(bitMaskTable, settingButton, guint(i & 1), guint((i & 1) + 1), guint(i >> 1), guint((i >> 1) + 1));
			gtk_button_set_label(GTK_BUTTON(settingButton), entryName.toASCIIString());

			if (str.find(entryName.toASCIIString()) != string::npos) { gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(settingButton), true); }
		}
	}

	/*
	 * TODO - Add an entry text somewhere to manage
	 * configuration through configuration manager !
	 */

	gtk_widget_show_all(GTK_WIDGET(bitMaskTable));
}
