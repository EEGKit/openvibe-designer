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

#include "mCRendererXYZPlot.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererXYZPlot::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	m_hasDepth      = rContext.hasXYZPlotDepth();
	m_plotDimension = (m_hasDepth ? 3 : 2);
	m_nPlot     = (rContext.getChannelCount() + m_plotDimension - 1) / m_plotDimension;
	m_vertex.resize(m_nPlot);
	const float inverseSampleCount = 1.0f / float(m_nSample < 2 ? 1 : (m_nSample - 1));
	for (uint32_t i = 0; i < m_nPlot; ++i)
	{
		m_vertex[i].resize(this->m_nSample);
		for (uint32_t j = 0; j < this->m_nSample; j++) { m_vertex[i][j].u = j * inverseSampleCount; }
	}

	m_historyIdx = 0;
}

void CRendererXYZPlot::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_nHistory) { return; }

	while (m_historyIdx < m_nHistory)
	{
		const uint32_t j = m_historyIdx % this->m_nSample;
		for (uint32_t i = 0; i < m_nPlot; ++i)
		{
			if (m_hasDepth)
			{
				const uint32_t i3 = i * 3;
				m_vertex[i][j].x  = (i3 < m_history.size() ? m_history[i3][m_historyIdx] : 0);
				m_vertex[i][j].y  = (i3 + 1 < m_history.size() ? m_history[i3 + 1][m_historyIdx] : 0);
				m_vertex[i][j].z  = (i3 + 2 < m_history.size() ? m_history[i3 + 2][m_historyIdx] : 0);
			}
			else
			{
				const uint32_t i3 = i * 2;
				m_vertex[i][j].x  = (i3 < m_history.size() ? m_history[i3][m_historyIdx] : 0);
				m_vertex[i][j].y  = (i3 + 1 < m_history.size() ? m_history[i3 + 1][m_historyIdx] : 0);
			}
		}
		m_historyIdx++;
	}
}

bool CRendererXYZPlot::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) { return false; }
	if (m_vertex.empty()) { return false; }
	if (!m_nHistory) { return false; }

	glPointSize(5);

	if (m_hasDepth)
	{
		const float d = 3.5;

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPerspective(60, rContext.getAspect(), .01, 100);
		glTranslatef(0, 0, -d);
		glRotatef(rContext.getRotationX() * 10, 1, 0, 0);
		glRotatef(rContext.getRotationY() * 10, 0, 1, 0);
	}

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glTranslatef(m_hasDepth ? 0 : 0.5f, m_hasDepth ? 0 : 0.5f, 0);
	glScalef(rContext.getZoom(), rContext.getZoom(), rContext.getZoom());

	if (rContext.isAxisDisplayed())
	{
		if (m_hasDepth) { this->draw3DCoordinateSystem(); }
		else { this->draw2DCoordinateSystem(); }
	}

	uint32_t n       = m_nSample;
	const uint32_t d = (m_historyIdx % m_nSample);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (uint32_t i = 0; i < m_nPlot; ++i)
	{
		glPushMatrix();
		glScalef(rContext.getScale(), rContext.getScale(), rContext.getScale());

		glVertexPointer(m_plotDimension, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[i][n - d].u);
		glDrawArrays(GL_POINTS, 0, d);

		glVertexPointer(m_plotDimension, GL_FLOAT, sizeof(CVertex), &m_vertex[i][d].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].u);
		glDrawArrays(GL_POINTS, 0, (m_historyIdx > n ? n : m_historyIdx) - d);

		glPopMatrix();
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	if (m_hasDepth)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	glMatrixMode(GL_MODELVIEW);

	return true;
}
