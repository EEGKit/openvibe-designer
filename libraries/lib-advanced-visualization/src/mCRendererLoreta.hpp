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

#if defined TARGET_HAS_ThirdPartyOpenGL

#ifndef __Mensia_AdvancedVisualization_CRendererLoreta_H__
#define __Mensia_AdvancedVisualization_CRendererLoreta_H__

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

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual bool render(const IRendererContext& rContext);

			virtual void clearRegionSelection();
			virtual uint32_t getRegionCategoryCount();
			virtual uint32_t getRegionCount(uint32_t ui32RegionCategory);
			virtual const char* getRegionCategoryName(uint32_t ui32RegionCategory);
			virtual const char* getRegionName(uint32_t ui32RegionCategory, uint32_t ui32RegionIndex);
			virtual void selectRegion(uint32_t ui32RegionCategory, const char* sRegionName);
			virtual void selectRegion(uint32_t ui32RegionCategory, uint32_t ui32RegionIndex);

			virtual void refreshBrainSubset();

		protected:

			std::vector < std::map < std::string, std::vector < uint32_t > > > m_vLookup;
			std::vector < bool > m_vSelected;

			C3DMesh m_oFace;
			C3DMesh m_oScalp;
			C3DMesh m_oBrain;

			std::vector < uint32_t > m_vBrainSubsetTriangle;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererLoreta_H__

#endif // TARGET_HAS_ThirdPartyOpenGL
