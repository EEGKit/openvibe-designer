#include <mensia/advanced-visualization.hpp>

using namespace Mensia::AdvancedVisualization;

int main()
{
	auto context = IRendererContext::create();
	context->clear();

	context->setDataType(IRendererContext::DataType_Matrix);

	IRenderer* rend = IRenderer::create(ERendererType::Bitmap, false);

	rend->setChannelCount(10);
	auto* tmp = new float[666];
	rend->feed(tmp);
	rend->refresh(*context);
}
