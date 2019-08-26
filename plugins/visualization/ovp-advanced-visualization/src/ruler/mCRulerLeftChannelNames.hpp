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
#include <map>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerLeftChannelNames : public IRuler
		{
		public:

			void renderLeft(GtkWidget* pWidget) override
			{
				gint w, h;
				gint lw, lh;

				char l_sLabel[1024];

				gdk_drawable_get_size(pWidget->window, &w, &h);
				GdkGC* l_pDrawGC = gdk_gc_new(pWidget->window);
				for (uint32_t i = 0; i < m_pRendererContext->getSelectedCount(); ++i)
				{
					const uint32_t l_ui32Index = m_pRendererContext->getSelected(i);
					sprintf(l_sLabel, "%s (%i)", m_pRendererContext->getChannelName(l_ui32Index).c_str(), l_ui32Index + 1);
					PangoLayout* l_pPangoLayout = gtk_widget_create_pango_layout(pWidget, l_sLabel);
					pango_layout_get_size(l_pPangoLayout, &lw, &lh);
					lw /= PANGO_SCALE;
					lh /= PANGO_SCALE;
					gdk_draw_layout(pWidget->window, l_pDrawGC, w - lw, gint(((i + 0.5) * h) / m_pRendererContext->getSelectedCount() - float(lh) / 2), l_pPangoLayout);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(l_pDrawGC);
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
