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
		class CRulerRightScale : public IRuler
		{
		public:

			CRulerRightScale() : m_fLastScale(-1) { }

			void renderRight(GtkWidget* pWidget) override
			{
				const uint32_t selectedCount = m_pRendererContext->getSelectedCount();
				if (!selectedCount) { return; }

				const float l_fScale = 1.f / m_pRendererContext->getScale();
				if (m_fLastScale != l_fScale)
				{
					if (m_pRendererContext->isPositiveOnly())
					{
						m_vRange = this->split_range(0, l_fScale, IRuler_SplitCount);
					}
					else
					{
						m_vRange = this->split_range(-l_fScale * .5, l_fScale * .5, IRuler_SplitCount);
					}
					m_fLastScale = l_fScale;
				}

				const float l_fOffset = m_pRendererContext->isPositiveOnly() ? 0 : 0.5;

				gint w, h, lw, lh;

				gdk_drawable_get_size(pWidget->window, &w, &h);
				GdkGC* l_pDrawGC = gdk_gc_new(pWidget->window);
				for (unsigned int i = 0; i < m_pRendererContext->getSelectedCount(); ++i)
				{
					for (it = m_vRange.begin(); it != m_vRange.end(); ++it)
					{
						PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, this->getLabel(*it).c_str());
						pango_layout_get_size(l_pPangoLayout, &lw, &lh);
						lw /= PANGO_SCALE;
						lh /= PANGO_SCALE;
						const gint y = gint((1 - (float(i) + l_fOffset + *it / l_fScale) / selectedCount) * h);
						gdk_draw_layout(pWidget->window, l_pDrawGC, 8, y - lh / 2, l_pPangoLayout);
						gdk_draw_line(pWidget->window, l_pDrawGC, 0, y, 3, y);
						g_object_unref(l_pPangoLayout);
					}
				}
				g_object_unref(l_pDrawGC);
			}

			float m_fLastScale = 1;
			std::vector<double> m_vRange;
			std::vector<double>::iterator it;
		};
	} // namespace AdvancedVisualization
}  // namespace Mensia
