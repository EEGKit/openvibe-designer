#include "ovpCTopographicMapDatabase.h"

#include <algorithm>

#include <cmath>

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace SimpleVisualization;

using namespace OpenViBEToolkit;

using namespace std;


CTopographicMapDatabase::CTopographicMapDatabase(TBoxAlgorithm<IBoxAlgorithm>& oPlugin, IAlgorithmProxy& rSphericalSplineInterpolation)
	: CBufferDatabase(oPlugin), m_rSphericalSplineInterpolation(rSphericalSplineInterpolation)
{
	//map input parameters
	//--------------------

	//spline order
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder)->setReferenceTarget(&m_splineOrder);
	//number of channels (or electrodes)
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount)->setReferenceTarget(&m_i64NbElectrodes);
	//matrix of pointers to electrode coordinates
	m_pElectrodeCoords = &m_electrodeCoords;
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates)->setReferenceTarget(&m_pElectrodeCoords);
	//matrix of potentials measured at each electrode
	m_pElectrodePotentials = &m_oElectrodePotentials;
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues)->setReferenceTarget(&m_pElectrodePotentials);
	//matrix holding sample coordinates mapped at runtime (its size is not known a priori and may vary)
	//

	//map output parameters
	//---------------------
	m_oMinSamplePointValue.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MinSamplePointValue));
	m_oMaxSamplePointValue.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MaxSamplePointValue));
}

void CTopographicMapDatabase::setMatrixDimensionSize(const uint32_t ui32DimensionIndex, const uint32_t ui32DimensionSize)
{
	CBufferDatabase::setMatrixDimensionSize(ui32DimensionIndex, ui32DimensionSize);

	if (ui32DimensionIndex == 0)
	{
		m_oElectrodePotentials.setDimensionCount(1);
		m_oElectrodePotentials.setDimensionSize(0, uint32_t(m_i64NbElectrodes));
	}
}

bool CTopographicMapDatabase::onChannelLocalisationBufferReceived(const uint32_t ui32ChannelLocalisationBufferIndex)
{
	CBufferDatabase::onChannelLocalisationBufferReceived(ui32ChannelLocalisationBufferIndex);

	if (!m_bChannelLookupTableInitialized || m_oChannelLocalisationStreamedCoords.empty() || m_i64NbElectrodes == 0)
	{
		m_oParentPlugin.getLogManager() << LogLevel_Warning
				<< "Channel localisation buffer received before channel lookup table was initialized! Can't process buffer!\n";
	}

	//static electrode coordinates
	if (!m_bDynamicChannelLocalisation)
	{
		//if streamed coordinates are cartesian
		if (m_bCartesianStreamedCoords)
		{
			//fill electrode coordinates matrix
			m_electrodeCoords.setDimensionCount(1);
			m_electrodeCoords.setDimensionSize(0, uint32_t(3 * m_i64NbElectrodes));
			const double* l_pCoords = m_oChannelLocalisationStreamedCoords[0].first->getBuffer();
			for (uint32_t i = 0; i < uint32_t(m_i64NbElectrodes); ++i)
			{
				const uint32_t l_ui32LookupIndex = m_oChannelLookupIndices[i];
				m_electrodeCoords[3 * i]         = *(l_pCoords + 3 * l_ui32LookupIndex);
				m_electrodeCoords[3 * i + 1]     = *(l_pCoords + 3 * l_ui32LookupIndex + 1);
				m_electrodeCoords[3 * i + 2]     = *(l_pCoords + 3 * l_ui32LookupIndex + 2);
			}

			//electrode coordinates initialized : it is now possible to interpolate potentials
			m_electrodeCoordsInitialized = true;
		}
	}

	return true;
}

void CTopographicMapDatabase::getLastBufferInterpolatedMinMaxValue(double& f64Min, double& f64Max) const
{
	f64Min = m_oMinSamplePointValue;
	f64Max = m_oMaxSamplePointValue;
}

