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
using namespace Kernel;

namespace
{
	template <int i>
	class toolbar_sort_changed_
	{
	public:
		static void callback(GtkButton* /*button*/, CBoxAlgorithmViz* pThis)
		{
			pThis->m_pRendererContext->sortSelectedChannel(i);
			pThis->m_bRedrawNeeded = true;
		}
	};

	void channel_selection_changed_(GtkTreeSelection* pTreeSelection, IRendererContext* pRendererContext)
	{
		uint32_t i                 = 0;
		GtkTreeView* l_pTreeView   = gtk_tree_selection_get_tree_view(pTreeSelection);
		GtkTreeModel* l_pTreeModel = gtk_tree_view_get_model(l_pTreeView);
		if (l_pTreeModel != nullptr)
		{
			GtkTreeIter l_oIter;
			if (gtk_tree_model_get_iter_first(l_pTreeModel, &l_oIter) != 0)
			{
				do
				{
					if (gtk_tree_selection_iter_is_selected(pTreeSelection, &l_oIter) != 0) { pRendererContext->selectChannel(i); }
					else { pRendererContext->unselectChannel(i); }
					i++;
				} while (gtk_tree_model_iter_next(l_pTreeModel, &l_oIter) != 0);
			}
		}
	}

	void spinbutton_time_scale_change_value_callback(GtkSpinButton* pSpinButton, IRendererContext* pRendererContext)
	{
		pRendererContext->setTimeScale(uint64_t(gtk_spin_button_get_value(pSpinButton) * (1LL << 32)));
	}

	void spinbutton_element_count_change_value_callback(GtkSpinButton* pSpinButton, IRendererContext* pRendererContext)
	{
		pRendererContext->setElementCount(uint64_t(gtk_spin_button_get_value(pSpinButton)));
	}

	void checkbutton_positive_toggled_callback(GtkToggleButton* button, IRendererContext* pRendererContext)
	{
		pRendererContext->setPositiveOnly(gtk_toggle_button_get_active(button) != 0);
	}

	void checkbutton_show_scale_toggled_callback(GtkToggleButton* button, IRendererContext* pRendererContext)
	{
		pRendererContext->setScaleVisibility(gtk_toggle_button_get_active(button) != 0);
	}

	void button_video_recording_pressed_callback(GtkButton* button, CBoxAlgorithmViz* pBox)
	{
		pBox->m_bIsVideoOutputWorking = !pBox->m_bIsVideoOutputWorking;
		gtk_button_set_label(button, pBox->m_bIsVideoOutputWorking ? GTK_STOCK_MEDIA_PAUSE : GTK_STOCK_MEDIA_RECORD);
	}

	void range_erp_value_changed_callback(GtkRange* pRange, GtkLabel* pLabel)
	{
		char l_sLabel[1024];
		sprintf(l_sLabel, "%.02f%%", gtk_range_get_value(pRange) * 100);
		gtk_label_set_text(pLabel, l_sLabel);
		getContext().stepERPFractionBy(float(gtk_range_get_value(pRange)) - getContext().getERPFraction());
	}

	void button_erp_play_pause_pressed_callback(GtkButton* /*button*/, IRendererContext* pRendererContext)
	{
		pRendererContext->setERPPlayerActive(!pRendererContext->isERPPlayerActive());
	}

	void spinbutton_freq_band_min_change_value_callback(GtkSpinButton* pSpinButton, IRendererContext* pRendererContext)
	{
		pRendererContext->setMinimumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(pSpinButton)));
	}

	void spinbutton_freq_band_max_change_value_callback(GtkSpinButton* pSpinButton, IRendererContext* pRendererContext)
	{
		pRendererContext->setMaximumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(pSpinButton)));
	}
} // namespace

bool CBoxAlgorithmViz::initialize()

