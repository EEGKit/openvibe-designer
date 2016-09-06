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

#ifndef __OpenViBEPlugins_TRulerConditionalPair_H__
#define __OpenViBEPlugins_TRulerConditionalPair_H__

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class T1, class T2, class TCondition>
		class TRulerConditionalPair : public IRuler
		{
		public:

			virtual void setRendererContext(const IRendererContext* pRendererContext)
			{
				IRuler::setRendererContext(pRendererContext);
				condition.setRendererContext(pRendererContext);
				first.setRendererContext(pRendererContext);
				second.setRendererContext(pRendererContext);
			}

			virtual void setRenderer(const IRenderer* pRenderer)
			{
				IRuler::setRenderer(pRenderer);
				condition.setRenderer(pRenderer);
				first.setRenderer(pRenderer);
				second.setRenderer(pRenderer);
			}

			virtual void render()
			{
				if(condition())
				{
					first.doRender();
				}
				else
				{
					second.doRender();
				}
			}

			virtual void renderLeft(::GtkWidget* pWidget)
			{
				if(condition())
				{
					first.doRenderLeft(pWidget);
				}
				else
				{
					second.doRenderLeft(pWidget);
				}
			}

			virtual void renderRight(::GtkWidget* pWidget)
			{
				if(condition())
				{
					first.doRenderRight(pWidget);
				}
				else
				{
					second.doRenderRight(pWidget);
				}
			}

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				if(condition())
				{
					first.doRenderBottom(pWidget);
				}
				else
				{
					second.doRenderBottom(pWidget);
				}
			}

			TCondition condition;
			T1 first;
			T2 second;
		};
	};
};

#endif // __OpenViBEPlugins_TRulerConditionalPair_H__
