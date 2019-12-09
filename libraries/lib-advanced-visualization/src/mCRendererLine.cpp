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
#include "mCRendererLine.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

CRendererLine::CRendererLine() = default;

void CRendererLine::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_vertices.clear();
	m_vertices.resize(m_nChannel);

	for (size_t i = 0; i < m_nChannel; ++i)
	{
		m_vertices[i].resize(m_nSample);
		for (size_t j = 0; j < m_nSample; ++j) { m_vertices[i][j].x = j * m_nInverseSample; }
	}

	m_historyIdx = 0;
}

void CRendererLine::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }

	size_t maxIdx;

	if (m_historyDrawIdx == 0) { maxIdx = m_nHistory; }	// Draw real-time 
	else { maxIdx = m_historyDrawIdx; }					// stay at the m_historyDrawIdx

	for (size_t i = 0; i < m_nChannel; ++i)
	{
		const size_t firstIdx = ((maxIdx - 1) / m_nSample) * m_nSample;
		std::vector<float>& history   = m_history[i];
		CVertex* vertex               = &m_vertices[i][0];

		for (size_t j = 0; j < m_nSample; ++j)
		{
			const size_t idx = firstIdx + j;

			if (idx < maxIdx) { vertex->y = history[idx]; }
			else if (idx >= m_nSample) { vertex->y = history[idx - m_nSample]; }

			vertex++;
		}
	}

	m_historyIdx = maxIdx;
}

bool CRendererLine::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount()) { return false; }
	if (!m_nHistory) { return false; }

	const auto nSample = int(m_nSample);


	// When the display is in continuous mode, there will be n1 samples
	// displayed until the 'cursor' and n2 samples will be displayed to
	// complete the whole window.
	// For example at 25s with a display size of 20s
	// <- n1 samples -><------------ n2 samples ---------------->
	// | ____         |                    ___                  |
	// |/    \        |      ___          /   \     ____        |
	// |------\------/|-----/---\--------/-----\___/----\-------|
	// |       \____/ |    /     \______/                \______|
	// |              |___/                                     |
	// Time          25s              10s                      20s

	const auto n1 = int(m_historyIdx % m_nSample);
	const auto n2 = int(nSample - n1);

	if (!nSample) { return false; }

	const float t1 = n2 * 1.F / nSample;
	const float t2 = -n1 * 1.F / nSample;

	glDisable(GL_TEXTURE_1D);

	glPushMatrix();
	glScalef(1, 1.F / ctx.getSelectedCount(), 1);
	glTranslatef(0, ctx.isPositiveOnly() ? 0 : 0.5F, 0);

	glPushAttrib(GL_CURRENT_BIT);
	glColor3f(.2F, .2F, .2F);
	glBegin(GL_LINES);
	for (size_t i = 0; i < ctx.getSelectedCount(); ++i)
	{
		glVertex2f(0, float(i));
		glVertex2f(1, float(i));
	}
	glEnd();
	glPopAttrib();

	glEnableClientState(GL_VERTEX_ARRAY);
	for (size_t i = 0; i < ctx.getSelectedCount(); ++i)
	{
		glPushMatrix();
		glTranslatef(0, float(ctx.getSelectedCount() - i) - 1.F, 0);
		glScalef(1, ctx.getScale(), 1);

		std::vector<CVertex>& vertices = m_vertices[ctx.getSelected(i)];
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
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

	return true;
}
