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

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class T1, class T2, class TCondition>
		class TRulerConditionalPair : public IRuler
		{
		public:

			void setRendererContext(const IRendererContext* pRendererContext) override
			{
				IRuler::setRendererContext(pRendererContext);
				condition.setRendererContext(pRendererContext);
				first.setRendererContext(pRendererContext);
				second.setRendererContext(pRendererContext);
			}

			void setRenderer(const IRenderer* pRenderer) override
			{
				IRuler::setRenderer(pRenderer);
				condition.setRenderer(pRenderer);
				first.setRenderer(pRenderer);
				second.setRenderer(pRenderer);
			}

			void render() override
			{
				if (condition()) { first.doRender(); }
				else { second.doRender(); }
			}

			void renderLeft(GtkWidget* pWidget) override
			{
				if (condition()) { first.doRenderLeft(pWidget); }
				else { second.doRenderLeft(pWidget); }
			}

			void renderRight(GtkWidget* pWidget) override
			{
				if (condition()) { first.doRenderRight(pWidget); }
				else { second.doRenderRight(pWidget); }
			}

			void renderBottom(GtkWidget* pWidget) override
			{
				if (condition()) { first.doRenderBottom(pWidget); }
				else { second.doRenderBottom(pWidget); }
			}

			TCondition condition;
			T1 first;
			T2 second;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
