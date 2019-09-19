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

	m_autoDecimationFactor = 1 + uint32_t((m_nSample - 1) / rContext.getMaximumSampleCountPerDisplay());

	m_vertex.clear();
	m_vertex.resize(m_nChannel);
	for (size_t i = 0; i < m_nChannel; ++i)
	{
		m_vertex[i].resize((m_nSample / m_autoDecimationFactor) * 4);
		for (size_t j = 0; j < m_nSample - m_autoDecimationFactor + 1; j += m_autoDecimationFactor)
		{
			const size_t l     = j / m_autoDecimationFactor;
			const size_t id    = l * 4;
			const float factor = m_autoDecimationFactor * m_nInverseSample;
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

	m_historyIdx = 0;
}

void CRendererBitmap::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_nHistory) { return; }
	if (m_vertex.empty()) { return; }

	for (uint32_t i = 0; i < m_nChannel; ++i)
	{
		uint32_t k                     = ((m_nHistory - 1) / m_nSample) * m_nSample;
		std::vector<float>& l_vHistory = m_history[i];
		CVertex* l_pVertex             = &m_vertex[i][0];
		for (uint32_t j = 0; j < m_nSample - m_autoDecimationFactor + 1; j += m_autoDecimationFactor, k += m_autoDecimationFactor)
		{
			if (k >= m_historyIdx && k < m_nHistory)
			{
				const float value = l_vHistory[k];
				l_pVertex++->u    = value;
				l_pVertex++->u    = value;
				l_pVertex++->u    = value;
				l_pVertex++->u    = value;
			}
			else { l_pVertex += 4; }
		}
	}
	m_historyIdx = m_nHistory;
}

bool CRendererBitmap::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) { return false; }
	if (m_vertex.empty()) { return false; }
	if (!m_nHistory) { return false; }

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
		glDrawArrays(GL_QUADS, 0, (m_nSample / m_autoDecimationFactor) * 4);
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
