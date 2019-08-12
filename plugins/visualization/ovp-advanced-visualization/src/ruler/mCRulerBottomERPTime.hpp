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
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomERPTime : public IRuler
		{
		public:

			void renderBottom(GtkWidget* pWidget) override
			{
				if (m_pRenderer == nullptr) { return; }
				if (m_pRenderer->getSampleCount() == 0) { return; }
				if (m_pRenderer->getHistoryCount() == 0) { return; }
				if (m_pRenderer->getHistoryIndex() == 0) { return; }

				const uint32_t sampleCount    = m_pRenderer->getSampleCount();
				const uint64_t sampleDuration = m_pRendererContext->getSampleDuration();
				const double duration         = double((sampleCount * sampleDuration) >> 16) / 65536.;

				std::vector<double> range = split_range(0, duration, 10);

				gint w, h;

				gdk_drawable_get_size(pWidget->window, &w, &h);
				GdkGC* drawGC = gdk_gc_new(pWidget->window);
				for (auto r : range)
				{
					const gint x                = gint((r / duration) * w);
					PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, getLabel(r).c_str());
					gdk_draw_layout(pWidget->window, drawGC, x, 5, l_pPangoLayout);
					gdk_draw_line(pWidget->window, drawGC, x, 0, x, 3);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(drawGC);
			}
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia
