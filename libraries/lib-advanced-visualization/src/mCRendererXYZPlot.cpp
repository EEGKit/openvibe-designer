///-------------------------------------------------------------------------------------------------
/// 
/// \file mCRendererXYZPlot.cpp
/// \copyright Copyright (C) 2022 Inria
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <https://www.gnu.org/licenses/>.
/// 
///-------------------------------------------------------------------------------------------------

#include "mCRendererXYZPlot.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {

void CRendererXYZPlot::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_hasDepth = ctx.hasXYZPlotDepth();
	m_plotDim  = (m_hasDepth ? 3 : 2);
	m_nPlot    = (ctx.getChannelCount() + m_plotDim - 1) / m_plotDim;
	m_vertex.resize(m_nPlot);
	const float inverseSampleCount = 1.0F / float(m_nSample < 2 ? 1 : (m_nSample - 1));
	for (size_t i = 0; i < m_nPlot; ++i) {
		m_vertex[i].resize(this->m_nSample);
		for (size_t j = 0; j < this->m_nSample; ++j) { m_vertex[i][j].u = float(j) * inverseSampleCount; }
	}

	m_historyIdx = 0;
}

void CRendererXYZPlot::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }

	while (m_historyIdx < m_nHistory) {
		const size_t j = m_historyIdx % this->m_nSample;
		for (size_t i = 0; i < m_nPlot; ++i) {
			if (m_hasDepth) {
				const size_t i3  = i * 3;
				m_vertex[i][j].x = (i3 < m_history.size() ? m_history[i3][m_historyIdx] : 0);
				m_vertex[i][j].y = (i3 + 1 < m_history.size() ? m_history[i3 + 1][m_historyIdx] : 0);
				m_vertex[i][j].z = (i3 + 2 < m_history.size() ? m_history[i3 + 2][m_historyIdx] : 0);
			}
			else {
				const size_t i3  = i * 2;
				m_vertex[i][j].x = (i3 < m_history.size() ? m_history[i3][m_historyIdx] : 0);
				m_vertex[i][j].y = (i3 + 1 < m_history.size() ? m_history[i3 + 1][m_historyIdx] : 0);
			}
		}
		m_historyIdx++;
	}
}

bool CRendererXYZPlot::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount() || m_vertex.empty() || !m_nHistory) { return false; }

	glPointSize(5);

	if (m_hasDepth) {
		const float d = 3.5;

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPerspective(60, double(ctx.getAspect()), .01, 100);
		glTranslatef(0, 0, -d);
		glRotatef(ctx.getRotationX() * 10, 1, 0, 0);
		glRotatef(ctx.getRotationY() * 10, 0, 1, 0);
	}

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glTranslatef(m_hasDepth ? 0 : 0.5F, m_hasDepth ? 0 : 0.5F, 0);
	glScalef(ctx.getZoom(), ctx.getZoom(), ctx.getZoom());

	if (ctx.isAxisDisplayed()) {
		if (m_hasDepth) { this->draw3DCoordinateSystem(); }
		else { this->draw2DCoordinateSystem(); }
	}

	const size_t n = m_nSample;
	const size_t d = (m_historyIdx % m_nSample);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (size_t i = 0; i < m_nPlot; ++i) {
		glPushMatrix();
		glScalef(ctx.getScale(), ctx.getScale(), ctx.getScale());

		glVertexPointer(GLint(m_plotDim), GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[i][n - d].u);
		glDrawArrays(GL_POINTS, 0, GLsizei(d));

		glVertexPointer(GLint(m_plotDim), GL_FLOAT, sizeof(CVertex), &m_vertex[i][d].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].u);
		glDrawArrays(GL_POINTS, 0, GLsizei((m_historyIdx > n ? n : m_historyIdx) - d));

		glPopMatrix();
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	if (m_hasDepth) {
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	glMatrixMode(GL_MODELVIEW);

	return true;
}

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
