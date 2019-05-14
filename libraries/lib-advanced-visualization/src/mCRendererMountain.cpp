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

#include "mCRendererMountain.hpp"

#include <cmath>
#include <algorithm>

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererMountain::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	size_t i, j, k;

	m_oMountain.m_vVertex.clear();
	m_oMountain.m_vVertex.resize(m_channelCount * m_sampleCount);
	for (i = 0, k = 0; i < m_channelCount; ++i)
	{
		for (j = 0; j < m_sampleCount; ++j)
		{
			const float a = i * 1.f / float(m_channelCount - 1);
			const float b = 1 - j * 1.f / float(m_sampleCount - 1);
			m_oMountain.m_vVertex[k].x = a;
			m_oMountain.m_vVertex[k].y = 0;
			m_oMountain.m_vVertex[k].z = b;
			m_oMountain.m_vVertex[k].u = 0;
			k++;
		}
	}

	m_oMountain.m_vTriangle.clear();
	m_oMountain.m_vTriangle.resize((m_channelCount - 1) * (m_sampleCount - 1) * 6);
	for (i = 0, k = 0; i < m_channelCount - 1; ++i)
	{
		for (j = 0; j < m_sampleCount - 1; ++j)
		{
			const size_t id = k * 6;
			const uint32_t v1 = uint32_t(i * m_sampleCount + j);
			const uint32_t v2 = v1 + m_sampleCount;
			m_oMountain.m_vTriangle[id] = v1;
			m_oMountain.m_vTriangle[id + 1] = v2;
			m_oMountain.m_vTriangle[id + 2] = v2 + 1;
			m_oMountain.m_vTriangle[id + 3] = v1;
			m_oMountain.m_vTriangle[id + 4] = v2 + 1;
			m_oMountain.m_vTriangle[id + 5] = v1 + 1;
			k++;
		}
	}

	m_historyIndex = 0;
}

void CRendererMountain::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_historyCount) { return; }

	size_t i, k;

	for (i = 0, k = 0; i < rContext.getSelectedCount(); ++i)
	{
		k = ((m_historyCount - 1) / m_sampleCount) * m_sampleCount;
		std::vector<float>& l_vHistory = m_history[rContext.getSelected(uint32_t(i))];
		CVertex* l_pVertex = &m_oMountain.m_vVertex[i * m_sampleCount];
		for (size_t j = 0; j < m_sampleCount; j++, k++)
		{
			if (/*k>=m_historyIndex && */k < m_historyCount)
			{
				l_pVertex[j].u = l_vHistory[k];
				l_pVertex[j].y = l_vHistory[k] / 2;
			}
		}
	}

	m_oMountain.compile();

	m_historyIndex = m_historyCount;
}

bool CRendererMountain::render(const IRendererContext& rContext)
{
	if (m_oMountain.m_vVertex.empty()) { return false; }
	if (!m_historyCount) { return false; }

	const float d = 2.5f;

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, rContext.getAspect(), .01, 100);
	glTranslatef(0, -.2f, -d);
	glRotatef(rContext.getRotationX() * 10, 1, 0, 0);
	glRotatef(rContext.getRotationY() * 10, 0, 1, 0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(rContext.getScale(), 1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(3 * rContext.getZoom(), 3 * rContext.getZoom(), 3 * rContext.getZoom());
	glTranslatef(-.5f, 0, -.5f);
	glScalef(m_channelCount * 1.f / rContext.getSelectedCount(), rContext.getScale(), 1);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_oMountain.m_vVertex[0].x);
	glNormalPointer(GL_FLOAT, sizeof(CVertex), &m_oMountain.m_vNormal[0].x);
	glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_oMountain.m_vVertex[0].u);

	glColor3f(rContext.getTranslucency(), rContext.getTranslucency(), rContext.getTranslucency());
	glDrawElements(GL_TRIANGLES, (rContext.getSelectedCount() - 1) * (m_sampleCount - 1) * 6, GL_UNSIGNED_INT, &m_oMountain.m_vTriangle[0]);
	/*
		::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		::glColor3f(0, 0, 0);
		::glDrawElements(GL_TRIANGLES, m_oMountain.m_vTriangle.size(), GL_UNSIGNED_INT, &m_oMountain.m_vTriangle[0]);
		::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
