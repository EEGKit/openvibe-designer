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

namespace OpenViBE {
namespace AdvancedVisualization {
class CRulerRightCount final : public IRuler
{
public:
	void renderRight(GtkWidget* widget) override
	{
		if (m_renderer == nullptr) { return; }
		if (m_renderer->getSampleCount() == 0) { return; }
		if (m_renderer->getHistoryCount() == 0) { return; }
		if (m_renderer->getHistoryIndex() == 0) { return; }

		const double nSample    = double(m_renderer->getSampleCount());
		const double historyIdx = double(m_renderer->getHistoryIndex());
			  
		const double leftIdx1  = historyIdx - double(size_t(historyIdx) % size_t(nSample));
		const double leftIdx2  = historyIdx;
		const double rightIdx1 = leftIdx2 - nSample;
		const double rightIdx2 = leftIdx1;

		const std::vector<double> range1 = splitRange(leftIdx1, leftIdx1 + nSample, 10);
		const std::vector<double> range2 = splitRange(rightIdx1, rightIdx1 + nSample, 10);

		gint w, h, y;

		gdk_drawable_get_size(widget->window, &w, &h);
		GdkGC* drawGC = gdk_gc_new(widget->window);

		for (const auto& i : range1) {
			if (i >= leftIdx1 && i < leftIdx2) {
				y                   = gint(((i - leftIdx1) / nSample) * h);
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
				gdk_draw_layout(widget->window, drawGC, 5, y, layout);
				gdk_draw_line(widget->window, drawGC, 0, y, 3, y);
				g_object_unref(layout);
			}
		}
		for (const auto& i : range2) {
			if (i >= rightIdx1 && i < rightIdx2) {
				y                   = gint(((i + nSample - leftIdx1) / nSample) * h);
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
				gdk_draw_layout(widget->window, drawGC, 5, y, layout);
				gdk_draw_line(widget->window, drawGC, 0, y, 3, y);
				g_object_unref(layout);
			}
		}
		g_object_unref(drawGC);
	}
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
