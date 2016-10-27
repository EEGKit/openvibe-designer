#pragma once

#include "ovp_global_defines.h"

//___________________________________________________________________//
//                                                                   //
// Basic includes                                                    //
//___________________________________________________________________//

#include "ovviz_defines.h"
#include "ovviz_base.h"

//___________________________________________________________________//
//                                                                   //
// Tools                                                             //
//___________________________________________________________________//

#include "ovvizColorGradient.h"
#include "ovvizIVisualizationContext.h"
#include "ovvizIVisualizationManager.h"
#include "ovvizIVisualizationTree.h"
#include "ovvizIVisualizationWidget.h"

namespace OpenViBEVisualizationToolkit
{
	OVVIZ_API bool initialize(const OpenViBE::Kernel::IKernelContext& kernelContext);
	OVVIZ_API bool uninitialize(const OpenViBE::Kernel::IKernelContext& kernelContext);
};

