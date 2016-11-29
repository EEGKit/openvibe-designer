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

#include "mCRendererLine.hpp"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

CRendererLine::CRendererLine(uint32_t ui32MultiCount)
{
	m_vMuliVertex.resize(ui32MultiCount);
}

void CRendererLine::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	uint32_t i, j, z;

	m_ui32AutoDecimationFactor=1+uint32_t((m_ui32SampleCount-1)/rContext.getMaximumSampleCountPerDisplay());

	for(z=0; z<m_vMuliVertex.size(); z++)
	{
		m_vMuliVertex[z].clear();
		m_vMuliVertex[z].resize(m_ui32ChannelCount);
		for(i=0; i<m_ui32ChannelCount; i++)
		{
			m_vMuliVertex[z][i].resize(m_ui32SampleCount/m_ui32AutoDecimationFactor);
			for(j=0; j<m_ui32SampleCount-m_ui32AutoDecimationFactor+1; j+=m_ui32AutoDecimationFactor)
			{
				m_vMuliVertex[z][i][j/m_ui32AutoDecimationFactor].x=j*m_f32InverseSampleCount;
			}
		}
	}

	m_ui32HistoryIndex=0;
}

void CRendererLine::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if(!m_ui32HistoryCount) return;

	uint32_t i, j, k, l, z, count, l_ui32HistoryIndexMax;
	float sum;

	if( m_ui32HistoryDrawIndex == 0 ) // Draw real-time
	{
		l_ui32HistoryIndexMax = m_ui32HistoryCount; 
	}
	else // stay at the m_ui32HistoryDrawIndex
	{
		l_ui32HistoryIndexMax = m_ui32HistoryDrawIndex;
	}

	for( z = 0; z < m_vMuliVertex.size(); z++ )
	{
		for( i = 0; i < m_ui32ChannelCount; i++ )
		{
			k = ((l_ui32HistoryIndexMax - 1 - z*m_vMuliVertex[z][i].size()) / m_ui32SampleCount)*m_ui32SampleCount;
			std::vector < float >& l_vHistory = m_vHistory[i];
			CVertex* l_pVertex = &m_vMuliVertex[z][i][0];

			for( j = 0; j < m_ui32SampleCount - m_ui32AutoDecimationFactor + 1; j += m_ui32AutoDecimationFactor, k += m_ui32AutoDecimationFactor )
			{
				sum = 0;
				count = 0;

				for( l = 0; l < m_ui32AutoDecimationFactor; l++ )
				{
					if(/*k+l>=m_ui32HistoryIndex && */k + l < l_ui32HistoryIndexMax )
					{
						sum += l_vHistory[k + l];
						count++;
					}
					else if(k+l >= m_ui32SampleCount)
					{
						sum+=l_vHistory[k+l-m_ui32SampleCount];
						count++;
					}
				}

				if(count)
				{
					l_pVertex++->y = sum / count;
				}
				else
				{
					l_pVertex++;
				}
			}
		}
	}

	m_ui32HistoryIndex = l_ui32HistoryIndexMax;
}

bool CRendererLine::render(const IRendererContext& rContext)
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

	::glDisable(GL_TEXTURE_1D);

	::glPushMatrix();
	::glScalef(1, 1.f/rContext.getSelectedCount(), 1);
	::glTranslatef(0, rContext.isPositiveOnly()?0:0.5f, 0);

	::glPushAttrib(GL_CURRENT_BIT);
	::glColor3f(.2f, .2f, .2f);
	::glBegin(GL_LINES);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		::glVertex2f(0, float(i));
		::glVertex2f(1, float(i));
	}
	::glEnd();
	::glPopAttrib();

/*
	::glPushAttrib(GL_CURRENT_BIT);
	::glColor3f(.2f, .2f, .2f);
	::glBegin(GL_LINES);
		::glVertex2f(n1*1.f/n, 0);
		::glVertex2f(n1*1.f/n, 1);
	::glEnd();
	::glPopAttrib();
*/

	::glEnableClientState(GL_VERTEX_ARRAY);
//	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		::glPushMatrix();
		::glTranslatef(0, rContext.getSelectedCount()-i-1.f, 0);
		::glScalef(1, rContext.getScale(), 1);
		for(z=0; z<m_vMuliVertex.size(); z++)
		{
			if(m_vMuliVertex[z].size())
			{
				std::vector < CVertex >& l_rVertex=m_vMuliVertex[z][rContext.getSelected(i)];
				if(rContext.isScrollModeActive())
				{
					::glPushMatrix();
					::glTranslatef(t1, 0, 0);
					::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
//					::glTextureCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].u);
					::glDrawArrays(GL_LINE_STRIP, 0, n1);
					::glPopMatrix();
					if(n2 > 0)
					{
						::glPushMatrix();
						::glTranslatef(t2, 0, 0);
						::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[n1].x);
//						::glTextureCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_rVertex[n1].u);
						::glDrawArrays(GL_LINE_STRIP, 0, n2);
						::glPopMatrix();

						if(n1 > 0)
						{
							::glBegin(GL_LINES);
//								::glTexCoord1fv(&l_rVertex[n2].u);
								::glVertex2f(l_rVertex[n-1].x + t2, l_rVertex[n-1].y);
								::glVertex2f(l_rVertex[0].x   + t1, l_rVertex[0].y);
							::glEnd();
						}
					}
				}
				else
				{
					::glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
//					::glTextureCoordPointer(1, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].u);
					::glDrawArrays(GL_LINE_STRIP, 0, n);
				}
			}
		}
		::glPopMatrix();
	}
//	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);

	::glPopMatrix();

	return true;
}
