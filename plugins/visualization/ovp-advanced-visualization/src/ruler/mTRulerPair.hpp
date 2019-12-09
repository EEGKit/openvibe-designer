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

			void setRendererContext(const CRendererContext* ctx) override
			{
				IRuler::setRendererContext(ctx);
				first.setRendererContext(ctx);
				second.setRendererContext(ctx);
			}

			void setRenderer(const IRenderer* renderer) override
			{
				IRuler::setRenderer(renderer);
				first.setRenderer(renderer);
				second.setRenderer(renderer);
			}

			void render() override
			{
				first.doRender();
				second.doRender();
			}

			void renderLeft(GtkWidget* widget) override
			{
				first.doRenderLeft(widget);
				second.doRenderLeft(widget);
			}

			void renderRight(GtkWidget* widget) override
			{
				first.doRenderRight(widget);
				second.doRenderRight(widget);
			}

			void renderBottom(GtkWidget* widget) override
			{
				first.doRenderBottom(widget);
				second.doRenderBottom(widget);
			}

			T1 first;
			T2 second;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
