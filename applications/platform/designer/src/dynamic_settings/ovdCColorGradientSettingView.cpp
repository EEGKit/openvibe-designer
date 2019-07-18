#include "ovdCColorGradientSettingView.h"
#include "../ovd_base.h"
#include <visualization-toolkit/ovvizColorGradient.h>

#include <iostream>
#include <cstring>
#include <cmath>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Setting;

static void on_color_gradient_color_button_pressed(GtkColorButton* button, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->colorChange(button);
}

static void on_button_setting_color_gradient_configure_pressed(GtkButton* /*button*/, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->configurePressed();
}

static void on_refresh_color_gradient(GtkWidget* /*widget*/, GdkEventExpose* /*event*/, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->refreshColorGradient();
}

static void on_gtk_widget_destroy_cb(GtkWidget* widget, gpointer /*data*/)
{
	gtk_widget_destroy(widget);
}

static void on_initialize_color_gradient(GtkWidget* /*widget*/, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->initializeGradient();
}

static void on_button_color_gradient_add_pressed(GtkButton* /*button*/, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->addColor();
}

static void on_button_color_gradient_remove_pressed(GtkButton* /*button*/, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->removeColor();
}

static void on_color_gradient_spin_button_value_changed(GtkSpinButton* button, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->spinChange(button);
}

static void on_change(GtkEntry* /*entry*/, gpointer data)
{
	static_cast<CColorGradientSettingView *>(data)->onChange();
}

CColorGradientSettingView::CColorGradientSettingView(Kernel::IBox& rBox, const uint32_t index, CString& rBuilderName, const Kernel::IKernelContext& rKernelContext)
	: CAbstractSettingView(rBox, index, rBuilderName, "settings_collection-hbox_setting_color_gradient"), m_kernelContext(rKernelContext), m_builderName(rBuilderName)
{
	GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector<GtkWidget*> l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_entry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_entry), "changed", G_CALLBACK(on_change), this);
	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_color_gradient_configure_pressed), this);

	initializeValue();
}


void CColorGradientSettingView::getValue(CString& value) const
{
	value = CString(gtk_entry_get_text(m_entry));
}


void CColorGradientSettingView::setValue(const CString& value)
{
	m_onValueSetting = true;
	gtk_entry_set_text(m_entry, value);
	m_onValueSetting = false;
}

void CColorGradientSettingView::configurePressed()
{
	GtkBuilder* l_pBuilderInterface = gtk_builder_new(); // glade_xml_new(l_oUserData.sGUIFilename.c_str(), "setting_editor-color_gradient-dialog", NULL);
	gtk_builder_add_from_file(l_pBuilderInterface, m_builderName.toASCIIString(), nullptr);
	gtk_builder_connect_signals(l_pBuilderInterface, nullptr);

	pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-dialog"));

	const CString initialGradient = m_kernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_entry));
	CMatrix colorGradient;

	OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(colorGradient, initialGradient);
	vColorGradient.resize(std::max<size_t>(colorGradient.getDimensionSize(1), 2));
	for (size_t i = 0; i < colorGradient.getDimensionSize(1); ++i)
	{
		const uint32_t idx = uint32_t(i * 4);
		vColorGradient[i].percent = colorGradient[idx];
		vColorGradient[i].color.red = guint(colorGradient[idx + 1] * .01 * 65535.);
		vColorGradient[i].color.green = guint(colorGradient[idx + 2] * .01 * 65535.);
		vColorGradient[i].color.blue = guint(colorGradient[idx + 3] * .01 * 65535.);
	}

	pContainer = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-vbox"));
	pDrawingArea = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-drawingarea"));

	g_signal_connect(G_OBJECT(pDialog), "show", G_CALLBACK(on_initialize_color_gradient), this);
	g_signal_connect(G_OBJECT(pDrawingArea), "expose_event", G_CALLBACK(on_refresh_color_gradient), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-add_button")), "pressed", G_CALLBACK(on_button_color_gradient_add_pressed), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-remove_button")), "pressed", G_CALLBACK(on_button_color_gradient_remove_pressed), this);

	if (gtk_dialog_run(GTK_DIALOG(pDialog)) == GTK_RESPONSE_APPLY)
	{
		CString finalGradient;
		CMatrix finalColorGradient;
		finalColorGradient.setDimensionCount(2);
		finalColorGradient.setDimensionSize(0, 4);
		finalColorGradient.setDimensionSize(1, uint32_t(vColorGradient.size()));
		for (uint32_t i = 0; i < uint32_t(vColorGradient.size()); i++)
		{
			const uint32_t idx = i * 4;
			finalColorGradient[idx] = vColorGradient[i].percent;
			finalColorGradient[idx + 1] = round(vColorGradient[i].color.red * 100. / 65535.);
			finalColorGradient[idx + 2] = round(vColorGradient[i].color.green * 100. / 65535.);
			finalColorGradient[idx + 3] = round(vColorGradient[i].color.blue * 100. / 65535.);
		}
		OpenViBEVisualizationToolkit::Tools::ColorGradient::format(finalGradient, finalColorGradient);
		if (!m_onValueSetting) { getBox().setSettingValue(getSettingIndex(), finalGradient.toASCIIString()); }
		setValue(finalGradient.toASCIIString());
	}

	gtk_widget_destroy(pDialog);
	g_object_unref(l_pBuilderInterface);
}