{
	m_pRendererContext    = IRendererContext::create();
	m_pSubRendererContext = IRendererContext::create();

	// Sets default setting values
	m_sLocalisation       = CString("");
	m_temporalCoherence   = OVP_TypeId_TemporalCoherence_TimeLocked.toUInteger();
	m_elementCount        = 50;
	m_timeScale           = 10LL << 32;
	m_bIsPositive         = false;
	m_bIsTimeLocked       = true;
	m_bIsScaleVisible     = true;
	m_textureId           = 0;
	m_ui64LastProcessTime = 0;
	m_time1               = 0;
	m_time2               = 0;
	m_sColor              = CString("100,100,100");
	m_sColorGradient      = CString("0:0,0,0; 100:100,100,100");
	m_bXYZPlotHasDepth    = false;
	m_f64DataScale        = 1;
	m_translucency        = 1;
	m_vColor.clear();

	// Initializes fast forward behavior
	m_fastForwardMaximumFactorHighDefinition = float(this->getConfigurationManager().expandAsFloat("${AdvancedViz_HighDefinition_FastForwardFactor}", 5.f));
	m_fastForwardMaximumFactorLowDefinition  = float(this->getConfigurationManager().expandAsFloat("${AdvancedViz_LowDefinition_FastForwardFactor}", 20.f));

	// Gets data stream type
	this->getStaticBoxContext().getInputType(0, m_oTypeIdentifier);

	// Prepares GUI
	m_pBuilder = gtk_builder_new();
	gtk_builder_add_from_file(m_pBuilder, std::string(Directories::getDataDir() + "/plugins/advanced-visualization.ui").c_str(), nullptr);

	GtkWidget* l_pMain    = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "table"));
	GtkWidget* l_pToolbar = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "toolbar-window"));
	m_pViewport           = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "viewport"));
	m_pTop                = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "label_top"));
	m_pLeft               = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "drawingarea_left"));
	m_pRight              = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "drawingarea_right"));
	m_pBottom             = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "drawingarea_bottom"));
	m_pCornerLeft         = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "label_corner_left"));
	m_pCornerRight        = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "label_corner_right"));

	// Gets important widgets
	m_pTimeScale        = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "spinbutton_time_scale"));
	m_pElementCount     = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "spinbutton_element_count"));
	m_pERPRange         = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "range_erp"));
	m_pERPPlayerButton  = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "button_erp_play_pause"));
	m_pERPPlayer        = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "table_erp"));
	m_pScaleVisible     = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "checkbutton_show_scale"));
	m_pFrequencyBandMin = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "spinbutton_freq_band_min"));
	m_pFrequencyBandMax = GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "spinbutton_freq_band_max"));

	m_pChannelTreeView  = GTK_TREE_VIEW(::gtk_builder_get_object(m_pBuilder, "expander_select_treeview"));
	m_pChannelListStore = GTK_LIST_STORE(::gtk_builder_get_object(m_pBuilder, "liststore_select"));

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(m_pChannelTreeView), GTK_SELECTION_MULTIPLE);

	// Sets default spectrum frequency range
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax), 48);

	// Connects GTK signals
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "expander_sort_button_default"), "pressed", G_CALLBACK(::toolbar_sort_changed_<1>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "expander_sort_button_alpha"), "pressed", G_CALLBACK(::toolbar_sort_changed_<2>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "expander_sort_button_reversed"), "pressed", G_CALLBACK(::toolbar_sort_changed_<3>::callback), this);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "expander_sort_button_left_right"), "pressed", G_CALLBACK(::toolbar_sort_changed_<4>::callback),
					 this);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "expander_sort_button_front_back"), "pressed", G_CALLBACK(::toolbar_sort_changed_<5>::callback),
					 this);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "spinbutton_time_scale"), "value-changed", G_CALLBACK(::spinbutton_time_scale_change_value_callback),
					 m_pRendererContext);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "spinbutton_element_count"), "value-changed",
					 G_CALLBACK(::spinbutton_element_count_change_value_callback), m_pRendererContext);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "spinbutton_element_count"), "value-changed",
					 G_CALLBACK(::spinbutton_element_count_change_value_callback), m_pSubRendererContext);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "checkbutton_positive"), "toggled", G_CALLBACK(::checkbutton_positive_toggled_callback),
					 m_pRendererContext);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "checkbutton_show_scale"), "toggled", G_CALLBACK(::checkbutton_show_scale_toggled_callback),
					 &Mensia::AdvancedVisualization::getContext());
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "button_erp_play_pause"), "pressed", G_CALLBACK(::button_erp_play_pause_pressed_callback),
					 &Mensia::AdvancedVisualization::getContext());
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "button_video_recording"), "pressed", G_CALLBACK(::button_video_recording_pressed_callback), this);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "range_erp"), "value-changed", G_CALLBACK(::range_erp_value_changed_callback),
					 ::gtk_builder_get_object(m_pBuilder, "label_erp_progress"));
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "spinbutton_freq_band_min"), "value-changed",
					 G_CALLBACK(::spinbutton_freq_band_min_change_value_callback), m_pRendererContext);
	g_signal_connect(::gtk_builder_get_object(m_pBuilder, "spinbutton_freq_band_max"), "value-changed",
					 G_CALLBACK(::spinbutton_freq_band_max_change_value_callback), m_pRendererContext);

	g_signal_connect(::gtk_tree_view_get_selection(m_pChannelTreeView), "changed", G_CALLBACK(channel_selection_changed_), m_pRendererContext);

	// Hides unnecessary widgets
	if (std::find(m_vParameter.begin(), m_vParameter.end(), S_DataScale) == m_vParameter.end())
	{
		gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "checkbutton_positive")));
	}
	if (std::find(m_vParameter.begin(), m_vParameter.end(), S_ChannelLocalisation) == m_vParameter.end())
	{
		gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "expander_sort")));
	}
	if (m_oTypeIdentifier != OV_TypeId_Spectrum) { gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "expander_freq_band"))); }
	gtk_widget_hide(m_pERPPlayer);

	// Prepares 3D View
	m_oGtkGLWidget.initialize(*this, m_pViewport, m_pLeft, m_pRight, m_pBottom);
	m_oGtkGLWidget.setPointSmoothingActive(this->getConfigurationManager().expandAsBoolean("${AdvancedViz_SmoothPoint}", false));

	// Fowards widgets to the OpenViBE viz context
	if (!this->canCreatePluginObject(OVP_ClassId_Plugin_VisualizationContext))
	{
		this->getLogManager() << LogLevel_Error << "Visualization framework is not loaded" << "\n";
		return false;
	}

	m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(
		OVP_ClassId_Plugin_VisualizationContext));
	m_visualizationContext->setWidget(*this, l_pMain);
	m_visualizationContext->setToolbar(*this, l_pToolbar);

	// Parses box settings
	uint32_t settingIndex = 0;
	for (int iParameter : m_vParameter)
	{
		double l_fValue;
		switch (iParameter)
		{
			case S_ChannelLocalisation:
				m_sLocalisation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_Caption:
				m_sCaption = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_Color:
				m_sColor = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_ColorGradient:
				m_sColorGradient = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_DataPositive:
				m_bIsPositive = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_TemporalCoherence:
				m_temporalCoherence = uint64_t(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex));
				break;
			case S_TimeScale:
				l_fValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				m_timeScale = uint64_t(l_fValue * (1LL << 32));
				break;
			case S_ElementCount:
				m_elementCount = uint64_t(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex));
				break;
			case S_DataScale:
				m_f64DataScale = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_FlowerRingCount:
				m_flowerRingCount = uint64_t(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex));
				break;
			case S_Translucency:
				m_translucency = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_ShowAxis:
				m_bShowAxis = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;
			case S_XYZPlotHasDepth:
				m_bXYZPlotHasDepth = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), settingIndex);
				break;

			case F_FixedChannelOrder:
				settingIndex--;
				gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "expander_sort")));
				break;
			case F_FixedChannelSelection:
				settingIndex--;
				gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "expander_select")));
				break;
			default:
				settingIndex--;
				break;
		}

		settingIndex++;
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(::gtk_builder_get_object(m_pBuilder, "checkbutton_positive")), gboolean(m_bIsPositive));

	// Parses color string
	parseColor(m_oColor, m_sColor.toASCIIString());
	m_vColor.push_back(m_oColor);

	// Parses color string - special for instant oscilloscope which can have several inputs
	for (uint32_t i = settingIndex; i < this->getStaticBoxContext().getSettingCount(); ++i)
	{
		m_sColor = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		parseColor(m_oColor, m_sColor.toASCIIString());
		m_vColor.push_back(m_oColor);
	}

	// Sets caption
	if (m_sCaption != CString("")) { gtk_label_set_text(GTK_LABEL(m_pTop), std::string(m_sCaption.toASCIIString()).c_str()); }

	// Sets time scale
	if (m_temporalCoherence == OVP_TypeId_TemporalCoherence_TimeLocked.toUInteger())
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilder, "spinbutton_time_scale")), (m_timeScale >> 22) / 1024.);
		m_bIsTimeLocked = true;
	}
	else { gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "vbox_time_scale"))); }

	// Sets matrix count
	if (m_temporalCoherence == OVP_TypeId_TemporalCoherence_Independant.toUInteger())
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilder, "spinbutton_element_count")), double(m_elementCount));
		m_bIsTimeLocked = false;
	}
	else { gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "vbox_element_count"))); }

	// Shows / hides scale
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pScaleVisible), gboolean(m_pRendererContext->getScaleVisibility()));

	// Reads channel localisation
	if (m_sLocalisation != CString(""))
	{
		IAlgorithmProxy* l_pChannelLocalisationReader = &this->getAlgorithmManager().getAlgorithm(
			this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_OVMatrixFileReader));
		l_pChannelLocalisationReader->initialize();

		const TParameterHandler<CString*> ip_sFilename(
			l_pChannelLocalisationReader->getInputParameter(OVP_GD_Algorithm_OVMatrixFileReader_InputParameterId_Filename));
		TParameterHandler<IMatrix*> op_pMatrix(l_pChannelLocalisationReader->getOutputParameter(OVP_GD_Algorithm_OVMatrixFileReader_OutputParameterId_Matrix));

		*ip_sFilename = m_sLocalisation;

		l_pChannelLocalisationReader->process();

		if (op_pMatrix->getDimensionCount() != 2 || op_pMatrix->getDimensionSize(1) != 3)
		{
			this->getLogManager() << LogLevel_Warning << "Invalid channel localisation file " << m_sLocalisation << "\n";
		}
		else
		{
			const uint32_t nChannel = op_pMatrix->getDimensionSize(0);
			double* buffer          = op_pMatrix->getBuffer();
			for (uint32_t i = 0; i < nChannel; ++i)
			{
				std::string l_sName = trim(op_pMatrix->getDimensionLabel(0, i));
				std::transform(l_sName.begin(), l_sName.end(), l_sName.begin(), tolower);
				m_vChannelLocalisation[l_sName] = CVertex(-buffer[1], buffer[2], -buffer[0]);
				buffer += 3;
			}
		}

		l_pChannelLocalisationReader->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*l_pChannelLocalisationReader);
		l_pChannelLocalisationReader = nullptr;
	}

	// Gets frame base path for video output, if the variable is not defined, no video output is performed
	const CString l_sFrameBasePath   = this->getConfigurationManager().expand("${AdvancedVisualization_VideoOutputPath}");
	const CString l_sFrameSessionId  = this->getConfigurationManager().expand("[$core{date}-$core{time}]");
	const CString l_sFrameWidgetName = this->getStaticBoxContext().getName();
	m_sFrameFilenameFormat           = l_sFrameBasePath + l_sFrameSessionId + l_sFrameWidgetName + CString("-%06i.png");
	m_bIsVideoOutputEnabled          = (l_sFrameBasePath != CString(""));
	m_bIsVideoOutputWorking          = false;
	m_ui32FrameId                    = 0;
	gtk_widget_set_visible(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "hbox_video_recording")), m_bIsVideoOutputEnabled ? TRUE : FALSE);
	gtk_label_set_markup(
		GTK_LABEL(::gtk_builder_get_object(m_pBuilder, "label_video_recording_filename")),
		(CString("<span foreground=\"darkblue\"><small>") + m_sFrameFilenameFormat + CString("</small></span>")).toASCIIString());

	m_width  = 0;
	m_height = 0;

	return true;
}

