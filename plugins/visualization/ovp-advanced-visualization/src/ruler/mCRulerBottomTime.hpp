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
class CRulerBottomTime : public IRuler
{
public:

	void renderBottom(GtkWidget* widget) override
	{
		if (m_renderer == nullptr) { return; }
		if (m_renderer->getSampleCount() == 0) { return; }
		if (m_renderer->getHistoryCount() == 0) { return; }
		if (m_renderer->getHistoryIndex() == 0) { return; }

		const size_t nSample          = m_renderer->getSampleCount();
		const size_t historyIdx       = m_renderer->getHistoryIndex();
		const uint64_t sampleDuration = m_rendererCtx->getSampleDuration();

		const size_t leftIdx  = historyIdx - historyIdx % nSample;
		const size_t midIdx   = historyIdx;
		double startTime      = double((leftIdx * sampleDuration) >> 16) / 65536.;
		double midTime        = double((midIdx * sampleDuration) >> 16) / 65536.;
		const double duration = double((nSample * sampleDuration) >> 16) / 65536.;

		const double offset = (m_renderer->getTimeOffset() >> 16) / 65536.;
		startTime += offset;
		midTime += offset;

		std::vector<double> range1 = this->splitRange(startTime - duration, startTime, 10);
		std::vector<double> range2 = this->splitRange(startTime, startTime + duration, 10);

        GdkWindow* window = gtk_widget_get_window(widget);
        const int w = gdk_window_get_width(window);
        
        cairo_region_t * cairoRegion = cairo_region_create();
        GdkDrawingContext* gdc = gdk_window_begin_draw_frame(window,cairoRegion);
        cairo_t* cr = gdk_drawing_context_get_cairo_context(gdc);
		for (const auto& i : range1)
		{
			if (i >= 0 && i + duration > midTime)
			{
				const gint x        = gint(((i + duration - startTime) / duration) * w);
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
                cairo_move_to(cr, x, 5);
                pango_cairo_show_layout(cr, layout);
                cairo_move_to(cr, x, 0);
                cairo_line_to(cr, x, 3);
				g_object_unref(layout);
			}
		}
		for (const auto& i : range2)
		{
			if (i >= 0 && i < midTime)
			{
				const gint x        = gint(((i - startTime) / duration) * w);
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
                cairo_move_to(cr, x, 5);
                pango_cairo_show_layout(cr, layout);
                cairo_move_to(cr, x, 0);
                cairo_line_to(cr, x, 3);
				g_object_unref(layout);
			}
		}
        cairo_stroke(cr); // Useful ??
        gdk_window_end_draw_frame(window,gdc);
        cairo_region_destroy(cairoRegion);
	}
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
