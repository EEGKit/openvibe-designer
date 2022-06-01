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

#include "mCRendererSlice.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {

void CRendererSlice::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);


	m_vertices.clear();
	m_vertices.resize(m_nSample * m_nChannel * 8);

	m_quads.clear();
	m_quads.resize(m_nSample * m_nChannel * 6 * 4);

	size_t k = 0, l = 0;
	for (size_t i = 0; i < m_nSample; ++i) {
		for (size_t j = 0; j < m_nChannel; ++j) {
			const float size = .5;

			m_quads[l++] = k + 0;
			m_quads[l++] = k + 1; // q0
			m_quads[l++] = k + 2;
			m_quads[l++] = k + 3;

			m_quads[l++] = k + 4;
			m_quads[l++] = k + 7; // q1
			m_quads[l++] = k + 6;
			m_quads[l++] = k + 5;

			m_quads[l++] = k + 5;
			m_quads[l++] = k + 1; // q2
			m_quads[l++] = k + 2;
			m_quads[l++] = k + 6;

			m_quads[l++] = k + 4;
			m_quads[l++] = k + 7; // q3
			m_quads[l++] = k + 3;
			m_quads[l++] = k + 0;

			m_quads[l++] = k + 5;
			m_quads[l++] = k + 4; // q4
			m_quads[l++] = k + 0;
			m_quads[l++] = k + 1;

			m_quads[l++] = k + 6;
			m_quads[l++] = k + 7; // q5
			m_quads[l++] = k + 3;
			m_quads[l++] = k + 2;

			const float ox = 0;
			const float oy = 0.5F * float(m_nChannel - 1 - j);
			const float oz = 0.5F * float(m_nSample - 1 - i);

			m_vertices[k].x = ox + size;
			m_vertices[k].y = oy - size; // v0
			m_vertices[k].z = oz + size;
			k++;

			m_vertices[k].x = ox + size;
			m_vertices[k].y = oy + size; // v1
			m_vertices[k].z = oz + size;
			k++;

			m_vertices[k].x = ox + size;
			m_vertices[k].y = oy + size; // v2
			m_vertices[k].z = oz - size;
			k++;

			m_vertices[k].x = ox + size;
			m_vertices[k].y = oy - size; // v3
			m_vertices[k].z = oz - size;
			k++;

			m_vertices[k].x = ox - size;
			m_vertices[k].y = oy - size; // v4
			m_vertices[k].z = oz + size;
			k++;

			m_vertices[k].x = ox - size;
			m_vertices[k].y = oy + size; // v5
			m_vertices[k].z = oz + size;
			k++;

			m_vertices[k].x = ox - size;
			m_vertices[k].y = oy + size; // v6
			m_vertices[k].z = oz - size;
			k++;

			m_vertices[k].x = ox - size;
			m_vertices[k].y = oy - size; // v7
			m_vertices[k].z = oz - size;
			k++;
		}
	}
	m_historyIdx = 0;
}

void CRendererSlice::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	for (size_t i = m_historyIdx; i < m_nHistory; ++i) {
		size_t k = (i % m_nSample) * m_nChannel * 8;
		for (size_t j = 0; j < m_nChannel; ++j) { for (size_t l = 0; l < 8; ++l) { m_vertices[k++].u = m_history[j][i]; } }
	}

	m_historyIdx = m_nHistory;
}

bool CRendererSlice::render(const CRendererContext& ctx)
{
	// size_t i, j;
	const float d = 3.5;

	if (!ctx.getSelectedCount() || !m_nHistory) { return false; }

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	// ::glEnable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, double(ctx.getAspect()), .01, 100);
	glTranslatef(0, 0, -d);
	glRotatef(ctx.getRotationX() * 10, 1, 0, 0);
	glRotatef(ctx.getRotationY() * 10, 0, 1, 0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(ctx.getScale(), 1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(ctx.getZoom(), ctx.getZoom(), ctx.getZoom());
	glScalef(1., 1., 3.);

	glPushMatrix();
	glScalef(1.0F / float(ctx.getStackCount()), 1.0F / float(m_nChannel), 1.0F / float(m_nSample));
	glTranslatef(float(ctx.getStackIndex()) - 0.5F * float(ctx.getStackCount() - 1), 0, 0);
	glColor4f(0.1F, 0.1F, 0.1F, ctx.getTranslucency());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_vertices[0].x);
	glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertices[0].u);
	glDrawElements(GL_QUADS, GLsizei(m_quads.size()), GL_UNSIGNED_INT, &m_quads[0]);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glDisable(GL_TEXTURE_1D);

	const float progress = 1 - 2.0F * float(m_historyIdx % m_nSample) / float(m_nSample);
	glScalef(.5, .5, .5);
	glBegin(GL_LINE_LOOP);
	glColor3f(1, 1, 1);
	glVertex3f(-1, -1, -1);
	glVertex3f(-1, 1, -1);
	glVertex3f(1, 1, -1);
	glVertex3f(1, -1, -1);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3f(1, 1, 1);
	glVertex3f(-1, -1, 1);
	glVertex3f(-1, 1, 1);
	glVertex3f(1, 1, 1);
	glVertex3f(1, -1, 1);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3f(1, 1, 1);
	glVertex3f(-1, -1, -1);
	glVertex3f(-1, 1, -1);
	glVertex3f(-1, 1, 1);
	glVertex3f(-1, -1, 1);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3f(1, 1, 1);
	glVertex3f(1, -1, -1);
	glVertex3f(1, 1, -1);
	glVertex3f(1, 1, 1);
	glVertex3f(1, -1, 1);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor4f(0.25F, 1, 0.25F, 0.9F / float(ctx.getStackCount()));
	glVertex3f(-1, -1, progress);
	glVertex3f(-1, 1, progress);
	glVertex3f(1, 1, progress);
	glVertex3f(1, -1, progress);
	glEnd();
	glBegin(GL_QUADS);
	glColor4f(0.25F, 1, 0.25F, 0.1F / float(ctx.getStackCount()));
	glVertex3f(-1, -1, progress);
	glVertex3f(-1, 1, progress);
	glVertex3f(1, 1, progress);
	glVertex3f(1, -1, progress);
	glEnd();

	glEnable(GL_TEXTURE_1D);

	if (ctx.getCheckBoardVisibility()) { this->drawCoordinateSystem(); }

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