void CColorGradientSettingView::initializeGradient()
{
	gtk_widget_hide(pContainer);

	gtk_container_foreach(GTK_CONTAINER(pContainer), on_gtk_widget_destroy_cb, nullptr);

	uint32_t i = 0;
	const uint32_t count = uint32_t(vColorGradient.size());
	vColorButtonMap.clear();
	vSpinButtonMap.clear();
	for (auto it = vColorGradient.begin(); it != vColorGradient.end(); ++it, i++)
	{
		GtkBuilder* l_pBuilderInterface = gtk_builder_new(); // glade_xml_new(l_pUserData->sGUIFilename.c_str(), "setting_editor-color_gradient-hbox", NULL);
		gtk_builder_add_from_file(l_pBuilderInterface, m_builderName.toASCIIString(), nullptr);
		gtk_builder_connect_signals(l_pBuilderInterface, nullptr);

		GtkWidget* l_pWidget = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-hbox"));

		it->colorButton = GTK_COLOR_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-colorbutton"));
		it->spinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-spinbutton"));

		gtk_color_button_set_color(it->colorButton, &it->color);
		gtk_spin_button_set_value(it->spinButton, it->percent);

		g_signal_connect(G_OBJECT(it->colorButton), "color-set", G_CALLBACK(on_color_gradient_color_button_pressed), this);
		g_signal_connect(G_OBJECT(it->spinButton), "value-changed", G_CALLBACK(on_color_gradient_spin_button_value_changed), this);

		g_object_ref(l_pWidget);
		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidget)), l_pWidget);
		gtk_container_add(GTK_CONTAINER(pContainer), l_pWidget);
		g_object_unref(l_pWidget);

		g_object_unref(l_pBuilderInterface);

		vColorButtonMap[it->colorButton] = i;
		vSpinButtonMap[it->spinButton] = i;
	}

	gtk_spin_button_set_value(vColorGradient[0].spinButton, 0);
	gtk_spin_button_set_value(vColorGradient[count - 1].spinButton, 100);

	gtk_widget_show(pContainer);
}

void CColorGradientSettingView::refreshColorGradient()
{
	const uint32_t steps = 100;
	gint sizex = 0;
	gint sizey = 0;
	gdk_drawable_get_size(pDrawingArea->window, &sizex, &sizey);

	CMatrix l_oGradientMatrix;
	l_oGradientMatrix.setDimensionCount(2);
	l_oGradientMatrix.setDimensionSize(0, 4);
	l_oGradientMatrix.setDimensionSize(1, uint32_t(vColorGradient.size()));
	for (uint32_t i = 0; i < vColorGradient.size(); ++i)
	{
		const uint32_t idx = i * 4;
		l_oGradientMatrix[idx] = vColorGradient[i].percent;
		l_oGradientMatrix[idx + 1] = vColorGradient[i].color.red * 100. / 65535.;
		l_oGradientMatrix[idx + 2] = vColorGradient[i].color.green * 100. / 65535.;
		l_oGradientMatrix[idx + 3] = vColorGradient[i].color.blue * 100. / 65535.;
	}

	CMatrix l_oInterpolatedMatrix;
	OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(l_oInterpolatedMatrix, l_oGradientMatrix, steps);

	GdkGC* l_pGC = gdk_gc_new(pDrawingArea->window);
	GdkColor l_oColor;

	for (uint32_t i = 0; i < steps; ++i)
	{
		l_oColor.red = guint(l_oInterpolatedMatrix[i * 4 + 1] * 65535 * .01);
		l_oColor.green = guint(l_oInterpolatedMatrix[i * 4 + 2] * 65535 * .01);
		l_oColor.blue = guint(l_oInterpolatedMatrix[i * 4 + 3] * 65535 * .01);
		gdk_gc_set_rgb_fg_color(l_pGC, &l_oColor);
		gdk_draw_rectangle(pDrawingArea->window, l_pGC, TRUE, (sizex * i) / steps, 0, (sizex * (i + 1)) / steps, sizey);
	}
	g_object_unref(l_pGC);
}

void CColorGradientSettingView::addColor()
{
	vColorGradient.resize(vColorGradient.size() + 1);
	vColorGradient[vColorGradient.size() - 1].percent = 100;
	initializeGradient();
	refreshColorGradient();
}

void CColorGradientSettingView::removeColor()
{
	if (vColorGradient.size() > 2)
	{
		vColorGradient.resize(vColorGradient.size() - 1);
		vColorGradient[vColorGradient.size() - 1].percent = 100;
		initializeGradient();
		refreshColorGradient();
	}
}

void CColorGradientSettingView::spinChange(GtkSpinButton* button)
{
	gtk_spin_button_update(button);

	const uint32_t i = vSpinButtonMap[button];
	GtkSpinButton* l_pPrevSpinButton = i > 0 ? vColorGradient[i - 1].spinButton : nullptr;
	GtkSpinButton* l_pNextSpinButton = i < vColorGradient.size() - 1 ? vColorGradient[i + 1].spinButton : nullptr;
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

	vColorGradient[i].percent = gtk_spin_button_get_value(button);

	refreshColorGradient();
}

void CColorGradientSettingView::colorChange(GtkColorButton* button)
{
	GdkColor l_oColor;
	gtk_color_button_get_color(button, &l_oColor);

	vColorGradient[vColorButtonMap[button]].color = l_oColor;

	refreshColorGradient();
}

void CColorGradientSettingView::onChange()
{
	if (!m_onValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_entry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
