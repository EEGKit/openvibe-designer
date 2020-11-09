/*********************************************************************
 * Software License Agreement (AGPL-3 License)
 *
 * OpenViBE Designer
 * Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
 * Copyright (C) Inria, 2015-2017,V1.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "mCBoxAlgorithmViz.hpp"

// OpenGL 1.2
#ifndef GL_BGRA
#	define GL_BGRA 0x80E1
#endif // GL_BGRA

using namespace Mensia;
using namespace AdvancedVisualization;

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;

namespace {
template <int i>
class toolbar_sort_changed_
{
public:
	static void callback(GtkButton* /*button*/, CBoxAlgorithmViz* box)
	{
		box->m_RendererCtx->sortSelectedChannel(i);
		box->m_RedrawNeeded = true;
	}
};

void channel_selection_changed_(GtkTreeSelection* selection, CRendererContext* ctx)
{
	size_t i                = 0;
	GtkTreeView* treeView   = gtk_tree_selection_get_tree_view(selection);
	GtkTreeModel* treeModel = gtk_tree_view_get_model(treeView);
	if (treeModel != nullptr)
	{
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first(treeModel, &iter) != 0)
		{
			do
			{
				if (gtk_tree_selection_iter_is_selected(selection, &iter) != 0) { ctx->selectChannel(i); }
				else { ctx->unselectChannel(i); }
				i++;
			} while (gtk_tree_model_iter_next(treeModel, &iter) != 0);
		}
	}
}

void spinbutton_time_scale_change_value_callback(GtkSpinButton* button, CRendererContext* ctx)
{
	ctx->setTimeScale(size_t(gtk_spin_button_get_value(button) * (1LL << 32)));
}

void spinbutton_element_count_change_value_callback(GtkSpinButton* button, CRendererContext* ctx)
{
	ctx->setElementCount(size_t(gtk_spin_button_get_value(button)));
}

void checkbutton_positive_toggled_callback(GtkToggleButton* button, CRendererContext* ctx) { ctx->setPositiveOnly(gtk_toggle_button_get_active(button) != 0); }

void checkbutton_show_scale_toggled_callback(GtkToggleButton* button, CRendererContext* ctx)
{
	ctx->setScaleVisibility(gtk_toggle_button_get_active(button) != 0);
}

void button_video_recording_pressed_callback(GtkButton* button, CBoxAlgorithmViz* box)
{
	box->m_IsVideoOutputWorking = !box->m_IsVideoOutputWorking;
	gtk_button_set_label(button, box->m_IsVideoOutputWorking ? GTK_STOCK_MEDIA_PAUSE : GTK_STOCK_MEDIA_RECORD);
}

void range_erp_value_changed_callback(GtkRange* range, GtkLabel* label)
{
	char str[1024];
	sprintf(str, "%.02f%%", gtk_range_get_value(range) * 100);
	gtk_label_set_text(label, str);
	getContext().stepERPFractionBy(float(gtk_range_get_value(range)) - getContext().getERPFraction());
}

void button_erp_play_pause_pressed_callback(GtkButton* /*button*/, CRendererContext* ctx) { ctx->setERPPlayerActive(!ctx->isERPPlayerActive()); }

void spinbutton_freq_band_min_change_value_callback(GtkSpinButton* button, CRendererContext* ctx)
{
	ctx->setMinimumSpectrumFrequency(size_t(gtk_spin_button_get_value(button)));
}

void spinbutton_freq_band_max_change_value_callback(GtkSpinButton* button, CRendererContext* ctx)
{
	ctx->setMaximumSpectrumFrequency(size_t(gtk_spin_button_get_value(button)));
}
}  // namespace

bool CBoxAlgorithmViz::initialize()

