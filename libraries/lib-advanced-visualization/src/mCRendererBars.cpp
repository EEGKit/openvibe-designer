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

#include "mCRendererBars.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererBars::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	uint32_t i, j;

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

	uint32_t i, j, k;

	for(i=0; i<m_ui32ChannelCount; i++)
	{
		k=((m_ui32HistoryCount-1)/m_ui32SampleCount)*m_ui32SampleCount;
		std::vector < float >& l_vHistory=m_vHistory[i];
		CVertex* l_pVertex=&m_vVertex[i][0];
		for(j=0; j<m_ui32SampleCount; j++, k++)
		{
			if(k>=m_ui32HistoryIndex && k<m_ui32HistoryCount)
			{
				float l_f32Value=l_vHistory[k];
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

bool CRendererBars::render(const IRendererContext& rContext)
{
	if(!rContext.getSelectedCount()) return false;
	if(m_vVertex.empty()) return false;
	if(!m_ui32HistoryCount) return false;

	uint32_t i;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(1, 1.f/rContext.getSelectedCount(), 1);
	glTranslatef(0, rContext.isPositiveOnly()?0:0.5f, 0);

	glPushAttrib(GL_CURRENT_BIT);
	glDisable(GL_TEXTURE_1D);
	glColor3f(.2f, .2f, .2f);
	glBegin(GL_LINES);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		glVertex2f(0, float(i));
		glVertex2f(1, float(i));
	}
	glEnd();
	glEnable(GL_TEXTURE_1D);
	glPopAttrib();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for(i=0; i<rContext.getSelectedCount(); i++)
	{
		glPushMatrix();
		glTranslatef(0, rContext.getSelectedCount()-i-1.f, 0);
		glScalef(1, rContext.getScale(), 1);
		glVertexPointer(2, GL_FLOAT, sizeof(CVertex), &m_vVertex[rContext.getSelected(i)][0].x);
		glTexCoordPointer(1, GL_FLOAT, sizeof(CVertex), &m_vVertex[rContext.getSelected(i)][0].u);
		glDrawArrays(GL_QUADS, 0, m_ui32SampleCount*4);
		glPopMatrix();
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}