bool CBoxAlgorithmViz::uninitialize()

{
	g_object_unref(m_pBuilder);
	m_pBuilder = nullptr;

	getContext().stepERPFractionBy(-getContext().getERPFraction());
	getContext().setERPPlayerActive(false);

	IRendererContext::release(m_pSubRendererContext);
	m_pSubRendererContext = nullptr;

	IRendererContext::release(m_pRendererContext);
	m_pRendererContext = nullptr;

	this->releasePluginObject(m_visualizationContext);

	return true;
}

bool CBoxAlgorithmViz::processClock(IMessageClock& /*rClock*/)
{
	const uint64_t currentTime = this->getPlayerContext().getCurrentTime();

	const uint64_t minDeltaTimeHighDefinition = (1LL << 32) / 16;
	const uint64_t minDeltaTimeLowDefinition  = (1LL << 32);
	const uint64_t minDeltaTimeLowDefinition2 = (1LL << 32) * 5;

	uint64_t minDeltaTime;
	if (this->getPlayerContext().getStatus() == PlayerStatus_Play) { minDeltaTime = minDeltaTimeHighDefinition; }
	else
	{
		const auto l_f32CurrentFastForwardMaximumFactor = float(this->getPlayerContext().getCurrentFastForwardMaximumFactor());
		if (l_f32CurrentFastForwardMaximumFactor <= m_fastForwardMaximumFactorHighDefinition) { minDeltaTime = minDeltaTimeHighDefinition; }
		else if (l_f32CurrentFastForwardMaximumFactor <= m_fastForwardMaximumFactorLowDefinition)
		{
			const float alpha = (l_f32CurrentFastForwardMaximumFactor - m_fastForwardMaximumFactorHighDefinition) / (
									m_fastForwardMaximumFactorLowDefinition - m_fastForwardMaximumFactorHighDefinition);
			minDeltaTime = uint64_t((minDeltaTimeLowDefinition * alpha) + minDeltaTimeHighDefinition * (1.f - alpha));
		}
		else { minDeltaTime = minDeltaTimeLowDefinition2; }
	}

	if (currentTime > m_ui64LastProcessTime + minDeltaTime || this->getPlayerContext().getStatus() == PlayerStatus_Step || this->getPlayerContext().getStatus()
		== PlayerStatus_Pause)
	{
		m_ui64LastProcessTime = currentTime;
		this->m_bRedrawNeeded = true;
		this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	}

	this->updateRulerVisibility();
	return true;
}

