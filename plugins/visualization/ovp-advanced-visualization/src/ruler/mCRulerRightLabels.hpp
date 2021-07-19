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

namespace Mensia {
namespace AdvancedVisualization {
template <size_t TDim>
class CRulerRightLabels : public IRuler
{
public:

	void renderRight(GtkWidget* widget) override
	{
        GdkWindow* window = gtk_widget_get_window(widget);
        const int h = gdk_window_get_height(window);
        
        cairo_region_t * cairoRegion = cairo_region_create();
        GdkDrawingContext* gdc = gdk_window_begin_draw_frame(window,cairoRegion);
        cairo_t* cr = gdk_drawing_context_get_cairo_context(gdc);

		const size_t nChannel = m_rendererCtx->getSelectedCount();
		for (size_t channel = 0; channel < nChannel; ++channel)
		{
			const auto labelCount = float(m_rendererCtx->getDimensionLabelCount(TDim));

			gint lastY = gint((channel + (-1 + 0.5F) / labelCount) * (h * 1.F / nChannel));

			for (size_t label = 0; label < m_rendererCtx->getDimensionLabelCount(TDim); ++label)
			{
				const gint y = gint((channel + (label + 0.5F) / labelCount) * (h * 1.F / nChannel));
				if (y >= lastY + 10)
				{
					PangoLayout* layout = gtk_widget_create_pango_layout(widget, m_rendererCtx->getDimensionLabel(TDim, label));

                    gint lw, lh;
					pango_layout_get_size(layout, &lw, &lh);
					lw /= PANGO_SCALE;
					lh /= PANGO_SCALE;
                    cairo_move_to(cr, 8, h - y - lh / 2);
                    pango_cairo_show_layout(cr, layout);
                    cairo_move_to(cr, 0, h - y);
                    cairo_line_to(cr, 3, h - y);
					g_object_unref(layout);
					lastY = y;
				}
			}
            cairo_stroke(cr); // Useful ??
            gdk_window_end_draw_frame(window,gdc);
            cairo_region_destroy(cairoRegion);
		}
	}
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
