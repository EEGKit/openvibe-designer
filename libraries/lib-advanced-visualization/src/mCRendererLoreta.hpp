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

#if defined TARGET_HAS_ThirdPartyOpenGL


#include "mCRenderer.hpp"
#include "mC3DMesh.hpp"
#include <vector>
#include <map>
#include <string>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererLoreta : public CRenderer
		{
		public:

			CRendererLoreta();

			void rebuild(const IRendererContext& rContext) override;
			void refresh(const IRendererContext& rContext) override;
			bool render(const IRendererContext& rContext) override;

			void clearRegionSelection() override;
			uint32_t getRegionCategoryCount() override;
			uint32_t getRegionCount(const uint32_t regionCategory) override;
			const char* getRegionCategoryName(const uint32_t regionCategory) override;
			const char* getRegionName(const uint32_t regionCategory, const uint32_t regionIndex) override;
			void selectRegion(const uint32_t regionCategory, const char* sRegionName) override;
			void selectRegion(const uint32_t regionCategory, const uint32_t regionIndex) override;

			virtual void refreshBrainSubset();

		protected:

			std::vector<std::map<std::string, std::vector<uint32_t>>> m_vLookup;
			std::vector<bool> m_vSelected;

			C3DMesh m_oFace;
			C3DMesh m_oScalp;
			C3DMesh m_oBrain;

			std::vector<uint32_t> m_vBrainSubsetTriangle;
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia

#endif // TARGET_HAS_ThirdPartyOpenGL
