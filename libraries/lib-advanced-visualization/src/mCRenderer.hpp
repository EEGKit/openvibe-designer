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

#ifndef __Mensia_AdvancedVisualization_CRenderer_H__
#define __Mensia_AdvancedVisualization_CRenderer_H__

#include "mIRenderer.h"
#include "mIRendererContext.h"
#include "mCVertex.hpp"

#if defined TARGET_OS_Windows
#include <windows.h>
#endif // TARGET_OS_Windows

#include <GL/gl.h>
#include <GL/glu.h>

#include <string>
#include <map>
#include <vector>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRenderer : public IRenderer
		{
		private:

			CRenderer(const CRenderer&);

		public:

			CRenderer(void);
			virtual ~CRenderer(void);

			virtual void setChannelLocalisation(const char* sFilename);
			virtual void setChannelCount(uint32 ui32ChannelCount);
			virtual void setSampleCount(uint32 ui32SampleCount);
			virtual void setHistoryDrawIndex(uint32 ui32HistoryDrawIndex);
			virtual void feed(const float32* pDataVector);
			virtual void feed(const float32* pDataVector, uint32 ui32SampleCount);
			virtual void feed(uint64 ui64StimulationDate, uint64 ui64StimulationId);
			virtual void prefeed(uint32 ui32PreFeedSampleCount);

			virtual float getSuggestedScale();

			virtual void clear(uint32 ui32SampleCountToKeep);

			virtual uint32 getChannelCount(void) const;
			virtual uint32 getSampleCount(void) const;
			virtual uint32 getHistoryCount(void) const;
			virtual uint32 getHistoryIndex(void) const;
			virtual boolean getSampleAtERPFraction(float32 f32Alpha, std::vector < float32 >& vSample) const;

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
//			virtual boolean render(const IRendererContext& rContext);

			virtual void clearRegionSelection(void) { }
			virtual uint32 getRegionCategoryCount(void) { return 0; }
			virtual uint32 getRegionCount(uint32 ui32RegionCategory) { return 0; }
			virtual const char* getRegionCategoryName(uint32 ui32RegionCategory) { return NULL; }
			virtual const char* getRegionName(uint32 ui32RegionCategory, uint32 ui32RegionIndex) { return NULL; }
			virtual void selectRegion(uint32 ui32RegionCategory, const char* sRegionName) { }
			virtual void selectRegion(uint32 ui32RegionCategory, uint32 ui32RegionIndex) { }
			
			virtual void SetFaceMeshVisible(boolean bVisible = true) { }

			virtual void draw3DCoordinateSystem(void);
			virtual void draw2DCoordinateSystem(void);

			virtual void drawCoordinateSystem(void) // for retro compatibility
			{
				this->draw3DCoordinateSystem();
			}

		protected:

			std::string m_sChannelLocalisationFilename;
			uint32 m_ui32HistoryIndex;
			uint32 m_ui32HistoryDrawIndex;
			uint32 m_ui32HistoryCount;
			uint32 m_ui32ChannelCount;
			uint32 m_ui32SampleCount;

			float32 m_f32InverseChannelCount;
			float32 m_f32InverseSampleCount;
			uint32 m_ui32AutoDecimationFactor;

			float32 m_f32ERPFraction;
			uint32 m_ui32SampleIndexERP;

//			std::map < std::string, CVertex > m_vChannelLocalisation;
			std::vector < std::pair < float64, uint64 > > m_vStimulationHistory;
			std::vector < std::vector < float32 > > m_vHistory;
			std::vector < std::vector < CVertex > > m_vVertex;
			std::vector < uint32 > m_vMesh;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRenderer_H__
