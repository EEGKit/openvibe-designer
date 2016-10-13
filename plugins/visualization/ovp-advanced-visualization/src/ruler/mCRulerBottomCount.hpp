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

#ifndef __OpenViBEPlugins_CRulerBottomCount_H__
#define __OpenViBEPlugins_CRulerBottomCount_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomCount : public IRuler
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

				std::vector < double > l_vRange1;
				std::vector < double > l_vRange2;
				std::vector < double >::iterator it;

				uint32 l_ui32LeftIndex1=l_ui32HistoryIndex-l_ui32HistoryIndex%l_ui32SampleCount;
				uint32 l_ui32LeftIndex2=l_ui32HistoryIndex;
				uint32 l_ui32RightIndex1=l_ui32LeftIndex2-l_ui32SampleCount;
				uint32 l_ui32RightIndex2=l_ui32LeftIndex1;

				l_vRange1=this->split_range(l_ui32LeftIndex1,  l_ui32LeftIndex1+l_ui32SampleCount,  10);
				l_vRange2=this->split_range(l_ui32RightIndex1, l_ui32RightIndex1+l_ui32SampleCount, 10);

				gint w, h, x;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(it=l_vRange1.begin(); it!=l_vRange1.end(); it++)
				{
					if(*it >= l_ui32LeftIndex1 && *it < l_ui32LeftIndex2)
					{
						x=gint(((*it-l_ui32LeftIndex1)/l_ui32SampleCount)*w);
						::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						::gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						::gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				for(it=l_vRange2.begin(); it!=l_vRange2.end(); it++)
				{
					if(*it >= l_ui32RightIndex1 && *it < l_ui32RightIndex2)
					{
						x=gint(((*it+l_ui32SampleCount-l_ui32LeftIndex1)/l_ui32SampleCount)*w);
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

#endif // __OpenViBEPlugins_CRulerBottomCount_H__
