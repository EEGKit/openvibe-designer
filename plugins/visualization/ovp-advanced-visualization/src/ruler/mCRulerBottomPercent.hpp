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

#ifndef __OpenViBEPlugins_CRulerBottomPercent_H__
#define __OpenViBEPlugins_CRulerBottomPercent_H__

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerBottomPercent : public IRuler
		{
		public:

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				char l_sLabel[1024];
				int i;

				gint w, h;
				gint lw, lh;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(i=0; i<=10; i+=2)
				{
					gint x=(i*(w-1))/10;
					::sprintf(l_sLabel, "%i%%", i*10);
					::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, l_sLabel);
					::pango_layout_get_size(l_pPangoLayout, &lw, &lh);
					lw/=PANGO_SCALE;
					lh/=PANGO_SCALE;
					::gdk_draw_layout(
						pWidget->window,
						l_pDrawGC,
						x,
						4,
						l_pPangoLayout);
					::gdk_draw_line(pWidget->window, l_pDrawGC, x, 0, x, 3);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(l_pDrawGC);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerBottomPercent_H__
