#include <mensia/advanced-visualization.h>

using namespace Mensia::AdvancedVisualization;

int main()
{
	auto m_pRendererContext = IRendererContext::create();
	m_pRendererContext->clear();

	m_pRendererContext->setDataType(IRendererContext::DataType_Matrix);

	IRenderer* rend = IRenderer::create(IRenderer::RendererType_Bitmap, false);

	rend->setChannelCount(10);
	auto* tmp = new float[666];
	rend->feed(tmp);
	rend->refresh(*m_pRendererContext);
}
