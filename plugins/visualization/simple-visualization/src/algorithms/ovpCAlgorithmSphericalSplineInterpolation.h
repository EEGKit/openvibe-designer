#pragma once

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <array>

namespace OpenViBE {
namespace Plugins {
namespace Test {

class CAlgorithmSphericalSplineInterpolation final : public Toolkit::TAlgorithm<IAlgorithm>
{
public:

	void release() override { delete this; }

	bool initialize() override;
	bool uninitialize() override;
	bool process() override;

	_IsDerivedFromClass_Final_(Toolkit::TAlgorithm<IAlgorithm>, OVP_ClassId_Algorithm_SphericalSplineInterpolation)

protected:

	//input parameters
	//----------------
	Kernel::TParameterHandler<int64_t> ip_splineOrder;
	Kernel::TParameterHandler<int64_t> ip_nControlPoints;
	Kernel::TParameterHandler<IMatrix*> ip_controlPointsCoords;
	Kernel::TParameterHandler<IMatrix*> ip_controlPointsValues;
	Kernel::TParameterHandler<IMatrix*> ip_samplePointsCoords;

	//output parameters
	//-----------------
	Kernel::TParameterHandler<IMatrix*> op_samplePointsValues;
	Kernel::TParameterHandler<double> op_minSamplePointValue;
	Kernel::TParameterHandler<double> op_maxSamplePointValue;

	//internal data
	//-------------
	bool m_firstProcess = true;
	std::vector<double> m_coords;
	std::vector<double*> m_coordsPtr;
	std::vector<double> m_splineCoefs;
	std::vector<double> m_laplacianCoefs;
	std::array<double, 2004> m_scd{};
	std::array<double, 2004> m_pot{};
};

class CAlgorithmSphericalSplineInterpolationDesc final : public IAlgorithmDesc
{
public:

	void release() override { }

	CString getName() const override { return CString("Spherical spline interpolation"); }
	CString getAuthorName() const override { return CString("Vincent Delannoy"); }
	CString getAuthorCompanyName() const override { return CString("INRIA/IRISA"); }
	CString getShortDescription() const override { return CString("Interpolates potentials/laplacians using a spherical spline"); }
	CString getDetailedDescription() const override { return CString(""); }
	CString getCategory() const override { return CString("Algorithm/Signal processing"); }
	CString getVersion() const override { return CString("1.0"); }
	CString getSoftwareComponent() const override { return CString("openvibe-designer"); }
	CString getAddedSoftwareVersion() const override { return CString("0.0.0"); }
	CString getUpdatedSoftwareVersion() const override { return CString("0.0.0"); }

	CIdentifier getCreatedClass() const override { return OVP_ClassId_Algorithm_SphericalSplineInterpolation; }
	IPluginObject* create() override { return new CAlgorithmSphericalSplineInterpolation(); }

	bool getAlgorithmPrototype(Kernel::IAlgorithmProto& prototype) const override
	{
		//input parameters
		prototype.addInputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder, "Spline order", Kernel::ParameterType_Integer);
		prototype.addInputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount, "Number of values", Kernel::ParameterType_Integer);
		prototype.addInputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates, "Values coordinates",
			Kernel::ParameterType_Matrix);
		prototype.addInputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues, "Values", Kernel::ParameterType_Matrix);
		prototype.addInputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates, "Coordinates where to interpolate values",
			Kernel::ParameterType_Matrix);
		//input triggers
		prototype.addInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_PrecomputeTables, CString("Precomputation"));
		prototype.addInputTrigger(
			OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeSplineCoefs, CString("Spline coefficients computation"));
		prototype.addInputTrigger(
			OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeLaplacianCoefs, CString("Laplacian coefficients computation"));
		prototype.addInputTrigger(
			OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, CString("Interpolation using spline coefficients"));
		prototype.addInputTrigger(
			OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, CString("Interpolation using laplacian coefficients"));
		//output parameters
		prototype.addOutputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues, "Interpolated values", Kernel::ParameterType_Matrix);
		prototype.addOutputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MinSamplePointValue, "Min interpolated value",
			Kernel::ParameterType_Float);
		prototype.addOutputParameter(
			OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MaxSamplePointValue, "Max interpolated value",
			Kernel::ParameterType_Float);

		return true;
	}

	_IsDerivedFromClass_Final_(IAlgorithmDesc, OVP_ClassId_Algorithm_SphericalSplineInterpolationDesc)
};

}  // namespace Test
}  // namespace Plugins
}  // namespace OpenViBE
