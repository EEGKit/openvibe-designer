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

#pragma once

#if defined TARGET_HAS_ThirdPartyOpenGL

#include <vector>
#include <array>
#include <cstdint>
#include <cstdlib>	// size_t for unix

#include "mCVertex.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {
class C3DMesh final
{
public:

	C3DMesh() { m_Color.fill(1.0); }
	//C3DMesh(const char* filename);
	~C3DMesh() = default;

	void clear();
	bool load(const void* buffer);
	bool compile();

	bool project(std::vector<CVertex>& out, const std::vector<CVertex>& in);

	std::vector<CVertex> m_Vertices;
	std::vector<CVertex> m_Normals;
	std::vector<uint32_t> m_Triangles;
	std::array<float, 3> m_Color;
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE


#endif // TARGET_HAS_ThirdPartyOpenGL
