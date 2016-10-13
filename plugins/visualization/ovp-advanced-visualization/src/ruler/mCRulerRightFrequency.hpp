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

#ifndef __OpenViBEPlugins_CRulerRightFrequency_H__
#define __OpenViBEPlugins_CRulerRightFrequency_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerRightFrequency : public IRuler
		{
		public:

			virtual void renderRight(::GtkWidget* pWidget)
			{
				float l_fScale=(float)m_pRendererContext->getSpectrumFrequencyRange();
				if(m_fLastScale!=l_fScale) { m_vRange=this->split_range(0, l_fScale); m_fLastScale=l_fScale; }

				gint w, h, y;
				gint lw, lh;

				uint32 l_ui32ChannelCount=m_pRendererContext->getSelectedCount();
				for(uint32 i=0; i<l_ui32ChannelCount; i++)
				{
					::gdk_drawable_get_size(pWidget->window, &w, &h);
					::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
					for(it=m_vRange.begin(); it!=m_vRange.end(); it++)
					{
						y=gint((i+*it/l_fScale)*(h*1.f/l_ui32ChannelCount));
						::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						::pango_layout_get_size(l_pPangoLayout, &lw, &lh);
						lw/=PANGO_SCALE;
						lh/=PANGO_SCALE;
						::gdk_draw_layout(pWidget->window, l_pDrawGC, 8, h-y-lh/2, l_pPangoLayout);
						::gdk_draw_line(pWidget->window, l_pDrawGC, 0, h-y, 3, h-y);
						g_object_unref(l_pPangoLayout);
					}
					g_object_unref(l_pDrawGC);
				}
			}

			float m_fLastScale;
			std::vector < double > m_vRange;
			std::vector < double >::iterator it;
		};
	};
};

#endif // __OpenViBEPlugins_CRulerRightFrequency_H__
