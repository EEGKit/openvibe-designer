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
		public:
			CRenderer();
			CRenderer(const CRenderer&) = delete;
			~CRenderer() override;

			void setChannelLocalisation(const char* sFilename) override;
			void setChannelCount(const uint32_t channelCount) override;
			void setSampleCount(const uint32_t sampleCount) override;
			void setHistoryDrawIndex(const uint32_t index) override;
			void feed(const float* pDataVector) override;
			void feed(const float* pDataVector, const uint32_t sampleCount) override;
			void feed(const uint64_t stimulationDate, const uint64_t stimulationId) override;
			void prefeed(const uint32_t preFeedSampleCount) override;

			float getSuggestedScale() override;

			void clear(const uint32_t sampleCountToKeep) override;

			uint32_t getChannelCount() const override;
			uint32_t getSampleCount() const override;
			uint32_t getHistoryCount() const override;
			uint32_t getHistoryIndex() const override;
			virtual bool getSampleAtERPFraction(const float fERPFraction, std::vector<float>& vSample) const;

			void setTimeOffset(const uint64_t offset) override { m_timeOffset = offset; };
			uint64_t getTimeOffset() const override { return m_timeOffset; }

			void rebuild(const IRendererContext& rContext) override;
			void refresh(const IRendererContext& rContext) override;
			//			virtual bool render(const IRendererContext& rContext);

			void clearRegionSelection() override { }
			uint32_t getRegionCategoryCount() override { return 0; }
			uint32_t getRegionCount(const uint32_t /*regionCategory*/) override { return 0; }
			const char* getRegionCategoryName(const uint32_t /*regionCategory*/) override { return nullptr; }
			const char* getRegionName(const uint32_t /*regionCategory*/, uint32_t /*regionIndex*/) override { return nullptr; }
			void selectRegion(const uint32_t /*regionCategory*/, const char* /*sRegionName*/) override { }
			void selectRegion(const uint32_t /*regionCategory*/, uint32_t /*regionIndex*/) override { }

			virtual void SetFaceMeshVisible(bool /*bVisible = true*/) { }

			virtual void draw3DCoordinateSystem();
			virtual void draw2DCoordinateSystem();

			virtual void drawCoordinateSystem() // for retro compatibility
			{
				this->draw3DCoordinateSystem();
			}

		protected:

			std::string m_channelLocalisationFilename;
			uint32_t m_historyIndex     = 0;
			uint32_t m_historyDrawIndex = 0;
			uint32_t m_historyCount     = 0;
			uint32_t m_channelCount     = 0;
			uint32_t m_sampleCount      = 1;

			float m_inverseChannelCount     = 1.0;
			float m_inverseSampleCount      = 1.0;
			uint32_t m_autoDecimationFactor = 1;

			float m_ERPFraction       = 0.0;
			uint32_t m_sampleIndexERP = 0;

			uint64_t m_timeOffset = 0;

			//			std::map < std::string, CVertex > m_channelLocalisation;
			std::vector<std::pair<double, uint64_t>> m_stimulationHistory;
			std::vector<std::vector<float>> m_history;
			std::vector<std::vector<CVertex>> m_vertex;
			std::vector<uint32_t> m_mesh;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
