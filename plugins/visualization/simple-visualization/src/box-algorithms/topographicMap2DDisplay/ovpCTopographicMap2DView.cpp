#include "ovpCTopographicMap2DView.h"

#include <iostream>

#include <sstream>

#include <cmath>
#include <cstring>

#ifdef TARGET_OS_Windows
#ifndef NDEBUG
#include <cassert>
#endif
#endif

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace SimpleVisualization;

using namespace OpenViBEToolkit;

using namespace std;

//#define INTERPOLATE_AT_CHANNEL_LOCATION

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		static const int s_nbColors = 13;
		static GdkColor s_palette[13];
		static uint8_t s_palette8[13 * 3];

		static gboolean redrawCallback(GtkWidget* widget, GdkEventExpose* event, gpointer data);
		static gboolean resizeCallback(GtkWidget* widget, GtkAllocation* allocation, gpointer data);
		static void toggleElectrodesCallback(GtkWidget* widget, gpointer data);
		static void setProjectionCallback(GtkWidget* widget, gpointer data);
		static void setViewCallback(GtkWidget* widget, gpointer data);
		static void setInterpolationCallback(GtkWidget* widget, gpointer data);
		static void setDelayCallback(GtkRange* range, gpointer data);

		CTopographicMap2DView::CTopographicMap2DView(CTopographicMapDatabase& rTopographicMapDatabase, const uint64_t ui64DefaultInterpolation, double delay)
			: m_topographicMapDatabase(rTopographicMapDatabase), m_currentInterpolation(ui64DefaultInterpolation)
		{
			m_sampleCoordinatesMatrix.setDimensionCount(2);

			//load the gtk builder interface
			m_builderInterface = gtk_builder_new();
			gtk_builder_add_from_file(m_builderInterface, Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-TopographicMap2D.ui", nullptr);

			if (m_builderInterface == nullptr)
			{
				g_warning("Couldn't load the interface!");
				return;
			}

			gtk_builder_connect_signals(m_builderInterface, nullptr);

			m_backgroundColor.pixel = 0;
			m_backgroundColor.red = 0xFFFF;
			m_backgroundColor.green = 0;//0xFFFF;
			m_backgroundColor.blue = 0;//0xFFFF;

			//toolbar
			//-------

			//get pointers to projection mode buttons
			m_axialProjectionButton = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "AxialProjection"));
			m_radialProjectionButton = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "RadialProjection"));

			g_signal_connect(G_OBJECT(m_axialProjectionButton), "toggled", G_CALLBACK(setProjectionCallback), this);
			g_signal_connect(G_OBJECT(m_radialProjectionButton), "toggled", G_CALLBACK(setProjectionCallback), this);

			//get pointers to view buttons
			m_topViewButton = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "TopView"));
			m_leftViewButton = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "LeftView"));
			m_rightViewButton = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "RightView"));
			m_backViewButton = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "BackView"));

			g_signal_connect(G_OBJECT(m_topViewButton), "toggled", G_CALLBACK(setViewCallback), this);
			g_signal_connect(G_OBJECT(m_leftViewButton), "toggled", G_CALLBACK(setViewCallback), this);
			g_signal_connect(G_OBJECT(m_rightViewButton), "toggled", G_CALLBACK(setViewCallback), this);
			g_signal_connect(G_OBJECT(m_backViewButton), "toggled", G_CALLBACK(setViewCallback), this);

			//get pointers to interpolation type buttons
			m_mapPotentials = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "MapPotentials"));
			m_mapCurrents = GTK_RADIO_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "MapCurrents"));

			g_signal_connect(G_OBJECT(m_mapPotentials), "toggled", G_CALLBACK(setInterpolationCallback), this);
			g_signal_connect(G_OBJECT(m_mapCurrents), "toggled", G_CALLBACK(setInterpolationCallback), this);

			//get pointer to electrodes toggle button
			m_electrodesToggleButton = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_builderInterface, "ToggleElectrodes"));

			g_signal_connect(G_OBJECT(m_electrodesToggleButton), "toggled", G_CALLBACK(toggleElectrodesCallback), this);

			//tell database about maximum delay
			m_topographicMapDatabase.adjustNumberOfDisplayedBuffers(m_maxDelay);
			//ensure default delay lies in [0, m_maxDelay]
			if (delay > m_maxDelay) { delay = m_maxDelay; }
			//set default delay
			setDelayCB(delay);
			//configure delay slider
			GtkWidget* l_pDelayScale = gtk_hscale_new_with_range(0.0, m_maxDelay, 0.1);
			gtk_range_set_value(GTK_RANGE(l_pDelayScale), delay);
			gtk_scale_set_value_pos(GTK_SCALE(l_pDelayScale), GTK_POS_TOP);
			gtk_range_set_update_policy(GTK_RANGE(l_pDelayScale), GTK_UPDATE_CONTINUOUS);
			gtk_widget_set_size_request(l_pDelayScale, 100, -1);
			gtk_widget_show_all(l_pDelayScale);
			g_signal_connect(G_OBJECT(l_pDelayScale), "value_changed", G_CALLBACK(setDelayCallback), this);

			//replace existing scale (which somehow can't be used) with the newly created one
			GtkWidget* l_pOldScale = GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "DelayScale"));
			GtkWidget* l_pScaleParent = gtk_widget_get_parent(l_pOldScale);
			if (l_pScaleParent != nullptr && GTK_IS_CONTAINER(l_pScaleParent))
			{
				gtk_container_remove(GTK_CONTAINER(l_pScaleParent), l_pOldScale);
				if (GTK_IS_BOX(l_pScaleParent))
				{
					gtk_box_pack_start(GTK_BOX(l_pScaleParent), l_pDelayScale, TRUE, TRUE, 0);
					gtk_box_reorder_child(GTK_BOX(l_pScaleParent), l_pDelayScale, 0);
				}
			}

			//color palettes
			s_palette[0].red = 255 * 65535 / 255;
			s_palette[0].green = 0 * 65535 / 255;
			s_palette[0].blue = 0 * 65535 / 255;
			s_palette[1].red = 234 * 65535 / 255;
			s_palette[1].green = 1 * 65535 / 255;
			s_palette[1].blue = 0 * 65535 / 255;
			s_palette[2].red = 205 * 65535 / 255;
			s_palette[2].green = 0 * 65535 / 255;
			s_palette[2].blue = 101 * 65535 / 255;
			s_palette[3].red = 153 * 65535 / 255;
			s_palette[3].green = 0 * 65535 / 255;
			s_palette[3].blue = 178 * 65535 / 255;
			s_palette[4].red = 115 * 65535 / 255;
			s_palette[4].green = 1 * 65535 / 255;
			s_palette[4].blue = 177 * 65535 / 255;
			s_palette[5].red = 77 * 65535 / 255;
			s_palette[5].green = 0 * 65535 / 255;
			s_palette[5].blue = 178 * 65535 / 255;
			s_palette[6].red = 0 * 65535 / 255;
			s_palette[6].green = 0 * 65535 / 255;
			s_palette[6].blue = 152 * 65535 / 255;
			s_palette[7].red = 0 * 65535 / 255;
			s_palette[7].green = 97 * 65535 / 255;
			s_palette[7].blue = 121 * 65535 / 255;
			s_palette[8].red = 0 * 65535 / 255;
			s_palette[8].green = 164 * 65535 / 255;
			s_palette[8].blue = 100 * 65535 / 255;
			s_palette[9].red = 0 * 65535 / 255;
			s_palette[9].green = 225 * 65535 / 255;
			s_palette[9].blue = 25 * 65535 / 255;
			s_palette[10].red = 150 * 65535 / 255;
			s_palette[10].green = 255 * 65535 / 255;
			s_palette[10].blue = 0 * 65535 / 255;
			s_palette[11].red = 200 * 65535 / 255;
			s_palette[11].green = 255 * 65535 / 255;
			s_palette[11].blue = 0 * 65535 / 255;
			s_palette[12].red = 255 * 65535 / 255;
			s_palette[12].green = 255 * 65535 / 255;
			s_palette[12].blue = 0 * 65535 / 255;

			s_palette8[0] = 255;
			s_palette8[1] = 0;
			s_palette8[2] = 0;
			s_palette8[3] = 234;
			s_palette8[4] = 1;
			s_palette8[5] = 0;
			s_palette8[6] = 205;
			s_palette8[7] = 0;
			s_palette8[8] = 101;
			s_palette8[9] = 153;
			s_palette8[10] = 0;
			s_palette8[11] = 178;
			s_palette8[12] = 115;
			s_palette8[13] = 1;
			s_palette8[14] = 177;
			s_palette8[15] = 77;
			s_palette8[16] = 0;
			s_palette8[17] = 178;
			s_palette8[18] = 0;
			s_palette8[19] = 0;
			s_palette8[20] = 152;
			s_palette8[21] = 0;
			s_palette8[22] = 97;
			s_palette8[23] = 121;
			s_palette8[24] = 0;
			s_palette8[25] = 164;
			s_palette8[26] = 100;
			s_palette8[27] = 0;
			s_palette8[28] = 225;
			s_palette8[29] = 25;
			s_palette8[30] = 150;
			s_palette8[31] = 255;
			s_palette8[32] = 0;
			s_palette8[33] = 200;
			s_palette8[34] = 255;
			s_palette8[35] = 0;
			s_palette8[36] = 255;
			s_palette8[37] = 255;
			s_palette8[38] = 0;
		}

		CTopographicMap2DView::~CTopographicMap2DView()
		{
			//destroy clip mask
			if (m_clipmask != nullptr)
			{
				g_object_unref(m_clipmask);
				m_clipmask = nullptr;
			}
			if (m_clipmaskGC != nullptr)
			{
				g_object_unref(m_clipmaskGC);
				m_clipmaskGC = nullptr;
			}
			//destroy visible region
			if (m_visibleRegion != nullptr)
			{
				gdk_region_destroy(m_visibleRegion);
				m_visibleRegion = nullptr;
			}
			//destroy pixmap
			if (m_skullRGBBuffer != nullptr)
			{
				delete m_skullRGBBuffer;
				m_skullRGBBuffer = nullptr;
			}

			//unref the xml file as it's not needed anymore
			g_object_unref(G_OBJECT(m_builderInterface));
			m_builderInterface = nullptr;
		}

		void CTopographicMap2DView::init()
		{
			m_drawingArea = GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "TopographicMap2DDrawingArea"));

			gtk_widget_set_double_buffered(m_drawingArea, TRUE);

			g_signal_connect(G_OBJECT(m_drawingArea), "expose_event", G_CALLBACK(redrawCallback), this);
			g_signal_connect(G_OBJECT(m_drawingArea), "size-allocate", G_CALLBACK(resizeCallback), this);

			gtk_widget_show(m_drawingArea);

			//set radial projection by default
			m_currentProjection = TopographicMap2DProjection_Radial;
			enableProjectionButtonSignals(false);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_radialProjectionButton), TRUE);
			enableProjectionButtonSignals(true);

			//set top view by default
			m_currentView = TopographicMap2DView_Top;
			enableViewButtonSignals(false);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_topViewButton), TRUE);
			enableViewButtonSignals(true);

			//reflect default interpolation type
			m_topographicMapDatabase.setInterpolationType(m_currentInterpolation);
			enableInterpolationButtonSignals(false);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_mapPotentials), static_cast<gboolean>(m_currentInterpolation == OVP_TypeId_SphericalLinearInterpolationType_Spline));
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_mapCurrents), static_cast<gboolean>(m_currentInterpolation == OVP_TypeId_SphericalLinearInterpolationType_Laplacian));
			enableInterpolationButtonSignals(true);

			//hide electrodes by default
			m_electrodesToggledOn = false;
			enableElectrodeButtonSignals(false);
			gtk_toggle_tool_button_set_active(m_electrodesToggleButton, static_cast<gboolean>(m_electrodesToggledOn));
			enableElectrodeButtonSignals(true);

			//recompute sample points coordinates
			m_needResize = true;
		}

		void CTopographicMap2DView::redraw()
		{
			if (m_drawingArea != nullptr && GTK_WIDGET_VISIBLE(m_drawingArea))
			{
				if (m_needResize) { resizeData(); }

				//draw face
				drawFace(0, 0, m_headWindowWidth, m_headWindowHeight);

				//draw head
				drawHead();

				//draw palette
				drawPalette(0, m_headWindowHeight, m_paletteWindowWidth, m_paletteWindowHeight);

				//don't clear screen at every redraw, it introduces major flickering
				//gdk_window_invalidate_rect(m_drawingArea->window, nullptr, true);
			}
		}

		void CTopographicMap2DView::getWidgets(GtkWidget* & pWidget, GtkWidget* & pToolbarWidget) const
		{
			pWidget = GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "TopographicMap2DDrawingArea"));
			pToolbarWidget = GTK_WIDGET(gtk_builder_get_object(m_builderInterface, "Toolbar"));
		}

		CTopographicMap2DView::ETopographicMap2DView CTopographicMap2DView::getCurrentView() const { return m_currentView; }

		CMatrix* CTopographicMap2DView::getSampleCoordinatesMatrix()
		{
			if (m_needResize) { resizeData(); }
			return &m_sampleCoordinatesMatrix;
		}

		bool CTopographicMap2DView::setSampleValuesMatrix(IMatrix* pSampleValuesMatrix)
		{
			//ensure matrix has the right size
			if (pSampleValuesMatrix == nullptr || pSampleValuesMatrix->getBufferElementCount() < m_sampleValues.size()) { return false; }

			//retrieve min/max potentials
			double l_f64MinPotential, l_f64MaxPotential;
			m_topographicMapDatabase.getLastBufferInterpolatedMinMaxValue(l_f64MinPotential, l_f64MaxPotential);

			const int32_t l_colorStartIndex = 0;
			const int32_t l_colorEndIndex = s_nbColors - 1;

			double l_f64InvPotentialStep = 0;

			if (l_f64MinPotential < l_f64MaxPotential)
			{
				l_f64InvPotentialStep = (l_colorEndIndex - l_colorStartIndex + 1) / (l_f64MaxPotential - l_f64MinPotential);
			}

			//determine color index of each sample
			for (uint32_t i = 0; i < m_sampleValues.size(); ++i)
			{
				const double value = *(pSampleValuesMatrix->getBuffer() + i);
				int32_t l_iColorIndex;

				if (value < l_f64MinPotential) { l_iColorIndex = 0; }
				else if (value > l_f64MaxPotential) { l_iColorIndex = s_nbColors - 1; }
				else //linear itp
				{
					l_iColorIndex = l_colorStartIndex + int32_t((value - l_f64MinPotential) * l_f64InvPotentialStep);
					if (l_iColorIndex > s_nbColors - 1) { l_iColorIndex = s_nbColors - 1; }
				}
				m_sampleValues[i] = l_iColorIndex;
			}

			refreshPotentials();

			//force redraw

			return true;
		}

		void CTopographicMap2DView::resizeCB(uint32_t /*ui32Width*/, uint32_t /*ui32Height*/) { m_needResize = true; }

		void CTopographicMap2DView::toggleElectrodesCB()
		{
			m_electrodesToggledOn = !m_electrodesToggledOn;

			if (!m_electrodesToggledOn)
			{
				//clear screen so that electrode labels are hidden
				if (m_drawingArea->window != nullptr) { gdk_window_invalidate_rect(m_drawingArea->window, nullptr, 1); }
			}
		}

		void CTopographicMap2DView::setProjectionCB(GtkWidget* pWidget)
		{
			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pWidget)) == FALSE) { return; }

			if (pWidget == GTK_WIDGET(m_axialProjectionButton))
			{
				m_currentProjection = TopographicMap2DProjection_Axial;
			}
			else if (pWidget == GTK_WIDGET(m_radialProjectionButton))
			{
				m_currentProjection = TopographicMap2DProjection_Radial;
			}

			//recompute sample points coordinates
			m_needResize = true;

			//clear screen
			if (m_drawingArea->window != nullptr) { gdk_window_invalidate_rect(m_drawingArea->window, nullptr, 1); }
		}

		void CTopographicMap2DView::setViewCB(GtkWidget* pWidget)
		{
			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pWidget)) == FALSE) { return; }

			if (pWidget == GTK_WIDGET(m_topViewButton)) { m_currentView = TopographicMap2DView_Top; }
			else if (pWidget == GTK_WIDGET(m_leftViewButton)) { m_currentView = TopographicMap2DView_Left; }
			else if (pWidget == GTK_WIDGET(m_rightViewButton)) { m_currentView = TopographicMap2DView_Right; }
			else if (pWidget == GTK_WIDGET(m_backViewButton)) { m_currentView = TopographicMap2DView_Back; }

			//recompute sample points coordinates, update clipmask
			m_needResize = true;

			//clear screen
			if (m_drawingArea->window != nullptr) { gdk_window_invalidate_rect(m_drawingArea->window, nullptr, 1); }
		}

		void CTopographicMap2DView::setInterpolationCB(GtkWidget* pWidget)
		{
			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pWidget)) == FALSE) { return; }

			if (pWidget == GTK_WIDGET(m_mapPotentials))
			{
				m_currentInterpolation = OVP_TypeId_SphericalLinearInterpolationType_Spline;
				m_topographicMapDatabase.setInterpolationType(OVP_TypeId_SphericalLinearInterpolationType_Spline);
			}
			else if (pWidget == GTK_WIDGET(m_mapCurrents))
			{
				m_currentInterpolation = OVP_TypeId_SphericalLinearInterpolationType_Laplacian;
				m_topographicMapDatabase.setInterpolationType(OVP_TypeId_SphericalLinearInterpolationType_Laplacian);
			}

			//recompute sample points coordinates
			m_needResize = true;
		}

		void CTopographicMap2DView::setDelayCB(const double f64Delay) const { m_topographicMapDatabase.setDelay(f64Delay); }

		void CTopographicMap2DView::drawPalette(const uint32_t ui32X, const uint32_t ui32Y, const uint32_t ui32Width, const uint32_t ui32Height) const
		{
			if (ui32Width == 0 || ui32Height == 0) { return; }

			// FIXME is it necessary to keep next line uncomment ?
			//bool l_bDrawText = true;

			//retrieve text size
			PangoLayout* l_pText = gtk_widget_create_pango_layout(GTK_WIDGET(m_drawingArea), "0");
			gint textHeight;
			pango_layout_get_pixel_size(l_pText, nullptr, &textHeight);

			//don't draw text if not enough room
			if (textHeight >= gint(ui32Height - m_minPaletteBarHeight))
			{
				// FIXME is it necessary to keep next line uncomment ?
				//l_bDrawText = false;
			}
			//determine palette bar dims
			const gint l_iPaletteBarWidth = gint(0.9 * ui32Width);
			gint l_iPaletteBarHeight = gint(ui32Height - textHeight);
			if (l_iPaletteBarHeight < gint(m_minPaletteBarHeight))
			{
				l_iPaletteBarHeight = gint(m_minPaletteBarHeight);
			}
			else if (l_iPaletteBarHeight > gint(m_maxPaletteBarHeight))
			{
				l_iPaletteBarHeight = gint(m_maxPaletteBarHeight);
			}
			const gint l_iPaletteBarStartX = gint(ui32X + (ui32Width - l_iPaletteBarWidth) / 2);
			const gint l_iPaletteBarStartY = gint(ui32Y);

			gint textWidth;
			const gint l_iLabelY = l_iPaletteBarStartY + l_iPaletteBarHeight + (ui32Height - l_iPaletteBarHeight - textHeight) / 2;

			//draw 0 label
			pango_layout_get_pixel_size(l_pText, &textWidth, nullptr);
			gint l_iLabelX = ui32X + (ui32Width - textWidth) / 2;

			gdk_draw_layout(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], l_iLabelX, l_iLabelY, l_pText);

			//draw + label
			pango_layout_set_text(l_pText, "+", 1);
			pango_layout_get_pixel_size(l_pText, &textWidth, nullptr);
			l_iLabelX = l_iPaletteBarStartX - textWidth / 2;
			if (l_iLabelX < 0)
			{
				l_iLabelX = 0;
			}
			gdk_draw_layout(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], l_iLabelX, l_iLabelY, l_pText);

			//draw - label
			pango_layout_set_text(l_pText, "-", 1);
			pango_layout_get_pixel_size(l_pText, &textWidth, nullptr);
			l_iLabelX = l_iPaletteBarStartX + l_iPaletteBarWidth - textWidth / 2;
			if (l_iLabelX + textWidth >= gint(ui32Width))
			{
				l_iLabelX = ui32Width - textWidth;
			}
			gdk_draw_layout(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							l_iLabelX, l_iLabelY, l_pText);

			//draw palette bar (typically reversed : high potentials to the left; low potentials to the right)
			gint l_iCurrentX = l_iPaletteBarStartX;

			for (int i = s_nbColors - 1; i >= 0; i--)
			{
				gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &s_palette[i]);

				gdk_draw_rectangle(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
								   TRUE, l_iCurrentX, l_iPaletteBarStartY, l_iPaletteBarWidth / s_nbColors, l_iPaletteBarHeight);

				l_iCurrentX += l_iPaletteBarWidth / 13;
			}

			//restore default black color
			GdkColor l_oBlack;
			l_oBlack.red = l_oBlack.green = l_oBlack.blue = 0;
			gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &l_oBlack);

			//delete pango layout
			g_object_unref(l_pText);
		}

		void CTopographicMap2DView::drawFace(uint32_t /*ui32X*/, uint32_t /*ui32Y*/, uint32_t /*ui32Width*/, uint32_t /*ui32Height*/) const
		{
			gdk_gc_set_line_attributes(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], 2, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			//head center
			const uint32_t skullCenterX = m_skullX + m_skullDiameter / 2;
			const uint32_t skullCenterY = m_skullY + m_skullDiameter / 2;

#ifndef M_PI
#	define M_PI 3.1415926535897932384626433832795
#endif
#define DEG2RAD(x) ((x)*M_PI/180.0)

			if (m_currentView == TopographicMap2DView_Top)
			{
				const float l_f32NoseHalfAngle = 6;

				//nose lower left anchor
				const uint32_t l_ui32NoseLowerLeftX = uint32_t(skullCenterX + 1.0 * m_skullDiameter / 2 * cos(DEG2RAD(90 + l_f32NoseHalfAngle)));
				const uint32_t l_ui32NoseLowerLeftY = uint32_t(skullCenterY - 1.0 * m_skullDiameter / 2 * sin(DEG2RAD(90 + l_f32NoseHalfAngle)));

				//nose lower right anchor
				const auto l_ui32NoseLowerRightX = uint32_t(skullCenterX + 1.0 * m_skullDiameter / 2 * cos(DEG2RAD(90 - l_f32NoseHalfAngle)));
				const auto l_ui32NoseLowerRightY = uint32_t(skullCenterY - 1.0 * m_skullDiameter / 2 * sin(DEG2RAD(90 - l_f32NoseHalfAngle)));

				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(l_ui32NoseLowerLeftX), gint(l_ui32NoseLowerLeftY), gint(skullCenterX), gint(m_noseY));

				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(l_ui32NoseLowerRightX), gint(l_ui32NoseLowerRightY), gint(skullCenterX), gint(m_noseY));
			}
			else if (m_currentView == TopographicMap2DView_Back)
			{
				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(m_skullOutlineLeftPointX), gint(m_skullOutlineLeftPointY), gint(m_leftNeckX), gint(m_leftNeckY));

				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(m_skullOutlineRightPointX), gint(m_skullOutlineRightPointY), gint(m_rightNeckX), gint(m_rightNeckY));
			}
			else if (m_currentView == TopographicMap2DView_Left || m_currentView == TopographicMap2DView_Right)
			{
				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(m_noseTopX), gint(m_noseTopY), gint(m_noseBumpX), gint(m_noseBumpY));

				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(m_noseBumpX), gint(m_noseBumpY), gint(m_noseTipX), gint(m_noseTipY));

				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(m_noseTipX), gint(m_noseTipY), gint(m_noseBaseX), gint(m_noseBaseY));

				gdk_draw_line(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
							  gint(m_noseBaseX), gint(m_noseBaseY), gint(m_noseBottomX), gint(m_noseBottomY));
			}
		}

		void CTopographicMap2DView::drawHead() const
		{
			//draw head outline
			gdk_gc_set_line_attributes(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], 2, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);
			gdk_draw_arc(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
						 FALSE, gint(m_skullX), gint(m_skullY),
						 gint(m_skullDiameter), gint(m_skullDiameter),
						 gint(64 * m_skullOutlineStartAngle), gint(64 * (m_skullOutlineEndAngle - m_skullOutlineStartAngle)));

			gdk_gc_set_line_attributes(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			gdk_gc_set_clip_origin(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], gint(m_skullX), gint(m_skullY));

			gdk_gc_set_clip_mask(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], m_clipmask);

			drawPotentials();

			drawElectrodes();

			gdk_gc_set_clip_mask(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], nullptr);
		}

		void CTopographicMap2DView::resizeData()
		{
			//window size
			const uint32_t l_iWindowWidth = m_drawingArea->allocation.width;
			const uint32_t l_iWindowHeight = m_drawingArea->allocation.height;

			//retrieve text size
			PangoLayout* l_pText = gtk_widget_create_pango_layout(GTK_WIDGET(m_drawingArea), "0");
			int textHeight;
			pango_layout_get_pixel_size(l_pText, nullptr, &textHeight);
			g_object_unref(l_pText);
			l_pText = nullptr;

			//palette sub-window dims
			m_paletteWindowWidth = l_iWindowWidth;
			m_paletteWindowHeight = uint32_t(0.1 * l_iWindowHeight);
			if (m_paletteWindowHeight > uint32_t(m_maxPaletteBarHeight + textHeight + 4))
			{
				m_paletteWindowHeight = m_maxPaletteBarHeight + textHeight + 4;
			}
			else if (m_paletteWindowHeight < uint32_t(m_minPaletteBarHeight + textHeight))
			{
				m_paletteWindowHeight = uint32_t(m_minPaletteBarHeight + textHeight);
			}

			//return if not enough room available
			if (m_paletteWindowHeight > l_iWindowHeight) { return; }

			//head sub-window dims
			m_headWindowWidth = l_iWindowWidth;
			m_headWindowHeight = l_iWindowHeight - m_paletteWindowHeight;

			uint32_t l_ui32HeadMaxSize;
			if (m_headWindowWidth < m_headWindowHeight)
			{
				l_ui32HeadMaxSize = uint32_t(0.9 * m_headWindowWidth);
			}
			else
			{
				l_ui32HeadMaxSize = uint32_t(0.9 * m_headWindowHeight);
			}

			if (m_currentView == TopographicMap2DView_Top)
			{
				//height used up by nose
				const auto l_ui32NoseProtrudingHeight = uint32_t(0.1 * l_ui32HeadMaxSize);
				//Y coordinate where nose starts
				m_noseY = uint32_t((m_headWindowHeight - l_ui32HeadMaxSize) / 2);
				//skull diameter
				m_skullDiameter = l_ui32HeadMaxSize - l_ui32NoseProtrudingHeight;
				//skull upper left corner
				m_skullX = (m_headWindowWidth - m_skullDiameter) / 2;
				m_skullY = m_noseY + l_ui32NoseProtrudingHeight;

				//skull outline and filled area start/end angles
				m_skullOutlineStartAngle = 0;
				m_skullOutlineEndAngle = 360;
				m_skullFillStartAngle = 0;
				m_skullFillEndAngle = 360;

				//clip mask
				m_clipmaskWidth = m_skullDiameter;
				m_clipmaskHeight = m_skullDiameter;
			}
			else if (m_currentView == TopographicMap2DView_Back)
			{
				//FIXME take into account width used up by ears

				//height used up by neck
				const uint32_t l_ui32NeckProtrudingHeight = uint32_t(0.072 * l_ui32HeadMaxSize);

				//skull diameter
				m_skullDiameter = l_ui32HeadMaxSize - l_ui32NeckProtrudingHeight;
				//skull upper left corner
				m_skullX = (m_headWindowWidth - m_skullDiameter) / 2;
				m_skullY = (m_headWindowHeight - l_ui32HeadMaxSize) / 2;

				//skull outline and filled area start/end angles
				m_skullOutlineStartAngle = -38;
				m_skullOutlineEndAngle = 180 - m_skullOutlineStartAngle;
				m_skullFillStartAngle = -30;
				m_skullFillEndAngle = 180 - m_skullFillStartAngle;

				const uint32_t skullCenterX = m_skullX + m_skullDiameter / 2;
				const uint32_t skullCenterY = m_skullY + m_skullDiameter / 2;

				m_skullOutlineLeftPointX = skullCenterX + uint32_t(1.0 * m_skullDiameter / 2 * cosf(float(DEG2RAD(m_skullOutlineEndAngle))));
				m_skullOutlineLeftPointY = skullCenterY - uint32_t(1.0 * m_skullDiameter / 2 * sinf(float(DEG2RAD(m_skullOutlineEndAngle))));
				m_skullOutlineRightPointX = skullCenterX + uint32_t(1.0 * m_skullDiameter / 2 * cosf(float(DEG2RAD(m_skullOutlineStartAngle))));
				m_skullOutlineRightPointY = skullCenterY - uint32_t(1.0 * m_skullDiameter / 2 * sinf(float(DEG2RAD(m_skullOutlineStartAngle))));

				m_skullFillLeftPointX = skullCenterX + uint32_t(1.0 * m_skullDiameter / 2 * cosf(float(DEG2RAD(m_skullFillEndAngle))));
				m_skullFillLeftPointY = skullCenterY - uint32_t(1.0 * m_skullDiameter / 2 * sinf(float(DEG2RAD(m_skullFillEndAngle))));
				m_skullFillRightPointX = skullCenterX + uint32_t(1.0 * m_skullDiameter / 2 * cosf(float(DEG2RAD(m_skullFillStartAngle))));
				m_skullFillRightPointY = skullCenterY - uint32_t(1.0 * m_skullDiameter / 2 * sinf(float(DEG2RAD(m_skullFillStartAngle))));
				m_skullFillBottomPointX = m_skullX + m_skullDiameter / 2;
				m_skullFillBottomPointY = m_skullFillRightPointY;

				//neck extremities
				m_leftNeckX = m_skullOutlineLeftPointX + uint32_t(0.025f * m_skullDiameter);
				m_leftNeckY = m_skullOutlineLeftPointY + l_ui32NeckProtrudingHeight;
				m_rightNeckX = m_skullOutlineRightPointX - uint32_t(0.025f * m_skullDiameter);
				m_rightNeckY = m_leftNeckY;

				//clip mask
				m_clipmaskWidth = m_skullDiameter;
				m_clipmaskHeight = m_skullFillBottomPointY - m_skullY + 1;
			}
			else if (m_currentView == TopographicMap2DView_Left || m_currentView == TopographicMap2DView_Right)
			{
				//width used up by nose
				const auto l_ui32NoseProtrudingWidth = uint32_t(0.06 * m_skullDiameter);//uint32_t(0.047 * m_skullDiameter);

				//skull diameter
				m_skullDiameter = l_ui32HeadMaxSize - l_ui32NoseProtrudingWidth;

				//topmost skull coordinate
				m_skullY = (m_headWindowHeight - m_skullDiameter) / 2;

				if (m_currentView == TopographicMap2DView_Left)
				{
					//X coordinate of nose tip
					m_noseTipX = (m_headWindowWidth - l_ui32HeadMaxSize) / 2;
					//leftmost skull coordinate
					m_skullX = m_noseTipX + l_ui32NoseProtrudingWidth;
					//skull outline and filled area start/end angles
					m_skullOutlineStartAngle = -41;
					m_skullOutlineEndAngle = 193;//194;
					m_skullFillStartAngle = -22;
					m_skullFillEndAngle = 188;

					const uint32_t skullCenterX = m_skullX + m_skullDiameter / 2;
					const uint32_t skullCenterY = m_skullY + m_skullDiameter / 2;

					//nose top = head outline left boundary
					m_noseTopX = skullCenterX + uint32_t(float(m_skullDiameter) / 2 * cosf(float(DEG2RAD(m_skullOutlineEndAngle))));
					m_noseTopY = skullCenterY - uint32_t(float(m_skullDiameter) / 2 * sinf(float(DEG2RAD(m_skullOutlineEndAngle))));
					//nose bump
					m_noseBumpX = m_noseTipX;
					m_noseBumpY = m_noseTopY + uint32_t(0.15f * m_skullDiameter);//uint32_t(0.179f * m_skullDiameter);
					//nose tip
					//m_noseTipX = m_noseBumpX;
					m_noseTipY = m_noseBumpY + uint32_t(0.03f * m_skullDiameter);//uint32_t(0.021f * m_skullDiameter);
					//nose base
					m_noseBaseX = m_noseTipX + uint32_t(0.1f * m_skullDiameter);
					m_noseBaseY = m_noseTipY;
					//nose bottom
					m_noseBottomX = m_noseBaseX;
					m_noseBottomY = m_noseBaseY + uint32_t(0.02f * m_skullDiameter);//uint32_t(0.016f * m_skullDiameter);
				}
				else
				{
					//X coordinate of nose tip
					m_noseTipX = (m_headWindowWidth + l_ui32HeadMaxSize) / 2;
					//leftmost skull coordinate
					m_skullX = (m_headWindowWidth - l_ui32HeadMaxSize) / 2;
					//skull outline and filled area start/end angles
					m_skullOutlineStartAngle = -13; //-14;
					m_skullOutlineEndAngle = 221;
					m_skullFillStartAngle = -8;
					m_skullFillEndAngle = 202;

					const uint32_t skullCenterX = m_skullX + m_skullDiameter / 2;
					const uint32_t skullCenterY = m_skullY + m_skullDiameter / 2;

					//nose top = head outline right boundary
					m_noseTopX = skullCenterX + uint32_t(float(m_skullDiameter) / 2 * cosf(float(DEG2RAD(m_skullOutlineStartAngle))));
					m_noseTopY = skullCenterY - uint32_t(float(m_skullDiameter) / 2 * sinf(float(DEG2RAD(m_skullOutlineStartAngle))));
					//nose bump
					m_noseBumpX = m_noseTipX;
					m_noseBumpY = m_noseTopY + uint32_t(0.15f * m_skullDiameter);//uint32_t(0.179f * m_skullDiameter);
					//nose tip
					//m_noseTipX = m_noseBumpX;
					m_noseTipY = m_noseBumpY + uint32_t(0.03f * m_skullDiameter);//uint32_t(0.021f * m_skullDiameter);
					//nose base
					m_noseBaseX = m_noseTipX - uint32_t(0.1f * m_skullDiameter);
					m_noseBaseY = m_noseTipY;
					//nose bottom
					m_noseBottomX = m_noseBaseX;
					m_noseBottomY = m_noseBaseY + uint32_t(0.02f * m_skullDiameter);//uint32_t(0.016f * m_skullDiameter);
				}

				const uint32_t skullCenterX = m_skullX + m_skullDiameter / 2;
				const uint32_t skullCenterY = m_skullY + m_skullDiameter / 2;
				m_skullFillLeftPointX = skullCenterX + uint32_t(float(m_skullDiameter) / 2 * cosf(float(DEG2RAD(m_skullFillEndAngle))));
				m_skullFillLeftPointY = skullCenterY - uint32_t(float(m_skullDiameter) / 2 * sinf(float(DEG2RAD(m_skullFillEndAngle))));
				m_skullFillRightPointX = skullCenterX + uint32_t(float(m_skullDiameter) / 2 * cosf(float(DEG2RAD(m_skullFillStartAngle))));
				m_skullFillRightPointY = skullCenterY - uint32_t(float(m_skullDiameter) / 2 * sinf(float(DEG2RAD(m_skullFillStartAngle))));

				m_skullFillBottomPointX = m_skullX + m_skullDiameter / 2;
				m_skullFillBottomPointY = m_skullY + uint32_t(0.684f * m_skullDiameter);

				//clip mask
				m_clipmaskWidth = m_skullDiameter;
				m_clipmaskHeight = m_skullFillBottomPointY - m_skullY + 1;
			}

			//free existing clipmask, if any
			if (m_clipmaskGC != nullptr) { g_object_unref(m_clipmaskGC); }
			if (m_clipmask != nullptr) { g_object_unref(m_clipmask); }

			//allocate clipmask
			m_clipmask = gdk_pixmap_new(m_drawingArea->window, m_clipmaskWidth, m_clipmaskHeight, 1);
			m_clipmaskGC = gdk_gc_new(GDK_DRAWABLE(m_clipmask));
			gdk_gc_set_colormap(m_clipmaskGC, gdk_gc_get_colormap(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)]));

			//redraw it
			redrawClipmask();

			//allocate main pixmap
			//TODO!

			//allocate RGB pixmap


			delete[] m_skullRGBBuffer;

			//align lines on 32bit boundaries
			m_rowStride = ((m_skullDiameter * 3) % 4 == 0) ? (m_skullDiameter * 3) : ((((m_skullDiameter * 3) >> 2) + 1) << 2);
			m_skullRGBBuffer = new guchar[m_rowStride * m_skullDiameter];

			//determine size of colored cells
