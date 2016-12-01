/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#pragma once

#include "../mIRuler.hpp"

#include <string>
#include <map>
#include <iostream>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template<size_t dimension>
		class CRulerRightLabels : public IRuler
		{
		public:

			virtual void renderRight(::GtkWidget* widget)
			{

				gint w, h, y;
				gint lw, lh;

				uint32_t channelCount = m_pRendererContext->getSelectedCount();
				for (uint32_t channel = 0; channel < channelCount; ++channel)
				{
					::gdk_drawable_get_size(widget->window, &w, &h);
					::GdkGC* drawGC = gdk_gc_new(widget->window);

					float labelCount = static_cast<float>(m_pRendererContext->getDimensionLabelCount(dimension));

					gint last_y = gint((channel + (-1 + 0.5f)/labelCount) * (h * 1.f / channelCount));;

					for (uint32_t label = 0; label < m_pRendererContext->getDimensionLabelCount(dimension); ++label)
					{
						y = gint((channel + (label + 0.5f)/labelCount) * (h * 1.f / channelCount));
						if (y >= last_y + 10)
						{
							::PangoLayout* l_pPangoLayout = ::gtk_widget_create_pango_layout(widget, m_pRendererContext->getDimensionLabel(dimension, label));
							::pango_layout_get_size(l_pPangoLayout, &lw, &lh);
							lw/=PANGO_SCALE;
							lh/=PANGO_SCALE;
							::gdk_draw_layout(widget->window, drawGC, 8, h-y-lh/2, l_pPangoLayout);
							::gdk_draw_line(widget->window, drawGC, 0, h-y, 3, h-y);
							g_object_unref(l_pPangoLayout);
							last_y = y;
						}
					}
					g_object_unref(drawGC);
				}
			}
		};
	}
}

