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

#ifndef __OpenViBEPlugins_CRulerLeftChannelNames_H__
#define __OpenViBEPlugins_CRulerLeftChannelNames_H__

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

			virtual void renderLeft(::GtkWidget* pWidget)
			{
				gint w, h;
				gint lw, lh;

				char l_sLabel[1024];
				uint32 i, l_ui32Index;

				::gdk_drawable_get_size(pWidget->window, &w, &h);
				::GdkGC* l_pDrawGC=gdk_gc_new(pWidget->window);
				for(i=0; i<m_pRendererContext->getSelectedCount(); i++)
				{
					l_ui32Index=m_pRendererContext->getSelected(i);
					::sprintf(l_sLabel, "%s (%i)", m_pRendererContext->getChannelName(l_ui32Index).c_str(), l_ui32Index+1);
					::PangoLayout* l_pPangoLayout=::gtk_widget_create_pango_layout(pWidget, l_sLabel);
					::pango_layout_get_size(l_pPangoLayout, &lw, &lh);
					lw/=PANGO_SCALE;
					lh/=PANGO_SCALE;
					::gdk_draw_layout(
						pWidget->window,
						l_pDrawGC,
						w-lw,
						gint(((i+.5)*h)/m_pRendererContext->getSelectedCount()-lh/2),
						l_pPangoLayout);
					g_object_unref(l_pPangoLayout);
				}
				g_object_unref(l_pDrawGC);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerLeftChannelNames_H__
