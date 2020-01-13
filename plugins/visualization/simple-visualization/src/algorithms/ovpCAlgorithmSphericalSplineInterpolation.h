#pragma once

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <array>

namespace OpenViBEPlugins
{
	namespace Test
	{
		class CAlgorithmSphericalSplineInterpolation final : public OpenViBEToolkit::TAlgorithm<OpenViBE::Plugins::IAlgorithm>
		{
		public:

			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_SphericalSplineInterpolation)

		protected:

			//input parameters
			//----------------
			OpenViBE::Kernel::TParameterHandler<int64_t> ip_splineOrder;
			OpenViBE::Kernel::TParameterHandler<int64_t> ip_nControlPoints;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMatrix*> ip_controlPointsCoords;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMatrix*> ip_controlPointsValues;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMatrix*> ip_samplePointsCoords;

			//output parameters
			//-----------------
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMatrix*> op_samplePointsValues;
			OpenViBE::Kernel::TParameterHandler<double> op_minSamplePointValue;
			OpenViBE::Kernel::TParameterHandler<double> op_maxSamplePointValue;

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

		class CAlgorithmSphericalSplineInterpolationDesc final : public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Spherical spline interpolation"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Vincent Delannoy"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("INRIA/IRISA"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Interpolates potentials/laplacians using a spherical spline"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString(""); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Algorithm/Signal processing"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getSoftwareComponent() const override { return OpenViBE::CString("openvibe-designer"); }
			OpenViBE::CString getAddedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }
			OpenViBE::CString getUpdatedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_Algorithm_SphericalSplineInterpolation; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CAlgorithmSphericalSplineInterpolation(); }

			bool getAlgorithmPrototype(OpenViBE::Kernel::IAlgorithmProto& prototype) const override
			{
				//input parameters
				prototype.addInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder, "Spline order", OpenViBE::Kernel::ParameterType_Integer);
				prototype.addInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount, "Number of values", OpenViBE::Kernel::ParameterType_Integer);
				prototype.addInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates, "Values coordinates", OpenViBE::Kernel::ParameterType_Matrix);
				prototype.addInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues, "Values", OpenViBE::Kernel::ParameterType_Matrix);
				prototype.addInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates, "Coordinates where to interpolate values", OpenViBE::Kernel::ParameterType_Matrix);
				//input triggers
				prototype.addInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_PrecomputeTables, OpenViBE::CString("Precomputation"));
				prototype.addInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeSplineCoefs, OpenViBE::CString("Spline coefficients computation"));
				prototype.addInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeLaplacianCoefs, OpenViBE::CString("Laplacian coefficients computation"));
				prototype.addInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, OpenViBE::CString("Interpolation using spline coefficients"));
				prototype.addInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, OpenViBE::CString("Interpolation using laplacian coefficients"));
				//output parameters
				prototype.addOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues, "Interpolated values", OpenViBE::Kernel::ParameterType_Matrix);
				prototype.addOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MinSamplePointValue, "Min interpolated value", OpenViBE::Kernel::ParameterType_Float);
				prototype.addOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MaxSamplePointValue, "Max interpolated value", OpenViBE::Kernel::ParameterType_Float);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_SphericalSplineInterpolationDesc)
		};
	} // namespace Test
} // namespace OpenViBEPlugins
