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

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerProgress : public IRuler
		{
		public:

			virtual void renderFinal(float fProgress) = 0;

			void render() override

			{
#if 0
				::printf("%p = %p\n", this, m_pRenderer);
#endif
				if (m_pRenderer == nullptr) { return; }
				if (m_pRenderer->getSampleCount() == 0) { return; }
				if (m_pRenderer->getHistoryCount() == 0) { return; }
				if (m_pRenderer->getHistoryIndex() == 0) { return; }

				const uint32_t sampleCount = m_pRenderer->getSampleCount();
				const uint32_t historyIndex = m_pRenderer->getHistoryIndex();

				const float l_fProgress = float(historyIndex - (float(historyIndex) / sampleCount) * sampleCount) / sampleCount;
				if (l_fProgress != 0 && l_fProgress != 1)
				{
					this->renderFinal(l_fProgress);
				}
			}
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia
