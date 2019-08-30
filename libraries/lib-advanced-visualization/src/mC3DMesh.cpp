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

#if defined TARGET_HAS_ThirdPartyOpenGL

#include "mC3DMesh.hpp"

#include <cstdio>
#include <cstring>

using namespace Mensia;
using namespace AdvancedVisualization;

namespace
{
	template <typename T>
	bool littleEndianToHost(const uint8_t* buffer, T* pValue)
	{
		if (!buffer) { return false; }
		if (!pValue) { return false; }
		memset(pValue, 0, sizeof(T));
		for (unsigned int i = 0; i < sizeof(T); ++i)
		{
			reinterpret_cast<uint8_t*>(pValue)[i] = buffer[i];
		}
		return true;
	}
} // namespace

C3DMesh::C3DMesh()
{
	m_vColor[0] = 1.0;
	m_vColor[1] = 1.0;
	m_vColor[2] = 1.0;
}

C3DMesh::~C3DMesh() = default;

void C3DMesh::clear()
{
	m_vColor[0] = 1.0;
	m_vColor[1] = 1.0;
	m_vColor[2] = 1.0;

	m_vVertex.clear();
	m_vNormal.clear();
	m_vTriangle.clear();
}

bool C3DMesh::load(const void* buffer, unsigned int /*size*/)
{
	const auto* tmp = reinterpret_cast<const uint32_t*>(buffer);

	uint32_t nVertex;
	uint32_t nTriangle;

	littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&tmp[0]), &nVertex);
	littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&tmp[1]), &nTriangle);

	m_vVertex.resize(nVertex);
	m_vTriangle.resize(size_t(nTriangle) * 3);

	uint32_t i, j = 2;

	for (i = 0; i < nVertex; ++i)
	{
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_vVertex[i].x);
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_vVertex[i].y);
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_vVertex[i].z);
	}

	for (i = 0; i < nTriangle * 3; ++i)
	{
		littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_vTriangle[i]);
	}

	this->compile();

	return true;
}

bool C3DMesh::compile()
{
	m_vNormal.clear();
	m_vNormal.resize(m_vVertex.size());
	for (size_t i = 0; i < m_vTriangle.size(); i += 3)
	{
		const uint32_t i1 = m_vTriangle[i];
		const uint32_t i2 = m_vTriangle[i + 1];
		const uint32_t i3 = m_vTriangle[i + 2];
		CVertex v1        = m_vVertex[i1];
		CVertex v2        = m_vVertex[i2];
		CVertex v3        = m_vVertex[i3];
		v2.x -= v1.x;
		v2.y -= v1.y;
		v2.z -= v1.z;
		v3.x -= v1.x;
		v3.y -= v1.y;
		v3.z -= v1.z;
		v1 = CVertex::cross(v2, v3);
		v1.normalize();
		m_vNormal[i1].x += v1.x;
		m_vNormal[i1].y += v1.y;
		m_vNormal[i1].z += v1.z;
		m_vNormal[i2].x += v1.x;
		m_vNormal[i2].y += v1.y;
		m_vNormal[i2].z += v1.z;
		m_vNormal[i3].x += v1.x;
		m_vNormal[i3].y += v1.y;
		m_vNormal[i3].z += v1.z;
	}

	for (auto& normal : m_vNormal) { normal.normalize(); }
	return true;
}

bool C3DMesh::project(std::vector<CVertex>& vProjectedChannelCoordinate, const std::vector<CVertex>& vChannelCoordinate)
{
	vProjectedChannelCoordinate.resize(vChannelCoordinate.size());
	for (size_t i = 0; i < vChannelCoordinate.size(); ++i)
	{
		CVertex p, q;
		p = vChannelCoordinate[i];
		//		q = vChannelCoordinate[i];
		for (size_t j = 0; j < this->m_vTriangle.size(); j += 3)
		{
			const uint32_t i1 = this->m_vTriangle[j];
			const uint32_t i2 = this->m_vTriangle[j + 1];
			const uint32_t i3 = this->m_vTriangle[j + 2];

			CVertex v1, v2, v3;
			v1 = this->m_vVertex[i1];
			v2 = this->m_vVertex[i2];
			v3 = this->m_vVertex[i3];

			CVertex e1(v1, v2);
			CVertex e2(v1, v3);
			CVertex n = CVertex::cross(e1, e2).normalize();

			const float t = CVertex::dot(v1, n) / CVertex::dot(p, n);
			q.x           = t * p.x;
			q.y           = t * p.y;
			q.z           = t * p.z;

			if (CVertex::isInTriangle(q, v1, v2, v3) && t >= 0)
			{
				vProjectedChannelCoordinate[i] = q;
			}
		}
		if (q.x == 0 && q.y == 0 && q.z == 0)
		{
			//			::printf("Could not project coordinates on mesh for channel %i [%s]\n", i+1, rContext.getChannelName(i).c_str());
		}
	}
	return true;
}

#endif // TARGET_HAS_ThirdPartyOpenGL
