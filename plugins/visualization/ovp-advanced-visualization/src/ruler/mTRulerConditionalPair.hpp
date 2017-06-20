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
