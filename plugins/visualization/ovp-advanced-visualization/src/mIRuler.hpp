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

#ifndef __OpenViBEPlugins_IRuler_H__
#define __OpenViBEPlugins_IRuler_H__

#include <mensia/advanced-visualization.h>

#include <gtk/gtk.h>

#if defined TARGET_OS_Windows
 #include <windows.h>
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
		private:

			IRuler(const IRuler&);

		public:

			IRuler(void)
				:m_pRendererContext(NULL)
				,m_pRenderer(NULL)
				,m_fBlackAlpha(.9f)
				,m_fWhiteAlpha(1.f)
			{
			}

			virtual ~IRuler(void)
			{
			}

			virtual void setRendererContext(const IRendererContext* pRendererContext)
			{
				m_pRendererContext=pRendererContext;
			}

			virtual void setRenderer(const IRenderer* pRenderer)
			{
#if 0
				::printf("%p = %p -> %p\n", this, m_pRenderer, pRenderer);
#endif
				m_pRenderer=pRenderer;
			}

			virtual void doRender(void)
			{
				// if(m_pRendererContext->getScaleVisibility())
				{
					this->render();
				}
			}

			virtual void doRenderLeft(::GtkWidget* pWidget)
			{
				if(m_pRendererContext->getScaleVisibility())
				{
					this->renderLeft(pWidget);
				}
			}

			virtual void doRenderRight(::GtkWidget* pWidget)
			{
				if(m_pRendererContext->getScaleVisibility())
				{
					this->renderRight(pWidget);
				}
			}

			virtual void doRenderBottom(::GtkWidget* pWidget)
			{
				if(m_pRendererContext->getScaleVisibility())
				{
					this->renderBottom(pWidget);
				}
			}

		protected:

			virtual void render(void)
			{
			}

			virtual void renderLeft(::GtkWidget* pWidget)
			{
			}

			virtual void renderRight(::GtkWidget* pWidget)
			{
			}

			virtual void renderBottom(::GtkWidget* pWidget)
			{
			}

		protected:

			inline std::vector < double > split_range(double fStart, double fStop, unsigned int uiCount=10)
			{
				std::vector < double > l_vResult;
				double l_fRange=fStop-fStart;
				double l_fOrder=::floor(::log(l_fRange) / ::log(10.) - .1f);
				double l_fStep=::pow(10, l_fOrder);
				double l_fStepCount=::trunc(l_fRange/l_fStep);

				while(l_fStepCount<uiCount) { l_fStepCount*=2; l_fStep/=2; }
				while(l_fStepCount>uiCount) { l_fStepCount/=2; l_fStep*=2; }

				double l_fValue=::trunc(fStart/l_fStep)*l_fStep;
				while(l_fValue<fStart)
				{
					l_fValue+=l_fStep;
				}
				while(l_fValue<=fStop)
				{
					l_vResult.push_back(std::abs(l_fValue) < std::abs(l_fRange/1000) ? 0 : l_fValue);
					l_fValue+=l_fStep;
				}
				return l_vResult;
			}

			std::string getLabel(double v)
			{
				char l_sLabel[1024];
#if 0
				::sprintf(l_sLabel, "%f", v);
				size_t i=::strlen(l_sLabel)-1;
				while(l_sLabel[i]=='0')
				{
					l_sLabel[i]='\0';
					i--;
				}
				if(l_sLabel[i]=='.') l_sLabel[i]='\0';
#else
				if(::fabs(v)<1E-10)
				{
					::sprintf(l_sLabel, "0");
				}
				else
				{
					::sprintf(l_sLabel, "%g", v);
				}
#endif
				return l_sLabel;
			}

			const IRendererContext* m_pRendererContext;
			const IRenderer* m_pRenderer;
			float m_fBlackAlpha;
			float m_fWhiteAlpha;
		};
	};
};

#endif // __OpenViBEPlugins_IRuler_H__
