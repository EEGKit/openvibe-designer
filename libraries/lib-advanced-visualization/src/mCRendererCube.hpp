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
#ifndef __Mensia_AdvancedVisualization_CRendererCube_H__
#define __Mensia_AdvancedVisualization_CRendererCube_H__

#include "mCRenderer.hpp"

#include <map>
#include <string>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererCube : public CRenderer
		{
		public:

			CRendererCube();

			void rebuild(const IRendererContext& rContext) override;
			void refresh(const IRendererContext& rContext) override;
			bool render(const IRendererContext& rContext) override;

			std::vector<CVertex> m_vVertex;
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia

#endif // __Mensia_AdvancedVisualization_CRendererCube_H__
