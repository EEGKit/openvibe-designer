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

#include "mCRendererBars.hpp"

using namespace OpenViBE;
using namespace AdvancedVisualization;

void CRendererBars::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_vertex.resize(m_nChannel);
	for (size_t i = 0; i < m_nChannel; ++i)
	{
		m_vertex[i].resize(size_t(m_nSample) * 4);
		for (size_t j = 0; j < m_nSample; ++j)
		{
			const size_t id       = j * 4;
			const float value     = j * m_nInverseSample;
			m_vertex[i][id].x     = value;
			m_vertex[i][id + 1].x = value + m_nInverseSample;
			m_vertex[i][id + 2].x = value + m_nInverseSample;
			m_vertex[i][id + 3].x = value;

			m_vertex[i][id].u     = value;
			m_vertex[i][id + 1].u = value;
			m_vertex[i][id + 2].u = value;
			m_vertex[i][id + 3].u = value;
		}
	}

	m_historyIdx = 0;
}

void CRendererBars::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }

	for (size_t i = 0; i < m_nChannel; ++i)
	{
		size_t k                    = ((m_nHistory - 1) / m_nSample) * m_nSample;
		std::vector<float>& history = m_history[i];
		CVertex* vertex             = &m_vertex[i][0];
		for (size_t j = 0; j < m_nSample; j++, k++)
		{
			if (k >= m_historyIdx && k < m_nHistory)
			{
				const float value = history[k];
				vertex++->y       = 0;
				vertex++->y       = 0;
				vertex++->y       = value;
				vertex++->y       = value;
			}
			else { vertex += 4; }
		}
	}
	m_historyIdx = m_nHistory;
}

bool CRendererBars::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount() || m_vertex.empty() || !m_nHistory) { return false; }

	size_t i;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(1, 1.F / ctx.getSelectedCount(), 1);
	glTranslatef(0, ctx.isPositiveOnly() ? 0 : 0.5F, 0);

	glPushAttrib(GL_CURRENT_BIT);
	glDisable(GL_TEXTURE_1D);
	glColor3f(.2F, .2F, .2F);
	glBegin(GL_LINES);
	for (i = 0; i < ctx.getSelectedCount(); ++i)
	{
		glVertex2f(0, float(i));
		glVertex2f(1, float(i));
	}
	glEnd();
	glEnable(GL_TEXTURE_1D);
	glPopAttrib();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (i = 0; i < ctx.getSelectedCount(); ++i)
	{
		glPushMatrix();
		glTranslatef(0, float(ctx.getSelectedCount()) - i - 1.F, 0);
		glScalef(1, ctx.getScale(), 1);
		glVertexPointer(2, GL_FLOAT, sizeof(CVertex), &m_vertex[ctx.getSelected(i)][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[ctx.getSelected(i)][0].u);
		glDrawArrays(GL_QUADS, 0, GLsizei(m_nSample * 4));
		glPopMatrix();
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}