{
	m_RendererCtx    = new CRendererContext();
	m_SubRendererCtx = new CRendererContext();

	// Sets default setting values
	m_Colors.clear();
	m_Localisation      = CString("");
	m_TemporalCoherence = ETemporalCoherence::TimeLocked;
	m_NElement          = 50;
	m_TimeScale         = 10LL << 32;
	m_IsPositive        = false;
	m_IsTimeLocked      = true;
	m_IsScaleVisible    = true;
	m_TextureID         = 0;
	m_lastProcessTime   = 0;
	m_Time1             = 0;
	m_Time2             = 0;
	CString color       = CString("100,100,100");
	m_ColorGradient     = CString("0:0,0,0; 100:100,100,100");
	m_XYZPlotHasDepth   = false;
	m_DataScale         = 1;
	m_Translucency      = 1;


	// Initializes fast forward behavior
	m_fastForwardMaxFactorHD = float(this->getConfigurationManager().expandAsFloat("${AdvancedViz_HighDefinition_FastForwardFactor}", 5.F));
	m_fastForwardMaxFactorLD = float(this->getConfigurationManager().expandAsFloat("${AdvancedViz_LowDefinition_FastForwardFactor}", 20.F));

	// Gets data stream type
	this->getStaticBoxContext().getInputType(0, m_TypeID);

	// Prepares GUI
	m_Builder = gtk_builder_new();
	gtk_builder_add_from_file(m_Builder, std::string(Directories::getDataDir() + "/plugins/advanced-visualization.ui").c_str(), nullptr);

	GtkWidget* main    = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "table"));
	GtkWidget* toolbar = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "toolbar-window"));
	m_Viewport         = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "viewport"));
	m_Top              = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "label_top"));
	m_Left             = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "drawingarea_left"));
	m_Right            = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "drawingarea_right"));
	m_Bottom           = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "drawingarea_bottom"));
	m_CornerLeft       = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "label_corner_left"));
	m_CornerRight      = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "label_corner_right"));

	// Gets important widgets
	m_TimeScaleW       = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "spinbutton_time_scale"));
	m_nElementW        = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "spinbutton_element_count"));
	m_ERPRange         = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "range_erp"));
	m_ERPPlayerButton  = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "button_erp_play_pause"));
	m_ERPPlayer        = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "table_erp"));
	m_ScaleVisible     = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "checkbutton_show_scale"));
	m_FrequencyBandMin = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "spinbutton_freq_band_min"));
	m_FrequencyBandMax = GTK_WIDGET(::gtk_builder_get_object(m_Builder, "spinbutton_freq_band_max"));

	m_ChannelTreeView  = GTK_TREE_VIEW(::gtk_builder_get_object(m_Builder, "expander_select_treeview"));
	m_ChannelListStore = GTK_LIST_STORE(::gtk_builder_get_object(m_Builder, "liststore_select"));

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(m_ChannelTreeView), GTK_SELECTION_MULTIPLE);

	// Sets default spectrum frequency range
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FrequencyBandMin), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FrequencyBandMax), 48);

	// Connects GTK signals
	g_signal_connect(::gtk_builder_get_object(m_Builder, "expander_sort_button_default"), "pressed", G_CALLBACK(::toolbar_sort_changed_<1>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "expander_sort_button_alpha"), "pressed", G_CALLBACK(::toolbar_sort_changed_<2>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "expander_sort_button_reversed"), "pressed", G_CALLBACK(::toolbar_sort_changed_<3>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "expander_sort_button_left_right"), "pressed", G_CALLBACK(::toolbar_sort_changed_<4>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "expander_sort_button_front_back"), "pressed", G_CALLBACK(::toolbar_sort_changed_<5>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "spinbutton_time_scale"), "value-changed",
					 G_CALLBACK(::spinbutton_time_scale_change_value_callback), m_RendererCtx);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "spinbutton_element_count"), "value-changed",
					 G_CALLBACK(::spinbutton_element_count_change_value_callback), m_RendererCtx);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "spinbutton_element_count"), "value-changed",
					 G_CALLBACK(::spinbutton_element_count_change_value_callback), m_SubRendererCtx);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "checkbutton_positive"), "toggled", G_CALLBACK(::checkbutton_positive_toggled_callback),
					 m_RendererCtx);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "checkbutton_show_scale"), "toggled", G_CALLBACK(::checkbutton_show_scale_toggled_callback),
					 &Mensia::AdvancedVisualization::getContext());
	g_signal_connect(::gtk_builder_get_object(m_Builder, "button_erp_play_pause"), "pressed", G_CALLBACK(::button_erp_play_pause_pressed_callback),
					 &Mensia::AdvancedVisualization::getContext());
	g_signal_connect(::gtk_builder_get_object(m_Builder, "button_video_recording"), "pressed", G_CALLBACK(::button_video_recording_pressed_callback), this);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "range_erp"), "value-changed", G_CALLBACK(::range_erp_value_changed_callback),
					 ::gtk_builder_get_object(m_Builder, "label_erp_progress"));
	g_signal_connect(::gtk_builder_get_object(m_Builder, "spinbutton_freq_band_min"), "value-changed",
					 G_CALLBACK(::spinbutton_freq_band_min_change_value_callback), m_RendererCtx);
	g_signal_connect(::gtk_builder_get_object(m_Builder, "spinbutton_freq_band_max"), "value-changed",
					 G_CALLBACK(::spinbutton_freq_band_max_change_value_callback), m_RendererCtx);

	g_signal_connect(::gtk_tree_view_get_selection(m_ChannelTreeView), "changed", G_CALLBACK(channel_selection_changed_), m_RendererCtx);

	// Hides unnecessary widgets
	if (std::find(m_Parameters.begin(), m_Parameters.end(), S_DataScale) == m_Parameters.end())
	{
		gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "checkbutton_positive")));
	}
	if (std::find(m_Parameters.begin(), m_Parameters.end(), S_ChannelLocalisation) == m_Parameters.end())
	{
		gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "expander_sort")));
	}
	if (m_TypeID != OV_TypeId_Spectrum) { gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "expander_freq_band"))); }
	gtk_widget_hide(m_ERPPlayer);

	// Prepares 3D View
	m_GtkGLWidget.initialize(*this, m_Viewport, m_Left, m_Right, m_Bottom);
	m_GtkGLWidget.setPointSmoothingActive(this->getConfigurationManager().expandAsBoolean("${AdvancedViz_SmoothPoint}", false));

	// Fowards widgets to the OpenViBE viz context
	if (!this->canCreatePluginObject(OVP_ClassId_Plugin_VisualizationCtx))
	{
		this->getLogManager() << LogLevel_Error << "Visualization framework is not loaded" << "\n";
		return false;
	}

	m_visualizationCtx = dynamic_cast<VisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationCtx));
	m_visualizationCtx->setWidget(*this, main);
	m_visualizationCtx->setToolbar(*this, toolbar);

	// Parses box settings
	size_t settingIndex = 0;
	for (int iParameter : m_Parameters)
	{
		double value;
		switch (iParameter)
		{
			case S_ChannelLocalisation:
				m_Localisation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_Caption:
				m_Caption = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_Color:
				color = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_ColorGradient:
				m_ColorGradient = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_DataPositive:
				m_IsPositive = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_TemporalCoherence:
				m_TemporalCoherence = ETemporalCoherence(size_t(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex)));
				break;
			case S_TimeScale:
				value = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				m_TimeScale = size_t(value * (1LL << 32));
				break;
			case S_ElementCount:
				m_NElement = size_t(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex));
				break;
			case S_DataScale:
				m_DataScale = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_FlowerRingCount:
				m_NFlowerRing = size_t(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex));
				break;
			case S_Translucency:
				m_Translucency = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_ShowAxis:
				m_ShowAxis = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_XYZPlotHasDepth:
				m_XYZPlotHasDepth = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;

			case F_FixedChannelOrder:
				settingIndex--;
				gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "expander_sort")));
				break;
			case F_FixedChannelSelection:
				settingIndex--;
				gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "expander_select")));
				break;
			default:
				settingIndex--;
				break;
		}

		settingIndex++;
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(::gtk_builder_get_object(m_Builder, "checkbutton_positive")), gboolean(m_IsPositive));

	// Parses color string
	parseColor(m_Color, color.toASCIIString());
	m_Colors.push_back(m_Color);

	// Parses color string - special for instant oscilloscope which can have several inputs
	for (size_t i = settingIndex; i < this->getStaticBoxContext().getSettingCount(); ++i)
	{
		color = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		parseColor(m_Color, color.toASCIIString());
		m_Colors.push_back(m_Color);
	}

	// Sets caption
	if (m_Caption != CString("")) { gtk_label_set_text(GTK_LABEL(m_Top), std::string(m_Caption.toASCIIString()).c_str()); }

	// Sets time scale
	if (m_TemporalCoherence == ETemporalCoherence::TimeLocked)
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_Builder, "spinbutton_time_scale")), (m_TimeScale >> 22) / 1024.);
		m_IsTimeLocked = true;
	}
	else { gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "vbox_time_scale"))); }

	// Sets matrix count
	if (m_TemporalCoherence == ETemporalCoherence::Independant)
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_Builder, "spinbutton_element_count")), double(m_NElement));
		m_IsTimeLocked = false;
	}
	else { gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "vbox_element_count"))); }

	// Shows / hides scale
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ScaleVisible), gboolean(m_RendererCtx->getScaleVisibility()));

	// Reads channel localisation
	if (m_Localisation != CString(""))
	{
		IAlgorithmProxy* channelPosReader = &this->getAlgorithmManager().getAlgorithm(
			this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_OVMatrixFileReader));
		channelPosReader->initialize();

		const TParameterHandler<CString*> ip_filename(
			channelPosReader->getInputParameter(OVP_GD_Algorithm_OVMatrixFileReader_InputParameterId_Filename));
		TParameterHandler<IMatrix*> op_matrix(channelPosReader->getOutputParameter(OVP_GD_Algorithm_OVMatrixFileReader_OutputParameterId_Matrix));

		*ip_filename = m_Localisation;

		channelPosReader->process();

		if (op_matrix->getDimensionCount() != 2 || op_matrix->getDimensionSize(1) != 3)
		{
			this->getLogManager() << LogLevel_Warning << "Invalid channel localisation file " << m_Localisation << "\n";
		}
		else
		{
			const size_t nChannel = op_matrix->getDimensionSize(0);
			double* buffer        = op_matrix->getBuffer();
			for (size_t i = 0; i < nChannel; ++i)
			{
				std::string name = trim(op_matrix->getDimensionLabel(0, i));
				std::transform(name.begin(), name.end(), name.begin(), tolower);
				m_ChannelPositions[name] = CVertex(-buffer[1], buffer[2], -buffer[0]);
				buffer += 3;
			}
		}

		channelPosReader->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*channelPosReader);
		channelPosReader = nullptr;
	}

	// Gets frame base path for video output, if the variable is not defined, no video output is performed
	const CString basePath   = this->getConfigurationManager().expand("${AdvancedVisualization_VideoOutputPath}");
	const CString sessionId  = this->getConfigurationManager().expand("[$core{date}-$core{time}]");
	const CString widgetName = this->getStaticBoxContext().getName();
	m_FrameFilenameFormat    = basePath + sessionId + widgetName + CString("-%06i.png");
	m_IsVideoOutputEnabled   = (basePath != CString(""));
	m_IsVideoOutputWorking   = false;
	m_FrameId                = 0;
	gtk_widget_set_visible(GTK_WIDGET(::gtk_builder_get_object(m_Builder, "hbox_video_recording")), m_IsVideoOutputEnabled ? TRUE : FALSE);
	gtk_label_set_markup(GTK_LABEL(::gtk_builder_get_object(m_Builder, "label_video_recording_filename")),
						 (CString("<span foreground=\"darkblue\"><small>") + m_FrameFilenameFormat + CString("</small></span>")).toASCIIString());

	m_Width  = 0;
	m_Height = 0;

	return true;
}

