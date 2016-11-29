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

#include "mCRendererMultiLine.hpp"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

bool CRendererMultiLine::render(const IRendererContext& rContext)
{
	if(!rContext.getSelectedCount()) return false;
	if(!m_vMuliVertex.size()) return false;
	if(!m_ui32HistoryCount) return false;

	uint32_t i, z;
	int32_t n  = (int32_t)(m_ui32SampleCount/m_ui32AutoDecimationFactor);
	int32_t n1 = (int32_t)(((m_ui32HistoryIndex % m_ui32SampleCount) * n)/m_ui32SampleCount);
	int32_t n2 = (int32_t)(n - n1);

	if(!n) return false;

	float t1 =  n2 * 1.f / n;
	float t2 = -n1 * 1.f / n;

	::glMatrixMode(GL_TEXTURE);
	::glPushMatrix();
	::glLoadIdentity();

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glTranslatef(0, rContext.isPositiveOnly()?0:0.5f, 0);
	::glScalef(1, rContext.getScale(), 1);
	::glEnableClientState(GL_VERTEX_ARRAY);
	for(z=0; z<m_vMuliVertex.size(); z++)
	{
		for(i=0; i<rContext.getSelectedCount(); i++)
		{
			std::vector < CVertex >& l_rVertex=m_vMuliVertex[z][rContext.getSelected(i)];
			::glTexCoord1f(1-(i+.5f)/rContext.getSelectedCount());
			if(rContext.isScrollModeActive())
			{
				::glPushMatrix();
				::glTranslatef(t1, 0, 0);
				::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
				::glDrawArrays(GL_LINE_STRIP, 0, n1);
				::glPopMatrix();
				if(n2 > 0)
				{
					::glPushMatrix();
					::glTranslatef(t2, 0, 0);
					::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[n1].x);
					::glDrawArrays(GL_LINE_STRIP, 0, n2);
					::glPopMatrix();

					if(n1 > 0)
					{
						::glBegin(GL_LINES);
							::glVertex2f(l_rVertex[n-1].x + t2, l_rVertex[n-1].y);
							::glVertex2f(l_rVertex[0].x   + t1, l_rVertex[0].y);
						::glEnd();
					}
				}
			}
			else
			{
				::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
				::glDrawArrays(GL_LINE_STRIP, 0, n);
			}
		}
	}
	::glDisableClientState(GL_VERTEX_ARRAY);
	::glPopMatrix();

	::glMatrixMode(GL_TEXTURE);
	::glPopMatrix();

	::glMatrixMode(GL_MODELVIEW);

	::glDisable(GL_TEXTURE_1D);
	::glColor3f(.2f, .2f, .2f);
	::glBegin(GL_LINES);
		::glVertex2f(0, rContext.isPositiveOnly()?0:0.5f);
		::glVertex2f(1, rContext.isPositiveOnly()?0:0.5f);
	::glEnd();

	return true;
}
