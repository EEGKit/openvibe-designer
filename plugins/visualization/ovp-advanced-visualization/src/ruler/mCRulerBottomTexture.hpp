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

#include "mCRulerTexture.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {

class CRulerBottomTexture : public CRulerTexture
{
public:

	void render() override

	{
		this->preRender();

		glColor4f(0, 0, 0, m_blackAlpha);
		glBegin(GL_QUADS);
		glTexCoord1f(0);
		glVertex2f(0, 0.00F);
		glVertex2f(0, 0.05F);
		glTexCoord1f(1);
		glVertex2f(1, 0.05F);
		glVertex2f(1, 0.00F);
		glEnd();

		glColor4f(1, 1, 1, m_whiteAlpha);
		glBegin(GL_QUADS);
		glTexCoord1f(0);
		glVertex2f(0, 0.00F);
		glVertex2f(0, 0.04F);
		glTexCoord1f(1);
		glVertex2f(1, 0.04F);
		glVertex2f(1, 0.00F);
		glEnd();

		this->postRender();
	}

	void renderBottom(GtkWidget* widget) override
	{
		const float scale = 1.F / m_rendererCtx->getScale();
		if (m_lastScale != scale)
		{
			m_range     = splitRange(-scale * .5, scale * .5);
			m_lastScale = scale;
		}

		gint w, h;
		gint lw, lh;

		gdk_drawable_get_size(widget->window, &w, &h);
		GdkGC* drawGC = gdk_gc_new(widget->window);
		for (const auto& i : m_range)
		{
			PangoLayout* layout = gtk_widget_create_pango_layout(widget, getLabel(i).c_str());
			pango_layout_get_size(layout, &lw, &lh);
			lw /= PANGO_SCALE;
			lh /= PANGO_SCALE;
			gdk_draw_layout(widget->window, drawGC, gint((.5 + i / scale) * w - lw * .5), 0, layout);
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