#if 1
			const uint32_t l_ui32CellMinSize = 6;
			const uint32_t l_ui32CellMaxSize = 6;
			const double l_f64CellOverSkullSizeRatio = 0.02;
			m_cellSize = uint32_t(m_skullDiameter * l_f64CellOverSkullSizeRatio);

			if (m_cellSize < l_ui32CellMinSize) { m_cellSize = l_ui32CellMinSize; }
			else if (m_cellSize > l_ui32CellMaxSize) { m_cellSize = l_ui32CellMaxSize; }
#else
			m_cellSize = m_skullDiameter / 2;
#endif
			if (m_cellSize == 0) { return; }

			//number of samples in a row or column
			m_gridSize = uint32_t(ceil(m_skullDiameter / double(m_cellSize)));

			//determine number of samples lying within skull
			const uint32_t l_ui32NbSamples = computeSamplesNormalizedCoordinates(false);

			//resize sample grids accordingly
			m_sample2DCoordinates.resize(l_ui32NbSamples);
			m_sampleCoordinatesMatrix.setDimensionSize(0, l_ui32NbSamples);
			m_sampleCoordinatesMatrix.setDimensionSize(1, 3);
			m_sampleValues.resize(l_ui32NbSamples);

			//compute samples normalized coordinates
			computeSamplesNormalizedCoordinates(true);

			//resizing completed
			m_needResize = false;
		}

		void CTopographicMap2DView::redrawClipmask()
		{
			//clear clipmask by drawing a black rectangle
			GdkColor l_oBlack;
			l_oBlack.red = l_oBlack.green = l_oBlack.blue = 0;
			gdk_gc_set_rgb_fg_color(m_clipmaskGC, &l_oBlack);
			gdk_draw_rectangle(m_clipmask, m_clipmaskGC, TRUE, 0, 0, m_clipmaskWidth, m_clipmaskHeight);

			//draw visible circular region with a white filled arc
			GdkColor l_oWhite;
			l_oWhite.red = l_oWhite.green = l_oWhite.blue = 65535;
			gdk_gc_set_rgb_fg_color(m_clipmaskGC, &l_oWhite);
			gdk_draw_arc(m_clipmask, m_clipmaskGC, TRUE, 0, 0, gint(m_skullDiameter), gint(m_skullDiameter),
						 gint(64 * m_skullFillStartAngle), gint(64 * (m_skullFillEndAngle - m_skullFillStartAngle)));

			//views other than top have an extra non-clipped area
			if (m_currentView == TopographicMap2DView_Left || m_currentView == TopographicMap2DView_Right || m_currentView == TopographicMap2DView_Back)
			{
				//draw polygon : { skullCenter, skullFillStartPoint, skullFillBottomPoint, skullFillEndPoint, skullCenter }
				GdkPoint l_pPolygon[4];
				l_pPolygon[0].x = m_skullX + m_skullDiameter / 2 - m_skullX;
				l_pPolygon[0].y = m_skullY + m_skullDiameter / 2 - m_skullY - 2;
				l_pPolygon[1].x = m_skullFillRightPointX - m_skullX;
				l_pPolygon[1].y = m_skullFillRightPointY - m_skullY - 2;
				l_pPolygon[2].x = m_skullFillBottomPointX - m_skullX;
				l_pPolygon[2].y = m_skullFillBottomPointY - m_skullY - 2;
				l_pPolygon[3].x = m_skullFillLeftPointX - m_skullX;
				l_pPolygon[3].y = m_skullFillLeftPointY - m_skullY - 2;
				gdk_draw_polygon(m_clipmask, m_clipmaskGC, TRUE, l_pPolygon, 4);
			}

			//restore default black color
			gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &l_oBlack);

			//update visible region
			if (m_visibleRegion != nullptr) { gdk_region_destroy(m_visibleRegion); }
			m_visibleRegion = gdk_drawable_get_visible_region(GDK_DRAWABLE(m_clipmask));
		}

		void CTopographicMap2DView::refreshPotentials()
		{
			uint32_t w, h;

#ifdef INTERPOLATE_AT_CHANNEL_LOCATION
			for (uint32_t i = uint32_t(m_topographicMapDatabase.getChannelCount()); i < uint32_t(m_sampleValues.size()); ++i)
#else
			for (uint32_t i = 0; i < m_sampleValues.size(); ++i)
#endif
			{
				//cells of last row and last column may be smaller than other ones
				if (m_sample2DCoordinates[i].first + m_cellSize >= m_skullDiameter)
				{
					w = m_skullDiameter - m_sample2DCoordinates[i].first;
				}
				else
				{
					w = m_cellSize;
				}

				if (m_sample2DCoordinates[i].second + m_cellSize >= m_skullDiameter)
				{
					h = m_skullDiameter - m_sample2DCoordinates[i].second;
				}
				else { h = m_cellSize; }

				uint32_t index = m_sampleValues[i];
				if (index > 12) { index = 12; }

				drawBoxToBuffer(m_sample2DCoordinates[i].first, m_sample2DCoordinates[i].second, w, h, s_palette8[3 * index], s_palette8[3 * index + 1], s_palette8[3 * index + 2]);
			}
		}

		void CTopographicMap2DView::drawPotentials() const
		{
			gdk_draw_rgb_image(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], m_skullX, m_skullY, 
							   m_skullDiameter, m_skullDiameter, GDK_RGB_DITHER_NONE, m_skullRGBBuffer, m_rowStride);
		}

		void CTopographicMap2DView::drawElectrodes() const
		{
			if (!m_electrodesToggledOn) { return; }

			//determine size of electrode rings
			const double l_f64ElectrodeRingOverSkullSizeRatio = 0.05;
			gint electrodeRingSize = gint(m_skullDiameter * l_f64ElectrodeRingOverSkullSizeRatio);

#if 0
			if (electrodeRingSize < (gint)electrodeRingMinSize) { electrodeRingSize = (gint)electrodeRingMinSize; }
			else if (electrodeRingSize > (gint)electrodeRingMaxSize) { electrodeRingSize = (gint)electrodeRingMaxSize; }
#else
			electrodeRingSize = 5;
#endif

			if (electrodeRingSize == 0) { return; }

			GdkColor l_oWhite;
			l_oWhite.red = 65535;
			l_oWhite.green = 65535;
			l_oWhite.blue = 65535;

			GdkColor l_oBlack;
			l_oBlack.red = 0;
			l_oBlack.green = 0;
			l_oBlack.blue = 0;

			//set electrode ring thickness
			const gint electrodeRingThickness = 1;
			gdk_gc_set_line_attributes(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], electrodeRingThickness, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			//electrode label
			CString electrodeLabel;
			PangoLayout* l_pElectrodeLabelLayout = gtk_widget_create_pango_layout(GTK_WIDGET(m_drawingArea), " ");
			gint textHeight, textWidth;
			pango_layout_get_pixel_size(l_pElectrodeLabelLayout, nullptr, &textHeight);

			//draw rings
			const uint32_t channelCount = uint32_t(m_topographicMapDatabase.getChannelCount());
			gint channelX, channelY;

			for (uint32_t i = 0; i < channelCount; ++i)
			{
				if (!getChannel2DPosition(i, channelX, channelY)) { continue; }

#ifdef INTERPOLATE_AT_CHANNEL_LOCATION
				//disk colored according to value interpolated at this channel location
				gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &s_palette[m_sampleValues[i]]);
#else
				//fill ring with white
				gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &l_oWhite);