void CBoxAlgorithmViz::updateRulerVisibility()
{
	if (m_bIsScaleVisible != m_pRendererContext->getScaleVisibility())
	{
		m_bIsScaleVisible = m_pRendererContext->getScaleVisibility();

		if ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pScaleVisible)) != 0) != m_bIsScaleVisible)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pScaleVisible), gboolean(m_bIsScaleVisible));
		}

		void (*l_fpAction)(GtkWidget*) = m_bIsScaleVisible ? gtk_widget_show : gtk_widget_hide;
		(*l_fpAction)(this->m_pTop);
		(*l_fpAction)(this->m_pLeft);
		(*l_fpAction)(this->m_pRight);
		(*l_fpAction)(this->m_pBottom);
		(*l_fpAction)(this->m_pCornerLeft);
		(*l_fpAction)(this->m_pCornerRight);
	}
}

void CBoxAlgorithmViz::reshape(const int width, const int height)
{
	m_width  = uint32_t(width);
	m_height = uint32_t(height);
	m_pRendererContext->setAspect(float(width) / float(height));
}

void CBoxAlgorithmViz::preDraw()

{
	this->updateRulerVisibility();

	if (m_textureId == 0u) { m_textureId = m_oGtkGLWidget.createTexture(m_sColorGradient.toASCIIString()); }
	glBindTexture(GL_TEXTURE_1D, m_textureId);

	m_pRendererContext->setAspect(m_pViewport->allocation.width * 1.f / m_pViewport->allocation.height);
}

