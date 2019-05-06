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
#include "m_RendererTools.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererSlice::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	uint32_t i, j, k = 0, l = 0;

	m_vVertex.clear();
	m_vVertex.resize(m_ui32SampleCount * m_ui32ChannelCount * 8);

	m_vQuad.clear();
	m_vQuad.resize(m_ui32SampleCount * m_ui32ChannelCount * 6 * 4);

	for (i = 0; i < m_ui32SampleCount; ++i)
	{
		for (j = 0; j < m_ui32ChannelCount; j++)
		{
			const float f32Size = .5;

			m_vQuad[l++] = k + 0;
			m_vQuad[l++] = k + 1; // q0
			m_vQuad[l++] = k + 2;
			m_vQuad[l++] = k + 3;

			m_vQuad[l++] = k + 4;
			m_vQuad[l++] = k + 7; // q1
			m_vQuad[l++] = k + 6;
			m_vQuad[l++] = k + 5;

			m_vQuad[l++] = k + 5;
			m_vQuad[l++] = k + 1; // q2
			m_vQuad[l++] = k + 2;
			m_vQuad[l++] = k + 6;

			m_vQuad[l++] = k + 4;
			m_vQuad[l++] = k + 7; // q3
			m_vQuad[l++] = k + 3;
			m_vQuad[l++] = k + 0;

			m_vQuad[l++] = k + 5;
			m_vQuad[l++] = k + 4; // q4
			m_vQuad[l++] = k + 0;
			m_vQuad[l++] = k + 1;

			m_vQuad[l++] = k + 6;
			m_vQuad[l++] = k + 7; // q5
			m_vQuad[l++] = k + 3;
			m_vQuad[l++] = k + 2;

			float ox = 0;
			float oy = ((m_ui32ChannelCount - 1) * .5f - j);
			float oz = ((m_ui32SampleCount - 1) * .5f - i);

			m_vVertex[k].x = ox + f32Size;
			m_vVertex[k].y = oy - f32Size; // v0
			m_vVertex[k].z = oz + f32Size;
			k++;

			m_vVertex[k].x = ox + f32Size;
			m_vVertex[k].y = oy + f32Size; // v1
			m_vVertex[k].z = oz + f32Size;
			k++;

			m_vVertex[k].x = ox + f32Size;
			m_vVertex[k].y = oy + f32Size; // v2
			m_vVertex[k].z = oz - f32Size;
			k++;

			m_vVertex[k].x = ox + f32Size;
			m_vVertex[k].y = oy - f32Size; // v3
			m_vVertex[k].z = oz - f32Size;
			k++;

			m_vVertex[k].x = ox - f32Size;
			m_vVertex[k].y = oy - f32Size; // v4
			m_vVertex[k].z = oz + f32Size;
			k++;

			m_vVertex[k].x = ox - f32Size;
			m_vVertex[k].y = oy + f32Size; // v5
			m_vVertex[k].z = oz + f32Size;
			k++;

			m_vVertex[k].x = ox - f32Size;
			m_vVertex[k].y = oy + f32Size; // v6
			m_vVertex[k].z = oz - f32Size;
			k++;

			m_vVertex[k].x = ox - f32Size;
			m_vVertex[k].y = oy - f32Size; // v7
			m_vVertex[k].z = oz - f32Size;
			k++;
		}
	}
	m_ui32HistoryIndex = 0;
}

void CRendererSlice::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	uint32_t i, j, k, l;

	for (i = m_ui32HistoryIndex; i < m_ui32HistoryCount; ++i)
	{
		k = (i % m_ui32SampleCount) * m_ui32ChannelCount * 8;
		for (j = 0; j < m_ui32ChannelCount; j++)
		{
			for (l = 0; l < 8; l++)
			{
				m_vVertex[k++].u = m_vHistory[j][i];
			}
		}
	}

	m_ui32HistoryIndex = m_ui32HistoryCount;
}

bool CRendererSlice::render(const IRendererContext& rContext)
{
	// uint32_t i, j;
	float d = 3.5;

	if (!rContext.getSelectedCount()) { return false; }
	if (!m_ui32HistoryCount) { return false; }

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	//	::glEnable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, rContext.getAspect(), .01, 100);
	glTranslatef(0, 0, -d);
	glRotatef(rContext.getRotationX() * 10, 1, 0, 0);
	glRotatef(rContext.getRotationY() * 10, 0, 1, 0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(rContext.getScale(), 1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(rContext.getZoom(), rContext.getZoom(), rContext.getZoom());
	glScalef(1., 1., 3.);

	glPushMatrix();
	glScalef(1.f / rContext.getStackCount(), 1.f / m_ui32ChannelCount, 1.f / m_ui32SampleCount);
	glTranslatef(rContext.getStackIndex() - (rContext.getStackCount() - 1) * .5f, 0, 0);
	glColor4f(.1f, .1f, .1f, rContext.getTranslucency());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_vVertex[0].x);
	glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vVertex[0].u);
	glDrawElements(GL_QUADS, m_vQuad.size(), GL_UNSIGNED_INT, &m_vQuad[0]);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glDisable(GL_TEXTURE_1D);

	float l_f32Progress = 1 - (m_ui32HistoryIndex % m_ui32SampleCount) * 2.f / m_ui32SampleCount;
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
	glColor4f(0.25f, 1, 0.25f, .9f / rContext.getStackCount());
	glVertex3f(-1, -1, l_f32Progress);
	glVertex3f(-1, 1, l_f32Progress);
	glVertex3f(1, 1, l_f32Progress);
	glVertex3f(1, -1, l_f32Progress);
	glEnd();
	glBegin(GL_QUADS);
	glColor4f(0.25f, 1, 0.25f, .1f / rContext.getStackCount());
	glVertex3f(-1, -1, l_f32Progress);
	glVertex3f(-1, 1, l_f32Progress);
	glVertex3f(1, 1, l_f32Progress);
	glVertex3f(1, -1, l_f32Progress);
	glEnd();

	glEnable(GL_TEXTURE_1D);

	if (rContext.getCheckBoardVisibility()) this->drawCoordinateSystem();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}
