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

			virtual void preRender()

			{
				glDisable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_TEXTURE_1D);
				glColor3f(1, 1, 1);

				glMatrixMode(GL_TEXTURE);
				glPushMatrix();
				glLoadIdentity();

				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
			}

			virtual void postRender()

			{
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();

				glMatrixMode(GL_TEXTURE);
				glPopMatrix();

				glMatrixMode(GL_MODELVIEW);
			}
		};
	};
};

#endif // __OpenViBEPlugins_CRulerTexture_H__
