#pragma once

#include "mCRendererTopo.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {
class CRendererTopo2D final : public CRendererTopo
{
public:
	void rebuild3DMeshesPre(const CRendererContext& ctx) override;
	void rebuild3DMeshesPost(const CRendererContext& ctx) override;
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
