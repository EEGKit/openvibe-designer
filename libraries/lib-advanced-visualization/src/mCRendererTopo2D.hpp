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

			virtual void rebuild3DMeshesPre(const IRendererContext& rContext);
			virtual void rebuild3DMeshesPost(const IRendererContext& rContext);
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererTopo2D_H__
