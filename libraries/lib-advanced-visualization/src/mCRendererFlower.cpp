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

CRendererFlower::CRendererFlower(uint32_t ui32MultiCount)
{
	m_vMuliVertex.resize(ui32MultiCount);
}

void CRendererFlower::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	uint32_t i;

	m_autoDecimationFactor = 1 + uint32_t((m_sampleCount - 1) / rContext.getMaximumSampleCountPerDisplay());

	uint32_t n = m_sampleCount / m_autoDecimationFactor;

	for (uint32_t z = 0; z < m_vMuliVertex.size(); z++)
	{
		m_vMuliVertex[z].clear();
		m_vMuliVertex[z].resize(m_channelCount);
		for (i = 0; i < m_channelCount; ++i)
		{
			m_vMuliVertex[z][i].resize(n);
			for (uint32_t j = 0; j < m_sampleCount - m_autoDecimationFactor + 1; j += m_autoDecimationFactor)
			{
				m_vMuliVertex[z][i][j / m_autoDecimationFactor].u = j * m_inverseSampleCount;
			}
		}
	}

	m_vCircle.clear();
	m_vCircle.resize(n);
	for (i = 0; i < n; ++i)
	{
		m_vCircle[i].x = cosf(rContext.getFlowerRingCount() * i * static_cast<float>(M_PI) * 2.f / n);
		m_vCircle[i].y = sinf(rContext.getFlowerRingCount() * i * static_cast<float>(M_PI) * 2.f / n);
		m_vCircle[i].z = 0;
	}

	m_historyIndex = 0;
}

void CRendererFlower::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_historyCount) { return; }

	for (uint32_t z = 0; z < m_vMuliVertex.size(); z++)
	{
		for (uint32_t i = 0; i < m_channelCount; ++i)
		{
			uint32_t k = ((m_historyCount - 1 - z * m_vMuliVertex[z][i].size()) / m_sampleCount) * m_sampleCount;
			std::vector<float>& l_vHistory = m_history[i];
			CVertex* l_pVertex = &m_vMuliVertex[z][i][0];
			CVertex* l_pCircleVertex = &m_vCircle[0];
			for (uint32_t j = 0; j < m_sampleCount - m_autoDecimationFactor + 1; j += m_autoDecimationFactor, k += m_autoDecimationFactor)
			{
				float sum = 0;
				uint32_t count = 0;

				for (uint32_t l = 0; l < m_autoDecimationFactor; l++)
				{
					if (/*k+l>=m_historyIndex && */k + l < m_historyCount)
					{
						sum += l_vHistory[k + l];
						count++;
					}
				}

				if (count)
				{
					float v = sum / count;
					l_pVertex->x = l_pCircleVertex->x * v;
					l_pVertex->y = l_pCircleVertex->y * v;
					l_pVertex->z = l_pCircleVertex->z * v;
					l_pVertex++;
					l_pCircleVertex++;
				}
				else
				{
					l_pVertex++;
					l_pCircleVertex++;
				}
			}
		}
	}

	m_historyIndex = m_historyCount;
}

bool CRendererFlower::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) { return false; }
	if (m_vMuliVertex.empty()) { return false; }
	if (!m_historyCount) { return false; }

	uint32_t n = m_sampleCount / m_autoDecimationFactor;
	uint32_t d = (m_historyIndex % m_sampleCount) / m_autoDecimationFactor;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (uint32_t i = 0; i < rContext.getSelectedCount(); ++i)
	{
		glPushMatrix();
		glTranslatef(.5f, .5f, 0);
		glScalef(rContext.getScale(), rContext.getScale(), rContext.getScale());
		for (uint32_t z = 0; z < m_vMuliVertex.size(); z++)
		{
			if (!m_vMuliVertex[z].empty())
			{
				std::vector<CVertex>& l_vVertex = m_vMuliVertex[z][rContext.getSelected(i)];

				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_vVertex[0].x);
				glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_vVertex[n - d].u);
				glDrawArrays(GL_LINE_STRIP, 0, d);

				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_vVertex[d].x);
				glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_vVertex[0].u);
				glDrawArrays(GL_LINE_STRIP, 0, n - d);

				glBegin(GL_LINES);
				glTexCoord1fv(&l_vVertex[n - d].u);
				glVertex2fv(&l_vVertex[n - 1].x);
				glVertex2fv(&l_vVertex[0].x);
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
