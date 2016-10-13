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

#ifndef __OpenViBEPlugins_CRulerTexture_H__
#define __OpenViBEPlugins_CRulerTexture_H__

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerTexture : public IRuler
		{
		protected:

			virtual void preRender(void)
			{
				::glDisable(GL_BLEND);
				::glDisable(GL_DEPTH_TEST);
				::glEnable(GL_TEXTURE_1D);
				::glColor3f(1, 1, 1);

				::glMatrixMode(GL_TEXTURE);
				::glPushMatrix();
				::glLoadIdentity();

				::glMatrixMode(GL_MODELVIEW);
				::glPushMatrix();
				::glLoadIdentity();
			}

			virtual void postRender(void)
			{
				::glMatrixMode(GL_MODELVIEW);
				::glPopMatrix();

				::glMatrixMode(GL_TEXTURE);
				::glPopMatrix();

				::glMatrixMode(GL_MODELVIEW);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerTexture_H__
