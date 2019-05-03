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

#include "mCRendererTopo.hpp"
#include "m_RendererTools.hpp"

#include <cmath>
#include <algorithm>

using namespace Mensia;
using namespace AdvancedVisualization;

const bool m_bMultiSlice = false;

namespace
{
	const unsigned int S = 1000;

	// Legendre polynomials
	// http://en.wikipedia.org/wiki/Legendre_polynomials

	void legendre(unsigned int n, double x, std::vector < double >& vLegendre)
	{
		vLegendre.resize(n + 1);
		vLegendre[0] = 1;
		vLegendre[1] = x;
		for (unsigned int i = 2; i <= n; ++i)
		{
			const double invi = 1. / i;
			vLegendre[i] = (2 - invi) * x * vLegendre[i - 1] - (1 - invi) * vLegendre[i - 2];
		}
	}

	// G function :
	// Spherical splines for scalp potential and current density mapping
	// http://www.sciencedirect.com/science/article/pii/0013469489901806

	double g(unsigned int n, unsigned int m, const std::vector < double > & vLegendre)
	{
		double result = 0;
		for (unsigned int i = 1; i <= n; ++i)
		{
			result += (2 * i + 1) / pow(double(i * (i + 1)), int(m)) * vLegendre[i];
		}
		return result / (4 * M_PI);
	}

	// H function :
	// Spherical splines for scalp potential and current density mapping
	// http://www.sciencedirect.com/science/article/pii/0013469489901806

	double h(unsigned int n, unsigned int m, const std::vector < double > & vLegendre)
	{
		double result = 0;
		for (unsigned int i = 1; i <= n; ++i)
		{
			result += (2 * i + 1) / pow(double(i * (i + 1)), int(m - 1)) * vLegendre[i];
		}
		return result / (4 * M_PI);
	}

	// Caching system

	void build(unsigned int n, unsigned int m, std::vector < double > & rGCache, std::vector < double > & rHCache)
	{
		rGCache.resize(2 * S + 1);
		rHCache.resize(2 * S + 1);
		for (unsigned int i = 0; i <= 2 * S; ++i)
		{
			std::vector < double > l_vLegendre;
			double cosine = (double(i) - S) / S;

			legendre(n, cosine, l_vLegendre);
			rGCache[i] = g(n, m, l_vLegendre);
			rHCache[i] = h(n, m, l_vLegendre);
		}
		rGCache.push_back(rGCache.back());
		rHCache.push_back(rHCache.back());
	}

	double cache(double x, std::vector < double > & rCache)
	{
		if (x < -1) { return rCache[0]; }
		if (x > 1) { return rCache[2 * S]; }
		double t = (x + 1) * S;
		int i1 = int(t);
		int i2 = int(t + 1);
		t -= i1;
		return rCache[i1] * (1 - t) + rCache[i2] * t;
	}
}  // namespace

