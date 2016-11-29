/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
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

			virtual void rebuild(const IRendererContext& rContext);
			virtual void refresh(const IRendererContext& rContext);
			virtual bool render(const IRendererContext& rContext);

			virtual void rebuild3DMeshesPre(const IRendererContext& rContext)=0; // Called before electrode projections and spherical interpolation parameters generations and might be used to load a mesh or generate a sphere for instance
			virtual void rebuild3DMeshesPost(const IRendererContext& rContext)=0; // Called after electrode projections and spherical interpolation parameters generations and might be used to unfold previously loaded mesh for instance

		private:

			void interpolate(const Eigen::VectorXd& V, Eigen::VectorXd& W, Eigen::VectorXd& Z);

		protected:

			std::vector < CVertex > m_vProjectedChannelCoordinate;

			C3DMesh m_oFace;
			C3DMesh m_oScalp;
			std::vector < Eigen::VectorXd > m_vInterpolatedSample;

			Eigen::MatrixXd A, B, D, Ai;
		};
	};
};

