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

#include <string>
#include <vector>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class LMAV_API IRenderer
		{
		public:

			typedef enum
			{
				RendererType_Default,

				RendererType_2DTopography,
				RendererType_3DTopography,
				RendererType_Bars,
				RendererType_Bitmap,
				RendererType_Connectivity,
				RendererType_Cube,
				RendererType_Flower,
				RendererType_Line,
				RendererType_Loreta,
				RendererType_Mountain,
				RendererType_MultiLine,
				RendererType_Slice,
				RendererType_XYZPlot,

				RendererType_Last,
			} ERendererType;

			static IRenderer* create(int eRendererType, bool stimulation);
			static void release(IRenderer* pRenderer);

			virtual ~IRenderer() = default;

			virtual void setChannelLocalisation(const char* sFilename) = 0;
			virtual void setChannelCount(const uint32_t ui32ChannelCount) = 0;
			virtual void setSampleCount(const uint32_t ui32SampleCount) = 0;
			virtual void setHistoryDrawIndex(const uint32_t ui32HistoryDrawIndex) = 0;
			virtual void feed(const float* pDataVector) = 0;
			virtual void feed(const float* pDataVector, const uint32_t ui32SampleCount) = 0;
			virtual void feed(const uint64_t ui64StimulationDate, const uint64_t ui64StimulationId) = 0;
			virtual void clear(const uint32_t ui32SampleCountToKeep) = 0;
			virtual void prefeed(const uint32_t ui32PreFeedSampleCount) = 0;

			virtual float getSuggestedScale() = 0;

			virtual uint32_t getChannelCount() const = 0;
			virtual uint32_t getSampleCount() const = 0;
			virtual uint32_t getHistoryCount() const = 0;
			virtual uint32_t getHistoryIndex() const = 0;

			virtual void setTimeOffset(const uint64_t offset) = 0;
			virtual uint64_t getTimeOffset() const = 0;

			virtual void rebuild(const IRendererContext& rContext) = 0;
			virtual void refresh(const IRendererContext& rContext) = 0;
			virtual bool render(const IRendererContext& rContext) = 0;

			// For regions of interest
			virtual void clearRegionSelection() = 0;
			virtual uint32_t getRegionCategoryCount() = 0;
			virtual uint32_t getRegionCount(const uint32_t regionCategory) = 0;
			virtual const char* getRegionCategoryName(const uint32_t regionCategory) = 0;
			virtual const char* getRegionName(const uint32_t regionCategory, const uint32_t regionIndex) = 0;
			virtual void selectRegion(const uint32_t regionCategory, const char* sRegionName) = 0;
			virtual void selectRegion(const uint32_t regionCategory, const uint32_t regionIndex) = 0;
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia
