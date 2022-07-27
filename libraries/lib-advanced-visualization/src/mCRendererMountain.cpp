///-------------------------------------------------------------------------------------------------
/// 
/// \file mCRendererMountain.cpp
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

#include "mCRendererMountain.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {

void CRendererMountain::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_mountain.m_Vertices.clear();
	m_mountain.m_Vertices.resize(m_nChannel * m_nSample);
	for (size_t i = 0, k = 0; i < m_nChannel; ++i) {
		for (size_t j = 0; j < m_nSample; ++j) {
			const float a              = float(i) * 1.0F / float(m_nChannel - 1);
			const float b              = 1 - float(j) * 1.0F / float(m_nSample - 1);
			m_mountain.m_Vertices[k].x = a;
			m_mountain.m_Vertices[k].y = 0;
			m_mountain.m_Vertices[k].z = b;
			m_mountain.m_Vertices[k].u = 0;
			k++;
		}
	}

	m_mountain.m_Triangles.clear();
	m_mountain.m_Triangles.resize((m_nChannel - 1) * (m_nSample - 1) * 6);
	size_t id = 0;
	for (size_t i = 0; i < m_nChannel - 1; ++i) {
		for (size_t j = 0; j < m_nSample - 1; ++j) {
			const uint32_t v1            = uint32_t(i * m_nSample + j);
			const uint32_t v2            = uint32_t(v1 + m_nSample);
			m_mountain.m_Triangles[id++] = v1;
			m_mountain.m_Triangles[id++] = v2;
			m_mountain.m_Triangles[id++] = v2 + 1;
			m_mountain.m_Triangles[id++] = v1;
			m_mountain.m_Triangles[id++] = v2 + 1;
			m_mountain.m_Triangles[id++] = v1 + 1;
		}
	}

	m_historyIdx = 0;
}

void CRendererMountain::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }


	for (size_t i = 0; i < ctx.getSelectedCount(); ++i) {
		size_t k                    = ((m_nHistory - 1) / m_nSample) * m_nSample;
		std::vector<float>& history = m_history[ctx.getSelected(i)];
		CVertex* vertex             = &m_mountain.m_Vertices[i * m_nSample];
		for (size_t j = 0; j < m_nSample; ++j, ++k) {
			if (/*k>=m_historyIdx && */k < m_nHistory) {
				vertex[j].u = history[k];
				vertex[j].y = history[k] / 2;
			}
		}
	}

	m_mountain.compile();

	m_historyIdx = m_nHistory;
}

bool CRendererMountain::render(const CRendererContext& ctx)
{
	if (m_mountain.m_Vertices.empty() || !m_nHistory) { return false; }

	const float d = 2.5F;

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, double(ctx.getAspect()), .01, 100);
	glTranslatef(0, -.2F, -d);
	glRotatef(ctx.getRotationX() * 10, 1, 0, 0);
	glRotatef(ctx.getRotationY() * 10, 0, 1, 0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(ctx.getScale(), 1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(3 * ctx.getZoom(), 3 * ctx.getZoom(), 3 * ctx.getZoom());
	glTranslatef(-.5F, 0, -.5F);
	glScalef(float(m_nChannel) * 1.0F / float(ctx.getSelectedCount()), ctx.getScale(), 1);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_mountain.m_Vertices[0].x);
	glNormalPointer(GL_FLOAT, sizeof(CVertex), &m_mountain.m_Normals[0].x);
	glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_mountain.m_Vertices[0].u);

	glColor3f(ctx.getTranslucency(), ctx.getTranslucency(), ctx.getTranslucency());
	glDrawElements(GL_TRIANGLES, GLsizei((ctx.getSelectedCount() - 1) * (m_nSample - 1) * 6), GL_UNSIGNED_INT, &m_mountain.m_Triangles[0]);
	/*
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0, 0, 0);
	glDrawElements(GL_TRIANGLES, m_mountain.m_Triangles.size(), GL_UNSIGNED_INT, &m_mountain.m_Triangles[0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	*/

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
