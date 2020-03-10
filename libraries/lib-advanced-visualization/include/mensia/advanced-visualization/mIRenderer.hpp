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

#include "m_defines.hpp"
#include "mCRendererContext.hpp"
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
		enum class ERendererType
		{
			Default, Topography2D, Topography3D, Bars, Bitmap, Connectivity, Cube, Flower, Line, Loreta, Mountain, MultiLine, Slice, XYZPlot, Last
		};

		class LMAV_API IRenderer
		{
		public:
			IRenderer();
			IRenderer(const IRenderer&) = delete;
			virtual ~IRenderer();

			static IRenderer* create(const ERendererType type, const bool stimulation);
			static void release(IRenderer* renderer) { delete renderer; }

			void setChannelLocalisation(const char* filename) { m_channelPosFilename = filename; }
			void setChannelCount(const size_t nChannel);
			void setSampleCount(const size_t nSample);
			void setHistoryDrawIndex(const size_t index);
			void feed(const float* data);
			void feed(const float* data, const size_t nSample);
			void feed(const uint64_t stimDate, const uint64_t stimID) { m_stimulationHistory.emplace_back((stimDate >> 16) / 65536., stimID); }
			void prefeed(const size_t nPreFeedSample);

			float getSuggestedScale();

			void clear(const size_t nSampleToKeep = 0);

			size_t getChannelCount() const { return m_nChannel; }
			size_t getSampleCount() const { return m_nSample; }
			size_t getHistoryCount() const { return m_nHistory; }
			size_t getHistoryIndex() const { return m_historyIdx; }
			bool getSampleAtERPFraction(const float erpFraction, std::vector<float>& samples) const;

			void setTimeOffset(const uint64_t offset) { m_timeOffset = offset; }
			uint64_t getTimeOffset() const { return m_timeOffset; }

			static void draw2DCoordinateSystem();
			static void draw3DCoordinateSystem();
			void drawCoordinateSystem() const { this->draw3DCoordinateSystem(); }

			virtual void rebuild(const CRendererContext& /*ctx*/) { }
			virtual void refresh(const CRendererContext& ctx);
			virtual bool render(const CRendererContext& ctx) = 0;

			virtual void clearRegionSelection() { }
			virtual size_t getRegionCategoryCount() { return 0; }
			virtual size_t getRegionCount(const size_t /*category*/) { return 0; }
			virtual const char* getRegionCategoryName(const size_t /*category*/) { return nullptr; }
			virtual const char* getRegionName(const size_t /*category*/, const size_t /*index*/) { return nullptr; }
			virtual void selectRegion(const size_t /*category*/, const char* /*name*/) { }
			virtual void selectRegion(const size_t /*category*/, const size_t /*index*/) { }


		protected:

			std::string m_channelPosFilename;
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

			// std::map < std::string, CVertex > m_channelPos;
			std::vector<std::pair<double, uint64_t>> m_stimulationHistory;
			std::vector<std::vector<float>> m_history;
			std::vector<std::vector<CVertex>> m_vertex;
			std::vector<size_t> m_mesh;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
