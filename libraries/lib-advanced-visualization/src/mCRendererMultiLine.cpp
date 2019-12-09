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

#include <cstdint>
#include "mCRendererMultiLine.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

bool CRendererMultiLine::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount()) { return false; }
	if (!m_nHistory) { return false; }

	const auto nSample = int(m_nSample);
	const auto n1      = int(m_historyIdx % m_nSample);
	const auto n2      = int(nSample - n1);

	if (!nSample) { return false; }

	const float t1 = n2 * 1.F / nSample;
	const float t2 = -n1 * 1.F / nSample;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0, ctx.isPositiveOnly() ? 0 : 0.5F, 0);
	glScalef(1, ctx.getScale(), 1);
	glEnableClientState(GL_VERTEX_ARRAY);
	for (size_t i = 0; i < ctx.getSelectedCount(); ++i)
	{
		std::vector<CVertex>& vertices = m_vertices[ctx.getSelected(i)];
		glTexCoord1f(1 - (i + .5F) / ctx.getSelectedCount());
		if (ctx.isScrollModeActive())
		{
			glPushMatrix();
			glTranslatef(t1, 0, 0);
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &vertices[0].x);
			glDrawArrays(GL_LINE_STRIP, 0, n1);
			glPopMatrix();
			if (n2 > 0)
			{
				glPushMatrix();
				glTranslatef(t2, 0, 0);
				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &vertices[n1].x);
				glDrawArrays(GL_LINE_STRIP, 0, n2);
				glPopMatrix();

				if (n1 > 0)
				{
					glBegin(GL_LINES);
					glVertex2f(vertices[nSample - 1].x + t2, vertices[nSample - 1].y);
					glVertex2f(vertices[0].x + t1, vertices[0].y);
					glEnd();
				}
			}
		}
		else
		{
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &vertices[0].x);
			glDrawArrays(GL_LINE_STRIP, 0, nSample);
		}
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_1D);
	glColor3f(.2F, .2F, .2F);
	glBegin(GL_LINES);
	glVertex2f(0, ctx.isPositiveOnly() ? 0 : 0.5F);
	glVertex2f(1, ctx.isPositiveOnly() ? 0 : 0.5F);
	glEnd();

	return true;
}
