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

bool CRendererMultiLine::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) { return false; }
	if (!m_nHistory) { return false; }

	const auto nSample = int(m_nSample);
	const auto n1          = int(m_historyIdx % m_nSample);
	const auto n2          = int(nSample - n1);

	if (!nSample) { return false; }

	const float t1 = n2 * 1.f / nSample;
	const float t2 = -n1 * 1.f / nSample;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0, rContext.isPositiveOnly() ? 0 : 0.5f, 0);
	glScalef(1, rContext.getScale(), 1);
	glEnableClientState(GL_VERTEX_ARRAY);
	for (uint32_t i = 0; i < rContext.getSelectedCount(); ++i)
	{
		std::vector<CVertex>& l_rVertex = m_Vertices[rContext.getSelected(i)];
		glTexCoord1f(1 - (i + .5f) / rContext.getSelectedCount());
		if (rContext.isScrollModeActive())
		{
			glPushMatrix();
			glTranslatef(t1, 0, 0);
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
			glDrawArrays(GL_LINE_STRIP, 0, n1);
			glPopMatrix();
			if (n2 > 0)
			{
				glPushMatrix();
				glTranslatef(t2, 0, 0);
				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[n1].x);
				glDrawArrays(GL_LINE_STRIP, 0, n2);
				glPopMatrix();

				if (n1 > 0)
				{
					glBegin(GL_LINES);
					glVertex2f(l_rVertex[nSample - 1].x + t2, l_rVertex[nSample - 1].y);
					glVertex2f(l_rVertex[0].x + t1, l_rVertex[0].y);
					glEnd();
				}
			}
		}
		else
		{
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
			glDrawArrays(GL_LINE_STRIP, 0, nSample);
		}
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_1D);
	glColor3f(.2f, .2f, .2f);
	glBegin(GL_LINES);
	glVertex2f(0, rContext.isPositiveOnly() ? 0 : 0.5f);
	glVertex2f(1, rContext.isPositiveOnly() ? 0 : 0.5f);
	glEnd();

	return true;
}
