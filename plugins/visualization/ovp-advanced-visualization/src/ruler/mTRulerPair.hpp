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
		template <class T1, class T2>
		class TRulerPair : public IRuler
		{
		public:

			void setRendererContext(const IRendererContext* pRendererContext) override
			{
				IRuler::setRendererContext(pRendererContext);
				first.setRendererContext(pRendererContext);
				second.setRendererContext(pRendererContext);
			}

			void setRenderer(const IRenderer* pRenderer) override
			{
				IRuler::setRenderer(pRenderer);
				first.setRenderer(pRenderer);
				second.setRenderer(pRenderer);
			}

			void render() override
			{
				first.doRender();
				second.doRender();
			}

			void renderLeft(GtkWidget* pWidget) override
			{
				first.doRenderLeft(pWidget);
				second.doRenderLeft(pWidget);
			}

			void renderRight(GtkWidget* pWidget) override
			{
				first.doRenderRight(pWidget);
				second.doRenderRight(pWidget);
			}

			void renderBottom(GtkWidget* pWidget) override
			{
				first.doRenderBottom(pWidget);
				second.doRenderBottom(pWidget);
			}

			T1 first;
			T2 second;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
