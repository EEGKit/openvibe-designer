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
#pragma once

#include "m_defines.hpp"
#include "m_VisualizationTools.hpp"
#include "m_GtkGL.hpp"

#include <openvibe/ov_all.h>
#include <visualization-toolkit/ovviz_all.h>

#if defined TARGET_OS_Windows
#include <Windows.h>
#endif

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <string>

// #define __DIRECT_RENDER__

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class TBox>
		class TGtkGLWidget
		{
		public:

			TGtkGLWidget() : m_box(nullptr) { }

			virtual ~TGtkGLWidget()
			{
				if (m_widget)
				{
					if (m_textureId)
					{
						GtkGL::preRender(m_widget);
						glDeleteTextures(1, &m_textureId);
						GtkGL::postRender(m_widget);
					}
					if (m_timeoutSrc) { g_source_destroy(m_timeoutSrc); }
					GtkGL::uninitialize(m_widget);
				}
			}

			virtual void initialize(TBox& box, GtkWidget* widget, GtkWidget* left, GtkWidget* right, GtkWidget* bottom)
			{
				GtkGL::initialize(widget);
				{
					m_box    = &box;
					m_widget = widget;
					m_left   = left;
					m_right  = right;
					m_bottom = bottom;

					::g_signal_connect(widget, "configure-event", G_CALLBACK(TGtkGLWidget<TBox>::configureCB), m_box);
					::g_signal_connect(widget, "expose-event", G_CALLBACK(TGtkGLWidget<TBox>::exposeCB), m_box);
					::g_signal_connect(widget, "button-press-event", G_CALLBACK(TGtkGLWidget<TBox>::mouseButtonCB), m_box);
					::g_signal_connect(widget, "button-release-event", G_CALLBACK(TGtkGLWidget<TBox>::mouseButtonCB), m_box);
					::g_signal_connect(widget, "motion-notify-event", G_CALLBACK(TGtkGLWidget<TBox>::motionNotifyCB), m_box);
					::g_signal_connect(widget, "enter-notify-event", G_CALLBACK(TGtkGLWidget<TBox>::enterNotifyCB), m_box);
					::g_signal_connect(gtk_widget_get_parent(widget), "key-press-event", G_CALLBACK(TGtkGLWidget<TBox>::keyPressCB), m_box);
					::g_signal_connect(gtk_widget_get_parent(widget), "key-release-event", G_CALLBACK(TGtkGLWidget<TBox>::keyReleaseCB), m_box);

					::g_signal_connect_after(left, "expose-event", G_CALLBACK(TGtkGLWidget<TBox>::exposeLeftCB), m_box);
					::g_signal_connect_after(right, "expose-event", G_CALLBACK(TGtkGLWidget<TBox>::exposeRightCB), m_box);
					::g_signal_connect_after(bottom, "expose-event", G_CALLBACK(TGtkGLWidget<TBox>::exposeBottomCB), m_box);

					m_timeoutSrc = g_timeout_source_new(250); // timeouts every 50 ms
					g_source_set_priority(m_timeoutSrc, G_PRIORITY_LOW);
					g_source_set_callback(m_timeoutSrc, GSourceFunc(timeoutRedrawCB), m_box, nullptr);
					g_source_attach(m_timeoutSrc, nullptr);

					gtk_widget_queue_resize(widget);
				}
			}

			virtual void redrawTopLevelWindow(const bool immediate = false)
			{
				GtkWidget* top = gtk_widget_get_toplevel(m_widget);
				if (top != nullptr)
				{
					if (immediate)
					{
						gdk_window_process_updates(top->window, false);
						gtk_widget_queue_draw(top);
					}
					else { gdk_window_invalidate_rect(top->window, nullptr, true); }
				}
			}

			virtual void redraw(const bool immediate = false)
			{
				if (immediate)
				{
					gdk_window_process_updates(m_widget->window, false);
					gtk_widget_queue_draw(m_widget);
				}
				else { gdk_window_invalidate_rect(m_widget->window, nullptr, true); }
			}

			virtual void redrawLeft(const bool immediate = false)
			{
				if (immediate)
				{
					gdk_window_process_updates(m_left->window, false);
					gtk_widget_queue_draw(m_left);
				}
				else { gdk_window_invalidate_rect(m_left->window, nullptr, true); }
			}

			virtual void redrawRight(const bool immediate = false)
			{
				if (immediate)
				{
					gdk_window_process_updates(m_right->window, false);
					gtk_widget_queue_draw(m_right);
				}
				else { gdk_window_invalidate_rect(m_right->window, nullptr, true); }
			}

			virtual void redrawBottom(const bool immediate = false)
			{
				if (immediate)
				{
					gdk_window_process_updates(m_bottom->window, false);
					gtk_widget_queue_draw(m_bottom);
				}
				else { gdk_window_invalidate_rect(m_bottom->window, nullptr, true); }
			}

			virtual void setPointSmoothingActive(const bool active = false)
			{
				if (active) { glEnable(GL_POINT_SMOOTH); }
				else { glDisable(GL_POINT_SMOOTH); }
			}

			virtual uint32_t createTexture(const std::string& value)
			{
#define M_GRADIENT_SIZE 128

				if (m_textureId == 0)
				{
					const std::string str = (value.empty() ? "0:0,0,100; 25:0,100,100; 50:0,49,0; 75:100,100,0; 100:100,0,0" : value);

					OpenViBE::CMatrix gradientBase, gradient;
					OpenViBE::VisualizationToolkit::ColorGradient::parse(gradientBase, str.c_str());
					OpenViBE::VisualizationToolkit::ColorGradient::interpolate(gradient, gradientBase, M_GRADIENT_SIZE);

					float texture[M_GRADIENT_SIZE][3];
					for (size_t i = 0; i < M_GRADIENT_SIZE; ++i)
					{
						texture[i][0] = float(gradient[i * 4 + 1] * .01);
						texture[i][1] = float(gradient[i * 4 + 2] * .01);
						texture[i][2] = float(gradient[i * 4 + 3] * .01);
					}

					glGenTextures(1, &m_textureId);
					glBindTexture(GL_TEXTURE_1D, m_textureId);
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR);
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
					//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
					//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, __SIZE__, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
					gluBuild1DMipmaps(GL_TEXTURE_1D, GL_RGB, M_GRADIENT_SIZE, GL_RGB, GL_FLOAT, texture);
				}

#undef M_GRADIENT_SIZE

				return m_textureId;
			}

		protected:

			GtkWidget* m_widget   = nullptr;
			GtkWidget* m_left     = nullptr;
			GtkWidget* m_right    = nullptr;
			GtkWidget* m_bottom   = nullptr;
			GSource* m_timeoutSrc = nullptr;

			TBox* m_box = nullptr;

			uint32_t m_textureId = 0;

		private:


			static gboolean timeoutRedrawCB(TBox* box)
			{
				box->redraw();
				return TRUE;
			}

			static gboolean configureCB(GtkWidget* widget, GdkEventConfigure* /*event*/, TBox* box)
			{
				GtkGL::preRender(widget);
				glViewport(0, 0, widget->allocation.width, widget->allocation.height);
				box->reshape(widget->allocation.width, widget->allocation.height);
				GtkGL::postRender(widget);
				return TRUE;
			}

			static gboolean exposeCB(GtkWidget* widget, GdkEventExpose* /*event*/, TBox* box)
			{
				const float d  = 1.F;
				const float dx = d / (widget->allocation.width - d);
				const float dy = d / (widget->allocation.height - d);

				GtkGL::preRender(widget);

				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glTranslatef(0.5, 0, 0);

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluOrtho2D(0 - dx, 1 + dx, 0 - dy, 1 + dy);

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1.0, 1.0);

				glEnable(GL_LINE_SMOOTH);
				glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
				glLineWidth(1);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				glEnable(GL_TEXTURE_1D);

				glDisable(GL_DEPTH_TEST);
				glClearDepth(100);
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glColor3f(1, 1, 1);

				// Lighting
				const float fAmbient  = 0.0F;
				const float fDiffuse  = 1.0F;
				const float fSpecular = 1.0F;
				GLfloat ambient[]     = { fAmbient, fAmbient, fAmbient, 1 };
				GLfloat diffuse[]     = { fDiffuse, fDiffuse, fDiffuse, 1 };
				GLfloat specular[]    = { fSpecular, fSpecular, fSpecular, 1 };
				GLfloat position0[]   = { 3, 1, 2, 1 };
				GLfloat position1[]   = { -3, 0, -2, 1 };
				glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
				glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
				glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
				glLightfv(GL_LIGHT0, GL_POSITION, position0);
				glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
				glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
				glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
				glLightfv(GL_LIGHT1, GL_POSITION, position1);
				glShadeModel(GL_SMOOTH);
				glEnable(GL_LIGHT0);
				glEnable(GL_LIGHT1);
				glEnable(GL_COLOR_MATERIAL);
				glDisable(GL_LIGHTING);

				box->draw();

				GtkGL::postRender(widget);

				return TRUE;
			}

			static gboolean enterNotifyCB(GtkWidget* /*widget*/, GdkEventCrossing* /*event*/, TBox* box)
			{
				box->redraw();
				//box->request();
				//box->m_redrawNeeded = true;
				return TRUE;
			}

			static gboolean exposeLeftCB(GtkWidget* /*widget*/, GdkEventExpose* /*event*/, TBox* box)
			{
				box->drawLeft();
				return TRUE;
			}

			static gboolean exposeRightCB(GtkWidget* /*widget*/, GdkEventExpose* /*event*/, TBox* box)
			{
				box->drawRight();
				return TRUE;
			}

			static gboolean exposeBottomCB(GtkWidget* /*widget*/, GdkEventExpose* /*event*/, TBox* box)
			{
				box->drawBottom();
				return TRUE;
			}

			static gboolean mouseButtonCB(GtkWidget* /*widget*/, GdkEventButton* event, TBox* box)
			{
				int status = 0;
				switch (event->type)
				{
					case GDK_BUTTON_PRESS: status = 1;
						break;
					case GDK_2BUTTON_PRESS: status = 2;
						break;
					case GDK_3BUTTON_PRESS: status = 3;
						break;
					default: break;
				}
				box->mouseButton(int(event->x), int(event->y), event->button, status);
				box->draw();
				return TRUE;
			}

			static gboolean motionNotifyCB(GtkWidget* /*widget*/, GdkEventMotion* event, TBox* box)
			{
				box->mouseMotion(int(event->x), int(event->y));
				return TRUE;
			}

			static gboolean keyPressCB(GtkWidget* /*widget*/, GdkEventKey* event, TBox* box)
			{
				box->keyboard(0, 0, /*event->x, event->y,*/ event->keyval, true);
				return TRUE;
			}

			static gboolean keyReleaseCB(GtkWidget* /*widget*/, GdkEventKey* event, TBox* box)
			{
				box->keyboard(0, 0, /*event->x, event->y,*/ event->keyval, false);
				return TRUE;
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
