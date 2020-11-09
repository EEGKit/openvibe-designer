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

#include "mIRenderer.hpp"
#include "mC3DMesh.hpp"

#include <Eigen/Eigen>

#include <vector>

namespace Mensia {
namespace AdvancedVisualization {
class CRendererTopo : public IRenderer
{
public:

	void rebuild(const CRendererContext& ctx) override;
	void refresh(const CRendererContext& ctx) override;
	bool render(const CRendererContext& ctx) override;

	// Called before electrode projections and spherical interpolation parameters generations and might be used to load a mesh or generate a sphere for instance
	virtual void rebuild3DMeshesPre(const CRendererContext& ctx) = 0;
	// Called after electrode projections and spherical interpolation parameters generations and might be used to unfold previously loaded mesh for instance
	virtual void rebuild3DMeshesPost(const CRendererContext& ctx) = 0;

private:

	void interpolate(const Eigen::VectorXd& v, Eigen::VectorXd& w, Eigen::VectorXd& z) const;

protected:

	std::vector<CVertex> m_projectedPositions;

	C3DMesh m_face;
	C3DMesh m_scalp;
	std::vector<Eigen::VectorXd> m_interpolatedSamples;

	Eigen::MatrixXd A, B, D, Ai;
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
