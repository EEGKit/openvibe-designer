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

#define _USE_MATH_DEFINES
#include <cmath>

#include "mCRendererFlower.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

CRendererFlower::CRendererFlower(const size_t multiCount) { m_muliVertices.resize(multiCount); }

void CRendererFlower::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_autoDecimationFactor = 1 + size_t((m_nSample - 1) / ctx.getMaximumSampleCountPerDisplay());

	const size_t n = m_nSample / m_autoDecimationFactor;

	for (auto& multivertex : m_muliVertices)
	{
		multivertex.clear();
		multivertex.resize(m_nChannel);
		for (size_t i = 0; i < m_nChannel; ++i)
		{
			multivertex[i].resize(n);
			for (size_t j = 0; j < m_nSample - m_autoDecimationFactor + 1; j += m_autoDecimationFactor)
			{
				multivertex[i][j / m_autoDecimationFactor].u = j * m_nInverseSample;
			}
		}
	}

	m_circles.clear();
	m_circles.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		m_circles[i].x = cosf(float(ctx.getFlowerRingCount()) * i * float(M_PI) * 2.F / n);
		m_circles[i].y = sinf(float(ctx.getFlowerRingCount()) * i * float(M_PI) * 2.F / n);
		m_circles[i].z = 0;
	}

	m_historyIdx = 0;
}

void CRendererFlower::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }

	for (size_t z = 0; z < m_muliVertices.size(); ++z)
	{
		for (size_t i = 0; i < m_nChannel; ++i)
		{
			size_t k                    = ((m_nHistory - 1 - z * m_muliVertices[z][i].size()) / m_nSample) * m_nSample;
			std::vector<float>& history = m_history[i];
			CVertex* vertex             = &m_muliVertices[z][i][0];
			CVertex* circleVertex       = &m_circles[0];
			for (size_t j = 0; j < m_nSample - m_autoDecimationFactor + 1; j += m_autoDecimationFactor, k += m_autoDecimationFactor)
			{
				float sum    = 0;
				size_t count = 0;

				for (size_t l = 0; l < m_autoDecimationFactor; ++l)
				{
					if (/*k+l>=m_historyIdx && */k + l < m_nHistory)
					{
						sum += history[k + l];
						count++;
					}
				}

				if (count)
				{
					const float v = sum / count;
					vertex->x     = circleVertex->x * v;
					vertex->y     = circleVertex->y * v;
					vertex->z     = circleVertex->z * v;
					vertex++;
					circleVertex++;
				}
				else
				{
					vertex++;
					circleVertex++;
				}
			}
		}
	}

	m_historyIdx = m_nHistory;
}

bool CRendererFlower::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount() || m_muliVertices.empty() || !m_nHistory) { return false; }

	const size_t n = m_nSample / m_autoDecimationFactor;
	const size_t d = (m_historyIdx % m_nSample) / m_autoDecimationFactor;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (size_t i = 0; i < ctx.getSelectedCount(); ++i)
	{
		glPushMatrix();
		glTranslatef(.5F, .5F, 0);
		glScalef(ctx.getScale(), ctx.getScale(), ctx.getScale());

		for (auto& multi : m_muliVertices)
		{
			if (!multi.empty())
			{
				std::vector<CVertex>& vertices = multi[ctx.getSelected(i)];

				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &vertices[0].x);
				glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &vertices[n - d].u);
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(d));

				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &vertices[d].x);
				glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &vertices[0].u);
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(n - d));

				glBegin(GL_LINES);
				glTexCoord1fv(&vertices[n - d].u);
				glVertex2fv(&vertices[n - 1].x);
				glVertex2fv(&vertices[0].x);
				glEnd();
			}
		}
		glPopMatrix();
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}
