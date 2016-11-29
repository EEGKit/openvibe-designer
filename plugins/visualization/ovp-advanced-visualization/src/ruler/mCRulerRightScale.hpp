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

#ifndef __OpenViBEPlugins_CRulerRightScale_H__
#define __OpenViBEPlugins_CRulerRightScale_H__

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerRightScale : public IRuler
		{
		public:

			CRulerRightScale(void)
				:m_fLastScale(-1)
			{
			}

			virtual void renderRight(::GtkWidget* pWidget)
			{
				uint32_t l_ui32SelectedCount=m_pRendererContext->getSelectedCount();
				if(!l_ui32SelectedCount) return;

				float l_fScale=1.f/m_pRendererContext->getScale();
				if(m_fLastScale!=l_fScale)
				{
					if(m_pRendererContext->isPositiveOnly())
					{
						m_vRange=this->split_range(0, l_fScale, IRuler_SplitCount);
					}
					else
					{
						m_vRange=this->split_range(-l_fScale*.5, l_fScale*.5, IRuler_SplitCount);
					}
					m_fLastScale=l_fScale;
				}

				float l_fOffset=m_pRendererContext->isPositiveOnly()?0:0.5f;

				gint w, h, y;
				gint lw, lh;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(unsigned int i=0; i<m_pRendererContext->getSelectedCount(); i++)
				{
					for(it=m_vRange.begin(); it!=m_vRange.end(); it++)
					{
						::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						::pango_layout_get_size(l_pPangoLayout, &lw, &lh);
						lw/=PANGO_SCALE;
						lh/=PANGO_SCALE;
						y=gint((1-(i + l_fOffset + *it/l_fScale)/l_ui32SelectedCount)*h);
						::gdk_draw_layout(pWidget->window, l_pDrawGC, 8, y-lh/2, l_pPangoLayout);
						::gdk_draw_line(pWidget->window, l_pDrawGC, 0, y, 3, y);
						g_object_unref(l_pPangoLayout);
					}
				}
				g_object_unref(l_pDrawGC);
			}

			float m_fLastScale;
			std::vector < double > m_vRange;
			std::vector < double >::iterator it;
		};
	};
};

#endif // __OpenViBEPlugins_CRulerRightScale_H__
