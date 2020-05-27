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

class CRulerBottomCount : public IRuler
{
public:

	void renderBottom(GtkWidget* widget) override
	{
		if (m_renderer == nullptr) { return; }
		if (m_renderer->getSampleCount() == 0) { return; }
		if (m_renderer->getHistoryCount() == 0) { return; }
		if (m_renderer->getHistoryIndex() == 0) { return; }

		const size_t nSample    = m_renderer->getSampleCount();
		const size_t historyIdx = m_renderer->getHistoryIndex();


		const size_t leftIdx1  = historyIdx - historyIdx % nSample;
		const size_t leftIdx2  = historyIdx;
		const size_t rightIdx1 = leftIdx2 - nSample;
		const size_t rightIdx2 = leftIdx1;

		std::vector<double> range1 = splitRange(double(leftIdx1), double(leftIdx1 + nSample), 10);
		std::vector<double> range2 = splitRange(double(rightIdx1), double(rightIdx1 + nSample), 10);

		gint w, h, x;

		gdk_drawable_get_size(widget->window, &w, &h);
		GdkGC* drawGC = gdk_gc_new(widget->window);
		for (const auto& i : range1)
		{
			if (i >= leftIdx1 && i < leftIdx2)
			{
				x                   = gint(((i - leftIdx1) / nSample) * w);
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
				gdk_draw_layout(widget->window, drawGC, x, 5, layout);
				gdk_draw_line(widget->window, drawGC, x, 0, x, 3);
				g_object_unref(layout);
			}
		}
		for (const auto& i : range2)
		{
			if (i >= rightIdx1 && i < rightIdx2)
			{
				x                   = gint(((i + nSample - leftIdx1) / nSample) * w);
				PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
				gdk_draw_layout(widget->window, drawGC, x, 5, layout);
				gdk_draw_line(widget->window, drawGC, x, 0, x, 3);
				g_object_unref(layout);
			}
		}
		g_object_unref(drawGC);
	}
};

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