void CBoxAlgorithmViz::postDraw()

{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	if (m_pRuler != nullptr) { m_pRuler->doRender(); }
	glPopAttrib();

	if (m_bIsVideoOutputEnabled && m_bIsVideoOutputWorking && m_width > 0 && m_height > 0)
	{
		// Builds up filename to save PNG to
		char l_sFilename[1024];
		sprintf(l_sFilename, m_sFrameFilenameFormat.toASCIIString(), ++m_ui32FrameId);

		// Reads OpenGL buffer and stores it to a cairo surface
		cairo_surface_t* l_pCairoSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, m_width, m_height);
		glReadPixels(0, 0, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(l_pCairoSurface));

		// OpenGL buffers are defined bottom to top while PNG are defined top to bottom, this flips the acquired image
		const uint32_t l_ui32BytesInPixel = 4; // should be 3
		std::vector<unsigned char> l_vSwap(m_width * l_ui32BytesInPixel);
		unsigned char* l_pSwap    = &l_vSwap[0];
		unsigned char* l_pSource1 = cairo_image_surface_get_data(l_pCairoSurface);
		unsigned char* l_pSource2 = cairo_image_surface_get_data(l_pCairoSurface) + m_width * (m_height - 1) * l_ui32BytesInPixel;
		for (uint32_t i = 0; i < m_height / 2; ++i)
		{
			System::Memory::copy(l_pSwap, l_pSource1, m_width * l_ui32BytesInPixel);
			System::Memory::copy(l_pSource1, l_pSource2, m_width * l_ui32BytesInPixel);
			System::Memory::copy(l_pSource2, l_pSwap, m_width * l_ui32BytesInPixel);
			l_pSource1 += m_width * l_ui32BytesInPixel;
			l_pSource2 -= m_width * l_ui32BytesInPixel;
		}

		// Pixels are ready to save
		cairo_surface_write_to_png(l_pCairoSurface, l_sFilename);
		cairo_surface_destroy(l_pCairoSurface);
	}
}

