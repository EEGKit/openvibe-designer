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

#if defined TARGET_HAS_ThirdPartyOpenGL

#ifndef __Mensia_AdvancedVisualization_C3DMesh_H__
#define __Mensia_AdvancedVisualization_C3DMesh_H__

#include "mCVertex.hpp"

#include <vector>

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
			virtual boolean load(const void* pBuffer, unsigned int uiBufferSize);
			virtual boolean load(const char* sFilename);
			virtual boolean save(const char* sFilename) const;
			virtual boolean compile(void);

			virtual boolean project(std::vector < CVertex >& vProjectedChannelCoordinate, const std::vector < CVertex >& vChannelCoordinate);

		public:

			std::vector < CVertex > m_vVertex;
			std::vector < CVertex > m_vNormal;
			std::vector < uint32 > m_vTriangle;
			float32 m_vColor[3];
		};
	};
};

#endif // __Mensia_AdvancedVisualization_C3DMesh_H__

#endif // TARGET_HAS_ThirdPartyOpenGL
