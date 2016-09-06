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

#ifndef __Mensia_AdvancedVisualization_CRendererBitmap_H__
#define __Mensia_AdvancedVisualization_CRendererBitmap_H__

#include "mCRenderer.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererBitmap : public CRenderer
		{
		public:

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual boolean render(const IRendererContext& rContext);
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererBitmap_H__
