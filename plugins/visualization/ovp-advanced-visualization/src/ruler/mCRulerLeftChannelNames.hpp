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
		gint w, h, lw, lh;
		gdk_drawable_get_size(widget->window, &w, &h);
		GdkGC* drawGC = gdk_gc_new(widget->window);
		for (size_t i = 0; i < m_rendererCtx->getSelectedCount(); ++i)
		{
			const size_t idx        = m_rendererCtx->getSelected(i);
			const std::string label = (m_rendererCtx->getChannelName(idx) + " (" + std::to_string(idx + 1) + ")");
			PangoLayout* layout     = gtk_widget_create_pango_layout(widget, label.c_str());
			pango_layout_get_size(layout, &lw, &lh);
			lw /= PANGO_SCALE;
			lh /= PANGO_SCALE;
			gdk_draw_layout(widget->window, drawGC, w - lw, gint(((i + 0.5) * h) / m_rendererCtx->getSelectedCount() - float(lh) / 2), layout);
			g_object_unref(layout);
		}
		g_object_unref(drawGC);
	}
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
