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
class CRulerRightScale : public IRuler
{
public:

	CRulerRightScale() : m_lastScale(-1) { }

	void renderRight(GtkWidget* widget) override
	{
		const size_t nSelected = m_rendererCtx->getSelectedCount();
		if (!nSelected) { return; }

		const float scale = 1.F / m_rendererCtx->getScale();
		if (m_lastScale != scale)
		{
			if (m_rendererCtx->isPositiveOnly()) { m_range = splitRange(0, scale, IRuler_SplitCount); }
			else { m_range = splitRange(-scale * .5, scale * .5, IRuler_SplitCount); }
			m_lastScale = scale;
		}

		const float offset = m_rendererCtx->isPositiveOnly() ? 0.0F : 0.5F;

        GdkWindow* window = gtk_widget_get_window(widget);
        const int h = gdk_window_get_height(window);
        
        cairo_region_t * cairoRegion = cairo_region_create();
        GdkDrawingContext* gdc = gdk_window_begin_draw_frame(window,cairoRegion);
        cairo_t* cr = gdk_drawing_context_get_cairo_context(gdc);
		for (size_t i = 0; i < m_rendererCtx->getSelectedCount(); ++i)
		{
			for (const auto& j : m_range)
			{
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(j).c_str());

                gint lw, lh;
				pango_layout_get_size(layout, &lw, &lh);
				lw /= PANGO_SCALE;
				lh /= PANGO_SCALE;
				const gint y = gint((1 - (float(i) + offset + j / scale) / nSelected) * h);
                cairo_move_to(cr, 8, y - lh / 2);
                pango_cairo_show_layout(cr, layout);
                cairo_move_to(cr, 0, y);
                cairo_line_to(cr, 3, y);
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
