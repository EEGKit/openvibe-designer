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

#include <cstdint>
#include <vector>

#include "mCVertex.hpp"


namespace Mensia
{
	namespace AdvancedVisualization
	{
		class C3DMesh
		{
		public:

			C3DMesh();
			C3DMesh(const char* sFilename);
			virtual ~C3DMesh();

			virtual void clear();
			virtual bool load(const void* pBuffer, unsigned int uiBufferSize);
			virtual bool compile();

			virtual bool project(std::vector<CVertex>& vProjectedChannelCoordinate, const std::vector<CVertex>& vChannelCoordinate);

		public:

			std::vector<CVertex> m_vVertex;
			std::vector<CVertex> m_vNormal;
			std::vector<uint32_t> m_vTriangle;
			float m_vColor[3];
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia


#endif // TARGET_HAS_ThirdPartyOpenGL
