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

#ifndef __OpenViBEPlugins_CRulerERPProgress_H__
#define __OpenViBEPlugins_CRulerERPProgress_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerERPProgress : public IRuler
		{
		public:

			virtual void render(void)
			{
				if(m_pRenderer == NULL) return;
				if(m_pRenderer->getSampleCount() == 0) return;
				if(m_pRenderer->getHistoryCount() == 0) return;
				if(m_pRenderer->getHistoryIndex() == 0) return;

				float l_fProgress=m_pRendererContext->getERPFraction();
				if(l_fProgress!=0 && l_fProgress!=1)
				{
					::glDisable(GL_TEXTURE_1D);

					::glLineWidth(4);
					::glColor3f(0, 0, 0);
					::glBegin(GL_LINES);
						::glVertex2f(l_fProgress, 0);
						::glVertex2f(l_fProgress, 1);
					::glEnd();

					::glLineWidth(2);
					::glColor3f(0.25, 1, 0.25);
					::glBegin(GL_LINES);
						::glVertex2f(l_fProgress, 0);
						::glVertex2f(l_fProgress, 1);
					::glEnd();
				}
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerERPProgress_H__
