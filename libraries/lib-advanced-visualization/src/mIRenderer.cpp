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

IRenderer* IRenderer::create(const int eRendererType, const bool stimulation)
{
	switch (eRendererType)
	{
		case RendererType_2DTopography: return (stimulation ? nullptr : new CRendererTopo2D);
		case RendererType_3DTopography: return (stimulation ? nullptr : new CRendererTopo3D);
		case RendererType_Bars: return (stimulation ? new TRendererStimulation<true, CRendererBars> : new CRendererBars);
		case RendererType_Bitmap: return (stimulation ? new TRendererStimulation<true, CRendererBitmap> : new CRendererBitmap);
		case RendererType_Connectivity: return (stimulation ? nullptr : new CRendererConnectivity);
		case RendererType_Cube: return (stimulation ? nullptr : new CRendererCube);
		case RendererType_Flower: return (stimulation ? nullptr : new CRendererFlower);
		case RendererType_Line: return (stimulation ? new TRendererStimulation<false, CRendererLine> : new CRendererLine);
		case RendererType_Loreta: return (stimulation ? nullptr : new CRendererLoreta);
		case RendererType_Mountain: return (stimulation ? nullptr : new CRendererMountain);
		case RendererType_MultiLine: return (stimulation ? new TRendererStimulation<false, CRendererMultiLine> : new CRendererMultiLine);
		case RendererType_Slice: return (stimulation ? nullptr : new CRendererSlice);
		case RendererType_XYZPlot: return (stimulation ? nullptr : new CRendererXYZPlot);
			//		case RendererType_:             return (stimulation?new TRendererStimulation < false, CRenderer >:new CRenderer);
		default:
			return nullptr;
	}
}

void IRenderer::release(IRenderer* pRenderer) { delete pRenderer; }
