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

#include <string>

namespace Mensia {
namespace AdvancedVisualization {
class CRulerLeftChannelNames : public IRuler
{
public:

	void renderLeft(GtkWidget* widget) override
	{
        GdkWindow* window = gtk_widget_get_window(widget);
        const int w = gdk_window_get_width(window);
        const int h = gdk_window_get_height(window);
        
        cairo_region_t * cairoRegion = cairo_region_create();
        GdkDrawingContext* gdc = gdk_window_begin_draw_frame(window,cairoRegion);
        cairo_t* cr = gdk_drawing_context_get_cairo_context(gdc);
		for (size_t i = 0; i < m_rendererCtx->getSelectedCount(); ++i)
		{
			const size_t idx        = m_rendererCtx->getSelected(i);
			const std::string label = (m_rendererCtx->getChannelName(idx) + " (" + std::to_string(idx + 1) + ")");
			PangoLayout* layout     = gtk_widget_create_pango_layout(widget, label.c_str());

            gint lw, lh;
			pango_layout_get_size(layout, &lw, &lh);
			lw /= PANGO_SCALE;
			lh /= PANGO_SCALE;
            cairo_move_to(cr, w - lw, ((i + 0.5) * h) / m_rendererCtx->getSelectedCount() - float(lh) / 2);
            pango_cairo_show_layout(cr, layout);
			g_object_unref(layout);
		}
        cairo_stroke(cr); // Useful ??
        gdk_window_end_draw_frame(window,gdc);
        cairo_region_destroy(cairoRegion);
	}
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
