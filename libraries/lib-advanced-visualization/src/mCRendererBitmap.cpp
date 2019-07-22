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

#include "mCRendererBitmap.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererBitmap::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	m_autoDecimationFactor = 1 + uint32_t((m_sampleCount - 1) / rContext.getMaximumSampleCountPerDisplay());

	m_vertex.clear();
	m_vertex.resize(m_channelCount);
	for (size_t i = 0; i < m_channelCount; ++i)
	{
		m_vertex[i].resize((m_sampleCount / m_autoDecimationFactor) * 4);
		for (size_t j = 0; j < m_sampleCount - m_autoDecimationFactor + 1; j += m_autoDecimationFactor)
		{
			const size_t l     = j / m_autoDecimationFactor;
			const size_t id    = l * 4;
			const float factor = m_autoDecimationFactor * m_inverseSampleCount;
			const float value  = l * factor;
			m_vertex[i][id].x  = value;
			m_vertex[i][id].y  = 0;

			m_vertex[i][id + 1].x = value + factor;
			m_vertex[i][id + 1].y = 0;

			m_vertex[i][id + 2].x = value + factor;
			m_vertex[i][id + 2].y = 1;

			m_vertex[i][id + 3].x = value;
			m_vertex[i][id + 3].y = 1;
		}
	}

	m_historyIndex = 0;
}

void CRendererBitmap::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_historyCount) { return; }
	if (m_vertex.empty())
	{
		return;
	}

	for (uint32_t i = 0; i < m_channelCount; ++i)
	{
		uint32_t k                     = ((m_historyCount - 1) / m_sampleCount) * m_sampleCount;
		std::vector<float>& l_vHistory = m_history[i];
		CVertex* l_pVertex             = &m_vertex[i][0];
		for (uint32_t j = 0; j < m_sampleCount - m_autoDecimationFactor + 1; j += m_autoDecimationFactor, k += m_autoDecimationFactor)
		{
			if (k >= m_historyIndex && k < m_historyCount)
			{
				const float value = l_vHistory[k];
				l_pVertex++->u    = value;
				l_pVertex++->u    = value;
				l_pVertex++->u    = value;
				l_pVertex++->u    = value;
			}
			else
			{
				l_pVertex += 4;
			}
		}
	}
	m_historyIndex = m_historyCount;
}

bool CRendererBitmap::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) { return false; }
	if (m_vertex.empty()) { return false; }
	if (!m_historyCount) { return false; }

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(rContext.getScale(), 1, 1);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glScalef(1, 1.f / rContext.getSelectedCount(), 1);
	for (uint32_t i = 0; i < rContext.getSelectedCount(); ++i)
	{
		glPushMatrix();
		glTranslatef(0, float(rContext.getSelectedCount() - i) - 1.f, 0);
		glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_vertex[rContext.getSelected(i)][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[rContext.getSelected(i)][0].u);
		glDrawArrays(GL_QUADS, 0, (m_sampleCount / m_autoDecimationFactor) * 4);
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
