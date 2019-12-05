#include "ovpCTopographicMapDatabase.h"

#include <cmath>

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace SimpleVisualization;

using namespace OpenViBEToolkit;

using namespace std;


CTopographicMapDatabase::CTopographicMapDatabase(TBoxAlgorithm<IBoxAlgorithm>& plugin, IAlgorithmProxy& interpolation)
	: CBufferDatabase(plugin), m_interpolation(interpolation)
{
	//map input parameters
	//--------------------

	//spline order
	m_interpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder)->setReferenceTarget(&m_splineOrder);
	//number of channels (or electrodes)
	m_interpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount)->setReferenceTarget(&m_NElectrodes);
	//matrix of pointers to electrode coordinates
	m_pElectrodeCoords = &m_electrodeCoords;
	m_interpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates)->setReferenceTarget(&m_pElectrodeCoords);
	//matrix of potentials measured at each electrode
	m_pElectrodePotentials = &m_electrodePotentials;
	m_interpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues)->setReferenceTarget(&m_pElectrodePotentials);
	//matrix holding sample coordinates mapped at runtime (its size is not known a priori and may vary)
	//

	//map output parameters
	//---------------------
	m_minSamplePointValue.initialize(m_interpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MinSamplePointValue));
	m_maxSamplePointValue.initialize(m_interpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MaxSamplePointValue));
}

void CTopographicMapDatabase::setMatrixDimensionSize(const size_t index, const size_t size)
{
	CBufferDatabase::setMatrixDimensionSize(index, size);

	if (index == 0)
	{
		m_electrodePotentials.setDimensionCount(1);
		m_electrodePotentials.setDimensionSize(0, size_t(m_NElectrodes));
	}
}

bool CTopographicMapDatabase::onChannelLocalisationBufferReceived(const size_t bufferIndex)
{
	CBufferDatabase::onChannelLocalisationBufferReceived(bufferIndex);

	if (!m_ChannelLookupTableInitialized || m_channelLocalisationCoords.empty() || m_NElectrodes == 0)
	{
		m_ParentPlugin.getLogManager() << LogLevel_Warning
				<< "Channel localisation buffer received before channel lookup table was initialized! Can't process buffer!\n";
	}

	//static electrode coordinates
	if (!m_dynamicChannelLocalisation)
	{
		//if streamed coordinates are cartesian
		if (m_cartesianCoords)
		{
			//fill electrode coordinates matrix
			m_electrodeCoords.setDimensionCount(1);
			m_electrodeCoords.setDimensionSize(0, size_t(3 * m_NElectrodes));
			const double* coords = m_channelLocalisationCoords[0].first->getBuffer();
			for (size_t i = 0; i < size_t(m_NElectrodes); ++i)
			{
				const size_t lookupIdx       = m_ChannelLookupIndices[i];
				m_electrodeCoords[3 * i]     = *(coords + 3 * lookupIdx);
				m_electrodeCoords[3 * i + 1] = *(coords + 3 * lookupIdx + 1);
				m_electrodeCoords[3 * i + 2] = *(coords + 3 * lookupIdx + 2);
			}

			//electrode coordinates initialized : it is now possible to interpolate potentials
			m_electrodeCoordsInitialized = true;
		}
	}

	return true;
}

void CTopographicMapDatabase::getLastBufferInterpolatedMinMaxValue(double& min, double& max) const
{
	min = m_minSamplePointValue;
	max = m_maxSamplePointValue;
}

bool CTopographicMapDatabase::processValues()
{
	//wait for electrode coordinates
	if (!m_electrodeCoordsInitialized) { return true; }

	if (m_firstProcess)
	{
		//done in CBufferDatabase::setMatrixBuffer
		//initialize the drawable object
		//m_Drawable->init();

		if (!checkElectrodeCoordinates()) { return false; }

		//precompute sin/cos tables
		m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_PrecomputeTables, true);

		m_firstProcess = false;
	}

	//retrieve electrode values
	//determine what buffer to use from delay
	size_t bufferIdx           = 0;
	const uint64_t currentTime = m_ParentPlugin.getPlayerContext().getCurrentTime();
	const uint64_t displayTime = currentTime - m_delay;
	getBufferIndexFromTime(displayTime, bufferIdx);

	//determine what sample to use
	size_t sampleIdx;
	if (displayTime <= m_StartTime[bufferIdx]) { sampleIdx = 0; }
	else if (displayTime >= m_EndTime[bufferIdx]) { sampleIdx = m_DimSizes[1] - 1; }
	else { sampleIdx = size_t(double(displayTime - m_StartTime[bufferIdx]) / double(m_BufferDuration) * m_DimSizes[1]); }

	for (int64_t i = 0; i < m_NElectrodes; ++i) { *(m_electrodePotentials.getBuffer() + i) = m_SampleBuffers[bufferIdx][i * m_DimSizes[1] + sampleIdx]; }

	//interpolate spline values (potentials)
	if (m_interpolationType == ESphericalLinearInterpolationType::Spline)
	{
		m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeSplineCoefs, true);
	}
	else //interpolate spline laplacian (currents)
	{
		m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeLaplacianCoefs, true);
	}

	//retrieve up-to-date pointer to sample matrix
	m_samplePointCoords = dynamic_cast<CTopographicMapDrawable*>(m_Drawable)->getSampleCoordinatesMatrix();

	if (m_samplePointCoords != nullptr)
	{
		//map pointer to input parameter
		m_interpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates)->
						setReferenceTarget(&m_samplePointCoords);

		if (m_interpolationType == ESphericalLinearInterpolationType::Spline)
		{
			m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, true);
		}
		else { m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, true); }
	}

	m_interpolation.process();
	bool process = true;
	if (m_interpolation.isOutputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error))
	{
		m_ParentPlugin.getLogManager() << LogLevel_Warning << "An error occurred while interpolating potentials!\n";
		process = false;
	}
	else
	{
		if (m_samplePointCoords != nullptr)
		{
			//retrieve interpolation results
			TParameterHandler<IMatrix*> sampleValuesMatrix;
			sampleValuesMatrix.initialize(
				m_interpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
			dynamic_cast<CTopographicMapDrawable*>(m_Drawable)->setSampleValuesMatrix(sampleValuesMatrix);

			//tells the drawable to redraw itself since the signal information has been updated
			m_Drawable->redraw();
		}
	}

	return process;
}

