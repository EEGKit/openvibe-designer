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

#include <cstring>

using namespace Mensia;
using namespace AdvancedVisualization;

namespace
{
	template <typename T>
	bool littleEndianToHost(const uint8_t* buffer, T* value)
	{
		if (!buffer) { return false; }
		if (!value) { return false; }
		memset(value, 0, sizeof(T));
		for (uint32_t i = 0; i < sizeof(T); ++i) { reinterpret_cast<uint8_t*>(value)[i] = buffer[i]; }
		return true;
	}
} // namespace


void C3DMesh::clear()
{
	m_Color.fill(1.0);

	m_Vertices.clear();
	m_Normals.clear();
	m_Triangles.clear();
}

bool C3DMesh::load(const void* buffer)
{
	const auto* tmp = reinterpret_cast<const uint32_t*>(buffer);

	uint32_t nVertex;
	uint32_t nTriangle;

	littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&tmp[0]), &nVertex);
	littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&tmp[1]), &nTriangle);

	m_Vertices.resize(nVertex);
	m_Triangles.resize(size_t(nTriangle) * 3);

	size_t j = 2;

	for (size_t i = 0; i < nVertex; ++i)
	{
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_Vertices[i].x);
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_Vertices[i].y);
		littleEndianToHost<float>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_Vertices[i].z);
	}

	for (size_t i = 0; i < nTriangle * 3; ++i) { littleEndianToHost<uint32_t>(reinterpret_cast<const uint8_t*>(&tmp[j++]), &m_Triangles[i]); }

	this->compile();

	return true;
}

bool C3DMesh::compile()
{
	m_Normals.clear();
	m_Normals.resize(m_Vertices.size());
	for (size_t i = 0; i < m_Triangles.size(); i += 3)
	{
		const uint32_t i1 = m_Triangles[i];
		const uint32_t i2 = m_Triangles[i + 1];
		const uint32_t i3 = m_Triangles[i + 2];
		CVertex v1        = m_Vertices[i1];
		CVertex v2        = m_Vertices[i2];
		CVertex v3        = m_Vertices[i3];
		v2.x -= v1.x;
		v2.y -= v1.y;
		v2.z -= v1.z;
		v3.x -= v1.x;
		v3.y -= v1.y;
		v3.z -= v1.z;
		v1 = CVertex::cross(v2, v3);
		v1.normalize();
		m_Normals[i1].x += v1.x;
		m_Normals[i1].y += v1.y;
		m_Normals[i1].z += v1.z;
		m_Normals[i2].x += v1.x;
		m_Normals[i2].y += v1.y;
		m_Normals[i2].z += v1.z;
		m_Normals[i3].x += v1.x;
		m_Normals[i3].y += v1.y;
		m_Normals[i3].z += v1.z;
	}

	for (auto& normal : m_Normals) { normal.normalize(); }
	return true;
}

bool C3DMesh::project(std::vector<CVertex>& out, const std::vector<CVertex>& in)
{
	out.resize(in.size());
	for (size_t i = 0; i < in.size(); ++i)
	{
		CVertex p, q;
		p = in[i];
		// q = vChannelCoordinate[i];
		for (size_t j = 0; j < this->m_Triangles.size(); j += 3)
		{
			const uint32_t i1 = this->m_Triangles[j];
			const uint32_t i2 = this->m_Triangles[j + 1];
			const uint32_t i3 = this->m_Triangles[j + 2];

			CVertex v1, v2, v3;
			v1 = this->m_Vertices[i1];
			v2 = this->m_Vertices[i2];
			v3 = this->m_Vertices[i3];

			CVertex e1(v1, v2);
			CVertex e2(v1, v3);
			CVertex n = CVertex::cross(e1, e2).normalize();

			const float t = CVertex::dot(v1, n) / CVertex::dot(p, n);
			q.x           = t * p.x;
			q.y           = t * p.y;
			q.z           = t * p.z;

			if (CVertex::isInTriangle(q, v1, v2, v3) && t >= 0) { out[i] = q; }
		}
		//  if (q.x == 0 && q.y == 0 && q.z == 0) { ::printf("Could not project coordinates on mesh for channel %i [%s]\n", i+1, ctx.getChannelName(i).c_str()); }
	}
	return true;
}

#endif // TARGET_HAS_ThirdPartyOpenGL
