///-------------------------------------------------------------------------------------------------
/// 
/// \file mCRulerConditionIsTimeLocked.hpp
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
class CRulerConditionIsTimeLocked final : public IRuler
{
public:
	CRulerConditionIsTimeLocked() : m_rendererCtx(nullptr), m_renderer(nullptr) { }

	void setRendererContext(const CRendererContext* ctx) override { m_rendererCtx = ctx; }
	void setRenderer(const IRenderer* renderer) override { m_renderer = renderer; }

	bool operator()() const { return m_rendererCtx->isTimeLocked(); }

protected:
	const CRendererContext* m_rendererCtx;
	const IRenderer* m_renderer;
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
