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

#include <cstdint>
#include "mCRendererLine.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

CRendererLine::CRendererLine() { }

void CRendererLine::rebuild(const IRendererContext& rContext)
{
	CRenderer::rebuild(rContext);

	m_Vertices.clear();
	m_Vertices.resize(m_ui32ChannelCount);

	for (size_t channel = 0; channel < m_ui32ChannelCount; channel++)
	{
		m_Vertices[channel].resize(m_ui32SampleCount);
		for (size_t sample = 0; sample < m_ui32SampleCount; sample++)
		{
			m_Vertices[channel][sample].x = sample * m_f32InverseSampleCount;
		}
	}

	m_ui32HistoryIndex = 0;
}

void CRendererLine::refresh(const IRendererContext& rContext)
{
	CRenderer::refresh(rContext);

	if (!m_ui32HistoryCount) return;

	uint32_t l_ui32HistoryIndexMax;

	if (m_ui32HistoryDrawIndex == 0) // Draw real-time
	{
		l_ui32HistoryIndexMax = m_ui32HistoryCount;
	}
	else // stay at the m_ui32HistoryDrawIndex
	{
		l_ui32HistoryIndexMax = m_ui32HistoryDrawIndex;
	}

	for (size_t channel = 0; channel < m_ui32ChannelCount; channel++)
	{
		uint32_t firstSampleIndex = ((l_ui32HistoryIndexMax - 1) / m_ui32SampleCount) * m_ui32SampleCount;
		std::vector<float> & l_vHistory = m_vHistory[channel];
		CVertex * l_pVertex = &m_Vertices[channel][0];

		for (uint32_t sample = 0; sample < m_ui32SampleCount; sample++)
		{
			uint32_t currentSampleIndex = firstSampleIndex + sample;

			if (currentSampleIndex < l_ui32HistoryIndexMax)
			{
				l_pVertex->y = l_vHistory[currentSampleIndex];
			}
			else if (currentSampleIndex >= m_ui32SampleCount)
			{
				l_pVertex->y = l_vHistory[currentSampleIndex - m_ui32SampleCount];
			}

			l_pVertex++;
		}
	}

	m_ui32HistoryIndex = l_ui32HistoryIndexMax;
}

bool CRendererLine::render(const IRendererContext& rContext)
{
	if (!rContext.getSelectedCount()) return false;
	if (!m_ui32HistoryCount) return false;

	int32_t sampleCount = static_cast<int32_t>(m_ui32SampleCount);


	// When the display is in continuous mode, there will be n1 samples
	// displayed until the 'cursor' and n2 samples will be displayed to
	// complete the whole window.
	// For example at 25s with a display size of 20s
	// <- n1 samples -><------------ n2 samples ---------------->
	// | ____         |                    ___                  |
	// |/    \        |      ___          /   \     ____        |
	// |------\------/|-----/---\--------/-----\___/----\-------|
	// |       \____/ |    /     \______/                \______|
	// |              |___/                                     |
	// Time          25s              10s                      20s

	int32_t n1 = static_cast<int32_t>(m_ui32HistoryIndex % m_ui32SampleCount);
	int32_t n2 = static_cast<int32_t>(sampleCount - n1);

	if (!sampleCount) return false;

	float t1 = n2 * 1.f / sampleCount;
	float t2 = -n1 * 1.f / sampleCount;

	glDisable(GL_TEXTURE_1D);

	glPushMatrix();
	glScalef(1, 1.f / rContext.getSelectedCount(), 1);
	glTranslatef(0, rContext.isPositiveOnly() ? 0 : 0.5f, 0);

	glPushAttrib(GL_CURRENT_BIT);
	glColor3f(.2f, .2f, .2f);
	glBegin(GL_LINES);
	for (uint32_t selectedChannel = 0; selectedChannel < rContext.getSelectedCount(); selectedChannel++)
	{
		glVertex2f(0, static_cast<float>(selectedChannel));
		glVertex2f(1, static_cast<float>(selectedChannel));
	}
	glEnd();
	glPopAttrib();

	glEnableClientState(GL_VERTEX_ARRAY);
	for (uint32_t selectedChannel = 0; selectedChannel < rContext.getSelectedCount(); selectedChannel++)
	{
		glPushMatrix();
		glTranslatef(0, rContext.getSelectedCount() - selectedChannel - 1.f, 0);
		glScalef(1, rContext.getScale(), 1);

		std::vector < CVertex >& l_rVertex = m_Vertices[rContext.getSelected(selectedChannel)];
		if (rContext.isScrollModeActive())
		{
			glPushMatrix();
			glTranslatef(t1, 0, 0);
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
			glDrawArrays(GL_LINE_STRIP, 0, n1);
			glPopMatrix();
			if (n2 > 0)
			{
				glPushMatrix();
				glTranslatef(t2, 0, 0);
				glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[n1].x);
				glDrawArrays(GL_LINE_STRIP, 0, n2);
				glPopMatrix();

				if (n1 > 0)
				{
					glBegin(GL_LINES);
					glVertex2f(l_rVertex[sampleCount - 1].x + t2, l_rVertex[sampleCount - 1].y);
					glVertex2f(l_rVertex[0].x + t1, l_rVertex[0].y);
					glEnd();
				}
			}
		}
		else
		{
			glVertexPointer(3, GL_FLOAT, sizeof(CVertex), &l_rVertex[0].x);
			glDrawArrays(GL_LINE_STRIP, 0, sampleCount);
		}
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

	return true;
}
