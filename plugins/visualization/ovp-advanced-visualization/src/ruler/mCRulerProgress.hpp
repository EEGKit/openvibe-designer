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

#ifndef __OpenViBEPlugins_CRulerProgress_H__
#define __OpenViBEPlugins_CRulerProgress_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerProgress : public IRuler
		{
		public:

			virtual void renderFinal(float fProgress)=0;

			virtual void render(void)
			{
#if 0
				::printf("%p = %p\n", this, m_pRenderer);
#endif
				if(m_pRenderer == NULL) return;
				if(m_pRenderer->getSampleCount() == 0) return;
				if(m_pRenderer->getHistoryCount() == 0) return;
				if(m_pRenderer->getHistoryIndex() == 0) return;

				uint32_t l_ui32SampleCount=m_pRenderer->getSampleCount();
				uint32_t l_ui32HistoryIndex=m_pRenderer->getHistoryIndex();

				float l_fProgress=(l_ui32HistoryIndex-(l_ui32HistoryIndex/l_ui32SampleCount)*l_ui32SampleCount)/float(l_ui32SampleCount);
				if(l_fProgress!=0 && l_fProgress!=1)
				{
					this->renderFinal(l_fProgress);
				}
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerProgress_H__
