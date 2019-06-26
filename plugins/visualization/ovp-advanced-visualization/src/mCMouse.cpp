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


using namespace Mensia;
using namespace AdvancedVisualization;

CMouse::CMouse(CBoxAlgorithmViz& rBoxAlgorithmViz) : m_rBoxAlgorithmViz(rBoxAlgorithmViz) { }

void CMouse::mouseButton(IRendererContext& rContext, const int32_t x, const int32_t y, const int32_t button, const int status)
{
	m_vButton[button] = status;

	if (m_vButton[1] == 2)
	{
		rContext.setScaleVisibility(!rContext.getScaleVisibility());
		m_rBoxAlgorithmViz.redrawTopLevelWindow();
	}

	m_mouseX = x;
	m_mouseY = y;
}

void CMouse::mouseMotion(IRendererContext& rContext, const int32_t x, const int32_t y)
{
	if (m_vButton[3])
	{
		rContext.scaleBy(powf(.99f, float(y - m_mouseY)));
	}
	if (m_vButton[2])
	{
		rContext.zoomBy(powf(.99f, float(y - m_mouseY)));
	}
	if (m_vButton[1])
	{
		rContext.rotateByY(float(x - m_mouseX) * .1f);
		rContext.rotateByX(float(y - m_mouseY) * .1f);
	}

	m_mouseX = x;
	m_mouseY = y;
}

bool CMouse::hasButtonPressed()

{
	for (std::map<int32_t, int>::const_iterator it = m_vButton.begin(); it != m_vButton.end(); ++it)
	{
		if (it->second) { return true; }
	}
	return false;
}
