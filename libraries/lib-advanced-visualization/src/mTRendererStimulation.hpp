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
#pragma once

#include "mCRenderer.hpp"

#include <string>
#include <vector>
#include <map>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		static const int s_iStimulationIndicatorSmoothness = 10;
		static const float s_fStimulationIndicatorRadius   = 0.01f;
		static const float s_fStimulationIndicatorSpacing  = 0.03f;

		template <bool bPreRender, class T>
		class TRendererStimulation : public T
		{
		public:
			std::vector<std::pair<float, float>> m_vCircle;
			std::map<uint64_t, int> m_mEncounteredStimulations;

			TRendererStimulation()
			{
				// Render a circle into a buffer so we don't have to do this each time

				for (int i = 0; i < s_iStimulationIndicatorSmoothness; ++i)
				{
					m_vCircle.push_back(std::make_pair(s_fStimulationIndicatorRadius * cosf(float(i) / float(s_iStimulationIndicatorSmoothness - 1) * 2 * float(M_PI)),
													   s_fStimulationIndicatorRadius * sinf(float(i) / float(s_iStimulationIndicatorSmoothness - 1) * 2 * float(M_PI))
					));
				}
			}

			bool render(const IRendererContext& rContext) override
			{
				bool l_bResult = true;

				if (bPreRender)
				{
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					l_bResult = T::render(rContext);
					glPopAttrib();
				}

				bool ok = true;
				ok &= (CRenderer::getSampleCount() != 0);
				ok &= (CRenderer::getHistoryCount() != 0);
				ok &= (CRenderer::getHistoryIndex() != 0);

				if (ok)
				{
					glPushAttrib(GL_ALL_ATTRIB_BITS);

					const uint32_t sampleCount    = CRenderer::getSampleCount();
					const uint32_t historyIndex   = CRenderer::getHistoryIndex();
					const uint64_t sampleDuration = rContext.getSampleDuration();

					glDisable(GL_TEXTURE_1D);
					glDisable(GL_BLEND);
					glDisable(GL_LINE_SMOOTH);

					const uint32_t leftIndex = historyIndex - historyIndex % sampleCount;
					const uint32_t midIndex  = historyIndex;
					const double startTime   = ((leftIndex * sampleDuration) >> 16) / 65536.;
					const double midTime     = ((midIndex * sampleDuration) >> 16) / 65536.;
					const double duration    = ((sampleCount * sampleDuration) >> 16) / 65536.;

					m_mEncounteredStimulations.clear();
					for (auto it = CRenderer::m_stimulationHistory.begin(); it != CRenderer::m_stimulationHistory.end(); ++it)
					{
						if (m_mEncounteredStimulations.count(it->second) == 0)
						{
							// we store the "position" of the indicator for each new encountered stimulation
							// if there are too many of them, we loop over to the beginning.
							m_mEncounteredStimulations[it->second] = m_mEncounteredStimulations.size() % int(1.0f / s_fStimulationIndicatorSpacing);
						}

						if (midTime - duration < it->first && it->first < midTime)
						{
							float l_fProgress;
							if (it->first > startTime)
							{
								l_fProgress = float((it->first - startTime) / duration);
							}
							else { l_fProgress = float((it->first + duration - startTime) / duration); }

							/*
							::glLineWidth(3);
							::glColor3f(0, 0, 0);
							::glBegin(GL_LINES);
								::glVertex2f(l_fProgress, 0);
								::glVertex2f(l_fProgress, 1);
							::glEnd();
							*/

							// draw a vertical line representing the stimulation
							glLineWidth(1);
							glColor3fv(getMarkerColor(it->second));
							glBegin(GL_LINES);
							glVertex2f(l_fProgress, 0);
							glVertex2f(l_fProgress, 1);
							glEnd();


							// draw a (ugly) disc representing a stimulation
							glBegin(GL_TRIANGLE_FAN);
							for (auto l_oVertex = m_vCircle.cbegin(); l_oVertex != m_vCircle.cend(); ++l_oVertex)
							{
								glVertex2f(float(l_oVertex->first / rContext.getAspect() + l_fProgress), float(l_oVertex->second + 0.95f - m_mEncounteredStimulations[it->second] * s_fStimulationIndicatorSpacing));
							}
							glEnd();


							// now draw
							glLineWidth(2);
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							glEnable(GL_LINE_SMOOTH);
							glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
							glBegin(GL_LINE_LOOP);
							for (auto l_oVertex = m_vCircle.cbegin(); l_oVertex != m_vCircle.cend(); ++l_oVertex)
							{
								glVertex2f(float(l_oVertex->first / rContext.getAspect() + l_fProgress), float(l_oVertex->second + 0.95f - m_mEncounteredStimulations[it->second] * s_fStimulationIndicatorSpacing));
							}
							glEnd();
							glDisable(GL_LINE_SMOOTH);

							glDisable(GL_BLEND);
						}
					}

					glEnable(GL_LINE_SMOOTH);
					glEnable(GL_BLEND);
					glEnable(GL_TEXTURE_1D);

					glPopAttrib();
				}

				if (!bPreRender)
				{
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					l_bResult = T::render(rContext);
					glPopAttrib();
				}

				return l_bResult && ok;
			}

			float* getMarkerColor(const uint64_t ui64Identifier)
			{
				static float color[4];
				const float alpha       = reverse<>(uint8_t(ui64Identifier & 255)) * 3.f / 255.f;
				const auto alphai       = int(alpha);
				color[(alphai + 0) % 3] = 1 - alpha / 3.f;
				color[(alphai + 1) % 3] = alpha / 3.f;
				color[(alphai + 2) % 3] = 0;
				color[3]                = .75f;
				return color;
			}

			template <typename V>
			V reverse(V v)
			{
				V l_uiResult = 0;
				for (V i = 0; i < sizeof(V) * 8; ++i)
				{
					l_uiResult <<= 1;
					l_uiResult |= ((v & (1 << i)) ? 1 : 0);
				}
				return l_uiResult;
			}
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia
