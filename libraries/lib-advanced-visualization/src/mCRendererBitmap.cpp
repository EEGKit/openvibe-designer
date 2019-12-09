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

void CRendererBitmap::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_autoDecimationFactor = 1 + size_t((m_nSample - 1) / ctx.getMaximumSampleCountPerDisplay());

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

void CRendererBitmap::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }
	if (m_vertex.empty()) { return; }

	for (size_t i = 0; i < m_nChannel; ++i)
	{
		size_t k                    = ((m_nHistory - 1) / m_nSample) * m_nSample;
		std::vector<float>& history = m_history[i];
		CVertex* vertex             = &m_vertex[i][0];
		for (size_t j = 0; j < m_nSample - m_autoDecimationFactor + 1; j += m_autoDecimationFactor, k += m_autoDecimationFactor)
		{
			if (k >= m_historyIdx && k < m_nHistory)
			{
				const float value = history[k];
				vertex++->u       = value;
				vertex++->u       = value;
				vertex++->u       = value;
				vertex++->u       = value;
			}
			else { vertex += 4; }
		}
	}
	m_historyIdx = m_nHistory;
}

bool CRendererBitmap::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount()) { return false; }
	if (m_vertex.empty()) { return false; }
	if (!m_nHistory) { return false; }

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(ctx.getScale(), 1, 1);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glScalef(1, 1.F / ctx.getSelectedCount(), 1);
	for (size_t i = 0; i < ctx.getSelectedCount(); ++i)
	{
		glPushMatrix();
		glTranslatef(0, float(ctx.getSelectedCount() - i) - 1.F, 0);
		glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_vertex[ctx.getSelected(i)][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[ctx.getSelected(i)][0].u);
		glDrawArrays(GL_QUADS, 0, GLsizei((m_nSample / m_autoDecimationFactor) * 4));
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
