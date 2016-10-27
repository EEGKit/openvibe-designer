/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#ifndef __OpenViBEPlugins_CRulerBottomTexture_H__
#define __OpenViBEPlugins_CRulerBottomTexture_H__

#include "mCRulerTexture.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomTexture : public CRulerTexture
		{
		public:

			virtual void render(void)
			{
				this->preRender();

				::glColor4f(0, 0, 0, m_fBlackAlpha);
				::glBegin(GL_QUADS);
					::glTexCoord1f(0);
					::glVertex2f(0, 0.00f);
					::glVertex2f(0, 0.05f);
					::glTexCoord1f(1);
					::glVertex2f(1, 0.05f);
					::glVertex2f(1, 0.00f);
				::glEnd();

				::glColor4f(1, 1, 1, m_fWhiteAlpha);
				::glBegin(GL_QUADS);
					::glTexCoord1f(0);
					::glVertex2f(0, 0.00f);
					::glVertex2f(0, 0.04f);
					::glTexCoord1f(1);
					::glVertex2f(1, 0.04f);
					::glVertex2f(1, 0.00f);
				::glEnd();

				this->postRender();
			}

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				float l_fScale=1.f/m_pRendererContext->getScale();
				if(m_fLastScale!=l_fScale) { m_vRange=this->split_range(-l_fScale*.5, l_fScale*.5); m_fLastScale=l_fScale; }

				gint w, h;
				gint lw, lh;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(it=m_vRange.begin(); it!=m_vRange.end(); it++)
				{
					::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
					::pango_layout_get_size(l_pPangoLayout, &lw, &lh);
					lw/=PANGO_SCALE;
					lh/=PANGO_SCALE;
					::gdk_draw_layout(
						pWidget->window,
						l_pDrawGC,
						gint((.5+*it/l_fScale)*w-lw*.5),
						0,
						l_pPangoLayout);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(l_pDrawGC);
			}

			float m_fLastScale;
			std::vector < double > m_vRange;
			std::vector < double >::iterator it;
		};
	};
};

#endif // __OpenViBEPlugins_CRulerBottomTexture_H__
