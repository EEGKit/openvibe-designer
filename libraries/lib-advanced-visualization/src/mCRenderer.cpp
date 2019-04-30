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

#include "mCRenderer.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>    // std::min_element, std::max_element

using namespace Mensia;
using namespace AdvancedVisualization;

static int iCount=0;

CRenderer::CRenderer()

	:m_ui32HistoryIndex(0)
	,m_ui32HistoryCount(0)
	,m_ui32ChannelCount(0)
	,m_ui32SampleCount(1)
	,m_ui32HistoryDrawIndex(0)
	,m_f32InverseChannelCount(1)
	,m_f32InverseSampleCount(1)
	,m_ui32AutoDecimationFactor(1)
	,m_f32ERPFraction(0)
	,m_ui32SampleIndexERP(0)
	, m_ui64TimeOffset(0)
{
	iCount++;
}

CRenderer::~CRenderer()

{
	iCount--;
//	::printf("CRenderer::~CRenderer - %i instances left\n", iCount);
}

void CRenderer::setChannelLocalisation(const char* sFilename)
{
	m_sChannelLocalisationFilename=sFilename;
}

void CRenderer::setChannelCount(uint32_t ui32ChannelCount)
{
	m_ui32ChannelCount=ui32ChannelCount;
	m_f32InverseChannelCount=(ui32ChannelCount?1.f/ui32ChannelCount:1);
	m_vVertex.clear();
	m_vMesh.clear();

	m_ui32HistoryIndex=0;
	m_ui32HistoryCount=0;
	m_vHistory.clear();
	m_vHistory.resize(ui32ChannelCount);
}

void CRenderer::setSampleCount(uint32_t ui32SampleCount)
{
	if(ui32SampleCount==0) ui32SampleCount=1;
	m_ui32SampleCount=ui32SampleCount;
	m_f32InverseSampleCount=(ui32SampleCount?1.f/ui32SampleCount:1);
	m_vVertex.clear();
	m_vMesh.clear();
}

void CRenderer::feed(const float* pDataVector)
{
	for(uint32_t i=0; i<m_ui32ChannelCount; i++)
	{
		m_vHistory[i].push_back(pDataVector[i]);
	}
	m_ui32HistoryCount++;
}

void CRenderer::feed(const float* pDataVector, uint32_t ui32SampleCount)
{
	for(uint32_t i=0; i<m_ui32ChannelCount; i++)
	{
		for(uint32_t j=0; j<ui32SampleCount; j++)
		{
			m_vHistory[i].push_back(pDataVector[j]);
		}
		pDataVector+=ui32SampleCount;
	}
	m_ui32HistoryCount+=ui32SampleCount;
}

void CRenderer::feed(uint64_t ui64StimulationDate, uint64_t ui64StimulationId)
{
	m_vStimulationHistory.push_back(std::make_pair((ui64StimulationDate>>16)/65536., ui64StimulationId));
}

void CRenderer::prefeed(uint32_t ui32PreFeedSampleCount)
{
	for(uint32_t i=0; i<m_ui32ChannelCount; i++)
	{
		m_vHistory[i].insert(m_vHistory[i].begin(), ui32PreFeedSampleCount, 0.f);
	}
	m_ui32HistoryCount+=ui32PreFeedSampleCount;
	m_ui32HistoryIndex=0;
}

float CRenderer::getSuggestedScale()
{
	if (m_ui32ChannelCount != 0)
	{
		std::vector<float> l_vf32Average;

		for (uint32_t i = 0; i < m_ui32ChannelCount; i++)
		{
			l_vf32Average.push_back(0);

			unsigned int l_ui32SamplesToAverage = (m_vHistory[i].size() < m_ui32SampleCount) ? m_vHistory[i].size() : m_ui32SampleCount;

			for (uint32_t j = m_vHistory[i].size(); j > (m_vHistory[i].size() - l_ui32SamplesToAverage) ; j--)
			{
				l_vf32Average.back() += m_vHistory[i][j - 1];
			}

			l_vf32Average.back() /= l_ui32SamplesToAverage;
		}

		return (1 / *std::max_element(l_vf32Average.begin(), l_vf32Average.end()));
	}
	return 0;
}

void CRenderer::clear(uint32_t ui32SampleCountToKeep = 0)
{
	if(!m_vHistory.empty())
	{
		if( ui32SampleCountToKeep == 0 )
		{
			for (std::vector<float>& vec : m_vHistory)
			{
				vec.clear();
			}
			
			m_ui32HistoryCount = 0;
		}  
		else if( ui32SampleCountToKeep < m_vHistory[0].size() )
		{
			uint32_t l_ui32SampleToDelete = m_vHistory[0].size() - ui32SampleCountToKeep;

			if( l_ui32SampleToDelete > 1 )
			{
				for( uint32_t i = 0; i < m_vHistory.size(); i++ )
				{
					std::vector<float>(m_vHistory[i].begin() + l_ui32SampleToDelete, m_vHistory[i].end()).swap(m_vHistory[i]);
				}

				m_ui32HistoryCount -= l_ui32SampleToDelete;
			}
		}
	}
	// We always delete all of the stimulations, ideally we would know the time
	// scale so we can keep the stimulations according to the kept samples
	m_vStimulationHistory.clear();
	m_ui32HistoryIndex=0;
}

