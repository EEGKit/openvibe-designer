#include "visualization-toolkit/ovviz_all.h"
#include "ovvizCVisualizationContext.hpp"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;

namespace
{
	OpenViBEVisualizationToolkit::CVisualizationContextDesc visualizationContextDesc;
} // namespace

bool OpenViBEVisualizationToolkit::initialize(const IKernelContext& ctx)
{
	ITypeManager& typeManager     = ctx.getTypeManager();
	IPluginManager& pluginManager = ctx.getPluginManager();

	typeManager.registerType(OV_TypeId_Color, "Color");
	typeManager.registerType(OV_TypeId_ColorGradient, "Color Gradient");

	pluginManager.registerPluginDesc(visualizationContextDesc);

	return true;
}

bool OpenViBEVisualizationToolkit::uninitialize(const IKernelContext& /*ctx*/) { return true; }
