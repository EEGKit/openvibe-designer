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
#include <Windows.h>
#endif // TARGET_OS_Windows

#include <gl/GL.h>
#include <gl/GLU.h>

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

			CRenderer(const CRenderer&) = delete;

		public:

			CRenderer();
			~CRenderer() override;

			void setChannelLocalisation(const char* sFilename) override;
			void setChannelCount(uint32_t ui32ChannelCount) override;
			void setSampleCount(uint32_t ui32SampleCount) override;
			void setHistoryDrawIndex(uint32_t ui32HistoryDrawIndex) override;
			void feed(const float* pDataVector) override;
			void feed(const float* pDataVector, uint32_t ui32SampleCount) override;
			void feed(uint64_t ui64StimulationDate, uint64_t ui64StimulationId) override;
			void prefeed(uint32_t ui32PreFeedSampleCount) override;

			float getSuggestedScale() override;

			void clear(uint32_t ui32SampleCountToKeep) override;

			uint32_t getChannelCount() const override;
			uint32_t getSampleCount() const override;
			uint32_t getHistoryCount() const override;
			uint32_t getHistoryIndex() const override;
			virtual bool getSampleAtERPFraction(float f32Alpha, std::vector<float>& vSample) const;

			void setTimeOffset(uint64_t offset) override { m_ui64TimeOffset = offset; };
			uint64_t getTimeOffset() const override { return m_ui64TimeOffset; }

			void rebuild(const IRendererContext& rContext) override;
			void refresh(const IRendererContext& rContext) override;
			//			virtual bool render(const IRendererContext& rContext);

			void clearRegionSelection() override { }
			uint32_t getRegionCategoryCount() override { return 0; }
			uint32_t getRegionCount(uint32_t ui32RegionCategory) override { return 0; }
			const char* getRegionCategoryName(uint32_t ui32RegionCategory) override { return nullptr; }
			const char* getRegionName(uint32_t ui32RegionCategory, uint32_t ui32RegionIndex) override { return nullptr; }
			void selectRegion(uint32_t ui32RegionCategory, const char* sRegionName) override { }
			void selectRegion(uint32_t ui32RegionCategory, uint32_t ui32RegionIndex) override { }

			virtual void SetFaceMeshVisible(bool bVisible = true) { }

			virtual void draw3DCoordinateSystem();
			virtual void draw2DCoordinateSystem();

			virtual void drawCoordinateSystem() // for retro compatibility
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

			uint64_t m_ui64TimeOffset;

			//			std::map < std::string, CVertex > m_vChannelLocalisation;
			std::vector<std::pair<double, uint64_t>> m_vStimulationHistory;
			std::vector<std::vector<float>> m_vHistory;
			std::vector<std::vector<CVertex>> m_vVertex;
			std::vector<uint32_t> m_vMesh;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia

#endif // __Mensia_AdvancedVisualization_CRenderer_H__