bool CBoxAlgorithmViz::uninitialize()

{
	g_object_unref(m_Builder);
	m_Builder = nullptr;

	getContext().stepERPFractionBy(-getContext().getERPFraction());
	getContext().setERPPlayerActive(false);

	delete m_SubRendererCtx;
	m_SubRendererCtx = nullptr;

	delete m_RendererCtx;
	m_RendererCtx = nullptr;

	this->releasePluginObject(m_visualizationCtx);

	return true;
}

bool CBoxAlgorithmViz::processClock(IMessageClock& /*clock*/)
{
	const uint64_t currentTime = this->getPlayerContext().getCurrentTime();

	const uint64_t minDeltaTimeHD  = (1LL << 32) / 16;
	const uint64_t minDeltaTimeLD  = (1LL << 32);
	const uint64_t minDeltaTimeLD2 = (1LL << 32) * 5;

	uint64_t minDeltaTime;
	if (this->getPlayerContext().getStatus() == EPlayerStatus::Play) { minDeltaTime = minDeltaTimeHD; }
	else
	{
		const auto fastForwardMaxFactor = float(this->getPlayerContext().getCurrentFastForwardMaximumFactor());
		if (fastForwardMaxFactor <= m_fastForwardMaxFactorHD) { minDeltaTime = minDeltaTimeHD; }
		else if (fastForwardMaxFactor <= m_fastForwardMaxFactorLD)
		{
			const float alpha = (fastForwardMaxFactor - m_fastForwardMaxFactorHD) / (m_fastForwardMaxFactorLD - m_fastForwardMaxFactorHD);
			minDeltaTime      = uint64_t((minDeltaTimeLD * alpha) + minDeltaTimeHD * (1.F - alpha));
		}
		else { minDeltaTime = minDeltaTimeLD2; }
	}

	if (currentTime > m_lastProcessTime + minDeltaTime || this->getPlayerContext().getStatus() == EPlayerStatus::Step || this->getPlayerContext().getStatus() ==
		EPlayerStatus::Pause)
	{
		m_lastProcessTime    = currentTime;
		this->m_RedrawNeeded = true;
		this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	}

	this->updateRulerVisibility();
	return true;
}

