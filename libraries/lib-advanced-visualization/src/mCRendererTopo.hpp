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

#include "mCRenderer.hpp"
#include "mC3DMesh.hpp"

#include <Eigen/Eigen>

#include <map>
#include <vector>
#include <string>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererTopo : public CRenderer
		{
		public:

			void rebuild(const IRendererContext& rContext) override;
			void refresh(const IRendererContext& rContext) override;
			bool render(const IRendererContext& rContext) override;

			virtual void rebuild3DMeshesPre(const IRendererContext& rContext) = 0; // Called before electrode projections and spherical interpolation parameters generations and might be used to load a mesh or generate a sphere for instance
			virtual void rebuild3DMeshesPost(const IRendererContext& rContext) = 0; // Called after electrode projections and spherical interpolation parameters generations and might be used to unfold previously loaded mesh for instance

		private:

			void interpolate(const Eigen::VectorXd& V, Eigen::VectorXd& W, Eigen::VectorXd& Z);

		protected:

			std::vector < CVertex > m_vProjectedChannelCoordinate;

			C3DMesh m_oFace;
			C3DMesh m_oScalp;
			std::vector < Eigen::VectorXd > m_vInterpolatedSample;

			Eigen::MatrixXd A, B, D, Ai;
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia

