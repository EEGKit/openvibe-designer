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
	template <typename T> bool littleEndianToHost(const uint8_t* pBuffer, T* pValue)
	{
		if (!pBuffer) { return false; }
		if (!pValue) { return false; }
		memset(pValue, 0, sizeof(T));
		for (unsigned int i = 0; i < sizeof(T); ++i)
		{
			((uint8_t*)pValue)[i] = pBuffer[i];
		}
		return true;
	}
}  // namespace

C3DMesh::C3DMesh()

{
	m_vColor[0] = 1;
	m_vColor[1] = 1;
	m_vColor[2] = 1;
}

C3DMesh::~C3DMesh() = default;

void C3DMesh::clear()

{
	m_vColor[0] = 1;
	m_vColor[1] = 1;
	m_vColor[2] = 1;

	m_vVertex.clear();
	m_vNormal.clear();
	m_vTriangle.clear();
}

bool C3DMesh::load(const void* pBuffer, unsigned int uiBufferSize)
{
	const auto* l_pBuffer = reinterpret_cast<const uint32_t*>(pBuffer);

	uint32_t l_ui32VertexCount;
	uint32_t l_ui32TriangleCount;

	littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&l_pBuffer[0]), &l_ui32VertexCount);
	littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&l_pBuffer[1]), &l_ui32TriangleCount);

	m_vVertex.resize(l_ui32VertexCount);
	m_vTriangle.resize(l_ui32TriangleCount * 3);

	uint32_t i, j = 2;

	for (i = 0; i < l_ui32VertexCount; ++i)
	{
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&l_pBuffer[j++]), &m_vVertex[i].x);
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&l_pBuffer[j++]), &m_vVertex[i].y);
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&l_pBuffer[j++]), &m_vVertex[i].z);
	}

	for (i = 0; i < l_ui32TriangleCount * 3; ++i)
	{
		littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&l_pBuffer[j++]), &m_vTriangle[i]);
	}

	this->compile();

	return true;
}

bool C3DMesh::compile()

{
	uint32_t i, i1, i2, i3;
	m_vNormal.clear();
	m_vNormal.resize(m_vVertex.size());
	for (i = 0; i < m_vTriangle.size(); i += 3)
	{
		i1 = m_vTriangle[i];
		i2 = m_vTriangle[i + 1];
		i3 = m_vTriangle[i + 2];
		CVertex v1 = m_vVertex[i1];
		CVertex v2 = m_vVertex[i2];
		CVertex v3 = m_vVertex[i3];
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
	for (i = 0; i < m_vNormal.size(); ++i)
	{
		m_vNormal[i].normalize();
	}
	return true;
}

bool C3DMesh::project(std::vector < CVertex >& vProjectedChannelCoordinate, const std::vector < CVertex >& vChannelCoordinate)
{
	size_t i, j;

	vProjectedChannelCoordinate.resize(vChannelCoordinate.size());
	for (i = 0; i < vChannelCoordinate.size(); ++i)
	{
		CVertex p, q;
		p = vChannelCoordinate[i];
		//		q = vChannelCoordinate[i];
		for (j = 0; j < this->m_vTriangle.size(); j += 3)
		{
			uint32_t i1, i2, i3;
			i1 = this->m_vTriangle[j];
			i2 = this->m_vTriangle[j + 1];
			i3 = this->m_vTriangle[j + 2];

			CVertex v1, v2, v3;
			v1 = this->m_vVertex[i1];
			v2 = this->m_vVertex[i2];
			v3 = this->m_vVertex[i3];

			CVertex e1(v1, v2);
			CVertex e2(v1, v3);
			CVertex n = CVertex::cross(e1, e2).normalize();

			float t = CVertex::dot(v1, n) / CVertex::dot(p, n);
			q.x = t * p.x;
			q.y = t * p.y;
			q.z = t * p.z;

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
