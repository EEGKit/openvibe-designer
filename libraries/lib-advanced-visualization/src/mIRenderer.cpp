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

IRenderer* IRenderer::create(const ERendererType type, const bool stimulation)
{
	switch (type)
	{
		case ERendererType::Topography2D: return (stimulation ? nullptr : new CRendererTopo2D);
		case ERendererType::Topography3D: return (stimulation ? nullptr : new CRendererTopo3D);
		case ERendererType::Bars: return (stimulation ? new TRendererStimulation<true, CRendererBars> : new CRendererBars);
		case ERendererType::Bitmap: return (stimulation ? new TRendererStimulation<true, CRendererBitmap> : new CRendererBitmap);
		case ERendererType::Connectivity: return (stimulation ? nullptr : new CRendererConnectivity);
		case ERendererType::Cube: return (stimulation ? nullptr : new CRendererCube);
		case ERendererType::Flower: return (stimulation ? nullptr : new CRendererFlower);
		case ERendererType::Line: return (stimulation ? new TRendererStimulation<false, CRendererLine> : new CRendererLine);
		case ERendererType::Loreta: return (stimulation ? nullptr : new CRendererLoreta);
		case ERendererType::Mountain: return (stimulation ? nullptr : new CRendererMountain);
		case ERendererType::MultiLine: return (stimulation ? new TRendererStimulation<false, CRendererMultiLine> : new CRendererMultiLine);
		case ERendererType::Slice: return (stimulation ? nullptr : new CRendererSlice);
		case ERendererType::XYZPlot: return (stimulation ? nullptr : new CRendererXYZPlot);
			// case ERendererType::Default: return (stimulation ? new TRendererStimulation<false, CRenderer> : new CRenderer);
		default:
			return nullptr;
	}
}

void IRenderer::release(IRenderer* pRenderer) { delete pRenderer; }
