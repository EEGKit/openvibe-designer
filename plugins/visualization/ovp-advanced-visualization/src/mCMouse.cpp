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

#include "mCMouse.hpp"
#include "mCBoxAlgorithmViz.hpp"

#include <cmath>

namespace OpenViBE {
namespace AdvancedVisualization {

void CMouse::mouseButton(CRendererContext& ctx, const int x, const int y, const int button, const int status)
{
	m_Buttons[button] = status;

	if (m_Buttons[1] == 2)
	{
		ctx.setScaleVisibility(!ctx.getScaleVisibility());
		m_BoxAlgorithmViz.redrawTopLevelWindow();
	}

	m_X = x;
	m_Y = y;
}

void CMouse::mouseMotion(CRendererContext& ctx, const int x, const int y)
{
	if (m_Buttons[3]) { ctx.scaleBy(powf(.99F, float(y - m_Y))); }
	if (m_Buttons[2]) { ctx.zoomBy(powf(.99F, float(y - m_Y))); }
	if (m_Buttons[1])
	{
		ctx.rotateByY(float(x - m_X) * .1F);
		ctx.rotateByX(float(y - m_Y) * .1F);
	}

	m_X = x;
	m_Y = y;
}

bool CMouse::hasButtonPressed()

{
	for (const auto& button : m_Buttons) { if (button.second) { return true; } }
	return false;
}

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
