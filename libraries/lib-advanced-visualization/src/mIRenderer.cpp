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
using namespace Mensia::AdvancedVisualization;

IRenderer* IRenderer::create(int eRendererType, bool bStimulation)
{
	switch(eRendererType)
	{
		case RendererType_2DTopography: return (bStimulation?NULL:new CRendererTopo2D);
		case RendererType_3DTopography: return (bStimulation?NULL:new CRendererTopo3D);
		case RendererType_Bars:         return (bStimulation?new TRendererStimulation < true, CRendererBars >:new CRendererBars);
		case RendererType_Bitmap:       return (bStimulation?new TRendererStimulation < true, CRendererBitmap >:new CRendererBitmap);
		case RendererType_Connectivity: return (bStimulation?NULL:new CRendererConnectivity);
		case RendererType_Cube:         return (bStimulation?NULL:new CRendererCube);
		case RendererType_Flower:       return (bStimulation?NULL:new CRendererFlower);
		case RendererType_Line:         return (bStimulation?new TRendererStimulation < false, CRendererLine >:new CRendererLine);
		case RendererType_Loreta:       return (bStimulation?NULL:new CRendererLoreta);
		case RendererType_Mountain:     return (bStimulation?NULL:new CRendererMountain);
		case RendererType_MultiLine:    return (bStimulation?new TRendererStimulation < false, CRendererMultiLine >:new CRendererMultiLine);
		case RendererType_Slice:        return (bStimulation?NULL:new CRendererSlice);
		case RendererType_XYZPlot:      return (bStimulation?NULL:new CRendererXYZPlot);
//		case RendererType_:             return (bStimulation?new TRendererStimulation < false, CRenderer >:new CRenderer);
		default:
			return NULL;
	}
	return NULL;
}

void IRenderer::release(IRenderer* pRenderer)
{
	delete pRenderer;
}
