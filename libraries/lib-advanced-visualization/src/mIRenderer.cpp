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

#include "mIRendererContext.h"

#include "mCRendererBitmap.hpp"
#include "mCRendererLine.hpp"
#include "mCRendererMultiLine.hpp"
#include "mCRendererBars.hpp"
#include "mCRendererTopo2D.hpp"
#include "mCRendererTopo3D.hpp"
#include "mCRendererCube.hpp"
#include "mCRendererLoreta.hpp"
#include "mCRendererFlower.hpp"
#include "mCRendererMountain.hpp"
#include "mCRendererSlice.hpp"
#include "mCRendererXYZPlot.hpp"
#include "mCRendererConnectivity.hpp"
#include "mTRendererStimulation.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

IRenderer* IRenderer::create(int eRendererType, bool bStimulation)
{
	switch(eRendererType)
	{
		case RendererType_2DTopography: return (bStimulation?nullptr:new CRendererTopo2D);
		case RendererType_3DTopography: return (bStimulation?nullptr:new CRendererTopo3D);
		case RendererType_Bars:         return (bStimulation?new TRendererStimulation < true, CRendererBars >:new CRendererBars);
		case RendererType_Bitmap:       return (bStimulation?new TRendererStimulation < true, CRendererBitmap >:new CRendererBitmap);
		case RendererType_Connectivity: return (bStimulation?nullptr:new CRendererConnectivity);
		case RendererType_Cube:         return (bStimulation?nullptr:new CRendererCube);
		case RendererType_Flower:       return (bStimulation?nullptr:new CRendererFlower);
		case RendererType_Line:         return (bStimulation?new TRendererStimulation < false, CRendererLine >:new CRendererLine);
		case RendererType_Loreta:       return (bStimulation?nullptr:new CRendererLoreta);
		case RendererType_Mountain:     return (bStimulation?nullptr:new CRendererMountain);
		case RendererType_MultiLine:    return (bStimulation?new TRendererStimulation < false, CRendererMultiLine >:new CRendererMultiLine);
		case RendererType_Slice:        return (bStimulation?nullptr:new CRendererSlice);
		case RendererType_XYZPlot:      return (bStimulation?nullptr:new CRendererXYZPlot);
//		case RendererType_:             return (bStimulation?new TRendererStimulation < false, CRenderer >:new CRenderer);
		default:
			return nullptr;
	}
	return nullptr;
}

void IRenderer::release(IRenderer* pRenderer)
{
	delete pRenderer;
}