uint32_t CRenderer::getChannelCount() const
{
	return m_ui32ChannelCount;
}

uint32_t CRenderer::getSampleCount() const
{
	return m_ui32SampleCount;
}

uint32_t CRenderer::getHistoryCount() const
{
	return m_ui32HistoryCount;
}

uint32_t CRenderer::getHistoryIndex() const
{
	return m_ui32HistoryIndex;
}

void CRenderer::setHistoryDrawIndex(uint32_t ui32Index)
{
	m_ui32HistoryDrawIndex = ui32Index;
	m_ui32HistoryIndex = 0;
}

bool CRenderer::getSampleAtERPFraction(float fERPFraction, std::vector < float >& vSample) const
{
	vSample.resize(m_ui32ChannelCount);

	if (m_ui32SampleCount > m_ui32HistoryCount) return false;

	float l_f32SampleIndexERP=(fERPFraction*(m_ui32SampleCount-1));
	float l_f32Alpha=l_f32SampleIndexERP-std::floor(l_f32SampleIndexERP);
	uint32_t l_ui32SampleIndexERP1=uint32_t(l_f32SampleIndexERP  )%m_ui32SampleCount;
	uint32_t l_ui32SampleIndexERP2=uint32_t(l_f32SampleIndexERP+1)%m_ui32SampleCount;

	for(uint32_t i=0; i<m_ui32ChannelCount; i++)
	{
		vSample[i]=m_vHistory[i][m_ui32HistoryCount-m_ui32SampleCount+l_ui32SampleIndexERP1]*(1-l_f32Alpha)
		          +m_vHistory[i][m_ui32HistoryCount-m_ui32SampleCount+l_ui32SampleIndexERP2]*(  l_f32Alpha);
	}

	return true;
}

void CRenderer::rebuild(const IRendererContext& rContext) { }

void CRenderer::refresh(const IRendererContext& rContext)
{
	if(!m_ui32SampleCount)
	{
		m_f32ERPFraction=0;
		m_ui32SampleIndexERP=0;
		return;
	}

	m_f32ERPFraction=rContext.getERPFraction();
	m_ui32SampleIndexERP=uint32_t(m_f32ERPFraction*(m_ui32SampleCount-1))%m_ui32SampleCount;
}

#if 0
bool CRenderer::render(const IRendererContext& rContext)
{
	::glLineWidth(7);
	::glColor3f(1.f, 0.9f, 0.1f);
	::glDisable(GL_TEXTURE_1D);
	::glBegin(GL_LINES);
		::glVertex2f(0, 0);
		::glVertex2f(1, 1);
		::glVertex2f(0, 1);
		::glVertex2f(1, 0);
	::glEnd();
	::glBegin(GL_LINE_LOOP);
		::glVertex2f(0, 0);
		::glVertex2f(0, 1);
		::glVertex2f(1, 1);
		::glVertex2f(1, 0);
	::glEnd();
}
#endif

void CRenderer::draw3DCoordinateSystem()

{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_1D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2);

	glPushMatrix();
	glColor3f(.2f, .2f, .2f);
	glScalef(.2f, .2f, .2f);
	glBegin(GL_LINES);
	for(int x=-10; x<=10; x++)
	{
		for(int z=-10; z<=10; z++)
		{
			if(x!=0)
			{
				glVertex3f(float(x), 0,  10.f);
				glVertex3f(float(x), 0, -10.f);
			}
			if(z!=0)
			{
				glVertex3f( 10.f, 0, float(z));
				glVertex3f(-10.f, 0, float(z));
			}
		}
	}
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0,  2.f);
		glVertex3f(0, 0, -3.f);
		glColor3f(0, 1, 0);
		glVertex3f(0,  1.25f, 0);
		glVertex3f(0, -1.25f, 0);
		glColor3f(1, 0, 0);
		glVertex3f( 2.f, 0, 0);
		glVertex3f(-2.f, 0, 0);
	glEnd();

	glPopAttrib();
}

void CRenderer::draw2DCoordinateSystem()

{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_1D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2);

	glPushMatrix();
	glColor3f(.2f, .2f, .2f);
	glScalef(.2f, .2f, .2f);
	glBegin(GL_LINES);
	for (int x = -10; x <= 10; x++)
	{
		for (int y = -10; y <= 10; y++)
		{
			if (x != 0)
			{
				glVertex2f(float(x), 10.f);
				glVertex2f(float(x), -10.f);
			}
			if (y != 0)
			{
				glVertex2f(10.f, float(y));
				glVertex2f(-10.f, float(y));
			}
		}
	}
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(0, 1, 0);
	glVertex2f(0, 2.0f);
	glVertex2f(0, -2.0f);
	glColor3f(1, 0, 0);
	glVertex2f(2.f, 0);
	glVertex2f(-2.f, 0);
	glEnd();

	glPopAttrib();
}
