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
#include <algorithm>

using namespace Mensia;
using namespace AdvancedVisualization;

CRendererCube::CRendererCube() = default;

void CRendererCube::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	m_vVertex.clear();
	m_vVertex.resize(rContext.getChannelCount());

	m_historyIndex = 0;
}

void CRendererCube::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_historyCount) { return; }

	float l_f32SampleIndexERP = (m_ERPFraction * (m_sampleCount - 1));
	float l_f32Alpha = l_f32SampleIndexERP - std::floor(l_f32SampleIndexERP);
	uint32_t l_ui32SampleIndexERP1 = uint32_t(l_f32SampleIndexERP) % m_sampleCount;
	uint32_t l_ui32SampleIndexERP2 = uint32_t(l_f32SampleIndexERP + 1) % m_sampleCount;

	for (uint32_t i = 0; i < m_vVertex.size(); ++i)
	{
		m_vVertex[i].u = m_history[i][m_historyCount - m_sampleCount + l_ui32SampleIndexERP1] * (1 - l_f32Alpha)
			+ m_history[i][m_historyCount - m_sampleCount + l_ui32SampleIndexERP2] * (l_f32Alpha);
	}

	m_historyIndex = m_historyCount;
}

bool CRendererCube::render(const IRendererContext& rContext)
{
	std::map<std::string, CVertex>::const_iterator it;

	if (!rContext.getSelectedCount()) { return false; }
	if (m_vVertex.empty()) { return false; }
	if (!m_historyCount) { return false; }

	float d = 3.5;

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
		uint32_t k = rContext.getSelected(j);
		rContext.getChannelLocalisation(k, v.x, v.y, v.z);
		/*
				std::string l_sName=rContext.getChannelName(k);
				std::transform(l_sName.begin(), l_sName.end(), l_sName.begin(), ::tolower);

				it=m_channelLocalisation.find(l_sName);
				if(it!=m_channelLocalisation.end())
				{
		*/
		float l_fCubeScale = .1f * (.25f + fabs(m_vVertex[k].u * rContext.getScale()));

		glPushMatrix();
		//			::glTranslatef(it->second.x, it->second.y, it->second.z);
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
		//		}
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
