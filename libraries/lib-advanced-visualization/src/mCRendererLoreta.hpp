///-------------------------------------------------------------------------------------------------
/// 
/// \file mCRendererLoreta.hpp
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

#if defined TARGET_HAS_ThirdPartyOpenGL

#include "mIRenderer.hpp"
#include "mC3DMesh.hpp"
#include <vector>
#include <map>
#include <string>

namespace OpenViBE {
namespace AdvancedVisualization {
class CRendererLoreta final : public IRenderer
{
public:
	CRendererLoreta();

	void rebuild(const CRendererContext& ctx) override { IRenderer::rebuild(ctx); }
	void refresh(const CRendererContext& ctx) override { IRenderer::refresh(ctx); }
	bool render(const CRendererContext& ctx) override;

	void clearRegionSelection() override;
	size_t getRegionCategoryCount() override { return m_lookups.size(); }
	size_t getRegionCount(const size_t category) override;
	const char* getRegionCategoryName(const size_t category) override;
	const char* getRegionName(const size_t category, const size_t index) override;
	void selectRegion(const size_t category, const char* name) override;
	void selectRegion(const size_t category, const size_t index) override;

	void refreshBrainSubset();

protected:
	std::vector<std::map<std::string, std::vector<size_t>>> m_lookups;
	std::vector<bool> m_selecteds;

	C3DMesh m_face, m_scalp, m_brain;

	std::vector<uint32_t> m_brainSubsetTriangles;
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE

#endif // TARGET_HAS_ThirdPartyOpenGL
