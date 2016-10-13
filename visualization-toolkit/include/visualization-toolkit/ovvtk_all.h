#pragma once

#include "ovp_global_defines.h"

//___________________________________________________________________//
//                                                                   //
// Basic includes                                                    //
//___________________________________________________________________//

#include "ovvtk_defines.h"
#include "ovvtk_base.h"

//___________________________________________________________________//
//                                                                   //
// Tools                                                             //
//___________________________________________________________________//

#include "ovvtkColorGradient.h"
#include "ovvtkIVisualizationContext.h"
#include "ovvtkIVisualizationManager.h"
#include "ovvtkIVisualizationTree.h"
#include "ovvtkIVisualizationWidget.h"

namespace OpenViBEVisualizationToolkit
{
	OVVTK_API bool initialize(const OpenViBE::Kernel::IKernelContext& kernelContext);
	OVVTK_API bool uninitialize(const OpenViBE::Kernel::IKernelContext& kernelContext);
};