#endif
				gdk_draw_arc(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], TRUE,
							 channelX - electrodeRingSize / 2, channelY - electrodeRingSize / 2,
							 electrodeRingSize, electrodeRingSize, 0, 64 * 360);

				//ring centered on channel location
				gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &l_oBlack);

				gdk_draw_arc(m_drawingArea->window,
							 m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], FALSE,
							 channelX - electrodeRingSize / 2, channelY - electrodeRingSize / 2,
							 electrodeRingSize, electrodeRingSize, 0, 64 * 360);

				//channel label
				gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &l_oBlack/*&l_oWhite*/);

				m_topographicMapDatabase.getChannelLabel(i, electrodeLabel);
				pango_layout_set_text(l_pElectrodeLabelLayout, electrodeLabel, int(strlen(electrodeLabel)));
				pango_layout_get_pixel_size(l_pElectrodeLabelLayout, &textWidth, nullptr);
				gdk_draw_layout(m_drawingArea->window, m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)],
								channelX - textWidth / 2,
								channelY - electrodeRingSize / 2 - textHeight - 5,
								l_pElectrodeLabelLayout);
			}

			//restore default line thickness
			gdk_gc_set_line_attributes(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			//restore default black color
			gdk_gc_set_rgb_fg_color(m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)], &l_oBlack);

			//delete pango layout
			g_object_unref(l_pElectrodeLabelLayout);
		}

		bool CTopographicMap2DView::getChannel2DPosition(const uint32_t ui32ChannelIndex, gint& channelX, gint& channelY) const
		{
			const uint32_t skullCenterX = m_skullX + m_skullDiameter / 2;
			const uint32_t skullCenterY = m_skullY + m_skullDiameter / 2;
			//get normalized coordinates
			double* l_pOriginalElectrodePosition;
			m_topographicMapDatabase.getChannelPosition(ui32ChannelIndex, l_pOriginalElectrodePosition);

			/* flip the eletrode positions in order to use the mensia coordinate system */
			const double x = l_pOriginalElectrodePosition[0];
			const double y = l_pOriginalElectrodePosition[1];
			const double z = l_pOriginalElectrodePosition[2];

			double electrodePosition[3] = { -y, x, z };

			if (m_currentView == TopographicMap2DView_Top)
			{
				if (m_currentProjection == TopographicMap2DProjection_Axial)
				{
					channelX = gint(skullCenterX + electrodePosition[0] * m_skullDiameter / 2);
					channelY = gint(skullCenterY - electrodePosition[1] * m_skullDiameter / 2);
				}
				else //radial
				{
					//compute back frame 2D coordinates
					const double l_f64Theta = getThetaFromCartesianCoordinates(electrodePosition);
					const double l_f64Phi = getPhiFromCartesianCoordinates(electrodePosition);
					compute2DCoordinates(l_f64Theta, l_f64Phi, skullCenterX, skullCenterY, channelX, channelY);
				}
			}
			else if (m_currentView == TopographicMap2DView_Back)
			{
				//if(electrodePosition[1] > 0) //electrode not visible
				if (electrodePosition[1] > sin(1.f / 90 * M_PI / 2)) { return false; }

				if (m_currentProjection == TopographicMap2DProjection_Axial)
				{
					channelX = gint(skullCenterX + electrodePosition[0] * m_skullDiameter / 2);
					channelY = gint(skullCenterY - electrodePosition[2] * m_skullDiameter / 2);
				}
				else //radial
				{
					//transform coordinates from top frame to back frame
					double l_pBackElectrodePosition[3];
					l_pBackElectrodePosition[0] = electrodePosition[0];
					l_pBackElectrodePosition[1] = electrodePosition[2];
					l_pBackElectrodePosition[2] = -electrodePosition[1];
					//compute back frame 2D coordinates
					const double l_f64Theta = getThetaFromCartesianCoordinates(l_pBackElectrodePosition);
					const double l_f64Phi = getPhiFromCartesianCoordinates(l_pBackElectrodePosition);
					compute2DCoordinates(l_f64Theta, l_f64Phi, skullCenterX, skullCenterY, channelX, channelY);
				}
			}
			else if (m_currentView == TopographicMap2DView_Left)
			{
				//if(electrodePosition[0] > 0) //electrode not visible
				if (electrodePosition[0] > cos(89.f / 90 * M_PI / 2)) { return false; }

				if (m_currentProjection == TopographicMap2DProjection_Axial)
				{
					channelX = gint(skullCenterX - electrodePosition[1] * m_skullDiameter / 2);
					channelY = gint(skullCenterY - electrodePosition[2] * m_skullDiameter / 2);
				}
				else //radial
				{
					//transform coordinates from top frame to left frame
					double l_pBackElectrodePosition[3];
					l_pBackElectrodePosition[0] = -electrodePosition[1];
					l_pBackElectrodePosition[1] = electrodePosition[2];
					l_pBackElectrodePosition[2] = -electrodePosition[0];
					//compute back frame 2D coordinates
					const double l_f64Theta = getThetaFromCartesianCoordinates(l_pBackElectrodePosition);
					const double l_f64Phi = getPhiFromCartesianCoordinates(l_pBackElectrodePosition);
					compute2DCoordinates(l_f64Theta, l_f64Phi, skullCenterX, skullCenterY, channelX, channelY);
				}
			}
			else if (m_currentView == TopographicMap2DView_Right)
			{
				//if(electrodePosition[0] < 0) //electrode not visible
				if (electrodePosition[0] < -cos(89.f / 90 * M_PI / 2)) { return false; }

				if (m_currentProjection == TopographicMap2DProjection_Axial)
				{
					channelX = gint(skullCenterX + electrodePosition[1] * m_skullDiameter / 2);
					channelY = gint(skullCenterY - electrodePosition[2] * m_skullDiameter / 2);
				}
				else //radial
				{
					//transform coordinates from top frame to left frame
					double l_pBackElectrodePosition[3];
					l_pBackElectrodePosition[0] = electrodePosition[1];
					l_pBackElectrodePosition[1] = electrodePosition[2];
					l_pBackElectrodePosition[2] = electrodePosition[0];
					//compute back frame 2D coordinates
					const double l_f64Theta = getThetaFromCartesianCoordinates(l_pBackElectrodePosition);
					const double l_f64Phi = getPhiFromCartesianCoordinates(l_pBackElectrodePosition);
					compute2DCoordinates(l_f64Theta, l_f64Phi, skullCenterX, skullCenterY, channelX, channelY);
				}
			}

			//make sure electrode is in the non clipped area of the display
			//TODO : perform this test once per view only!
			return gdk_region_point_in(m_visibleRegion, channelX - int(m_skullX), channelY - int(m_skullY)) != 0;
		}

		void CTopographicMap2DView::drawBoxToBuffer(const uint32_t ui32X, const uint32_t ui32Y, const uint32_t ui32Width, const uint32_t ui32Height, const uint8_t ui8Red, const uint8_t ui8Green, const uint8_t ui8Blue) const
		{
#ifdef TARGET_OS_Windows
#ifndef NDEBUG
			//m_skullRGBBuffer == m_rowStride*m_skullDiameter
			assert(ui32X < m_skullDiameter);
			assert(ui32Y < m_skullDiameter);
			assert((m_rowStride * ui32Y) + (ui32X * 3) + 2 < m_rowStride * m_skullDiameter);
#endif
#endif
			guchar* lineBase = m_skullRGBBuffer + (m_rowStride * ui32Y) + (ui32X * 3);

			for (uint32_t j = 0; j < ui32Height; j++)
			{
				for (uint32_t i = 0; i < (ui32Width * 3); i += 3)
				{
					*(lineBase + i) = ui8Red;
					*(lineBase + i + 1) = ui8Green;
					*(lineBase + i + 2) = ui8Blue;
				}

				lineBase += (m_rowStride);
			}
		}

		uint32_t CTopographicMap2DView::computeSamplesNormalizedCoordinates(const bool bComputeCoordinates)
		{
			uint32_t curSample = 0;

#ifdef INTERPOLATE_AT_CHANNEL_LOCATION
			uint32_t channelCount = (uint32_t)m_topographicMapDatabase.getChannelCount();
			double* electrodePosition = nullptr;

			//sampling at electrode locations
			for (curSample = 0; curSample < channelCount; curSample++)
			{
				m_topographicMapDatabase.getChannelPosition(curSample, electrodePosition);

				//dummy 2D coords - actual coords are computed when drawing electrode rings
				m_sample2DCoordinates[curSample].first = 0;
				m_sample2DCoordinates[curSample].second = 0;

				*(m_sampleCoordinatesMatrix.getBuffer() + 3 * curSample) = *electrodePosition;
				*(m_sampleCoordinatesMatrix.getBuffer() + 3 * curSample + 1) = *(electrodePosition + 1);
				*(m_sampleCoordinatesMatrix.getBuffer() + 3 * curSample + 2) = *(electrodePosition + 2);
			}
#endif

			//sampling over skull area
			const float skullCenterX = m_skullX + m_skullDiameter / 2.f;
			const float skullCenterY = m_skullY + m_skullDiameter / 2.F;
			double* buffer = m_sampleCoordinatesMatrix.getBuffer();

			//for each row
			float curY = float(m_skullY);
			for (size_t i = 0; i < m_gridSize; ++i, curY += m_cellSize)
			{
				//for each column
				float curX = float(m_skullX);
				for (size_t j = 0; j < m_gridSize; j++, curX += m_cellSize)
				{
					//find corner closest to skull center
					const float closestX = fabs(curX - skullCenterX) < fabs(curX + m_cellSize - skullCenterX) ? curX : (curX + m_cellSize);
					const float closestY = fabs(curY - skullCenterY) < fabs(curY + m_cellSize - skullCenterY) ? curY : (curY + m_cellSize);

					//make sure electrode is in the non clipped area of the display
					//TODO : perform this test once per view only!
					//ensure closest corner lies within "skull sphere"
					if ((closestX - skullCenterX) * (closestX - skullCenterX) +
						(closestY - skullCenterY) * (closestY - skullCenterY) <= (float(m_skullDiameter * m_skullDiameter) / 4.f))
					{
						//ensure this point is in the non clipped skull area
						//FIXME : the previous test remains necessary to get rid of all points lying outside "skull sphere"... Bug in gdk_region_point_in()?
						if (gdk_region_point_in(m_visibleRegion, int(closestX - m_skullX), int(closestY - m_skullY)))
						{
							if (bComputeCoordinates)
							{
								m_sample2DCoordinates[curSample].first = uint32_t(j * m_cellSize);
								m_sample2DCoordinates[curSample].second = uint32_t(i * m_cellSize);

								//compute normalized coordinates to be fed to spherical spline algorithm
								//----------------------------------------------------------------------
								const uint32_t baseIndex = 3 * curSample;

								//normalized X, Y coords in (X, Y) projection plane
								const float x = (closestX - skullCenterX) / (m_skullDiameter / 2.f);
								const float y = -(closestY - skullCenterY) / (m_skullDiameter / 2.f); //y axis down in 2D but up in 3D convention

								if (m_currentProjection == TopographicMap2DProjection_Axial)
								{
									if (m_currentView == TopographicMap2DView_Top)
									{
										*(buffer + baseIndex) = x;
										*(buffer + baseIndex + 1) = y;
										//z = sqrt(1-x*x-y*y)
										const float squareXYSum = x * x + y * y;
										*(buffer + baseIndex + 2) = (squareXYSum >= 1) ? 0 : sqrt(1 - squareXYSum);
									}
									else if (m_currentView == TopographicMap2DView_Back)
									{
										*(buffer + baseIndex) = x;
										*(buffer + baseIndex + 2) = y;
										//y = sqrt(1-x*x-z*z)
										const float squareXYSum = x * x + y * y;
										*(buffer + baseIndex + 1) = (squareXYSum >= 1) ? 0 : sqrt(1 - squareXYSum);
									}
									else if (m_currentView == TopographicMap2DView_Left)
									{
										*(buffer + baseIndex + 1) = -x;
										*(buffer + baseIndex + 2) = y;
										//x = sqrt(1-y*y-z*z)
										const float squareXYSum = x * x + y * y;
										*(buffer + baseIndex) = (squareXYSum >= 1) ? 0 : sqrt(1 - squareXYSum);
									}
									else if (m_currentView == TopographicMap2DView_Right)
									{
										*(buffer + baseIndex + 1) = x;
										*(buffer + baseIndex + 2) = y;
										//x = sqrt(1-y*y-z*z)
										const float squareXYSum = x * x + y * y;
										*(buffer + baseIndex) = (squareXYSum >= 1) ? 0 : sqrt(1 - squareXYSum);
									}
								}
								else //radial
								{
									//theta = (X,Y) arc length
									const float l_f32Theta = float(M_PI / 2 * sqrtf(x * x + y * y));
									const float scalingFactor = (l_f32Theta <= 1e-3) ? 0 : (sinf(l_f32Theta) / l_f32Theta);
									float sampleLocalCoordinates[3];
									//x = sin(theta) / theta * X
									sampleLocalCoordinates[0] = float(scalingFactor * x * (M_PI / 2));
									//y = sin(theta) / theta * Y
									sampleLocalCoordinates[1] = float(scalingFactor * y * (M_PI / 2));
									//z = cos(theta)
									sampleLocalCoordinates[2] = cosf(l_f32Theta);

									if (m_currentView == TopographicMap2DView_Top)
									{
										*(buffer + baseIndex) = sampleLocalCoordinates[0];
										*(buffer + baseIndex + 1) = sampleLocalCoordinates[1];
										*(buffer + baseIndex + 2) = sampleLocalCoordinates[2];
									}
									else if (m_currentView == TopographicMap2DView_Back)
									{
										*(buffer + baseIndex) = sampleLocalCoordinates[0];
										*(buffer + baseIndex + 1) = -sampleLocalCoordinates[2];
										*(buffer + baseIndex + 2) = sampleLocalCoordinates[1];
									}
									else if (m_currentView == TopographicMap2DView_Left)
									{
										*(buffer + baseIndex) = -sampleLocalCoordinates[2];
										*(buffer + baseIndex + 1) = -sampleLocalCoordinates[0];
										*(buffer + baseIndex + 2) = sampleLocalCoordinates[1];
									}
									else if (m_currentView == TopographicMap2DView_Right)
									{
										*(buffer + baseIndex) = sampleLocalCoordinates[2];
										*(buffer + baseIndex + 1) = sampleLocalCoordinates[0];
										*(buffer + baseIndex + 2) = sampleLocalCoordinates[1];
									}
								}
							}

							curSample++;
						} //point in non clipped area
					} //point in "skull sphere"
				}
			}

			return curSample;
		}

		void CTopographicMap2DView::enableElectrodeButtonSignals(const bool enable)
		{
			if (enable)
			{
				g_signal_connect(G_OBJECT(m_electrodesToggleButton), "toggled", G_CALLBACK(toggleElectrodesCallback), this);
			}
			else
			{
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_electrodesToggleButton), reinterpret_cast<void*>(G_CALLBACK(toggleElectrodesCallback)), this);
			}
		}

		void CTopographicMap2DView::enableProjectionButtonSignals(const bool enable)
		{
			if (enable)
			{
				g_signal_connect(G_OBJECT(m_axialProjectionButton), "toggled", G_CALLBACK(setProjectionCallback), this);
				g_signal_connect(G_OBJECT(m_radialProjectionButton), "toggled", G_CALLBACK(setProjectionCallback), this);
			}
			else
			{
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_axialProjectionButton), reinterpret_cast<void*>(G_CALLBACK(setProjectionCallback)), this);
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_radialProjectionButton), reinterpret_cast<void*>(G_CALLBACK(setProjectionCallback)), this);
			}
		}

		void CTopographicMap2DView::enableViewButtonSignals(const bool enable)
		{
			if (enable)
			{
				g_signal_connect(G_OBJECT(m_topViewButton), "toggled", G_CALLBACK(setViewCallback), this);
				g_signal_connect(G_OBJECT(m_leftViewButton), "toggled", G_CALLBACK(setViewCallback), this);
				g_signal_connect(G_OBJECT(m_rightViewButton), "toggled", G_CALLBACK(setViewCallback), this);
				g_signal_connect(G_OBJECT(m_backViewButton), "toggled", G_CALLBACK(setViewCallback), this);
			}
			else
			{
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_topViewButton), reinterpret_cast<void*>(G_CALLBACK(setViewCallback)), this);
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_leftViewButton), reinterpret_cast<void*>(G_CALLBACK(setViewCallback)), this);
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_rightViewButton), reinterpret_cast<void*>(G_CALLBACK(setViewCallback)), this);
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_backViewButton), reinterpret_cast<void*>(G_CALLBACK(setViewCallback)), this);
			}
		}

		void CTopographicMap2DView::enableInterpolationButtonSignals(const bool enable)
		{
			if (enable)
			{
				g_signal_connect(G_OBJECT(m_mapPotentials), "toggled", G_CALLBACK(setInterpolationCallback), this);
				g_signal_connect(G_OBJECT(m_mapCurrents), "toggled", G_CALLBACK(setInterpolationCallback), this);
			}
			else
			{
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_mapPotentials), reinterpret_cast<void*>(G_CALLBACK(setInterpolationCallback)), this);
				g_signal_handlers_disconnect_by_func(G_OBJECT(m_mapCurrents), reinterpret_cast<void*>(G_CALLBACK(setInterpolationCallback)), this);
			}
		}

		double CTopographicMap2DView::getThetaFromCartesianCoordinates(const double* pCartesianCoords) const { return acos(pCartesianCoords[2]); }

		double CTopographicMap2DView::getPhiFromCartesianCoordinates(const double* cartesianCoords) const
		{
			double phi;
			if (cartesianCoords[0] > 0.001)
			{
				phi = atan(cartesianCoords[1] / cartesianCoords[0]);
				if (phi < 0) { phi += 2 * M_PI; }
			}
			else if (cartesianCoords[0] < -0.001)
			{
				phi = atan(cartesianCoords[1] / cartesianCoords[0]) + M_PI;
			}
			else
			{
				phi = cartesianCoords[1] > 0 ? (M_PI / 2) : (3 * M_PI / 2);
			}

			return phi;
		}

		bool CTopographicMap2DView::compute2DCoordinates(const double f64Theta, const double f64Phi, const uint32_t ui32SkullCenterX, const uint32_t ui32SkullCenterY, gint& rX, gint& rY) const
		{
			//linear plotting along radius
			const double length = f64Theta / (M_PI / 2) * m_skullDiameter / 2;
			//determine coordinates on unit circle
			const double X = cos(f64Phi);
			const double Y = sin(f64Phi);
			//scale vector so that it is l_f64Length long
			rX = gint(ui32SkullCenterX + length * X);
			rY = gint(ui32SkullCenterY - length * Y);
			return true;
		}

		//CALLBACKS

		gboolean redrawCallback(GtkWidget* /*widget*/, GdkEventExpose* /*event*/, gpointer data)
		{
			reinterpret_cast<CTopographicMap2DView*>(data)->redraw();
			return TRUE;
		}

		gboolean resizeCallback(GtkWidget* /*pWidget*/, GtkAllocation* allocation, gpointer data)
		{
			reinterpret_cast<CTopographicMap2DView*>(data)->resizeCB(allocation->width, allocation->height);
			return FALSE;
		}

		void toggleElectrodesCallback(GtkWidget* /*pWidget*/, gpointer data)
		{
			auto* l_pTopographicMap2DView = reinterpret_cast<CTopographicMap2DView*>(data);
			l_pTopographicMap2DView->toggleElectrodesCB();
		}

		void setProjectionCallback(GtkWidget* widget, gpointer data)
		{
			auto* l_pTopographicMap2DView = reinterpret_cast<CTopographicMap2DView*>(data);
			l_pTopographicMap2DView->setProjectionCB(widget);
		}

		void setViewCallback(GtkWidget* widget, gpointer data)
		{
			auto* l_pTopographicMap2DView = reinterpret_cast<CTopographicMap2DView*>(data);
			l_pTopographicMap2DView->setViewCB(widget);
		}

		void setInterpolationCallback(GtkWidget* widget, gpointer data)
		{
			auto* l_pTopographicMap2DView = reinterpret_cast<CTopographicMap2DView*>(data);
			l_pTopographicMap2DView->setInterpolationCB(widget);
		}

		void setDelayCallback(GtkRange* range, gpointer data)
		{
			auto* l_pTopographicMap2DView = reinterpret_cast<CTopographicMap2DView*>(data);
			l_pTopographicMap2DView->setDelayCB(gtk_range_get_value(range));
		}
	}  // namespace SimpleVisualization;
} // namespace OpenViBEPlugins;
