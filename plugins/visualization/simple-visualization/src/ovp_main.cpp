#include "ovp_defines.h"

//#include "algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"
//#include "box-algorithms/ovpCTopographicMap2DDisplay.h"

#include "box-algorithms/ovpCBoxAlgorithmDisplayCueImage.h"

#include "box-algorithms/ovpCBoxAlgorithmMatrixDisplay.h"

#include "box-algorithms/ovpCBoxAlgorithmKeyboardStimulator.h"

OVP_Declare_Begin()

//	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SphericalLinearInterpolationType, "Spherical linear interpolation type");
//	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline (potentials)", OVP_TypeId_SphericalLinearInterpolationType_Spline);
//	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline laplacian (currents)",  OVP_TypeId_SphericalLinearInterpolationType_Laplacian);


//	OVP_Declare_New(OpenViBEPlugins::Test::CAlgorithmSphericalSplineInterpolationDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmDisplayCueImageDesc)

//	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CTopographicMap2DDisplayDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmMatrixDisplayDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmKeyboardStimulatorDesc)

OVP_Declare_End()