void CBoxAlgorithmViz::draw() { }

void CBoxAlgorithmViz::drawLeft() { if (m_pRuler != nullptr) { m_pRuler->doRenderLeft(m_pLeft); } }

void CBoxAlgorithmViz::drawRight() { if (m_pRuler != nullptr) { m_pRuler->doRenderRight(m_pRight); } }

void CBoxAlgorithmViz::drawBottom() { if (m_pRuler != nullptr) { m_pRuler->doRenderBottom(m_pBottom); } }

void CBoxAlgorithmViz::mouseButton(const int x, const int y, const int button, const int status)
{
#if 0
	// Mouse interacts with local renderer context
	m_oMouseHandler.mouseButton(*m_pRendererContext, x, y, button, status);
#else
	// Mouse interacts with global renderer context
	m_oMouseHandler.mouseButton(getContext(), x, y, button, status);
#endif

	this->redraw();
}

void CBoxAlgorithmViz::mouseMotion(const int x, const int y)
{
#if 0
	// Mouse interacts with local renderer context
	m_oMouseHandler.mouseMotion(*m_pRendererContext, x, y);
#else
	// Mouse interacts with global renderer context
	m_oMouseHandler.mouseMotion(getContext(), x, y);
#endif

	if (m_oMouseHandler.hasButtonPressed()) { this->redraw(); }
}

void CBoxAlgorithmViz::keyboard(const int x, const int y, const uint32_t key, const bool status)
{
	printf("keyboard : x=%i y=%i key=%u status=%s", x, y, key, status ? "pressed" : "released");
}

void CBoxAlgorithmViz::parseColor(TColor& rColor, const std::string& sColor)
{
	float r, g, b;
	if (sscanf(sColor.c_str(), "%f,%f,%f", &r, &g, &b) == 3)
	{
		rColor.r = r * .01f;
		rColor.g = g * .01f;
		rColor.b = b * .01f;
	}
	else
	{
		rColor.r = 1;
		rColor.g = 1;
		rColor.b = 1;
	}
}
