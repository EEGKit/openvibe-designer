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
			virtual void setChannelCount(uint32_t ui32ChannelCount);
			virtual void setSampleCount(uint32_t ui32SampleCount);
			virtual void setHistoryDrawIndex(uint32_t ui32HistoryDrawIndex);
			virtual void feed(const float* pDataVector);
			virtual void feed(const float* pDataVector, uint32_t ui32SampleCount);
			virtual void feed(uint64_t ui64StimulationDate, uint64_t ui64StimulationId);
			virtual void prefeed(uint32_t ui32PreFeedSampleCount);

			virtual float getSuggestedScale();

			virtual void clear(uint32_t ui32SampleCountToKeep);

			virtual uint32_t getChannelCount(void) const;
			virtual uint32_t getSampleCount(void) const;
			virtual uint32_t getHistoryCount(void) const;
			virtual uint32_t getHistoryIndex(void) const;
			virtual bool getSampleAtERPFraction(float f32Alpha, std::vector < float >& vSample) const;

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
//			virtual bool render(const IRendererContext& rContext);

			virtual void clearRegionSelection(void) { }
			virtual uint32_t getRegionCategoryCount(void) { return 0; }
			virtual uint32_t getRegionCount(uint32_t ui32RegionCategory) { return 0; }
			virtual const char* getRegionCategoryName(uint32_t ui32RegionCategory) { return NULL; }
			virtual const char* getRegionName(uint32_t ui32RegionCategory, uint32_t ui32RegionIndex) { return NULL; }
			virtual void selectRegion(uint32_t ui32RegionCategory, const char* sRegionName) { }
			virtual void selectRegion(uint32_t ui32RegionCategory, uint32_t ui32RegionIndex) { }
			
			virtual void SetFaceMeshVisible(bool bVisible = true) { }

			virtual void draw3DCoordinateSystem(void);
			virtual void draw2DCoordinateSystem(void);

			virtual void drawCoordinateSystem(void) // for retro compatibility
			{
				this->draw3DCoordinateSystem();
			}

		protected:

			std::string m_sChannelLocalisationFilename;
			uint32_t m_ui32HistoryIndex;
			uint32_t m_ui32HistoryDrawIndex;
			uint32_t m_ui32HistoryCount;
			uint32_t m_ui32ChannelCount;
			uint32_t m_ui32SampleCount;

			float m_f32InverseChannelCount;
			float m_f32InverseSampleCount;
			uint32_t m_ui32AutoDecimationFactor;

			float m_f32ERPFraction;
			uint32_t m_ui32SampleIndexERP;

//			std::map < std::string, CVertex > m_vChannelLocalisation;
			std::vector < std::pair < double, uint64_t > > m_vStimulationHistory;
			std::vector < std::vector < float > > m_vHistory;
			std::vector < std::vector < CVertex > > m_vVertex;
			std::vector < uint32_t > m_vMesh;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRenderer_H__
