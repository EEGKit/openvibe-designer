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

#include "mCMouse.hpp"
#include "mCBoxAlgorithmViz.hpp"

#include <cmath>

#define bool bool

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

CMouse::CMouse(CBoxAlgorithmViz& rBoxAlgorithmViz)
	:m_rBoxAlgorithmViz(rBoxAlgorithmViz)
	,m_i32MouseX(0)
	,m_i32MouseY(0)
{
}

void CMouse::mouseButton(IRendererContext& rContext, int32_t x, int32_t y, int32_t button, int status)
{
	m_vButton[button]=status;

	if(m_vButton[1]==2)
	{
		rContext.setScaleVisibility(!rContext.getScaleVisibility());
		m_rBoxAlgorithmViz.redrawTopLevelWindow();
	}

	m_i32MouseX=x;
	m_i32MouseY=y;
}

void CMouse::mouseMotion(IRendererContext& rContext, int32_t x, int32_t y)
{
	if(m_vButton[3])
	{
		rContext.scaleBy(::powf(.99f, float(y-m_i32MouseY)));
	}
	if(m_vButton[2])
	{
		rContext.zoomBy(::powf(.99f, float(y-m_i32MouseY)));
	}
	if(m_vButton[1])
	{
		rContext.rotateByY((x-m_i32MouseX)*.1f);
		rContext.rotateByX((y-m_i32MouseY)*.1f);
	}

	m_i32MouseX=x;
	m_i32MouseY=y;
}

bool CMouse::hasButtonPressed(void)
{
	std::map < int32_t, int >::const_iterator it;
	for(it=m_vButton.begin(); it!=m_vButton.end(); it++)
	{
		if(it->second) return true;
	}
	return false;
}