void CRendererTopo::rebuild(const IRendererContext & rContext)
{
	CRenderer::rebuild(rContext);

	uint32_t i, j;

	this->rebuild3DMeshesPre(rContext);

	// Projects electrode coordinates to 3D mesh

	std::vector < CVertex > l_vProjectedChannelCoordinate;
	std::vector < CVertex > l_vChannelCoordinate;
	l_vChannelCoordinate.resize(rContext.getChannelCount());
	for (i = 0; i < rContext.getChannelCount(); ++i)
	{
		rContext.getChannelLocalisation(i, l_vChannelCoordinate[i].x, l_vChannelCoordinate[i].y, l_vChannelCoordinate[i].z);
	}
	m_oScalp.project(l_vProjectedChannelCoordinate, l_vChannelCoordinate);
	m_vProjectedChannelCoordinate = l_vProjectedChannelCoordinate;

#if 0

	m_vProjectedChannelCoordinate.resize(rContext.getChannelCount());
	for (i = 0; i < rContext.getChannelCount(); ++i)
	{
		CVertex p, q;
		rContext.getChannelLocalisation(i, p.x, p.y, p.z);
		for (j = 0; j < m_oScalp.m_vTriangle.size(); j += 3)
		{
			uint32_t i1, i2, i3;
			i1 = m_oScalp.m_vTriangle[j];
			i2 = m_oScalp.m_vTriangle[j + 1];
			i3 = m_oScalp.m_vTriangle[j + 2];

			CVertex v1, v2, v3;
			v1 = m_oScalp.m_vVertex[i1];
			v2 = m_oScalp.m_vVertex[i2];
			v3 = m_oScalp.m_vVertex[i3];

			CVertex e1(v1, v2);
			CVertex e2(v1, v3);
			CVertex n = CVertex::cross(e1, e2).normalize();

			float t = CVertex::dot(v1, n) / CVertex::dot(p, n);
			q.x = t * p.x;
			q.y = t * p.y;
			q.z = t * p.z;

			if (CVertex::isInTriangle(q, v1, v2, v3) && t >= 0)
			{
				m_vProjectedChannelCoordinate[i].x = q.x;
				m_vProjectedChannelCoordinate[i].y = q.y;
				m_vProjectedChannelCoordinate[i].z = q.z;
			}
		}
		if (m_vProjectedChannelCoordinate[i].x == 0 && m_vProjectedChannelCoordinate[i].y == 0 && m_vProjectedChannelCoordinate[i].z == 0)
		{
			//			::printf("Could not project coordinates on mesh for channel %i [%s]\n", i+1, rContext.getChannelName(i).c_str());
		}
	}

#endif

	// Generates transformation matrices based spherical spline interpolations

	unsigned int M = 3;
	auto N = (unsigned int)pow(10., 10. / (2 * M - 2));

	std::vector < double > l_vLegendre;
	std::vector < double > l_vGCache;
	std::vector < double > l_vHCache;
	build(N, M, l_vGCache, l_vHCache);

	uint32_t nc = rContext.getChannelCount();
	uint32_t vc = m_oScalp.m_vVertex.size();

	A = Eigen::MatrixXd(nc + 1, nc + 1);
	A(nc, nc) = 0;
	for (i = 0; i < nc; ++i)
	{
		A(i, nc) = 1;
		A(nc, i) = 1;
		for (j = 0; j <= i; j++)
		{
			CVertex v1, v2;
			rContext.getChannelLocalisation(i, v1.x, v1.y, v1.z);
			rContext.getChannelLocalisation(j, v2.x, v2.y, v2.z);

			double cosine = CVertex::dot(v1, v2);
			A(i, j) = cache(cosine, l_vGCache);
			A(j, i) = cache(cosine, l_vGCache);
		}
	}

	B = Eigen::MatrixXd(vc + 1, nc + 1);
	D = Eigen::MatrixXd(vc + 1, nc + 1);
	B(vc, nc) = 0;
	D(vc, nc) = 0;
	for (i = 0; i < vc; ++i)
	{
		B(i, nc) = 1;
		D(i, nc) = 1;
		for (j = 0; j < nc; j++)
		{
			B(vc, j) = 1;
			D(vc, j) = 1;
			CVertex v1, v2;
			v1 = m_oScalp.m_vVertex[i];
			v1.normalize();
			rContext.getChannelLocalisation(j, v2.x, v2.y, v2.z);

			double cosine = CVertex::dot(v1, v2);
			B(i, j) = cache(cosine, l_vGCache);
			D(i, j) = cache(cosine, l_vHCache);
		}
	}

	Ai = A.inverse();

	// Post processed 3D meshes when needed

	this->rebuild3DMeshesPost(rContext);

	// Rebuilds texture coordinates array

	if (m_bMultiSlice)
	{
		m_vInterpolatedSample.clear();
		m_vInterpolatedSample.resize(m_ui32SampleCount, Eigen::VectorXd::Zero(m_oScalp.m_vVertex.size()));
	}

	// Finalizes

	m_ui32HistoryIndex = 0;
}

// V has sensor potentials
// W has interpolated potentials
// Z has interpolated current densities
void CRendererTopo::interpolate(const Eigen::VectorXd & V, Eigen::VectorXd & W, Eigen::VectorXd & Z)
{
	Eigen::VectorXd C = Ai * V;

	W = B * C;

	C[V.size() - 1] = 0;

	Z = D * C;
}

void CRendererTopo::refresh(const IRendererContext & rContext)
{
	CRenderer::refresh(rContext);

	if (!m_ui32HistoryCount) { return; }

	uint32_t i, j, k;

	uint32_t nc = rContext.getChannelCount();
	uint32_t vc = m_oScalp.m_vVertex.size();

	std::vector < float > l_vSample;
	Eigen::VectorXd V = Eigen::VectorXd::Zero(nc + 1);
	Eigen::VectorXd W;
	Eigen::VectorXd Z;

	if (!m_bMultiSlice)
	{
		this->getSampleAtERPFraction(m_f32ERPFraction, l_vSample);
		for (i = 0; i < nc; ++i) { V(i) = l_vSample[i]; }
		this->interpolate(V, W, Z);
		for (j = 0; j < vc; j++) { m_oScalp.m_vVertex[j].u = float(W(j)); }
	}
	else
	{
		if (m_ui32HistoryCount >= m_ui32SampleCount)
		{
			for (k = 0; k < m_ui32SampleCount; k++)
			{
				for (i = 0; i < nc; ++i) { V(i) = m_vHistory[i][m_ui32HistoryCount - m_ui32SampleCount + k]; }
				this->interpolate(V, W, Z);
				m_vInterpolatedSample[k] = W;
			}
		}
	}

	m_ui32HistoryIndex = m_ui32HistoryCount;
}