bool CTopographicMapDatabase::processValues()
{
	//wait for electrode coordinates
	if (!m_electrodeCoordsInitialized) { return true; }

	if (m_firstProcess)
	{
		//done in CBufferDatabase::setMatrixBuffer
		//initialize the drawable object
		//m_pDrawable->init();

		if (!checkElectrodeCoordinates()) { return false; }

		//precompute sin/cos tables
		m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_PrecomputeTables, true);

		m_firstProcess = false;
	}

	//retrieve electrode values
	//determine what buffer to use from delay
	uint32_t l_ui32BufferIndex;
	const uint64_t currentTime       = m_oParentPlugin.getPlayerContext().getCurrentTime();
	const uint64_t l_ui64DisplayTime = currentTime - m_delay;
	getBufferIndexFromTime(l_ui64DisplayTime, l_ui32BufferIndex);

	//determine what sample to use
	uint64_t l_ui64SampleIndex;
	if (l_ui64DisplayTime <= m_oStartTime[l_ui32BufferIndex]) { l_ui64SampleIndex = 0; }
	else if (l_ui64DisplayTime >= m_oEndTime[l_ui32BufferIndex])
	{
		l_ui64SampleIndex = m_pDimensionSizes[1] - 1;
	}
	else
	{
		l_ui64SampleIndex = uint64_t(double(l_ui64DisplayTime - m_oStartTime[l_ui32BufferIndex]) / double(m_ui64BufferDuration) * m_pDimensionSizes[1]);
	}

	for (int64_t i = 0; i < m_i64NbElectrodes; ++i)
	{
		*(m_oElectrodePotentials.getBuffer() + i) = m_oSampleBuffers[l_ui32BufferIndex][i * m_pDimensionSizes[1] + l_ui64SampleIndex];
	}

	//interpolate spline values (potentials)
	if (m_interpolationType == OVP_TypeId_SphericalLinearInterpolationType_Spline)
	{
		m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeSplineCoefs, true);
	}
	else //interpolate spline laplacian (currents)
	{
		m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeLaplacianCoefs, true);
	}

	//retrieve up-to-date pointer to sample matrix
	m_pSamplePointCoords = dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->getSampleCoordinatesMatrix();

	if (m_pSamplePointCoords != nullptr)
	{
		//map pointer to input parameter
		m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates)->setReferenceTarget(&m_pSamplePointCoords);

		if (m_interpolationType == OVP_TypeId_SphericalLinearInterpolationType_Spline)
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, true);
		}
		else
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, true);
		}
	}

	m_rSphericalSplineInterpolation.process();
	bool l_bProcess = true;
	if (m_rSphericalSplineInterpolation.isOutputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error))
	{
		m_oParentPlugin.getLogManager() << LogLevel_Warning << "An error occurred while interpolating potentials!\n";
		l_bProcess = false;
	}
	else
	{
		if (m_pSamplePointCoords != nullptr)
		{
			//retrieve interpolation results
			TParameterHandler<IMatrix*> l_oSampleValuesMatrix;
			l_oSampleValuesMatrix.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
			dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->setSampleValuesMatrix(l_oSampleValuesMatrix);

			//tells the drawable to redraw itself since the signal information has been updated
			m_pDrawable->redraw();
		}
	}

	return l_bProcess;
}

bool CTopographicMapDatabase::setDelay(const double delay)
{
	if (delay > m_totalDuration) { return false; }

	//convert delay to 32:32 format
	m_delay = int64_t(delay * (1LL << 32)); // $$$ Casted in (int64_t) because of Ubuntu 7.10 crash !
	return true;
}

bool CTopographicMapDatabase::setInterpolationType(const uint64_t interpolationType)
{
	m_interpolationType = interpolationType;
	return true;
}

bool CTopographicMapDatabase::interpolateValues()
{
	//can't interpolate before first buffer has been received
	if (m_firstProcess) { return false; }

	//retrieve up-to-date pointer to sample matrix
	m_pSamplePointCoords = dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->getSampleCoordinatesMatrix();

	if (m_pSamplePointCoords != nullptr)
	{
		//map pointer to input parameter
		m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates)->setReferenceTarget(&m_pSamplePointCoords);

		//interpolate using spline or laplacian coefficients depending on interpolation mode
		if (m_interpolationType == OVP_TypeId_SphericalLinearInterpolationType_Spline)
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, true);
		}
		else
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, true);
		}
	}

	m_rSphericalSplineInterpolation.process();

	if (m_rSphericalSplineInterpolation.isOutputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error))
	{
		m_oParentPlugin.getLogManager() << LogLevel_Warning << "An error occurred while interpolating potentials!\n";
	}
	else
	{
		if (m_pSamplePointCoords != nullptr)
		{
			//retrieve interpolation results
			TParameterHandler<IMatrix*> l_oSampleValuesMatrix;
			l_oSampleValuesMatrix.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
			dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->setSampleValuesMatrix(l_oSampleValuesMatrix);
		}
	}

	return true;
}

bool CTopographicMapDatabase::getBufferIndexFromTime(const uint64_t ui64Time, uint32_t& rBufferIndex)
{
	if (m_oSampleBuffers.empty()) { return false; }

	if (ui64Time < m_oStartTime[0])
	{
		rBufferIndex = 0;
		return false;
	}
	if (ui64Time > m_oEndTime.back())
	{
		rBufferIndex = uint32_t(m_oSampleBuffers.size() - 1);
		return false;
	}
	for (uint32_t i = 0; i < m_oSampleBuffers.size(); ++i)
	{
		if (ui64Time <= m_oEndTime[i])
		{
			rBufferIndex = i;
			break;
		}
	}

	return true;
}

bool CTopographicMapDatabase::checkElectrodeCoordinates()
{
	const uint64_t channelCount = getChannelCount();

	for (uint32_t i = 0; i < channelCount; ++i)
	{
		double* l_pNormalizedChannelCoords = nullptr;
		if (!getChannelPosition(i, l_pNormalizedChannelCoords))
		{
			CString l_sChannelLabel;
			getChannelLabel(i, l_sChannelLabel);
			m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager()
					<< LogLevel_Error
					<< "Couldn't retrieve coordinates of electrode #" << i
					<< "(" << l_sChannelLabel << "), aborting model frame electrode coordinates computation\n";
			return false;
		}

#define MY_THRESHOLD 0.01
		if (fabs(l_pNormalizedChannelCoords[0] * l_pNormalizedChannelCoords[0] +
				 l_pNormalizedChannelCoords[1] * l_pNormalizedChannelCoords[1] +
				 l_pNormalizedChannelCoords[2] * l_pNormalizedChannelCoords[2] - 1.) > MY_THRESHOLD)
#undef MY_THRESHOLD
		{
			CString l_sChannelLabel;
			getChannelLabel(i, l_sChannelLabel);
			m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager()
					<< LogLevel_Error
					<< "Coordinates of electrode #" << i
					<< "(" << l_sChannelLabel << "), are not normalized, aborting model frame electrode coordinates computation\n";
			return false;
		}
	}

	return true;
}
