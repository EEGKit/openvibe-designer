#include "ovp_defines.h"

#include "algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"

#include "box-algorithms/ovpCBoxAlgorithmTopographicMap2DDisplay.h"
#include "box-algorithms/ovpCBoxAlgorithmMatrixDisplay.h"


OVP_Declare_Begin()
	context.getTypeManager().registerEnumerationType(OVP_TypeId_SphericalLinearInterpolationType, "Spherical linear interpolation type");
	context.getTypeManager().registerEnumerationEntry(
		OVP_TypeId_SphericalLinearInterpolationType, "Spline (potentials)", OVP_TypeId_SphericalLinearInterpolationType_Spline);
	context.getTypeManager().registerEnumerationEntry(
		OVP_TypeId_SphericalLinearInterpolationType, "Spline laplacian (currents)", OVP_TypeId_SphericalLinearInterpolationType_Laplacian);


	OVP_Declare_New(OpenViBEPlugins::Test::CAlgorithmSphericalSplineInterpolationDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmTopographicMap2DDisplayDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmMatrixDisplayDesc)


OVP_Declare_End()
