#ifndef __Mensia_AdvancedVisualization_CRendererTopo2D_H__
#define __Mensia_AdvancedVisualization_CRendererTopo2D_H__

#include "mCRendererTopo.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererTopo2D : public CRendererTopo
		{
		public:

			void rebuild3DMeshesPre(const IRendererContext& rContext) override;
			void rebuild3DMeshesPost(const IRendererContext& rContext) override;
		};
	} // namespace AdvancedVisualization
}  // namespace Mensia

#endif // __Mensia_AdvancedVisualization_CRendererTopo2D_H__
