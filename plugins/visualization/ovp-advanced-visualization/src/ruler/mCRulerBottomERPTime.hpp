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

#ifndef __OpenViBEPlugins_CRulerBottomERPTime_H__
#define __OpenViBEPlugins_CRulerBottomERPTime_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomERPTime : public IRuler
		{
		public:

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				if(m_pRenderer == NULL) return;
				if(m_pRenderer->getSampleCount() == 0) return;
				if(m_pRenderer->getHistoryCount() == 0) return;
				if(m_pRenderer->getHistoryIndex() == 0) return;

				uint32_t l_ui32SampleCount=m_pRenderer->getSampleCount();
				uint64_t l_ui64SampleDuration=m_pRendererContext->getSampleDuration();

				double l_fDuration=((l_ui32SampleCount*l_ui64SampleDuration)>>16)/65536.;
				std::vector < double > l_vRange;
				std::vector < double >::iterator it;

				l_vRange=this->split_range(0, l_fDuration, 10);

				gint w, h, x;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(it=l_vRange.begin(); it!=l_vRange.end(); it++)
				{
					x=gint((*it/l_fDuration)*w);
					::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
					::gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
					::gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(l_pDrawGC);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerBottomERPTime_H__
