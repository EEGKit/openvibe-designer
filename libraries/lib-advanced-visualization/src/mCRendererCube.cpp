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

#include "mCRendererCube.hpp"
#include "m_RendererTools.hpp"

#include <cmath>
#include <algorithm>

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

CRendererCube::CRendererCube(void)
{
}

void CRendererCube::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	m_vVertex.clear();
	m_vVertex.resize(rContext.getChannelCount());

	m_ui32HistoryIndex=0;
}

void CRendererCube::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if(!m_ui32HistoryCount) return;

	float l_f32SampleIndexERP=(m_f32ERPFraction*(m_ui32SampleCount-1));
	float l_f32Alpha=l_f32SampleIndexERP-std::floor(l_f32SampleIndexERP);
	uint32_t l_ui32SampleIndexERP1=uint32_t(l_f32SampleIndexERP  )%m_ui32SampleCount;
	uint32_t l_ui32SampleIndexERP2=uint32_t(l_f32SampleIndexERP+1)%m_ui32SampleCount;

	for(uint32_t i=0; i<m_vVertex.size(); i++)
	{
		m_vVertex[i].u=m_vHistory[i][m_ui32HistoryCount-m_ui32SampleCount+l_ui32SampleIndexERP1]*(1-l_f32Alpha)
		              +m_vHistory[i][m_ui32HistoryCount-m_ui32SampleCount+l_ui32SampleIndexERP2]*(  l_f32Alpha);
	}

	m_ui32HistoryIndex=m_ui32HistoryCount;
}

bool CRendererCube::render(const IRendererContext& rContext)
{
	std::map < std::string, CVertex >::const_iterator it;

	if(!rContext.getSelectedCount()) return false;
	if(!m_vVertex.size()) return false;
	if(!m_ui32HistoryCount) return false;

	uint32_t j, k;
	float d=3.5;

	::glEnable(GL_DEPTH_TEST);
	::glDisable(GL_BLEND);

	::glMatrixMode(GL_PROJECTION);
	::glPushMatrix();
	::glLoadIdentity();
	::gluPerspective(60, rContext.getAspect(), .01, 100);
	::glTranslatef(0, 0, -d);
	::glRotatef(rContext.getRotationX()*10, 1, 0, 0);
	::glRotatef(rContext.getRotationY()*10, 0, 1, 0);

	::glMatrixMode(GL_TEXTURE);
	::glPushMatrix();
	::glScalef(rContext.getScale(), 1, 1);

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadIdentity();
	::glScalef(rContext.getZoom(), rContext.getZoom(), rContext.getZoom());

	::glPushMatrix();
	::glRotatef(19, 1, 0, 0);
	for(j=0; j<rContext.getSelectedCount(); j++)
	{
		CVertex v;
		k=rContext.getSelected(j);
		rContext.getChannelLocalisation(k, v.x, v.y, v.z);
/*
		std::string l_sName=rContext.getChannelName(k);
		std::transform(l_sName.begin(), l_sName.end(), l_sName.begin(), ::tolower);

		it=m_vChannelLocalisation.find(l_sName);
		if(it!=m_vChannelLocalisation.end())
		{
*/
			float l_fCubeScale=.1f*(.25f+::fabs(m_vVertex[k].u*rContext.getScale()));

			::glPushMatrix();
//			::glTranslatef(it->second.x, it->second.y, it->second.z);
			::glTranslatef(v.x, v.y, v.z);
			::glTexCoord1f(m_vVertex[k].u);
			::glScalef(l_fCubeScale, l_fCubeScale, l_fCubeScale);

::glColor3f(1, 1, 1);
::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			cube();

::glColor3f(0, 0, 0);
::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			cube();

			::glPopMatrix();
//		}
	}
	::glPopMatrix();

::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(rContext.getCheckBoardVisibility()) this->drawCoordinateSystem();

	::glMatrixMode(GL_MODELVIEW);
	::glPopMatrix();

	::glMatrixMode(GL_TEXTURE);
	::glPopMatrix();

	::glMatrixMode(GL_PROJECTION);
	::glPopMatrix();

	::glMatrixMode(GL_MODELVIEW);

	return true;
}
