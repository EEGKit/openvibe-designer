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
#include "mCRendererCube.hpp"
#include "m_RendererTools.hpp"

#include <cmath>

using namespace Mensia;
using namespace AdvancedVisualization;

CRendererCube::CRendererCube() = default;

void CRendererCube::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	m_vVertex.clear();
	m_vVertex.resize(rContext.getChannelCount());

	m_historyIdx = 0;
}

void CRendererCube::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_nHistory) { return; }

	const float sampleIndexERP     = (m_ERPFraction * float(m_nSample - 1));
	const float alpha              = sampleIndexERP - std::floor(sampleIndexERP);
	const uint32_t sampleIndexERP1 = uint32_t(sampleIndexERP) % m_nSample;
	const uint32_t sampleIndexERP2 = uint32_t(sampleIndexERP + 1) % m_nSample;

	for (uint32_t i = 0; i < m_vVertex.size(); ++i)
	{
		m_vVertex[i].u = m_history[i][m_nHistory - m_nSample + sampleIndexERP1] * (1 - alpha)
						 + m_history[i][m_nHistory - m_nSample + sampleIndexERP2] * (alpha);
	}

	m_historyIdx = m_nHistory;
}

bool CRendererCube::render(const IRendererContext& rContext)
{

	if (!rContext.getSelectedCount()) { return false; }
	if (m_vVertex.empty()) { return false; }
	if (!m_nHistory) { return false; }

	const float d = 3.5;

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

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

	glPushMatrix();
	glRotatef(19, 1, 0, 0);
	for (uint32_t j = 0; j < rContext.getSelectedCount(); j++)
	{
		CVertex v;
		const uint32_t k = rContext.getSelected(j);
		rContext.getChannelLocalisation(k, v.x, v.y, v.z);
		const float l_fCubeScale = .1f * (.25f + fabs(m_vVertex[k].u * rContext.getScale()));

		glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		glTexCoord1f(m_vVertex[k].u);
		glScalef(l_fCubeScale, l_fCubeScale, l_fCubeScale);

		glColor3f(1, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		cube();

		glColor3f(0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		cube();

		glPopMatrix();
	}
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (rContext.getCheckBoardVisibility()) { this->drawCoordinateSystem(); }

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}
