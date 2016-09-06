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

#ifndef __Mensia_AdvancedVisualization_CRendererXYZPlot_H__
#define __Mensia_AdvancedVisualization_CRendererXYZPlot_H__

#include "mCRenderer.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererXYZPlot : public CRenderer
		{
		public:

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual boolean render(const IRendererContext& rContext);

			boolean m_bHasDepth;
			uint32 m_ui32PlotDimension;
			uint32 m_ui32PlotCount;
			std::vector < std::vector < CVertex > > m_vVertex;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererXYZPlot_H__
