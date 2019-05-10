#pragma once

#include "mCRendererTopo.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererTopo3D : public CRendererTopo
		{
		public:

			void rebuild3DMeshesPre(const IRendererContext& rContext) override;
			void rebuild3DMeshesPost(const IRendererContext& rContext) override;
		};
	} // namespace AdvancedVisualization
}  // namespace Mensia
