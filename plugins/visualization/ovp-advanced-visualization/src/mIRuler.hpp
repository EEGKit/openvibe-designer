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

#include <mensia/advanced-visualization.h>

#include <gtk/gtk.h>

#if defined TARGET_OS_Windows
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <cmath>
#include <vector>
#include <cstring>
#include <cstdlib>

#define IRuler_SplitCount 5

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class IRuler
		{
		public:

			IRuler() { }
			IRuler(const IRuler&) = delete;
			virtual ~IRuler()     = default;

			virtual void setRendererContext(const IRendererContext* pRendererContext) { m_pRendererContext = pRendererContext; }

			virtual void setRenderer(const IRenderer* pRenderer)
			{
#if 0
				::printf("%p = %p -> %p\n", this, m_pRenderer, pRenderer);
#endif
				m_pRenderer = pRenderer;
			}

			virtual void doRender()
			{
				// if(m_pRendererContext->getScaleVisibility()) { this->render(); }
				this->render();
			}

			virtual void doRenderLeft(GtkWidget* pWidget) { if (m_pRendererContext->getScaleVisibility()) { this->renderLeft(pWidget); } }

			virtual void doRenderRight(GtkWidget* pWidget) { if (m_pRendererContext->getScaleVisibility()) { this->renderRight(pWidget); } }

			virtual void doRenderBottom(GtkWidget* pWidget) { if (m_pRendererContext->getScaleVisibility()) { this->renderBottom(pWidget); } }

		protected:

			virtual void render() { }

			virtual void renderLeft(GtkWidget* /*pWidget*/) { }

			virtual void renderRight(GtkWidget* /*pWidget*/) { }

			virtual void renderBottom(GtkWidget* /*pWidget*/) { }

			std::vector<double> split_range(const double fStart, const double fStop, const uint32_t uiCount = 10) const
			{
				std::vector<double> l_vResult;
				const double l_fRange = fStop - fStart;
				const double l_fOrder = floor(log(l_fRange) / log(10.) - .1f);
				double l_fStep        = pow(10, l_fOrder);
				double l_fStepCount   = trunc(l_fRange / l_fStep);

				while (l_fStepCount < uiCount)
				{
					l_fStepCount *= 2;
					l_fStep /= 2;
				}
				while (l_fStepCount > uiCount)
				{
					l_fStepCount /= 2;
					l_fStep *= 2;
				}

				double l_fValue = trunc(fStart / l_fStep) * l_fStep;
				while (l_fValue < fStart) { l_fValue += l_fStep; }
				while (l_fValue <= fStop)
				{
					l_vResult.push_back(std::abs(l_fValue) < std::abs(l_fRange / 1000) ? 0 : l_fValue);
					l_fValue += l_fStep;
				}
				return l_vResult;
			}

			static std::string getLabel(const double v)
			{
				char l_sLabel[1024];
#if 0
				::sprintf(l_sLabel, "%f", v);
				size_t i = ::strlen(l_sLabel) - 1;
				while (l_sLabel[i] == '0')
				{
					l_sLabel[i] = '\0';
					i--;
				}
				if (l_sLabel[i] == '.') l_sLabel[i] = '\0';
#else
				if (fabs(v) < 1E-10) { sprintf(l_sLabel, "0"); }
				else { sprintf(l_sLabel, "%g", v); }
#endif
				return l_sLabel;
			}

			const IRendererContext* m_pRendererContext = nullptr;
			const IRenderer* m_pRenderer               = nullptr;
			float m_fBlackAlpha                        = 0.9f;
			float m_fWhiteAlpha                        = 1.0f;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
