#include "ovpCAlgorithmSphericalSplineInterpolation.h"

//INSERM lib
#include "spline_sph.h"

#include <cfloat> //DBL_MAX
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace Test;

bool CAlgorithmSphericalSplineInterpolation::initialize()

{
	m_bFirstProcess         = true;
	m_pDoubleCoords         = nullptr;
	m_pInsermCoords         = nullptr;
	m_pSplineCoefs          = nullptr;
	m_pLaplacianSplineCoefs = nullptr;

	ip_splineOrder.initialize(getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder));
	ip_controlPointsCount.initialize(getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount));
	ip_controlPointsCoords.initialize(getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates));
	ip_controlPointsValues.initialize(getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues));
	ip_samplePointsCoords.initialize(getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates));

	op_samplePointsValues.initialize(getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
	op_samplePointsValues->setDimensionCount(1);
	op_minSamplePointValue.initialize(getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MinSamplePointValue));
	op_maxSamplePointValue.initialize(getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MaxSamplePointValue));

	return true;
}

bool CAlgorithmSphericalSplineInterpolation::uninitialize()

{
	ip_splineOrder.uninitialize();
	ip_controlPointsCount.uninitialize();
	ip_controlPointsCoords.uninitialize();
	ip_controlPointsValues.uninitialize();
	ip_samplePointsCoords.uninitialize();

	op_samplePointsValues.uninitialize();
	op_minSamplePointValue.uninitialize();
	op_maxSamplePointValue.uninitialize();

	delete[] m_pDoubleCoords;
	delete[] m_pInsermCoords;
	delete[] m_pSplineCoefs;
	delete[] m_pLaplacianSplineCoefs;

	return true;
}

bool CAlgorithmSphericalSplineInterpolation::process()

