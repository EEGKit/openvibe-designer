#include <mensia/advanced-visualization.hpp>

using namespace Mensia::AdvancedVisualization;

int main()
{
	CRendererContext context;
	context.clear();

	context.setDataType(CRendererContext::DataType_Matrix);

	IRenderer* rend = IRenderer::create(ERendererType::Bitmap, false);

	rend->setChannelCount(10);
	auto* tmp = new float[666];
	rend->feed(tmp);
	rend->refresh(context);
}
