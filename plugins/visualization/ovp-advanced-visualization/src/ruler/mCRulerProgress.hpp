///-------------------------------------------------------------------------------------------------
/// 
/// \file mCRulerProgress.hpp
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
class CRulerProgress : public IRuler
{
public:
	virtual void renderFinal(const float progress) = 0;

	void render() override
	{
		if (m_renderer == nullptr) { return; }
		if (m_renderer->getSampleCount() == 0) { return; }
		if (m_renderer->getHistoryCount() == 0) { return; }
		if (m_renderer->getHistoryIndex() == 0) { return; }

		const float nSample    = float(m_renderer->getSampleCount());
		const float historyIdx = float(m_renderer->getHistoryIndex());

		const float progress = (historyIdx - (historyIdx / nSample) * nSample) / nSample;
		if (std::fabs(progress) > FLT_EPSILON && std::fabs(progress - 1) > FLT_EPSILON) { this->renderFinal(progress); }
	}
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
