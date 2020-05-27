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

//constexpr size_t COUNT = 16; //Macro modernization, Not yet with jenkins (not the last visual 2013 which it works)
#define COUNT 16

using namespace OpenViBE;
using namespace AdvancedVisualization;

static void q_rotate(Eigen::VectorXd& dst, const Eigen::VectorXd& src, const Eigen::Quaterniond& q) { dst = q.matrix() * src; }

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

void CRendererConnectivity::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);


	Eigen::Quaterniond q         = Eigen::Quaterniond::Identity();
	const Eigen::Quaterniond qId = Eigen::Quaterniond::Identity();
	Eigen::Quaterniond qDiff     = Eigen::Quaterniond::Identity();
	Eigen::VectorXd v(3);
	Eigen::VectorXd v1(3);
	Eigen::VectorXd v2(3);

	C3DMesh scalp;
	scalp.load(SCALP_DATA);

	// Projects electrode coordinates to 3D mesh

	std::vector<CVertex> projectedChannelPos;
	std::vector<CVertex> channelPos;
	channelPos.resize(ctx.getChannelCount());
	for (size_t i = 0; i < ctx.getChannelCount(); ++i) { ctx.getChannelLocalisation(i, channelPos[i].x, channelPos[i].y, channelPos[i].z); }
	scalp.project(projectedChannelPos, channelPos);

	// Generates arcs

	m_vertex.clear();
	m_vertex.resize(m_nChannel * (m_nChannel - 1) / 2);
	size_t l = 0;
	for (size_t i = 0; i < m_nChannel; ++i)
	{
		for (size_t j = 0; j < i; ++j)
		{
			m_vertex[l].resize(COUNT);

			CVertex vi, vj;
			vi = channelPos[i];
			vj = channelPos[j];

			const float viLen = projectedChannelPos[i].length();
			const float vjLen = projectedChannelPos[j].length();

			q_from_polar(qDiff, v1, v2, vi, vj);

			const double alpha = 0;
			const double dot   = (1 - CVertex::dot(vi, vj)) * .5;

			for (size_t k = 0; k < COUNT; ++k)
			{
				const float t = float(k * 1. / (COUNT - 1));
				auto s        = float((t - .5) * 2);
				s             = float(1 + .5 * (1 - s * s) * dot);

				q = qId.slerp(t, qDiff);

				q_rotate(v, v1, q);

				const float len  = (viLen * (1 - t) + vjLen * t);
				m_vertex[l][k].x = float(s * v[0] * len);
				m_vertex[l][k].y = float(s * v[1] * len);
				m_vertex[l][k].z = float(s * v[2] * len);
				m_vertex[l][k].u = float(alpha);
			}

			l++;
		}
	}

	m_historyIdx = 0;
}

void CRendererConnectivity::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }
	if (m_nHistory < m_nChannel) { return; }

	size_t l = 0;
	for (size_t i = 0; i < m_nChannel; ++i)
	{
		for (size_t j = 0; j < i; ++j)
		{
			for (size_t k = 0; k < COUNT; ++k) { m_vertex[l][k].u = m_history[i][m_nHistory - 1 - j]; }
			l++;
		}
	}

	m_historyIdx = m_nHistory;
}

bool CRendererConnectivity::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount() || m_vertex.empty() || !m_nHistory) { return false; }

	const float d = 3.5;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, ctx.getAspect(), .01, 100);
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

	const float rgb = 1.F;
	glColor4f(rgb, rgb, rgb, ctx.getTranslucency());
	glPushMatrix();

	glTranslatef(0, .5F, 0);
	glRotatef(19, 1, 0, 0);
	glTranslatef(0, -.2F, .35F);
	// ::glScalef(1.8f, 1.8f, 1.8f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (size_t i = 0; i < m_nChannel * (m_nChannel - 1) / 2; ++i)
	{
		glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vertex[i][0].u);
		glDrawArrays(GL_LINE_STRIP, 0, COUNT);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

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

// #endif // TARGET_HAS_ThirdPartyVRPN
