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

namespace OpenViBE {
namespace AdvancedVisualization {

class CRulerERPProgress : public IRuler
{
public:

	void render() override

	{
		if (m_renderer == nullptr) { return; }
		if (m_renderer->getSampleCount() == 0) { return; }
		if (m_renderer->getHistoryCount() == 0) { return; }
		if (m_renderer->getHistoryIndex() == 0) { return; }

		const float progress = m_rendererCtx->getERPFraction();
		if (progress != 0 && progress != 1)
		{
			glDisable(GL_TEXTURE_1D);

			glLineWidth(4);
			glColor3f(0, 0, 0);
			glBegin(GL_LINES);
			glVertex2f(progress, 0);
			glVertex2f(progress, 1);
			glEnd();

			glLineWidth(2);
			glColor3f(0.25, 1, 0.25);
			glBegin(GL_LINES);
			glVertex2f(progress, 0);
			glVertex2f(progress, 1);
			glEnd();
		}
	}
};

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
