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
class CRulerRightFrequency : public IRuler
{
public:

	void renderRight(GtkWidget* widget) override
	{
		const auto scale = float(m_rendererCtx->getSpectrumFrequencyRange());
		if (m_lastScale != scale)
		{
			m_range     = splitRange(0, scale);
			m_lastScale = scale;
		}

        GdkWindow* window = gtk_widget_get_window(widget);
        const int h = gdk_window_get_height(window);

        cairo_region_t * cairoRegion = cairo_region_create();
        GdkDrawingContext* gdc = gdk_window_begin_draw_frame(window,cairoRegion);
        cairo_t* cr = gdk_drawing_context_get_cairo_context(gdc);

		const size_t nChannel = m_rendererCtx->getSelectedCount();
		for (size_t i = 0; i < nChannel; ++i)
		{
			for (const auto& r : m_range)
			{
				const gint y        = gint((i + r / scale) * (h * 1.F / nChannel));
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(r).c_str());

                gint lw, lh;
				pango_layout_get_size(layout, &lw, &lh);
				lw /= PANGO_SCALE;
				lh /= PANGO_SCALE;
                cairo_move_to(cr, 8, h - y - lh / 2);
                pango_cairo_show_layout(cr, layout);
                cairo_move_to(cr, 0, h - y);
                cairo_line_to(cr, 3, h - y);
				g_object_unref(layout);
			}
		}
        cairo_stroke(cr); // Useful ??
        gdk_window_end_draw_frame(window,gdc);
        cairo_region_destroy(cairoRegion);
	}

protected:
	float m_lastScale = 1;
	std::vector<double> m_range;
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
