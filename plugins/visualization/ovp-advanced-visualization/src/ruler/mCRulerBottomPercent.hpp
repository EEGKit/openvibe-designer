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
		class CRulerBottomPercent : public IRuler
		{
		public:

			void renderBottom(GtkWidget* widget) override
			{
				gint w, h;
				gint lw, lh;

				gdk_drawable_get_size(widget->window, &w, &h);
				GdkGC* drawGC = gdk_gc_new(widget->window);
				for (int i = 0; i <= 10; i += 2)
				{
					const gint x            = (i * (w - 1)) / 10;
					const std::string label = (std::to_string(i * 10) + "%");
					PangoLayout* layout = gtk_widget_create_pango_layout(widget, label.c_str());
					pango_layout_get_size(layout, &lw, &lh);
					lw /= PANGO_SCALE;
					lh /= PANGO_SCALE;
					gdk_draw_layout(widget->window, drawGC, x, 4, layout);
					gdk_draw_line(widget->window, drawGC, x, 0, x, 3);
					g_object_unref(layout);
				}
				g_object_unref(drawGC);
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