bool CRendererTopo::render(const IRendererContext & rContext)
{
	std::map < std::string, CVertex >::const_iterator it;

	if (!rContext.getSelectedCount()) { return false; }
	if (m_oScalp.m_vVertex.empty()) { return false; }
	if (!m_ui32HistoryCount) { return false; }

	uint32_t j;
	float d = 3.5;

	//	::glEnable(GL_DEPTH_TEST);
	//	::glDisable(GL_BLEND);

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

	// Now renders

	glPushMatrix();
#if 1
	glTranslatef(0, .5f, 0);
	glRotatef(19, 1, 0, 0);
	glTranslatef(0, -.2f, .35f);
	//	::glScalef(1.8f, 1.8f, 1.8f);
#else
	::glRotatef(19, 1, 0, 0);
	::glTranslatef(0, -.2f, .35f);
	//	::glScalef(1.8f, 1.8f, 1.8f);
#endif

	if (rContext.isFaceMeshVisible())
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_1D);
		if (!m_oFace.m_vTriangle.empty())
		{
			if (!m_oFace.m_vNormal.empty())
			{
				glEnable(GL_LIGHTING);
				glEnableClientState(GL_NORMAL_ARRAY);
			}
			glColor3f(m_oFace.m_vColor[0], m_oFace.m_vColor[1], m_oFace.m_vColor[2]);
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_oFace.m_vVertex[0].x);
			if (!m_oFace.m_vNormal.empty())
			{
				glNormalPointer(GL_FLOAT, sizeof(CVertex), &m_oFace.m_vNormal[0].x);
			}
			glDrawElements(GL_TRIANGLES, m_oFace.m_vTriangle.size(), GL_UNSIGNED_INT, &m_oFace.m_vTriangle[0]);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisable(GL_LIGHTING);
		}
	}

	if (rContext.isScalpMeshVisible())
	{
		glEnable(GL_TEXTURE_1D);
		if (!m_oScalp.m_vTriangle.empty())
		{
			if (!m_oScalp.m_vNormal.empty())
			{
				glEnable(GL_LIGHTING);
				glEnableClientState(GL_NORMAL_ARRAY);
			}
			glColor3f(m_oScalp.m_vColor[0], m_oScalp.m_vColor[1], m_oScalp.m_vColor[2]);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &m_oScalp.m_vVertex[0].x);
			if (!m_oScalp.m_vNormal.empty())
			{
				glNormalPointer(GL_FLOAT, sizeof(CVertex), &m_oScalp.m_vNormal[0].x);
			}
			if (!m_bMultiSlice)
			{
				glColor3f(1, 1, 1);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);
				glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_oScalp.m_vVertex[0].u);
				glDrawElements(GL_TRIANGLES, m_oScalp.m_vTriangle.size(), GL_UNSIGNED_INT, &m_oScalp.m_vTriangle[0]);
			}
			else
			{
				glColor4f(1.f, 1.f, 1.f, 4.f / m_ui32SampleCount);
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				for (uint32_t i = 0; i < m_ui32SampleCount; ++i)
				{
					float l_f32Scale = 1.f + i * 0.25f / m_ui32SampleCount;
					glPushMatrix();
					glScalef(l_f32Scale, l_f32Scale, l_f32Scale);
					glTexCoordPointer(1, GL_DOUBLE, 0, &m_vInterpolatedSample[i][0]);
					glDrawElements(GL_TRIANGLES, m_oScalp.m_vTriangle.size(), GL_UNSIGNED_INT, &m_oScalp.m_vTriangle[0]);
					glPopMatrix();
				}
			}
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisable(GL_LIGHTING);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_1D);

	glLineWidth(3);
	for (j = 0; j < rContext.getChannelCount(); j++)
	{
		float l_fCubeScale = .025f;
		CVertex v;
		v = m_vProjectedChannelCoordinate[j];
		//rContext.getChannelLocalisation(j, v.x, v.y, v.z);

		glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		glScalef(l_fCubeScale, l_fCubeScale, l_fCubeScale);

		float l_vSelected[] = { 1, 1, 1 };
		float l_vUnselected[] = { .2f, .2f, .2f };
		glColor3fv(rContext.isSelected(j) ? l_vSelected : l_vUnselected);
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
