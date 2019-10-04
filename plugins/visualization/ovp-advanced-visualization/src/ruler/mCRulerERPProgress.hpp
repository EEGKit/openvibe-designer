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

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerERPProgress : public IRuler
		{
		public:

			void render() override

			{
				if (m_pRenderer == nullptr) { return; }
				if (m_pRenderer->getSampleCount() == 0) { return; }
				if (m_pRenderer->getHistoryCount() == 0) { return; }
				if (m_pRenderer->getHistoryIndex() == 0) { return; }

				const float l_fProgress = m_pRendererContext->getERPFraction();
				if (l_fProgress != 0 && l_fProgress != 1)
				{
					glDisable(GL_TEXTURE_1D);

					glLineWidth(4);
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex2f(l_fProgress, 0);
					glVertex2f(l_fProgress, 1);
					glEnd();

					glLineWidth(2);
					glColor3f(0.25, 1, 0.25);
					glBegin(GL_LINES);
					glVertex2f(l_fProgress, 0);
					glVertex2f(l_fProgress, 1);
					glEnd();
				}
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
