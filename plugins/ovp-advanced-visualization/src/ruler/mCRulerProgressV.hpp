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

#ifndef __OpenViBEPlugins_CRulerProgressV_H__
#define __OpenViBEPlugins_CRulerProgressV_H__

#include "mCRulerProgress.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerProgressV : public CRulerProgress
		{
		public:

			virtual void renderFinal(float fProgress)
			{
				::glDisable(GL_TEXTURE_1D);
				::glDisable(GL_BLEND);

				::glLineWidth(4);
				::glColor3f(0, 0, 0);
				::glBegin(GL_LINES);
					::glVertex2f(fProgress, 0);
					::glVertex2f(fProgress, 1);
				::glEnd();

				::glLineWidth(2);
				::glColor3f(0.25, 1, 0.25);
				::glBegin(GL_LINES);
					::glVertex2f(fProgress, 0);
					::glVertex2f(fProgress, 1);
				::glEnd();
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerProgressV_H__
