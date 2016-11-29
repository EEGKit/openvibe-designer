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
