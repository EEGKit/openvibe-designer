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

#ifndef __Mensia_AdvancedVisualization_CRendererSlice_H__
#define __Mensia_AdvancedVisualization_CRendererSlice_H__

#include "mCRenderer.hpp"

#include "mC3DMesh.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererSlice : public CRenderer
		{
		public:

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual bool render(const IRendererContext& rContext);

			std::vector < CVertex > m_vVertex;
			std::vector < uint32_t > m_vQuad;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererSlice_H__
