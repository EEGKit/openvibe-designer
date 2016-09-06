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

#include "mCRendererBars.hpp"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;
#define boolean Mensia::boolean

void CRendererBars::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	uint32 i, j;

	m_vVertex.resize(m_ui32ChannelCount);
	for(i=0; i<m_ui32ChannelCount; i++)
	{
		m_vVertex[i].resize(m_ui32SampleCount*4);
		for(j=0; j<m_ui32SampleCount; j++)
		{
			m_vVertex[i][j*4  ].x=(j  )*m_f32InverseSampleCount;
			m_vVertex[i][j*4+1].x=(j+1)*m_f32InverseSampleCount;
			m_vVertex[i][j*4+2].x=(j+1)*m_f32InverseSampleCount;
			m_vVertex[i][j*4+3].x=(j  )*m_f32InverseSampleCount;

			m_vVertex[i][j*4  ].u=(j  )*m_f32InverseSampleCount;
			m_vVertex[i][j*4+1].u=(j  )*m_f32InverseSampleCount;
			m_vVertex[i][j*4+2].u=(j  )*m_f32InverseSampleCount;
			m_vVertex[i][j*4+3].u=(j  )*m_f32InverseSampleCount;
		}
	}

	m_ui32HistoryIndex=0;
}

void CRendererBars::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if(!m_ui32HistoryCount) return;

	uint32 i, j, k;

	for(i=0; i<m_ui32ChannelCount; i++)
	{
		k=((m_ui32HistoryCount-1)/m_ui32SampleCount)*m_ui32SampleCount;
		std::vector < float32 >& l_vHistory=m_vHistory[i];
		CVertex* l_pVertex=&m_vVertex[i][0];
		for(j=0; j<m_ui32SampleCount; j++, k++)
		{
			if(k>=m_ui32HistoryIndex && k<m_ui32HistoryCount)
			{
				float32 l_f32Value=l_vHistory[k];
				l_pVertex++->y=0;
				l_pVertex++->y=0;
				l_pVertex++->y=l_f32Value;
				l_pVertex++->y=l_f32Value;
			}
			else
			{
				l_pVertex+=4;
			}
		}
	}
	m_ui32HistoryIndex=m_ui32HistoryCount;
}

boolean CRendererBars::render(const IRendererContext& rContext)
{
	if(!rContext.getSelectedCount()) return false;
	if(!m_vVertex.size()) return false;
	if(!m_ui32HistoryCount) return false;

	uint32 i;

	::glMatrixMode(GL_TEXTURE);
	::glPushMatrix();
	::glLoadIdentity();

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glScalef(1, 1.f/rContext.getSelectedCount(), 1);
	::glTranslatef(0, rContext.isPositiveOnly()?0:0.5f, 0);

	::glPushAttrib(GL_CURRENT_BIT);
	::glDisable(GL_TEXTURE_1D);
	::glColor3f(.2f, .2f, .2f);
	::glBegin(GL_LINES);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		::glVertex2f(0, float(i));
		::glVertex2f(1, float(i));
	}
	::glEnd();
	::glEnable(GL_TEXTURE_1D);
	::glPopAttrib();

	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		::glPushMatrix();
		::glTranslatef(0, rContext.getSelectedCount()-i-1.f, 0);
		::glScalef(1, rContext.getScale(), 1);
		::glVertexPointer(2, GL_FLOAT, sizeof(CVertex), &m_vVertex[rContext.getSelected(i)][0].x);
		::glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vVertex[rContext.getSelected(i)][0].u);
		::glDrawArrays(GL_QUADS, 0, m_ui32SampleCount*4);
		::glPopMatrix();
	}
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);
	::glPopMatrix();

	::glMatrixMode(GL_TEXTURE);
	::glPopMatrix();

	::glMatrixMode(GL_MODELVIEW);

	return true;
}
