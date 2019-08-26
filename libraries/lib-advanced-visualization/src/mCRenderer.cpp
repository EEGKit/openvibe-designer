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

static int iCount = 0;

CRenderer::CRenderer() { iCount++; }

CRenderer::~CRenderer()

{
	iCount--;
	//	::printf("CRenderer::~CRenderer - %i instances left\n", iCount);
}

void CRenderer::setChannelLocalisation(const char* sFilename)
{
	m_channelLocalisationFilename = sFilename;
}

void CRenderer::setChannelCount(const uint32_t channelCount)
{
	m_channelCount        = channelCount;
	m_inverseChannelCount = (channelCount ? 1.f / channelCount : 1);
	m_vertex.clear();
	m_mesh.clear();

	m_historyIndex = 0;
	m_historyCount = 0;
	m_history.clear();
	m_history.resize(channelCount);
}

void CRenderer::setSampleCount(const uint32_t sampleCount)
{
	m_sampleCount        = sampleCount == 0 ? 1 : sampleCount;
	m_inverseSampleCount = (m_sampleCount ? 1.f / m_sampleCount : 1);
	m_vertex.clear();
	m_mesh.clear();
}

void CRenderer::feed(const float* pDataVector)
{
	for (uint32_t i = 0; i < m_channelCount; ++i) { m_history[i].push_back(pDataVector[i]); }
	m_historyCount++;
}

void CRenderer::feed(const float* pDataVector, const uint32_t sampleCount)
{
	for (uint32_t i = 0; i < m_channelCount; ++i)
	{
		for (uint32_t j = 0; j < sampleCount; j++) { m_history[i].push_back(pDataVector[j]); }
		pDataVector += sampleCount;
	}
	m_historyCount += sampleCount;
}

void CRenderer::feed(const uint64_t stimulationDate, const uint64_t stimulationId)
{
	m_stimulationHistory.emplace_back((stimulationDate >> 16) / 65536., stimulationId);
}

void CRenderer::prefeed(const uint32_t preFeedSampleCount)
{
	for (uint32_t i = 0; i < m_channelCount; ++i)
	{
		m_history[i].insert(m_history[i].begin(), preFeedSampleCount, 0.f);
	}
	m_historyCount += preFeedSampleCount;
	m_historyIndex = 0;
}

float CRenderer::getSuggestedScale()
{
	if (m_channelCount != 0)
	{
		std::vector<float> l_vf32Average;

		for (size_t i = 0; i < m_channelCount; ++i)
		{
			l_vf32Average.push_back(0);

			const size_t samplesToAverage = (m_history[i].size() < m_sampleCount) ? m_history[i].size() : m_sampleCount;

			for (size_t j = m_history[i].size(); j > (m_history[i].size() - samplesToAverage); --j)
			{
				l_vf32Average.back() += m_history[i][j - 1];
			}

			l_vf32Average.back() /= float(samplesToAverage);
		}

		return (1 / *std::max_element(l_vf32Average.begin(), l_vf32Average.end()));
	}
	return 0;
}

void CRenderer::clear(const uint32_t sampleCountToKeep = 0)
{
	if (!m_history.empty())
	{
		if (sampleCountToKeep == 0)
		{
			for (auto& vec : m_history) { vec.clear(); }
			m_historyCount = 0;
		}
		else if (sampleCountToKeep < m_history[0].size())
		{
			const size_t sampleToDelete = m_history[0].size() - sampleCountToKeep;

			if (sampleToDelete > 1)
			{
				for (auto& vec : m_history) { std::vector<float>(vec.begin() + sampleToDelete, vec.end()).swap(vec); }
				m_historyCount -= uint32_t(sampleToDelete);
			}
		}
	}
	// We always delete all of the stimulations, ideally we would know the time
	// scale so we can keep the stimulations according to the kept samples
	m_stimulationHistory.clear();
	m_historyIndex = 0;
}

uint32_t CRenderer::getChannelCount() const { return m_channelCount; }

uint32_t CRenderer::getSampleCount() const { return m_sampleCount; }

uint32_t CRenderer::getHistoryCount() const { return m_historyCount; }

uint32_t CRenderer::getHistoryIndex() const { return m_historyIndex; }

void CRenderer::setHistoryDrawIndex(const uint32_t index)
{
	m_historyDrawIndex = index;
	m_historyIndex     = 0;
}

bool CRenderer::getSampleAtERPFraction(const float fERPFraction, std::vector<float>& vSample) const
{
	vSample.resize(m_channelCount);

	if (m_sampleCount > m_historyCount) { return false; }

	const float sampleIndexERP     = (fERPFraction * float(m_sampleCount - 1));
	const float alpha              = sampleIndexERP - std::floor(sampleIndexERP);
	const uint32_t sampleIndexERP1 = uint32_t(sampleIndexERP) % m_sampleCount;
	const uint32_t sampleIndexERP2 = uint32_t(sampleIndexERP + 1) % m_sampleCount;

	for (uint32_t i = 0; i < m_channelCount; ++i)
	{
		vSample[i] = m_history[i][m_historyCount - m_sampleCount + sampleIndexERP1] * (1 - alpha)
					 + m_history[i][m_historyCount - m_sampleCount + sampleIndexERP2] * (alpha);
	}

	return true;
}

void CRenderer::rebuild(const IRendererContext& /*rContext*/) { }

void CRenderer::refresh(const IRendererContext& rContext)
{
	if (!m_sampleCount)
	{
		m_ERPFraction    = 0;
		m_sampleIndexERP = 0;
		return;
	}

	m_ERPFraction    = rContext.getERPFraction();
	m_sampleIndexERP = uint32_t(m_ERPFraction * float(m_sampleCount - 1)) % m_sampleCount;
}

#if 0
bool CRenderer::render(const IRendererContext & rContext)
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
	for (int x = -10; x <= 10; x++)
	{
		for (int z = -10; z <= 10; z++)
		{
			if (x != 0)
			{
				glVertex3f(float(x), 0, 10.f);
				glVertex3f(float(x), 0, -10.f);
			}
			if (z != 0)
			{
				glVertex3f(10.f, 0, float(z));
				glVertex3f(-10.f, 0, float(z));
			}
		}
	}
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 2.f);
	glVertex3f(0, 0, -3.f);
	glColor3f(0, 1, 0);
	glVertex3f(0, 1.25f, 0);
	glVertex3f(0, -1.25f, 0);
	glColor3f(1, 0, 0);
	glVertex3f(2.f, 0, 0);
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
