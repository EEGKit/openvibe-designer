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

#include <cmath>
#include <algorithm>	// std::min_element, std::max_element

using namespace Mensia;
using namespace AdvancedVisualization;

static int iCount = 0;

CRenderer::CRenderer() { iCount++; }
CRenderer::~CRenderer() { iCount--; }

void CRenderer::setChannelCount(const size_t nChannel)
{
	m_nChannel        = nChannel;
	m_nInverseChannel = (nChannel ? 1.F / nChannel : 1);
	m_vertex.clear();
	m_mesh.clear();

	m_historyIdx = 0;
	m_nHistory   = 0;
	m_history.clear();
	m_history.resize(nChannel);
}

void CRenderer::setSampleCount(const size_t nSample)
{
	m_nSample        = nSample == 0 ? 1 : nSample;
	m_nInverseSample = (m_nSample ? 1.F / m_nSample : 1);
	m_vertex.clear();
	m_mesh.clear();
}

void CRenderer::feed(const float* data)
{
	for (size_t i = 0; i < m_nChannel; ++i) { m_history[i].push_back(data[i]); }
	m_nHistory++;
}

void CRenderer::feed(const float* data, const size_t nSample)
{
	for (size_t i = 0; i < m_nChannel; ++i)
	{
		for (size_t j = 0; j < nSample; ++j) { m_history[i].push_back(data[j]); }
		data += nSample;
	}
	m_nHistory += nSample;
}

void CRenderer::prefeed(const size_t nPreFeedSample)
{
	for (size_t i = 0; i < m_nChannel; ++i) { m_history[i].insert(m_history[i].begin(), nPreFeedSample, 0.F); }
	m_nHistory += nPreFeedSample;
	m_historyIdx = 0;
}

float CRenderer::getSuggestedScale()
{
	if (m_nChannel != 0)
	{
		std::vector<float> averages;

		for (size_t i = 0; i < m_nChannel; ++i)
		{
			averages.push_back(0);

			const size_t n = (m_history[i].size() < m_nSample) ? m_history[i].size() : m_nSample;

			for (size_t j = m_history[i].size(); j > (m_history[i].size() - n); --j) { averages.back() += m_history[i][j - 1]; }

			averages.back() /= float(n);
		}

		return (1 / *std::max_element(averages.begin(), averages.end()));
	}
	return 0;
}

void CRenderer::clear(const size_t nSampleToKeep)
{
	if (!m_history.empty())
	{
		if (nSampleToKeep == 0)
		{
			for (auto& vec : m_history) { vec.clear(); }
			m_nHistory = 0;
		}
		else if (nSampleToKeep < m_history[0].size())
		{
			const size_t sampleToDelete = m_history[0].size() - nSampleToKeep;

			if (sampleToDelete > 1)
			{
				for (auto& vec : m_history) { std::vector<float>(vec.begin() + sampleToDelete, vec.end()).swap(vec); }
				m_nHistory -= size_t(sampleToDelete);
			}
		}
	}
	// We always delete all of the stimulations, ideally we would know the time
	// scale so we can keep the stimulations according to the kept samples
	m_stimulationHistory.clear();
	m_historyIdx = 0;
}

void CRenderer::setHistoryDrawIndex(const size_t index)
{
	m_historyDrawIdx = index;
	m_historyIdx     = 0;
}

bool CRenderer::getSampleAtERPFraction(const float erpFraction, std::vector<float>& samples) const
{
	samples.resize(m_nChannel);

	if (m_nSample > m_nHistory) { return false; }

	const float sampleIndexERP   = (erpFraction * float(m_nSample - 1));
	const float alpha            = sampleIndexERP - std::floor(sampleIndexERP);
	const size_t sampleIndexERP1 = size_t(sampleIndexERP) % m_nSample;
	const size_t sampleIndexERP2 = size_t(sampleIndexERP + 1) % m_nSample;

	for (size_t i = 0; i < m_nChannel; ++i)
	{
		samples[i] = m_history[i][m_nHistory - m_nSample + sampleIndexERP1] * (1 - alpha)
					 + m_history[i][m_nHistory - m_nSample + sampleIndexERP2] * (alpha);
	}

	return true;
}

void CRenderer::refresh(const IRendererContext& ctx)
{
	if (!m_nSample)
	{
		m_erpFraction    = 0;
		m_sampleIndexERP = 0;
		return;
	}

	m_erpFraction    = ctx.getERPFraction();
	m_sampleIndexERP = size_t(m_erpFraction * float(m_nSample - 1)) % m_nSample;
}

/*
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
*/

void CRenderer::draw3DCoordinateSystem()

{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_1D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2);

	glPushMatrix();
	glColor3f(.2F, .2F, .2F);
	glScalef(.2F, .2F, .2F);
	glBegin(GL_LINES);
	for (int x = -10; x <= 10; ++x)
	{
		for (int z = -10; z <= 10; ++z)
		{
			if (x != 0)
			{
				glVertex3f(float(x), 0, 10.F);
				glVertex3f(float(x), 0, -10.F);
			}
			if (z != 0)
			{
				glVertex3f(10.F, 0, float(z));
				glVertex3f(-10.F, 0, float(z));
			}
		}
	}
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 2.F);
	glVertex3f(0, 0, -3.F);
	glColor3f(0, 1, 0);
	glVertex3f(0, 1.25F, 0);
	glVertex3f(0, -1.25F, 0);
	glColor3f(1, 0, 0);
	glVertex3f(2.F, 0, 0);
	glVertex3f(-2.F, 0, 0);
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
	glColor3f(.2F, .2F, .2F);
	glScalef(.2F, .2F, .2F);
	glBegin(GL_LINES);
	for (int x = -10; x <= 10; ++x)
	{
		for (int y = -10; y <= 10; ++y)
		{
			if (x != 0)
			{
				glVertex2f(float(x), 10.F);
				glVertex2f(float(x), -10.F);
			}
			if (y != 0)
			{
				glVertex2f(10.F, float(y));
				glVertex2f(-10.F, float(y));
			}
		}
	}
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(0, 1, 0);
	glVertex2f(0, 2.0F);
	glVertex2f(0, -2.0F);
	glColor3f(1, 0, 0);
	glVertex2f(2.F, 0);
	glVertex2f(-2.F, 0);
	glEnd();

	glPopAttrib();
}
