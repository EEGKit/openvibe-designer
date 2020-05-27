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

class CRulerBottomFrequency : public IRuler
{
public:

	void renderBottom(GtkWidget* widget) override
	{
		const auto scale = float(m_rendererCtx->getSpectrumFrequencyRange());
		if (m_lastScale != scale)
		{
			m_range     = splitRange(0, scale);
			m_lastScale = scale;
		}

		gint w, h;

		gdk_drawable_get_size(widget->window, &w, &h);
		GdkGC* drawGC = gdk_gc_new(widget->window);
		for (const auto& i : m_range)
		{
			const gint x        = gint((i / scale) * w);
			PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
			gdk_draw_layout(widget->window, drawGC, x, 5, layout);
			gdk_draw_line(widget->window, drawGC, x, 0, x, 3);
			g_object_unref(layout);
		}
		g_object_unref(drawGC);
	}

protected:
	float m_lastScale = 0;
	std::vector<double> m_range;
};

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
