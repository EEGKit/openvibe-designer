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

			void setChannelLocalisation(const char* filename) override { m_channelLocalisationFilename = filename; }
			void setChannelCount(const size_t nChannel) override;
			void setSampleCount(const size_t nSample) override;
			void setHistoryDrawIndex(const size_t index) override;
			void feed(const float* data) override;
			void feed(const float* data, const size_t nSample) override;
			void feed(const uint64_t stimDate, const uint64_t stimID) override { m_stimulationHistory.emplace_back((stimDate >> 16) / 65536., stimID); }
			void prefeed(const size_t nPreFeedSample) override;

			float getSuggestedScale() override;

			void clear(const size_t nSampleToKeep = 0) override;

			size_t getChannelCount() const override { return m_nChannel; }
			size_t getSampleCount() const override { return m_nSample; }
			size_t getHistoryCount() const override { return m_nHistory; }
			size_t getHistoryIndex() const override { return m_historyIdx; }
			virtual bool getSampleAtERPFraction(const float erpFraction, std::vector<float>& samples) const;

			void setTimeOffset(const uint64_t offset) override { m_timeOffset = offset; }
			uint64_t getTimeOffset() const override { return m_timeOffset; }

			void rebuild(const IRendererContext& /*ctx*/) override { }
			void refresh(const IRendererContext& ctx) override;
			// bool render(const IRendererContext& ctx) override;

			void clearRegionSelection() override { }
			size_t getRegionCategoryCount() override { return 0; }
			size_t getRegionCount(const size_t /*category*/) override { return 0; }
			const char* getRegionCategoryName(const size_t /*category*/) override { return nullptr; }
			const char* getRegionName(const size_t /*category*/, const size_t /*index*/) override { return nullptr; }
			void selectRegion(const size_t /*category*/, const char* /*name*/) override { }
			void selectRegion(const size_t /*category*/, const size_t /*index*/) override { }

			virtual void setFaceMeshVisible(bool /*visible = true*/) { }

			virtual void draw3DCoordinateSystem();
			virtual void draw2DCoordinateSystem();

			virtual void drawCoordinateSystem() { this->draw3DCoordinateSystem(); }

		protected:

			std::string m_channelLocalisationFilename;
			size_t m_historyIdx     = 0;
			size_t m_historyDrawIdx = 0;
			size_t m_nHistory       = 0;
			size_t m_nChannel       = 0;
			size_t m_nSample        = 1;

			float m_nInverseChannel       = 1.0;
			float m_nInverseSample        = 1.0;
			size_t m_autoDecimationFactor = 1;

			float m_erpFraction     = 0.0;
			size_t m_sampleIndexERP = 0;

			uint64_t m_timeOffset = 0;

			// std::map < std::string, CVertex > m_channelLocalisation;
			std::vector<std::pair<double, uint64_t>> m_stimulationHistory;
			std::vector<std::vector<float>> m_history;
			std::vector<std::vector<CVertex>> m_vertex;
			std::vector<uint32_t> m_mesh;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
