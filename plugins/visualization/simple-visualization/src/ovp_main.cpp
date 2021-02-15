#include "ovp_defines.h"

#include "algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"

#include "box-algorithms/ovpCBoxAlgorithmTopographicMap2DDisplay.h"
#include "box-algorithms/ovpCBoxAlgorithmMatrixDisplay.h"

OVP_Declare_Begin()
	context.getTypeManager().registerEnumerationType(OVP_TypeId_SphericalLinearInterpolationType, "Spherical linear interpolation type");
	context.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline (potentials)", size_t(EInterpolationType::Spline));
	context.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline laplacian (currents)",
													  size_t(EInterpolationType::Laplacian));

	OVP_Declare_New(OpenViBE::Plugins::Test::CAlgorithmSphericalSplineInterpolationDesc)

	OVP_Declare_New(OpenViBE::Plugins::SimpleVisualization::CBoxAlgorithmTopographicMap2DDisplayDesc)
	OVP_Declare_New(OpenViBE::Plugins::SimpleVisualization::CBoxAlgorithmMatrixDisplayDesc)


OVP_Declare_End()
