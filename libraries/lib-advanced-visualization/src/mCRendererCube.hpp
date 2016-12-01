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

#ifndef __Mensia_AdvancedVisualization_CRendererCube_H__
#define __Mensia_AdvancedVisualization_CRendererCube_H__

#include "mCRenderer.hpp"

#include <map>
#include <string>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererCube : public CRenderer
		{
		public:

			CRendererCube(void);

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual bool render(const IRendererContext& rContext);

			std::vector < CVertex > m_vVertex;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererCube_H__
