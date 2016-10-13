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

#ifndef __OpenViBEPlugins_CRulerLeftTexture_H__
#define __OpenViBEPlugins_CRulerLeftTexture_H__

#include "mCRulerTexture.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerLeftTexture : public CRulerTexture
		{
		public:

			virtual void render(void)
			{
				this->preRender();

				::glColor4f(0, 0, 0, m_fBlackAlpha);
				::glBegin(GL_QUADS);
					::glTexCoord1f(0);
					::glVertex2f(0.00f, 0);
					::glVertex2f(0.05f, 0);
					::glTexCoord1f(1);
					::glVertex2f(0.05f, 1);
					::glVertex2f(0.00f, 1);
				::glEnd();

				::glColor4f(1, 1, 1, m_fWhiteAlpha);
				::glBegin(GL_QUADS);
					::glTexCoord1f(0);
					::glVertex2f(0.00f, 0);
					::glVertex2f(0.04f, 0);
					::glTexCoord1f(1);
					::glVertex2f(0.04f, 1);
					::glVertex2f(0.00f, 1);
				::glEnd();

				this->postRender();
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerLeftTexture_H__
