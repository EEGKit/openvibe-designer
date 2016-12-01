/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
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

			C3DMesh(void);
			C3DMesh(const char* sFilename);
			virtual ~C3DMesh(void);

			virtual void clear(void);
			virtual bool load(const void* pBuffer, unsigned int uiBufferSize);
			virtual bool compile(void);

			virtual bool project(std::vector < CVertex >& vProjectedChannelCoordinate, const std::vector < CVertex >& vChannelCoordinate);

		public:

			std::vector < CVertex > m_vVertex;
			std::vector < CVertex > m_vNormal;
			std::vector < uint32_t > m_vTriangle;
			float m_vColor[3];
		};
	};
};


#endif // TARGET_HAS_ThirdPartyOpenGL
