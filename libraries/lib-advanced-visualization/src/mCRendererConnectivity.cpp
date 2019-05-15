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

#include "mCRendererConnectivity.hpp"
#include "mC3DMesh.hpp"

// #include "content/Face.obj.hpp"
#include "content/Scalp.obj.hpp"

#include <Eigen/Geometry>

#include <cmath>
#include <algorithm>

//constexpr size_t COUNT = 16; //Macro modernization, Not yet with jenkins (not the last visual 2013 which it works)
#define COUNT 16

using namespace Mensia;
using namespace AdvancedVisualization;

static void q_rotate(Eigen::VectorXd& vDest, const Eigen::VectorXd& vSrc, const Eigen::Quaterniond& q)
{
	vDest = q.matrix() * vSrc;
}

static void q_from_polar(Eigen::Quaterniond& q, Eigen::VectorXd& v1, Eigen::VectorXd& v2, const CVertex& cv1, const CVertex& cv2)
{
	v1(0) = cv1.x;
	v1(1) = cv1.y;
	v1(2) = cv1.z;

	v2(0) = cv2.x;
	v2(1) = cv2.y;
	v2(2) = cv2.z;

	q.setFromTwoVectors(v1, v2);
}

void CRendererConnectivity::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	std::map<std::string, CVertex>::const_iterator it1;
	std::map<std::string, CVertex>::const_iterator it2;

	uint32_t i, l = 0;

	Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
	const Eigen::Quaterniond q_id = Eigen::Quaterniond::Identity();
	Eigen::Quaterniond q_diff = Eigen::Quaterniond::Identity();
	Eigen::VectorXd v(3);
	Eigen::VectorXd v1(3);
	Eigen::VectorXd v2(3);

	C3DMesh l_oScalp;
	l_oScalp.load(g_pScalpData, sizeof(g_pScalpData));

	// Projects electrode coordinates to 3D mesh

	std::vector<CVertex> l_vProjectedChannelCoordinate;
	std::vector<CVertex> l_vChannelCoordinate;
	l_vChannelCoordinate.resize(rContext.getChannelCount());
	for (i = 0; i < rContext.getChannelCount(); ++i)
	{
		rContext.getChannelLocalisation(i, l_vChannelCoordinate[i].x, l_vChannelCoordinate[i].y, l_vChannelCoordinate[i].z);
	}
	l_oScalp.project(l_vProjectedChannelCoordinate, l_vChannelCoordinate);

	// Generates arcs

	m_vertex.clear();
	m_vertex.resize(m_channelCount * (m_channelCount - 1) / 2);
	for (i = 0; i < m_channelCount; ++i)
	{
		for (uint32_t j = 0; j < i; j++)
		{
			m_vertex[l].resize(COUNT);

			CVertex vi, vj;
			vi = l_vChannelCoordinate[i];
			vj = l_vChannelCoordinate[j];

			const float vi_len = l_vProjectedChannelCoordinate[i].length();
			const float vj_len = l_vProjectedChannelCoordinate[j].length();

			q_from_polar(q_diff, v1, v2, vi, vj);

			const double alpha = 0;
			const double dot = (1 - CVertex::dot(vi, vj)) * .5;

			for (uint32_t k = 0; k < COUNT; k++)
			{
				const float t = float(k * 1. / (COUNT - 1));
				auto s = float((t - .5) * 2);
				s = float(1 + .5 * (1 - s * s) * dot);

				q = q_id.slerp(t, q_diff);

				q_rotate(v, v1, q);

				const float len = (vi_len * (1 - t) + vj_len * t);
				m_vertex[l][k].x = float(s * v[0] * len);
				m_vertex[l][k].y = float(s * v[1] * len);
				m_vertex[l][k].z = float(s * v[2] * len);
				m_vertex[l][k].u = float(alpha);
			}

			l++;
		}
	}

	m_historyIndex = 0;
}

void CRendererConnectivity::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_historyCount) { return; }
	if (m_historyCount < m_channelCount) { return; }

	uint32_t l = 0;
	for (uint32_t i = 0; i < m_channelCount; ++i)
	{
		for (uint32_t j = 0; j < i; j++)
		{
			for (uint32_t k = 0; k < COUNT; k++)
			{
				m_vertex[l][k].u = m_history[i][m_historyCount - 1 - j];
			}
			l++;
		}
	}

	m_historyIndex = m_historyCount;
}

bool CRendererConnectivity::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) { return false; }
	if (m_vertex.empty()) { return false; }
	if (!m_historyCount) { return false; }

	std::map<std::string, std::pair<float, float>>::const_iterator it;
	const float d = 3.5;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

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

	//	uint32_t l_ui32ChannelCountSquare=m_channelCount*m_channelCount;
	const float rgb = 1.f;
	glColor4f(rgb, rgb, rgb, rContext.getTranslucency());
	glPushMatrix();
#if 1
	glTranslatef(0, .5f, 0);
	glRotatef(19, 1, 0, 0);
	glTranslatef(0, -.2f, .35f);
	//	::glScalef(1.8f, 1.8f, 1.8f);
#else
	::glRotatef(19, 1, 0, 0);
#endif

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (uint32_t i = 0; i < m_channelCount * (m_channelCount - 1) / 2; ++i)
	{
		glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].u);
		glDrawArrays(GL_LINE_STRIP, 0, COUNT);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

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

// #endif // TARGET_HAS_ThirdPartyVRPN
