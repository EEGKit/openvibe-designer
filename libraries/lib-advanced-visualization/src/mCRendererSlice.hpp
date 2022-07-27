///-------------------------------------------------------------------------------------------------
/// 
/// \file mCRendererSlice.hpp
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

#include "mIRenderer.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {
class CRendererSlice final : public IRenderer
{
public:
	void rebuild(const CRendererContext& ctx) override;
	void refresh(const CRendererContext& ctx) override;
	bool render(const CRendererContext& ctx) override;

protected:
	std::vector<CVertex> m_vertices;
	std::vector<size_t> m_quads;
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