void CBoxAlgorithmViz::updateRulerVisibility()
{
	if (m_IsScaleVisible != m_RendererCtx->getScaleVisibility())
	{
		m_IsScaleVisible = m_RendererCtx->getScaleVisibility();

		if ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ScaleVisible)) != 0) != m_IsScaleVisible)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ScaleVisible), gboolean(m_IsScaleVisible));
		}

		void (*action)(GtkWidget*) = m_IsScaleVisible ? gtk_widget_show : gtk_widget_hide;
		(*action)(this->m_Top);
		(*action)(this->m_Left);
		(*action)(this->m_Right);
		(*action)(this->m_Bottom);
		(*action)(this->m_CornerLeft);
		(*action)(this->m_CornerRight);
	}
}

void CBoxAlgorithmViz::reshape(const int width, const int height)
{
	m_Width  = size_t(width);
	m_Height = size_t(height);
	m_RendererCtx->setAspect(float(width) / float(height));
}

void CBoxAlgorithmViz::preDraw()
{
	this->updateRulerVisibility();

	if (m_TextureID == 0U) { m_TextureID = m_GtkGLWidget.createTexture(m_ColorGradient.toASCIIString()); }
	glBindTexture(GL_TEXTURE_1D, m_TextureID);

	m_RendererCtx->setAspect(m_Viewport->allocation.width * 1.F / m_Viewport->allocation.height);
}

