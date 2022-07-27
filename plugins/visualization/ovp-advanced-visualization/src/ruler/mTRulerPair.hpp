///-------------------------------------------------------------------------------------------------
/// 
/// \file mTRulerPair.hpp
/// \copyright Copyright (C) 2022 Inria
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <https://www.gnu.org/licenses/>.
/// 
///-------------------------------------------------------------------------------------------------

#pragma once

#include "../mIRuler.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {
template <class T1, class T2>
class TRulerPair final : public IRuler
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
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
