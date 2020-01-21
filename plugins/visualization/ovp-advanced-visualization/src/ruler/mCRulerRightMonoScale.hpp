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
		class CRulerRightMonoScale : public IRuler
		{
		public:

			CRulerRightMonoScale() : m_lastScale(-1) { }

			void renderRight(GtkWidget* widget) override
			{
				const float scale = 1.F / m_rendererCtx->getScale();
				if (m_lastScale != scale)
				{
					if (m_rendererCtx->isPositiveOnly()) { m_range = splitRange(0, scale, IRuler_SplitCount); }
					else { m_range = splitRange(-scale * .5, scale * .5, IRuler_SplitCount); }
					m_lastScale = scale;
				}

				const float offset = m_rendererCtx->isPositiveOnly() ? 0 : 0.5F;

				gint w, h, lw, lh;

				gdk_drawable_get_size(widget->window, &w, &h);
				GdkGC* drawGC = gdk_gc_new(widget->window);
				for (const auto& i : m_range)
				{
					PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
					pango_layout_get_size(layout, &lw, &lh);
					lw /= PANGO_SCALE;
					lh /= PANGO_SCALE;
					const gint y = gint((1 - (offset + i / scale)) * h);
					gdk_draw_layout(widget->window, drawGC, 8, y - lh / 2, layout);
					gdk_draw_line(widget->window, drawGC, 0, y, 3, y);
					g_object_unref(layout);
				}
				g_object_unref(drawGC);
			}

		protected:
			float m_lastScale = 1;
			std::vector<double> m_range;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
