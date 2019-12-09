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

				const size_t nSample    = m_renderer->getSampleCount();
				const size_t historyIdx = m_renderer->getHistoryIndex();

				const float progress = float(historyIdx - (float(historyIdx) / nSample) * nSample) / nSample;
				if (progress != 0 && progress != 1) { this->renderFinal(progress); }
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
