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