void CBoxAlgorithmViz::postDraw()

{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	if (m_Ruler != nullptr) { m_Ruler->doRender(); }
	glPopAttrib();

	if (m_IsVideoOutputEnabled && m_IsVideoOutputWorking && m_Width > 0 && m_Height > 0)
	{
		// Builds up filename to save PNG to
		char filename[1024];
		sprintf(filename, m_FrameFilenameFormat.toASCIIString(), ++m_FrameId);

		// Reads OpenGL buffer and stores it to a cairo surface
		cairo_surface_t* cairoSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, int(m_Width), int(m_Height));
		glReadPixels(0, 0, GLsizei(m_Width), GLsizei(m_Height), GL_BGRA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(cairoSurface));

		// OpenGL buffers are defined bottom to top while PNG are defined top to bottom, this flips the acquired image
		const size_t size = 4; // should be 3
		std::vector<unsigned char> swaps(m_Width * size);
		unsigned char* swap = &swaps[0];
		unsigned char* src1 = cairo_image_surface_get_data(cairoSurface);
		unsigned char* src2 = cairo_image_surface_get_data(cairoSurface) + m_Width * (m_Height - 1) * size;
		for (size_t i = 0; i < m_Height / 2; ++i)
		{
			memcpy(swap, src1, m_Width * size);
			memcpy(src1, src2, m_Width * size);
			memcpy(src2, swap, m_Width * size);
			src1 += m_Width * size;
			src2 -= m_Width * size;
		}

		// Pixels are ready to save
		cairo_surface_write_to_png(cairoSurface, filename);
		cairo_surface_destroy(cairoSurface);
	}
}

void CBoxAlgorithmViz::mouseButton(const int x, const int y, const int button, const int status)
{
	// Mouse interacts with local renderer context
	// m_MouseHandler.mouseButton(*m_rendererCtx, x, y, button, status);
	// Mouse interacts with global renderer context
	m_MouseHandler.mouseButton(getContext(), x, y, button, status);
	this->redraw();
}

void CBoxAlgorithmViz::mouseMotion(const int x, const int y)
{
	// Mouse interacts with local renderer context
	// m_MouseHandler.mouseMotion(*m_rendererCtx, x, y);
	// Mouse interacts with global renderer context
	m_MouseHandler.mouseMotion(getContext(), x, y);
	if (m_MouseHandler.hasButtonPressed()) { this->redraw(); }
}

void CBoxAlgorithmViz::keyboard(const int x, const int y, const size_t key, const bool status)
{
	std::cout << "keyboard : x=" << x << " y=" << y << " key=" << key << " status=" << (status ? "pressed" : "released");
}

void CBoxAlgorithmViz::parseColor(color_t& rColor, const std::string& sColor)
{
	float r, g, b;
	if (sscanf(sColor.c_str(), "%f,%f,%f", &r, &g, &b) == 3)
	{
		rColor.r = r * .01F;
		rColor.g = g * .01F;
		rColor.b = b * .01F;
	}
	else
	{
		rColor.r = 1;
		rColor.g = 1;
		rColor.b = 1;
	}
}
