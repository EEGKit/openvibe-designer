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

			virtual void renderBottom(GtkWidget* pWidget)
			{
				if (m_pRenderer == nullptr) return;
				if (m_pRenderer->getSampleCount() == 0) return;
				if (m_pRenderer->getHistoryCount() == 0) return;
				if (m_pRenderer->getHistoryIndex() == 0) return;

				uint32_t l_ui32SampleCount = m_pRenderer->getSampleCount();
				uint32_t l_ui32HistoryIndex = m_pRenderer->getHistoryIndex();
				uint64_t l_ui64SampleDuration = m_pRendererContext->getSampleDuration();

				std::vector < double > l_vRange1;
				std::vector < double > l_vRange2;
				std::vector < double >::iterator it;

				uint32_t l_ui32LeftIndex = l_ui32HistoryIndex - l_ui32HistoryIndex % l_ui32SampleCount;
				uint32_t l_ui32MidIndex = l_ui32HistoryIndex;
				double l_f64StartTime = ((l_ui32LeftIndex * l_ui64SampleDuration) >> 16) / 65536.;
				double l_f64MidTime = ((l_ui32MidIndex * l_ui64SampleDuration) >> 16) / 65536.;
				double l_f64Duration = ((l_ui32SampleCount * l_ui64SampleDuration) >> 16) / 65536.;

				double l_f64Offset = (m_pRenderer->getTimeOffset() >> 16) / 65536.;
				l_f64StartTime += l_f64Offset;
				l_f64MidTime += l_f64Offset;

				l_vRange1 = this->split_range(l_f64StartTime - l_f64Duration, l_f64StartTime, 10);
				l_vRange2 = this->split_range(l_f64StartTime, l_f64StartTime + l_f64Duration, 10);

				gint w, h, x;

				gdk_drawable_get_size(pWidget->window, &w, &h);
				GdkGC * l_pDrawGC = gdk_gc_new(pWidget->window);
				for (it = l_vRange1.begin(); it != l_vRange1.end(); it++)
				{
					if (*it >= 0 && *it + l_f64Duration > l_f64MidTime)
					{
						x = gint(((*it + l_f64Duration - l_f64StartTime) / l_f64Duration) * w);
						PangoLayout * l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				for (it = l_vRange2.begin(); it != l_vRange2.end(); it++)
				{
					if (*it >= 0 && *it < l_f64MidTime)
					{
						x = gint(((*it - l_f64StartTime) / l_f64Duration) * w);
						PangoLayout * l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				g_object_unref(l_pDrawGC);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerBottomTime_H__
