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

#ifndef __OpenViBEPlugins_CRulerBottomFrequency_H__
#define __OpenViBEPlugins_CRulerBottomFrequency_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomFrequency : public IRuler
		{
		public:

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				float l_fScale=(float)m_pRendererContext->getSpectrumFrequencyRange();
				if(m_fLastScale!=l_fScale) { m_vRange=this->split_range(0, l_fScale); m_fLastScale=l_fScale; }

				gint w, h, x;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(it=m_vRange.begin(); it!=m_vRange.end(); it++)
				{
					x=gint((*it/l_fScale)*w);
					::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
					::gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
					::gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
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

#endif // __OpenViBEPlugins_CRulerBottomFrequency_H__
