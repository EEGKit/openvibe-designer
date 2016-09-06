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

#ifndef __OpenViBEPlugins_TRulerPair_H__
#define __OpenViBEPlugins_TRulerPair_H__

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class T1, class T2>
		class TRulerPair : public IRuler
		{
		public:

			virtual void setRendererContext(const IRendererContext* pRendererContext)
			{
				IRuler::setRendererContext(pRendererContext);
				first.setRendererContext(pRendererContext);
				second.setRendererContext(pRendererContext);
			}

			virtual void setRenderer(const IRenderer* pRenderer)
			{
				IRuler::setRenderer(pRenderer);
				first.setRenderer(pRenderer);
				second.setRenderer(pRenderer);
			}

			virtual void render()
			{
				first.doRender();
				second.doRender();
			}

			virtual void renderLeft(::GtkWidget* pWidget)
			{
				first.doRenderLeft(pWidget);
				second.doRenderLeft(pWidget);
			}

			virtual void renderRight(::GtkWidget* pWidget)
			{
				first.doRenderRight(pWidget);
				second.doRenderRight(pWidget);
			}

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				first.doRenderBottom(pWidget);
				second.doRenderBottom(pWidget);
			}

			T1 first;
			T2 second;
		};
	};
};

#endif // __OpenViBEPlugins_TRulerPair_H__
