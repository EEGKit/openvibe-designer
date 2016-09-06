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

#ifndef __Mensia_AdvancedVisualization_IRenderer_H__
#define __Mensia_AdvancedVisualization_IRenderer_H__

#include "m_defines.h"
#include "mIRendererContext.h"

#include <mensia/base.h>

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

			static IRenderer* create(int eRendererType, bool bStimulation);
			static void release(IRenderer* pRenderer);

		public:

			virtual ~IRenderer(void) { }

			virtual void setChannelLocalisation(const char* sFilename)=0;
			virtual void setChannelCount(uint32 ui32ChannelCount)=0;
			virtual void setSampleCount(uint32 ui32SampleCount)=0;
			virtual void setHistoryDrawIndex(uint32 ui32HistoryDrawIndex) = 0;
			virtual void feed(const float32* pDataVector)=0;
			virtual void feed(const float32* pDataVector, uint32 ui32SampleCount)=0;
			virtual void feed(uint64 ui64StimulationDate, uint64 ui64StimulationId)=0;
			virtual void clear(uint32 ui32SampleCountToKeep) = 0;
			virtual void prefeed(uint32 ui32PreFeedSampleCount)=0;

			virtual float32 getSuggestedScale() = 0;

			virtual uint32 getChannelCount(void) const=0;
			virtual uint32 getSampleCount(void) const=0;
			virtual uint32 getHistoryCount(void) const=0;
			virtual uint32 getHistoryIndex(void) const=0;

			virtual void rebuild(const IRendererContext& rContext)=0;
			virtual void refresh(const IRendererContext& rContext)=0;
			virtual boolean render(const IRendererContext& rContext)=0;

			// For regions of interest
			virtual void clearRegionSelection(void)=0;
			virtual uint32 getRegionCategoryCount(void)=0;
			virtual uint32 getRegionCount(uint32 ui32RegionCategory)=0;
			virtual const char* getRegionCategoryName(uint32 ui32RegionCategory)=0;
			virtual const char* getRegionName(uint32 ui32RegionCategory, uint32 ui32RegionIndex)=0;
			virtual void selectRegion(uint32 ui32RegionCategory, const char* sRegionName)=0;
			virtual void selectRegion(uint32 ui32RegionCategory, uint32 ui32RegionIndex)=0;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_IRenderer_H__
