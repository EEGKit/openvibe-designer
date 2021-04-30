#include <mensia/advanced-visualization.hpp>

namespace OAV = OpenViBE::AdvancedVisualization;

int main()
{
	OAV::CRendererContext context;
	context.clear();

	context.setDataType(OAV::CRendererContext::EDataType::Matrix);

	OAV::IRenderer* rend = OAV::IRenderer::create(OAV::ERendererType::Bitmap, false);

	rend->setChannelCount(10);
	auto* tmp = new float[666];
	rend->feed(tmp);
	rend->refresh(context);
}
