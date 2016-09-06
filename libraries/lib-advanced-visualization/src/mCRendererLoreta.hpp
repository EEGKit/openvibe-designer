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

			CRendererLoreta(void);

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual boolean render(const IRendererContext& rContext);

			virtual void clearRegionSelection(void);
			virtual uint32 getRegionCategoryCount(void);
			virtual uint32 getRegionCount(uint32 ui32RegionCategory);
			virtual const char* getRegionCategoryName(uint32 ui32RegionCategory);
			virtual const char* getRegionName(uint32 ui32RegionCategory, uint32 ui32RegionIndex);
			virtual void selectRegion(uint32 ui32RegionCategory, const char* sRegionName);
			virtual void selectRegion(uint32 ui32RegionCategory, uint32 ui32RegionIndex);

			virtual void refreshBrainSubset(void);

		protected:

			std::vector < std::map < std::string, std::vector < uint32 > > > m_vLookup;
			std::vector < boolean > m_vSelected;

			C3DMesh m_oFace;
			C3DMesh m_oScalp;
			C3DMesh m_oBrain;

			std::vector < uint32 > m_vBrainSubsetTriangle;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererLoreta_H__

#endif // TARGET_HAS_ThirdPartyOpenGL
