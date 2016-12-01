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

#define _USE_MATH_DEFINES
#include <cmath>

#include "mCRendererFlower.hpp"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

CRendererFlower::CRendererFlower(uint32_t ui32MultiCount)
{
	m_vMuliVertex.resize(ui32MultiCount);
}

void CRendererFlower::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	uint32_t i, j, z;

	m_ui32AutoDecimationFactor=1+uint32_t((m_ui32SampleCount-1)/rContext.getMaximumSampleCountPerDisplay());

	uint32_t n = m_ui32SampleCount/m_ui32AutoDecimationFactor;

	for(z=0; z<m_vMuliVertex.size(); z++)
	{
		m_vMuliVertex[z].clear();
		m_vMuliVertex[z].resize(m_ui32ChannelCount);
		for(i=0; i<m_ui32ChannelCount; i++)
		{
			m_vMuliVertex[z][i].resize(n);
			for(j=0; j<m_ui32SampleCount-m_ui32AutoDecimationFactor+1; j+=m_ui32AutoDecimationFactor)
			{
				m_vMuliVertex[z][i][j/m_ui32AutoDecimationFactor].u=j*m_f32InverseSampleCount;
			}
		}
	}

	m_vCircle.clear();
	m_vCircle.resize(n);
	for(i=0; i<n; i++)
	{
		m_vCircle[i].x=::cosf(rContext.getFlowerRingCount()*i*static_cast<float>(M_PI)*2.f/n);
		m_vCircle[i].y=::sinf(rContext.getFlowerRingCount()*i*static_cast<float>(M_PI)*2.f/n);
		m_vCircle[i].z=0;
	}

	m_ui32HistoryIndex=0;
}

void CRendererFlower::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if(!m_ui32HistoryCount) return;

	uint32_t i, j, k, l, z, count;
	float sum;

	for(z=0; z<m_vMuliVertex.size(); z++)
	{
		for(i=0; i<m_ui32ChannelCount; i++)
		{
			k=((m_ui32HistoryCount-1-z*m_vMuliVertex[z][i].size())/m_ui32SampleCount)*m_ui32SampleCount;
			std::vector < float >& l_vHistory=m_vHistory[i];
			CVertex* l_pVertex=&m_vMuliVertex[z][i][0];
			CVertex* l_pCircleVertex=&m_vCircle[0];
			for(j=0; j<m_ui32SampleCount-m_ui32AutoDecimationFactor+1; j+=m_ui32AutoDecimationFactor, k+=m_ui32AutoDecimationFactor)
			{
				sum=0;
				count=0;

				for(l=0; l<m_ui32AutoDecimationFactor; l++)
				{
					if(/*k+l>=m_ui32HistoryIndex && */k+l<m_ui32HistoryCount)
					{
						sum+=l_vHistory[k+l];
						count++;
					}
				}

				if(count)
				{
					float v=sum/count;
					l_pVertex->x=l_pCircleVertex->x*v;
					l_pVertex->y=l_pCircleVertex->y*v;
					l_pVertex->z=l_pCircleVertex->z*v;
					l_pVertex++;
					l_pCircleVertex++;
				}
				else
				{
					l_pVertex++;
					l_pCircleVertex++;
				}
			}
		}
	}

	m_ui32HistoryIndex=m_ui32HistoryCount;
}

bool CRendererFlower::render(const IRendererContext& rContext)
{
	if(!rContext.getSelectedCount()) return false;
	if(!m_vMuliVertex.size()) return false;
	if(!m_ui32HistoryCount) return false;

	uint32_t i, z;
	uint32_t n = m_ui32SampleCount/m_ui32AutoDecimationFactor;
	uint32_t d = (m_ui32HistoryIndex%m_ui32SampleCount)/m_ui32AutoDecimationFactor;

	::glMatrixMode(GL_TEXTURE);
	::glPushMatrix();
	::glLoadIdentity();

	::glMatrixMode(GL_MODELVIEW);

	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		::glPushMatrix();
		::glTranslatef(.5f, .5f, 0);
		::glScalef(rContext.getScale(), rContext.getScale(), rContext.getScale());
		for(z=0; z<m_vMuliVertex.size(); z++)
		{
			if(m_vMuliVertex[z].size())
			{
				std::vector < CVertex >& l_vVertex=m_vMuliVertex[z][rContext.getSelected(i)];

				::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_vVertex[0].x);
				::glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_vVertex[n-d].u);
				::glDrawArrays(GL_LINE_STRIP, 0, d);

				::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_vVertex[d].x);
				::glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_vVertex[0].u);
				::glDrawArrays(GL_LINE_STRIP, 0, n-d);

				::glBegin(GL_LINES);
					::glTexCoord1fv(&l_vVertex[n-d].u);
					::glVertex2fv(&l_vVertex[n-1].x);
					::glVertex2fv(&l_vVertex[0].x);
				::glEnd();
			}
		}
		::glPopMatrix();
	}
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);

	::glMatrixMode(GL_TEXTURE);
	::glPopMatrix();

	::glMatrixMode(GL_MODELVIEW);

	return true;
}
