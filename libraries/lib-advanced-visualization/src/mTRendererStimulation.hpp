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

#ifndef __Mensia_AdvancedVisualization_TRendererStimulation_H__
#define __Mensia_AdvancedVisualization_TRendererStimulation_H__

#include "mCRenderer.hpp"

#include <string>
#include <vector>
#include <map>

namespace Mensia
{
	namespace AdvancedVisualization
	{

		static const int s_iStimulationIndicatorSmoothness = 10;
		static const float s_fStimulationIndicatorRadius = 0.01f;
		static const float s_fStimulationIndicatorSpacing = 0.03f;

		template < bool bPreRender, class T >
		class TRendererStimulation : public T
		{
		public:
			std::vector<std::pair<float, float>> m_vCircle;
			std::map<uint64, int> m_mEncounteredStimulations;

			TRendererStimulation()
			{
				// Render a circle into a buffer so we don't have to do this each time

				for (int i = 0; i < s_iStimulationIndicatorSmoothness; i++)
				{
					m_vCircle.push_back(std::make_pair(
					                        s_fStimulationIndicatorRadius * cosf(i / float(s_iStimulationIndicatorSmoothness - 1) * 2 * float(M_PI)),
					                        s_fStimulationIndicatorRadius * sinf(i / float(s_iStimulationIndicatorSmoothness - 1) * 2 * float(M_PI))
					                        ));

				}
			}

			virtual boolean render(const IRendererContext& rContext)
			{
				boolean l_bResult=true;

				if(bPreRender)
				{
					::glPushAttrib(GL_ALL_ATTRIB_BITS);
					l_bResult = T::render(rContext);
					::glPopAttrib();
				}

				bool ok=true;
				ok&=(CRenderer::getSampleCount() != 0);
				ok&=(CRenderer::getHistoryCount() != 0);
				ok&=(CRenderer::getHistoryIndex() != 0);

				if(ok)
				{
					::glPushAttrib(GL_ALL_ATTRIB_BITS);

					uint32 l_ui32SampleCount=CRenderer::getSampleCount();
					uint32 l_ui32HistoryIndex=CRenderer::getHistoryIndex();
					uint64 l_ui64SampleDuration=rContext.getSampleDuration();

					::glDisable(GL_TEXTURE_1D);
					::glDisable(GL_BLEND);
					::glDisable(GL_LINE_SMOOTH);

					uint32 l_ui32LeftIndex=l_ui32HistoryIndex-l_ui32HistoryIndex%l_ui32SampleCount;
					uint32 l_ui32MidIndex =l_ui32HistoryIndex;
					float64 l_f64StartTime=((l_ui32LeftIndex  *l_ui64SampleDuration)>>16)/65536.;
					float64 l_f64MidTime  =((l_ui32MidIndex   *l_ui64SampleDuration)>>16)/65536.;
					float64 l_f64Duration =((l_ui32SampleCount*l_ui64SampleDuration)>>16)/65536.;

					std::vector < std::pair < float64, uint64 > >::const_iterator it;
					for(it=CRenderer::m_vStimulationHistory.begin(); it!=CRenderer::m_vStimulationHistory.end(); it++)
					{
						if (m_mEncounteredStimulations.count(it->second) == 0)
						{
							// we store the "position" of the indicator for each new encountered stimulation
							// if there are too many of them, we loop over to the beginning.
							m_mEncounteredStimulations[it->second] = m_mEncounteredStimulations.size() % int(1.0f / s_fStimulationIndicatorSpacing);
						}

						if(l_f64MidTime - l_f64Duration < it->first && it->first < l_f64MidTime)
						{
							float l_fProgress;
							if(it->first>l_f64StartTime) l_fProgress=float((it->first-l_f64StartTime)/l_f64Duration);
							else                         l_fProgress=float((it->first+l_f64Duration-l_f64StartTime)/l_f64Duration);

							/*
							::glLineWidth(3);
							::glColor3f(0, 0, 0);
							::glBegin(GL_LINES);
								::glVertex2f(l_fProgress, 0);
								::glVertex2f(l_fProgress, 1);
							::glEnd();
							*/

							// draw a vertical line representing the stimulation
							::glLineWidth(1);
							::glColor3fv(getMarkerColor(it->second));
							::glBegin(GL_LINES);
								::glVertex2f(l_fProgress, 0);
								::glVertex2f(l_fProgress, 1);
							::glEnd();


							// draw a (ugly) disc representing a stimulation
							::glBegin(GL_TRIANGLE_FAN);
							for (auto l_oVertex = m_vCircle.cbegin(); l_oVertex != m_vCircle.cend(); l_oVertex++)
							{
								::glVertex2f(float(l_oVertex->first / rContext.getAspect() + l_fProgress), float(l_oVertex->second + 0.95f - m_mEncounteredStimulations[it->second] * s_fStimulationIndicatorSpacing));
							}
							::glEnd();


							// now draw
							::glLineWidth(2);
							::glEnable(GL_BLEND);
							::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							::glEnable(GL_LINE_SMOOTH);
							::glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
							::glBegin(GL_LINE_LOOP);
							for (auto l_oVertex = m_vCircle.cbegin(); l_oVertex != m_vCircle.cend(); l_oVertex++)
							{
								::glVertex2f(float(l_oVertex->first / rContext.getAspect() + l_fProgress), float(l_oVertex->second + 0.95f - m_mEncounteredStimulations[it->second] * s_fStimulationIndicatorSpacing));
							}
							::glEnd();
							::glDisable(GL_LINE_SMOOTH);

							::glDisable(GL_BLEND);
						}
					}

					::glEnable(GL_LINE_SMOOTH);
					::glEnable(GL_BLEND);
					::glEnable(GL_TEXTURE_1D);

					::glPopAttrib();
				}

				if(!bPreRender)
				{
					::glPushAttrib(GL_ALL_ATTRIB_BITS);
					l_bResult = T::render(rContext);
					::glPopAttrib();
				}

				return l_bResult && ok;
			}

			float32* getMarkerColor(uint64 ui64Identifier)
			{
				static float32 color[4];
				float32 alpha=reverse<>(uint8(ui64Identifier&255))*3.f/255.f;
				int32 alphai=int32(alpha);
				color[(alphai+0)%3]=1-alpha/3.f;
				color[(alphai+1)%3]=alpha/3.f;
				color[(alphai+2)%3]=0;
				color[3]=.75f;
				return color;
			}

			template <typename V>
			V reverse(V v)
			{
				V l_uiResult=0;
				for(V i=0; i<sizeof(V)*8; i++)
				{
					l_uiResult<<=1;
					l_uiResult|=((v&(1<<i))?1:0);
				}
				return l_uiResult;
			}
		};
	};
};

#endif // __Mensia_AdvancedVisualization_TRendererStimulation_H__
