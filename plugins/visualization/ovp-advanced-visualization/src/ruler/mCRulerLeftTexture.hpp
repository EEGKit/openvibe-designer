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

			virtual void render()

			{
				this->preRender();

				glColor4f(0, 0, 0, m_fBlackAlpha);
				glBegin(GL_QUADS);
				glTexCoord1f(0);
				glVertex2f(0.00f, 0);
				glVertex2f(0.05f, 0);
				glTexCoord1f(1);
				glVertex2f(0.05f, 1);
				glVertex2f(0.00f, 1);
				glEnd();

				glColor4f(1, 1, 1, m_fWhiteAlpha);
				glBegin(GL_QUADS);
				glTexCoord1f(0);
				glVertex2f(0.00f, 0);
				glVertex2f(0.04f, 0);
				glTexCoord1f(1);
				glVertex2f(0.04f, 1);
				glVertex2f(0.00f, 1);
				glEnd();

				this->postRender();
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerLeftTexture_H__