{
	if (m_bFirstProcess)
	{
		//store coords as doubles
		m_pDoubleCoords = new double[size_t(3 * ip_controlPointsCount)];
		//set up matrix of pointers to double coords matrix
		m_pInsermCoords = new double* [size_t(ip_controlPointsCount)];
		//fill both matrices
		for (int i = 0; i < int(ip_controlPointsCount); ++i)
		{
			const uint32_t id       = 3 * i;
			m_pDoubleCoords[id]     = double((*ip_controlPointsCoords)[id]);
			m_pDoubleCoords[id + 1] = double((*ip_controlPointsCoords)[id + 1]);
			m_pDoubleCoords[id + 2] = double((*ip_controlPointsCoords)[id + 2]);
			m_pInsermCoords[i]      = id + m_pDoubleCoords;
		}
		m_bFirstProcess = false;
	}

	//do we want to precompute tables?
	if (isInputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_PrecomputeTables))
	{
		//compute cos/sin values used in spline polynomias
		const int result = spline_tables(int(ip_splineOrder), m_PotTable, m_ScdTable);

		if (result != 0)
		{
			getLogManager() << LogLevel_ImportantWarning << "Spline tables precomputation failed!\n";
			activateOutputTrigger(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error, true);
		}
	}

	if (isInputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeSplineCoefs))
	{
		if (m_pSplineCoefs == nullptr && int(ip_controlPointsCount) != 0)
		{
			m_pSplineCoefs = new double[int(ip_controlPointsCount) + 1];
		}

		//compute spline ponderation coefficients using spline values
		//FIXME : have a working copy of control points values stored as doubles?
		const int result = spline_coef(int(ip_controlPointsCount), m_pInsermCoords, static_cast<double*>(ip_controlPointsValues->getBuffer()), m_PotTable, m_pSplineCoefs);

		if (result != 0)
		{
			getLogManager() << LogLevel_ImportantWarning << "Spline coefficients computation failed!\n";

			const ELogLevel l_eLogLevel = LogLevel_Debug;

			getLogManager() << l_eLogLevel << "CtrlPointsCount = " << int(ip_controlPointsCount) << "\n";
			const auto size = size_t(ip_controlPointsCount);
			char buf[1024];
			sprintf(buf, "CtrlPointsCoords= ");
			for (size_t i = 0; i < size; ++i)
			{
				sprintf(buf + strlen(buf), "[%.1f %.1f %.1f] ", float(m_pInsermCoords[i][0]), float(m_pInsermCoords[i][1]), float(m_pInsermCoords[i][2]));
			}
			sprintf(buf + strlen(buf), "\n");
			getLogManager() << l_eLogLevel << buf;

			sprintf(buf, "CtrlPointsValues= ");
			for (size_t i = 0; i < size; ++i)
			{
				sprintf(buf + strlen(buf), "%.1f ", float(* (static_cast<double*>(ip_controlPointsValues->getBuffer()) + i)));
			}
			sprintf(buf + strlen(buf), "\n");
			getLogManager() << l_eLogLevel << buf;

			sprintf(buf, "Spline Coeffs   = ");
			for (size_t i = 0; i <= size; ++i)
			{
				sprintf(buf + strlen(buf), "%.1f ", float(m_pSplineCoefs[i]));
			}
			sprintf(buf + strlen(buf), "\n");
			getLogManager() << l_eLogLevel << buf;

			sprintf(buf, "PotTable coeffs = ");
			for (size_t i = 0; i < 10; ++i)
			{
				sprintf(buf + strlen(buf), "%.1f ", float(m_PotTable[i]));
			}
			sprintf(buf + strlen(buf), " ... ");
			for (size_t i = 2001; i < 2004; ++i)
			{
				sprintf(buf + strlen(buf), "%.1f ", float(m_PotTable[i]));
			}
			sprintf(buf + strlen(buf), "\n");
			getLogManager() << l_eLogLevel << buf;

			activateOutputTrigger(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error, true);
		}
	}

	if (isInputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeLaplacianCoefs))
	{
		if (m_pLaplacianSplineCoefs == nullptr && int(ip_controlPointsCount) != 0)
		{
			m_pLaplacianSplineCoefs = new double[int(ip_controlPointsCount) + 1];
		}

		//compute spline ponderation coefficients using spline values
		//FIXME : have a working copy of control points values stored as doubles?
		const int result                                    = spline_coef(int(ip_controlPointsCount), m_pInsermCoords, static_cast<double*>(ip_controlPointsValues->getBuffer()), m_PotTable, m_pLaplacianSplineCoefs);
		m_pLaplacianSplineCoefs[int(ip_controlPointsCount)] = 0;

		if (result != 0)
		{
			getLogManager() << LogLevel_ImportantWarning << "Laplacian coefficients computation failed!\n";
			activateOutputTrigger(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error, true);
		}
	}

	if (isInputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline))
	{
		bool ok = true;

		//ensure we got enough storage space for interpolated values
		if (op_samplePointsValues->getDimensionSize(0) != ip_samplePointsCoords->getDimensionSize(0))
		{
			op_samplePointsValues->setDimensionSize(0, ip_samplePointsCoords->getDimensionSize(0));
		}

		//compute interpolated values using spline
		double* sampleValue = static_cast<double*>(op_samplePointsValues->getBuffer());

		op_minSamplePointValue = +DBL_MAX;
		op_maxSamplePointValue = -DBL_MAX;

		for (uint32_t i = 0; i < ip_samplePointsCoords->getDimensionSize(0); i++, sampleValue++)
		{
#if defined TARGET_OS_Windows
#ifndef NDEBUG
			if (_finite(*(ip_samplePointsCoords->getBuffer() + 3 * i)) == 0 ||
				_finite(*(ip_samplePointsCoords->getBuffer() + 3 * i + 1)) == 0 ||
				_finite(*(ip_samplePointsCoords->getBuffer() + 3 * i + 2)) == 0) //tests whether a double is infinite or a NaN
			{
				getLogManager() << LogLevel_ImportantWarning << "Bad interpolation point coordinates !\n";
				getLogManager() << LogLevel_ImportantWarning << *(ip_samplePointsCoords->getBuffer() + 3 * i) << "\n";
				getLogManager() << LogLevel_ImportantWarning << *(ip_samplePointsCoords->getBuffer() + 3 * i + 1) << "\n";
				getLogManager() << LogLevel_ImportantWarning << *(ip_samplePointsCoords->getBuffer() + 3 * i + 2) << "\n";
				ok = false;
			}
#endif
#endif

			* sampleValue = spline_interp(int(ip_controlPointsCount), //number of fixed values
										  m_pInsermCoords, //coordinates of fixed values
										  m_PotTable, //sin/cos table for spline
										  m_pSplineCoefs, //spline coefficients
										  *(ip_samplePointsCoords->getBuffer() + 3 * i),
										  *(ip_samplePointsCoords->getBuffer() + 3 * i + 1),
										  *(ip_samplePointsCoords->getBuffer() + 3 * i + 2) //coordinate where to interpolate
										 );

#if defined TARGET_OS_Windows
#ifndef NDEBUG
			if (_finite(*sampleValue) == 0) //tests whether a double is infinite or a NaN
			{
				getLogManager() << LogLevel_ImportantWarning << "Interpolation fails !\n";
				getLogManager() << LogLevel_ImportantWarning << *(ip_samplePointsCoords->getBuffer() + 3 * i) << "\n";
				getLogManager() << LogLevel_ImportantWarning << *(ip_samplePointsCoords->getBuffer() + 3 * i + 1) << "\n";
				getLogManager() << LogLevel_ImportantWarning << *(ip_samplePointsCoords->getBuffer() + 3 * i + 2) << "\n";
				ok = false;
				break;
			}
#endif
#endif

			if (*sampleValue < op_minSamplePointValue) { op_minSamplePointValue = *sampleValue; }
			if (*sampleValue > op_maxSamplePointValue) { op_maxSamplePointValue = *sampleValue; }
		}

		if (!ok) { activateOutputTrigger(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error, true); }
	}
	else if (isInputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian))
	{
		const bool ok = true;
		//ensure we got enough storage space for interpolated values
		if (op_samplePointsValues->getDimensionSize(0) != ip_samplePointsCoords->getDimensionSize(0))
		{
			op_samplePointsValues->setDimensionSize(0, ip_samplePointsCoords->getDimensionSize(0));
		}

		//compute interpolated values using spline
		auto* sampleValue = static_cast<double*>(op_samplePointsValues->getBuffer());

		op_minSamplePointValue = +DBL_MAX;
		op_maxSamplePointValue = -DBL_MAX;

		for (uint32_t i = 0; i < ip_samplePointsCoords->getDimensionSize(0); i++, sampleValue++)
		{
			*sampleValue = spline_interp(int(ip_controlPointsCount), //number of fixed values
										 m_pInsermCoords, //coordinates of fixed values
										 m_ScdTable, //sin/cos table for laplacian
										 m_pLaplacianSplineCoefs, //laplacian coefficients
										 *(ip_samplePointsCoords->getBuffer() + 3 * i),
										 *(ip_samplePointsCoords->getBuffer() + 3 * i + 1),
										 *(ip_samplePointsCoords->getBuffer() + 3 * i + 2)); //coordinate where to interpolate

			/***************************************************************************/
			/*** Units : potential values being very often expressed as micro-Volts  ***/
			/***         SCD values should be multiplied by a scaling factor         ***/
			/***         to get nano-Amperes/m3                                      ***/
			/***         Since the output of spline_interp corresponds to the        ***/
			/***         Laplacian operator only, SCD are obtained with a scaling    ***/
			/***         factor equal to 10-3 * sigma / (R*R)                        ***/
			/***         with sigma = conductivity of the scalp    = 0.45 Siemens/m  ***/
			/***         and  R     = radius of the spherical head = 0.09 m          ***/
			/***************************************************************************/
			* sampleValue = *sampleValue * (0.001 * 0.45 / 0.09 / 0.09);

			if (*sampleValue < op_minSamplePointValue) { op_minSamplePointValue = *sampleValue; }
			if (*sampleValue > op_maxSamplePointValue) { op_maxSamplePointValue = *sampleValue; }
		}

		if (!ok) { activateOutputTrigger(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error, true); }
	}

	return true;
}
