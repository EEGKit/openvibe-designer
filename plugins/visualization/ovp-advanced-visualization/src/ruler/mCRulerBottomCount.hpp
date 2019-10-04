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

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomCount : public IRuler
		{
		public:

			void renderBottom(GtkWidget* pWidget) override
			{
				if (m_pRenderer == nullptr) { return; }
				if (m_pRenderer->getSampleCount() == 0) { return; }
				if (m_pRenderer->getHistoryCount() == 0) { return; }
				if (m_pRenderer->getHistoryIndex() == 0) { return; }

				const uint32_t nSample  = m_pRenderer->getSampleCount();
				const uint32_t historyIndex = m_pRenderer->getHistoryIndex();

				std::vector<double>::iterator it;

				const uint32_t leftIndex1  = historyIndex - historyIndex % nSample;
				const uint32_t leftIndex2  = historyIndex;
				const uint32_t rightIndex1 = leftIndex2 - nSample;
				const uint32_t rightIndex2 = leftIndex1;

				std::vector<double> l_vRange1 = split_range(leftIndex1, leftIndex1 + nSample, 10);
				std::vector<double> l_vRange2 = split_range(rightIndex1, rightIndex1 + nSample, 10);

				gint w, h, x;

				gdk_drawable_get_size(pWidget->window, &w, &h);
				GdkGC* l_pDrawGC = gdk_gc_new(pWidget->window);
				for (it = l_vRange1.begin(); it != l_vRange1.end(); ++it)
				{
					if (*it >= leftIndex1 && *it < leftIndex2)
					{
						x                           = gint(((*it - leftIndex1) / nSample) * w);
						PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, getLabel(*it).c_str());
						gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				for (it = l_vRange2.begin(); it != l_vRange2.end(); ++it)
				{
					if (*it >= rightIndex1 && *it < rightIndex2)
					{
						x                           = gint(((*it + nSample - leftIndex1) / nSample) * w);
						PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, getLabel(*it).c_str());
						gdk_draw_layout(pWidget->window, l_pDrawGC, x, 5, l_pPangoLayout);
						gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
						g_object_unref(l_pPangoLayout);
					}
				}
				g_object_unref(l_pDrawGC);
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
