#pragma once

#include "mCRendererTopo.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererTopo2D : public CRendererTopo
		{
		public:

			void rebuild3DMeshesPre(const IRendererContext& ctx) override;
			void rebuild3DMeshesPost(const IRendererContext& ctx) override;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
