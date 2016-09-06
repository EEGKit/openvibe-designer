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

#ifndef __OpenViBEPlugins_CRulerBottomTime_H__
#define __OpenViBEPlugins_CRulerBottomTime_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomTime : public IRuler
		{
		public:

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				if(m_pRenderer == NULL) return;
				if(m_pRenderer->getSampleCount() == 0) return;
				if(m_pRenderer->getHistoryCount() == 0) return;
				if(m_pRenderer->getHistoryIndex() == 0) return;

				uint32 l_ui32SampleCount=m_pRenderer->getSampleCount();
				uint32 l_ui32HistoryIndex=m_pRenderer->getHistoryIndex();
				uint64 l_ui64SampleDuration=m_pRendererContext->getSampleDuration();

				std::vector < double > l_vRange1;
				std::vector < double > l_vRange2;
				std::vector < double >::iterator it;

				uint32 l_ui32LeftIndex=l_ui32HistoryIndex-l_ui32HistoryIndex%l_ui32SampleCount;
				uint32 l_ui32MidIndex =l_ui32HistoryIndex;
				float64 l_f64StartTime=((l_ui32LeftIndex  *l_ui64SampleDuration)>>16)/65536.;
				float64 l_f64MidTime  =((l_ui32MidIndex   *l_ui64SampleDuration)>>16)/65536.;
				float64 l_f64Duration =((l_ui32SampleCount*l_ui64SampleDuration)>>16)/65536.;

				l_vRange1=this->split_range(l_f64StartTime-l_f64Duration, l_f64StartTime              , 10);
				l_vRange2=this->split_range(l_f64StartTime              , l_f64StartTime+l_f64Duration, 10);

				gint w, h, x;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(it=l_vRange1.begin(); it!=l_vRange1.end(); it++)
				{
					if(*it >= 0 && *it+l_f64Duration > l_f64MidTime)
					{
						x=gint(((*it+l_f64Duration-l_f64StartTime)/l_f64Duration)*w);
						::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						::gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						::gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				for(it=l_vRange2.begin(); it!=l_vRange2.end(); it++)
				{
					if(*it >= 0 && *it < l_f64MidTime)
					{
						x=gint(((*it-l_f64StartTime)/l_f64Duration)*w);
						::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						::gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						::gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				g_object_unref(l_pDrawGC);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerBottomTime_H__