bool CTopographicMapDatabase::setDelay(const double delay)
{
	if (delay > m_TotalDuration) { return false; }

	//convert delay to 32:32 format
	m_delay = int64_t(delay * (1LL << 32)); // $$$ Casted in (int64_t) because of Ubuntu 7.10 crash !
	return true;
}

bool CTopographicMapDatabase::setInterpolationType(const uint64_t type)
{
	m_interpolationType = type;
	return true;
}

bool CTopographicMapDatabase::interpolateValues()
{
	//can't interpolate before first buffer has been received
	if (m_firstProcess) { return false; }

	//retrieve up-to-date pointer to sample matrix
	m_samplePointCoords = dynamic_cast<CTopographicMapDrawable*>(m_Drawable)->getSampleCoordinatesMatrix();

	if (m_samplePointCoords != nullptr)
	{
		//map pointer to input parameter
		m_interpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates)->
						setReferenceTarget(&m_samplePointCoords);

		//interpolate using spline or laplacian coefficients depending on interpolation mode
		if (m_interpolationType == ESphericalLinearInterpolationType::Spline)
		{
			m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, true);
		}
		else { m_interpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, true); }
	}

	m_interpolation.process();

	if (m_interpolation.isOutputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error))
	{
		m_ParentPlugin.getLogManager() << LogLevel_Warning << "An error occurred while interpolating potentials!\n";
	}
	else
	{
		if (m_samplePointCoords != nullptr)
		{
			//retrieve interpolation results
			TParameterHandler<IMatrix*> sampleValuesMatrix;
			sampleValuesMatrix.initialize(m_interpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
			dynamic_cast<CTopographicMapDrawable*>(m_Drawable)->setSampleValuesMatrix(sampleValuesMatrix);
		}
	}

	return true;
}

bool CTopographicMapDatabase::getBufferIndexFromTime(const uint64_t time, size_t& bufferIndex)
{
	if (m_SampleBuffers.empty()) { return false; }

	if (time < m_StartTime[0])
	{
		bufferIndex = 0;
		return false;
	}
	if (time > m_EndTime.back())
	{
		bufferIndex = size_t(m_SampleBuffers.size() - 1);
		return false;
	}
	for (size_t i = 0; i < m_SampleBuffers.size(); ++i)
	{
		if (time <= m_EndTime[i])
		{
			bufferIndex = i;
			break;
		}
	}

	return true;
}

bool CTopographicMapDatabase::checkElectrodeCoordinates()
{
	const size_t nChannel = getChannelCount();

	for (size_t i = 0; i < nChannel; ++i)
	{
		double* normalizedChannelCoords = nullptr;
		if (!getChannelPosition(i, normalizedChannelCoords))
		{
			CString channelLabel;
			getChannelLabel(i, channelLabel);
			m_ParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error
					<< "Couldn't retrieve coordinates of electrode #" << i
					<< "(" << channelLabel << "), aborting model frame electrode coordinates computation\n";
			return false;
		}

#define MY_THRESHOLD 0.01
		if (fabs(normalizedChannelCoords[0] * normalizedChannelCoords[0] +
				 normalizedChannelCoords[1] * normalizedChannelCoords[1] +
				 normalizedChannelCoords[2] * normalizedChannelCoords[2] - 1.) > MY_THRESHOLD)
#undef MY_THRESHOLD
		{
			CString channelLabel;
			getChannelLabel(i, channelLabel);
			m_ParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error 
					<< "Coordinates of electrode #" << i << "(" << channelLabel 
					<< "), are not normalized, aborting model frame electrode coordinates computation\n";
			return false;
		}
	}

	return true;
}
