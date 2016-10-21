#include <cstdio>

#include "visualization-toolkit/ovvtk_all.h"
#include "ovvtkCVisualizationContext.hpp"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;

namespace
{
	OpenViBEVisualizationToolkit::CVisualizationContextDesc visualisationContextDesc;
}

bool OpenViBEVisualizationToolkit::initialize(const IKernelContext& kernelContext)
{
	ITypeManager& typeManager = kernelContext.getTypeManager();
	IPluginManager& pluginManager = kernelContext.getPluginManager();

	typeManager.registerType(OV_TypeId_Color, "Color");
	typeManager.registerType(OV_TypeId_ColorGradient, "Color Gradient");

	pluginManager.registerPluginDesc(visualisationContextDesc);

	return true;
}

bool OpenViBEVisualizationToolkit::uninitialize(const IKernelContext& kernelContext)
{
	(void)kernelContext;
	return true;
}
