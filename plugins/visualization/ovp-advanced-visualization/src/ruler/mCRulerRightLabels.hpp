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
		template <size_t dimension>
		class CRulerRightLabels : public IRuler
		{
		public:

			void renderRight(GtkWidget* widget) override
			{
				gint w, h;
				gint lw, lh;

				const uint32_t nChannel = m_pRendererContext->getSelectedCount();
				for (uint32_t channel = 0; channel < nChannel; ++channel)
				{
					gdk_drawable_get_size(widget->window, &w, &h);
					GdkGC* drawGC = gdk_gc_new(widget->window);

					const auto labelCount = float(m_pRendererContext->getDimensionLabelCount(dimension));

					gint last_y = gint((channel + (-1 + 0.5f) / labelCount) * (h * 1.f / nChannel));

					for (uint32_t label = 0; label < m_pRendererContext->getDimensionLabelCount(dimension); ++label)
					{
						const gint y = gint((channel + (label + 0.5f) / labelCount) * (h * 1.f / nChannel));
						if (y >= last_y + 10)
						{
							PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(widget, m_pRendererContext->getDimensionLabel(dimension, label));
							pango_layout_get_size(l_pPangoLayout, &lw, &lh);
							lw /= PANGO_SCALE;
							lh /= PANGO_SCALE;
							gdk_draw_layout(widget->window, drawGC, 8, h - y - lh / 2, l_pPangoLayout);
							gdk_draw_line(widget->window, drawGC, 0, h - y, 3, h - y);
							g_object_unref(l_pPangoLayout);
							last_y = y;
						}
					}
					g_object_unref(drawGC);
				}
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
