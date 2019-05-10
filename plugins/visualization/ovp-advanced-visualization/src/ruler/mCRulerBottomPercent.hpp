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

			void renderBottom(GtkWidget* pWidget) override
			{
				char l_sLabel[1024];

				gint w, h;
				gint lw, lh;

				gdk_drawable_get_size(pWidget->window, &w, &h);
				GdkGC* l_pDrawGC = gdk_gc_new(pWidget->window);
				for (int i = 0; i <= 10; i += 2)
				{
					const gint x = (i * (w - 1)) / 10;
					sprintf(l_sLabel, "%i%%", i * 10);
					PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, l_sLabel);
					pango_layout_get_size(l_pPangoLayout, &lw, &lh);
					lw /= PANGO_SCALE;
					lh /= PANGO_SCALE;
					gdk_draw_layout(pWidget->window, l_pDrawGC, x, 4, l_pPangoLayout);
					gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(l_pDrawGC);
			}
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia
