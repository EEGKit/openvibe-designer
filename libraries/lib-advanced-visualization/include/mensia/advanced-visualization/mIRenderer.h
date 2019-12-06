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

#include "m_defines.h"
#include "mIRendererContext.h"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		enum class ERendererType
		{
			Default,
			Topography2D,
			Topography3D,
			Bars,
			Bitmap,
			Connectivity,
			Cube,
			Flower,
			Line,
			Loreta,
			Mountain,
			MultiLine,
			Slice,
			XYZPlot,
			Last
		};
		
		class LMAV_API IRenderer
		{
		public:

			static IRenderer* create(ERendererType type, bool stimulation);
			static void release(IRenderer* renderer) { delete renderer; }

			virtual ~IRenderer() = default;

			virtual void setChannelLocalisation(const char* filename) = 0;
			virtual void setChannelCount(const size_t nChannel) = 0;
			virtual void setSampleCount(const size_t nSample) = 0;
			virtual void setHistoryDrawIndex(const size_t index) = 0;
			virtual void feed(const float* data) = 0;
			virtual void feed(const float* data, const size_t nSample) = 0;
			virtual void feed(const uint64_t stimDate, const uint64_t stimID) = 0;
			virtual void clear(const size_t nSampleToKeep) = 0;
			virtual void prefeed(const size_t preFeedNSample) = 0;

			virtual float getSuggestedScale() = 0;

			virtual size_t getChannelCount() const = 0;
			virtual size_t getSampleCount() const = 0;
			virtual size_t getHistoryCount() const = 0;
			virtual size_t getHistoryIndex() const = 0;

			virtual void setTimeOffset(const uint64_t offset) = 0;
			virtual uint64_t getTimeOffset() const = 0;

			virtual void rebuild(const IRendererContext& ctx) = 0;
			virtual void refresh(const IRendererContext& ctx) = 0;
			virtual bool render(const IRendererContext& ctx) = 0;

			// For regions of interest
			virtual void clearRegionSelection() = 0;
			virtual size_t getRegionCategoryCount() = 0;
			virtual size_t getRegionCount(const size_t category) = 0;
			virtual const char* getRegionCategoryName(const size_t category) = 0;
			virtual const char* getRegionName(const size_t category, const size_t index) = 0;
			virtual void selectRegion(const size_t category, const char* name) = 0;
			virtual void selectRegion(const size_t category, const size_t index) = 0;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
