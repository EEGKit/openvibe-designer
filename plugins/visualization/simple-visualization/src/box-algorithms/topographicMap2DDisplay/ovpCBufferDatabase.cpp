#include "ovpCBufferDatabase.h"

#include <system/ovCMemory.h>
#include <openvibe/ovTimeArithmetics.h>

#include <algorithm>
#include <cmath>
#include <cstring>

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace SimpleVisualization;

using namespace OpenViBEToolkit;

using namespace std;

CBufferDatabase::CBufferDatabase(TBoxAlgorithm<IBoxAlgorithm>& oPlugin)
	: m_parentPlugin(oPlugin), m_oDisplayMode(OVP_TypeId_SignalDisplayMode_Scan)
{
	m_pChannelLocalisationStreamDecoder = &m_parentPlugin.getAlgorithmManager().getAlgorithm(
		m_parentPlugin.getAlgorithmManager().createAlgorithm(
			OVP_GD_ClassId_Algorithm_ChannelLocalisationStreamDecoder));

	m_pChannelLocalisationStreamDecoder->initialize();

	m_pDimensionSizes[0] = m_pDimensionSizes[1] = 0;
}

CBufferDatabase::~CBufferDatabase()
{
	m_pChannelLocalisationStreamDecoder->uninitialize();
	m_parentPlugin.getAlgorithmManager().releaseAlgorithm(*m_pChannelLocalisationStreamDecoder);

	//delete all the remaining buffers
	while (!m_oSampleBuffers.empty())
	{
		delete[] m_oSampleBuffers.front();
		m_oSampleBuffers.pop_front();
	}

	//delete channel localisation matrices
	while (!m_oChannelLocalisationStreamedCoords.empty())
	{
		delete m_oChannelLocalisationStreamedCoords.front().first;
		m_oChannelLocalisationStreamedCoords.pop_front();
	}

	/*while(m_oChannelLocalisationAlternateCoords.size() > 0)
	{
		delete[] m_oChannelLocalisationAlternateCoords.front().first;
		m_oChannelLocalisationAlternateCoords.pop_front();
	}*/
}

bool CBufferDatabase::decodeChannelLocalisationMemoryBuffer(const IMemoryBuffer* pMemoryBuffer, uint64_t ui64StartTime, uint64_t ui64EndTime)
{
	//feed memory buffer to decoder
	m_pChannelLocalisationStreamDecoder->getInputParameter(OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_InputParameterId_MemoryBufferToDecode)->
										 setReferenceTarget(&pMemoryBuffer);

	//process buffer
	m_pChannelLocalisationStreamDecoder->process();

	//copy header if needed
	if (m_pChannelLocalisationStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputTriggerId_ReceivedHeader))
	{
		//retrieve matrix header
		TParameterHandler<IMatrix*> l_oMatrix;
		l_oMatrix.initialize(
			m_pChannelLocalisationStreamDecoder->getOutputParameter(OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputParameterId_Matrix));

		//copy channel labels
		m_oChannelLocalisationLabels.resize(l_oMatrix->getDimensionSize(0));
		for (vector<CString>::size_type i = 0; i < m_oChannelLocalisationLabels.size(); ++i)
		{
			m_oChannelLocalisationLabels[i] = l_oMatrix->getDimensionLabel(0, uint32_t(i));
		}

		//retrieve dynamic flag
		TParameterHandler<bool> l_bDynamic;
		l_bDynamic.initialize(
			m_pChannelLocalisationStreamDecoder->getOutputParameter(OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputParameterId_Dynamic));
		m_bDynamicChannelLocalisation = l_bDynamic;

		if (l_oMatrix->getDimensionSize(1) == 3)
		{
			m_bCartesianStreamedCoords = true;
			/*m_pChannelLocalisationCartesianCoords = &m_oChannelLocalisationStreamedCoords;
			m_pChannelLocalisationSphericalCoords = &m_oChannelLocalisationAlternateCoords;*/
		}
		else if (l_oMatrix->getDimensionSize(1) == 2)
		{
			m_bCartesianStreamedCoords = false;
			/*m_pChannelLocalisationCartesianCoords = &m_oChannelLocalisationAlternateCoords;
			m_pChannelLocalisationSphericalCoords = &m_oChannelLocalisationStreamedCoords;*/
		}
		else
		{
			m_parentPlugin.getLogManager() << LogLevel_Error
					<< "Wrong size found for dimension 1 of Channel localisation header! Can't process header!\n";
			return false;
		}

		//header information received
		m_bChannelLocalisationHeaderReceived = true;
	}

	//has a chanloc buffer been received?
	if (m_pChannelLocalisationStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputTriggerId_ReceivedBuffer))
	{
		//number of buffers required to cover displayed time range
		uint64_t l_ui64MaxBufferCount = 1;

		//resize channel localisation queue if necessary
		if (m_bDynamicChannelLocalisation)
		{
			const uint64_t l_ui64BufferDuration = ui64EndTime - ui64StartTime;
			if (l_ui64BufferDuration != 0)
			{
				l_ui64MaxBufferCount = uint64_t(ceil(m_totalDuration / l_ui64BufferDuration));
				if (l_ui64MaxBufferCount == 0) { l_ui64MaxBufferCount = 1; }
			}

			//if new number of buffers decreased, resize list and destroy useless buffers
			while (m_oChannelLocalisationStreamedCoords.size() > l_ui64MaxBufferCount)
			{
				delete[] m_oChannelLocalisationStreamedCoords.front().first;
				m_oChannelLocalisationStreamedCoords.pop_front();
				// delete[] m_oChannelLocalisationAlternateCoords.front().first;
				// m_oChannelLocalisationAlternateCoords.pop_front();
				m_oChannelLocalisationTimes.pop_front();
			}
		}

		//retrieve coordinates matrix
		TParameterHandler<IMatrix*> l_oMatrix;
		l_oMatrix.initialize(
			m_pChannelLocalisationStreamDecoder->getOutputParameter(OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputParameterId_Matrix));

		//get pointer to destination matrix
		CMatrix* l_pChannelLocalisation = nullptr;
		//CMatrix* l_pAlternateChannelLocalisation = nullptr;
		if (m_oChannelLocalisationStreamedCoords.size() < l_ui64MaxBufferCount)
		{
			//create a new matrix and resize it
			l_pChannelLocalisation = new CMatrix();
			Tools::Matrix::copyDescription(*l_pChannelLocalisation, *l_oMatrix);
			// l_pAlternateChannelLocalisation = new CMatrix();
			// TODO : resize it appropriately depending on whether it is spherical or cartesian
		}
		else //m_oChannelLocalisationStreamedCoords.size() == l_ui64MaxBufferCount
		{
			l_pChannelLocalisation = m_oChannelLocalisationStreamedCoords.front().first;
			m_oChannelLocalisationStreamedCoords.pop_front();
			// l_pAlternateChannelLocalisation = m_oChannelLocalisationAlternateCoords.front().first;
			// m_oChannelLocalisationAlternateCoords.pop_front();
			m_oChannelLocalisationTimes.pop_front();
		}

		if (l_pChannelLocalisation != nullptr)
		{
			//copy coordinates and times
			Tools::Matrix::copyContent(*l_pChannelLocalisation, *l_oMatrix);
			m_oChannelLocalisationStreamedCoords.emplace_back(l_pChannelLocalisation, true);
			//m_oChannelLocalisationAlternateCoords.push_back(std::pair<CMatrix*, bool>(l_pAlternateChannelLocalisation, true));
			m_oChannelLocalisationTimes.emplace_back(ui64StartTime, ui64EndTime);
		}
	}

	return true;
}

void CBufferDatabase::setDrawable(CSignalDisplayDrawable* pDrawable) { m_pDrawable = pDrawable; }

bool CBufferDatabase::getErrorStatus() { return m_bError; }

bool CBufferDatabase::onChannelLocalisationBufferReceived(const uint32_t ui32ChannelLocalisationBufferIndex)
{
	m_oChannelLocalisationStreamedCoords[ui32ChannelLocalisationBufferIndex].second = false;

	return true;
}

bool CBufferDatabase::isFirstBufferReceived() { return m_bFirstBufferReceived; }

bool CBufferDatabase::isFirstChannelLocalisationBufferProcessed()
{
	//at least one chanloc buffer must have been received and processed
	return (!m_oChannelLocalisationStreamedCoords.empty()) && (!m_oChannelLocalisationStreamedCoords[0].second);
}

bool CBufferDatabase::adjustNumberOfDisplayedBuffers(const double f64NumberOfSecondsToDisplay)
{
	bool l_bNumberOfBufferToDisplayChanged = false;

	if (f64NumberOfSecondsToDisplay > 0)
	{
		m_totalDuration   = f64NumberOfSecondsToDisplay;
		m_ovTotalDuration = 0;
		m_totalStep       = 0;
	}

	//return if buffer length is not known yet
	if (m_pDimensionSizes[1] == 0) { return false; }

	uint64_t newNbufferToDisplay = uint64_t(ceil((m_totalDuration * m_samplingFrequency) / m_pDimensionSizes[1]));

	//displays at least one buffer
	newNbufferToDisplay = (newNbufferToDisplay == 0) ? 1 : newNbufferToDisplay;
	if (newNbufferToDisplay != m_nBufferToDisplay || f64NumberOfSecondsToDisplay <= 0)
	{
		m_nBufferToDisplay            = newNbufferToDisplay;
		l_bNumberOfBufferToDisplayChanged = true;

		//if new number of buffers decreased, resize lists and destroy useless buffers
		while (m_nBufferToDisplay < m_oSampleBuffers.size())
		{
			delete[] m_oSampleBuffers.front();
			m_oSampleBuffers.pop_front();

			m_oStartTime.pop_front();

			m_oEndTime.pop_front();

			//suppress the corresponding minmax values
			for (size_t c = 0; c < size_t(m_pDimensionSizes[0]); c++) { m_oLocalMinMaxValue[c].pop_front(); }
		}
	}

	return l_bNumberOfBufferToDisplayChanged;
}

uint64_t CBufferDatabase::getChannelCount() const { return m_pDimensionSizes[0]; }

double CBufferDatabase::getDisplayedTimeIntervalWidth() const { return (m_nBufferToDisplay * ((m_pDimensionSizes[1] * 1000.0) / m_samplingFrequency)); }

void CBufferDatabase::setMatrixDimensionCount(const uint32_t nDimension)
{
	if (nDimension != 2)
	{
		m_bError = true;
		m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Caller tried to set a " << nDimension <<
				"-dimensional matrix. Only 2-dimensional matrices are supported (e.g. [rows X cols]).\n";
	}
	if (nDimension == 1)
	{
		m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error <<
				"Note: For 1-dimensional matrices, you may try Matrix Transpose box to upgrade the stream to [N X 1] first.\n";
	}
}

void CBufferDatabase::setMatrixDimensionSize(const uint32_t ui32DimensionIndex, const uint32_t ui32DimensionSize)
{
	if (ui32DimensionIndex >= 2)
	{
		m_bError = true;
		m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Tried to access dimension " << ui32DimensionIndex <<
				", only 0 and 1 supported\n";
		return;
	}

	if (m_pDimensionSizes[ui32DimensionIndex] != 0 && m_pDimensionSizes[ui32DimensionIndex] != ui32DimensionSize)
	{
		m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error <<
				"Upstream tried to change the data chunk size after the first header, this is not supported.\n";
		m_bError = true;
		return;
	}

	m_pDimensionSizes[ui32DimensionIndex] = ui32DimensionSize;
	m_pDimensionLabels[ui32DimensionIndex].resize(ui32DimensionSize);

	if (ui32DimensionIndex == 0)
	{
		m_i64NbElectrodes = m_pDimensionSizes[ui32DimensionIndex];

		//resize min/max values vector
		m_oLocalMinMaxValue.resize(size_t(m_i64NbElectrodes));
	}
}

void CBufferDatabase::setMatrixDimensionLabel(const uint32_t ui32DimensionIndex, const uint32_t ui32DimensionEntryIndex, const char* sDimensionLabel)
{
	if (m_bError) { return; }

	if (ui32DimensionIndex >= 2)
	{
		m_bError = true;
		m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Tried to access dimension " << ui32DimensionIndex <<
				", only 0 and 1 supported\n";
		return;
	}

	m_pDimensionLabels[ui32DimensionIndex][ui32DimensionEntryIndex] = sDimensionLabel;
}

bool CBufferDatabase::setMatrixBuffer(const double* buffer, const uint64_t ui64StartTime, const uint64_t ui64EndTime)
{
	//if an error has occurred, do nothing
	if (m_bError) { return false; }

	// Check for time-continuity
	if (ui64StartTime < m_lastBufferEndTime && !m_bWarningPrinted)
	{
		m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<
				"Your signal does not appear to be continuous in time. "
				<< "Previously inserted buffer ended at " << TimeArithmetics::timeToSeconds(m_lastBufferEndTime)
				<< "s, the current starts at " << TimeArithmetics::timeToSeconds(ui64StartTime)
				<< "s. The display may be incorrect.\n";
		m_bWarningPrinted = true;
	}
	m_lastBufferEndTime = ui64EndTime;


	//if this the first buffer, perform some precomputations
	if (!m_bFirstBufferReceived)
	{
		m_bufferDuration = ui64EndTime - ui64StartTime;

		//test if it is equal to zero : Error
		if (m_bufferDuration == 0)
		{
			m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<
					"Error : buffer start time and end time are equal : " << ui64StartTime << "\n";

			m_bError = true;

			return false;
		}

		//computes the sampling frequency for sanity checking or if the setter has not been called
		const uint64_t sampleDuration = (uint64_t(1) << 32) * m_pDimensionSizes[1];
		uint32_t l_ui32EstimatedFrequency   = uint32_t(sampleDuration / m_bufferDuration);
		if (l_ui32EstimatedFrequency == 0)
		{
			// Complain if estimate is bad
			m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<
					"The integer sampling frequency was estimated from the chunk size to be 0"
					<< " (nSamples " << m_pDimensionSizes[1] << " / bufferLength " << TimeArithmetics::timeToSeconds(m_bufferDuration) <<
					"s = 0). This is not supported. Forcing the rate to 1. This may lead to problems.\n";
			l_ui32EstimatedFrequency = 1;
		}
		if (m_samplingFrequency == 0)
		{
			// use chunking duration estimate if setter hasn't been used
			m_samplingFrequency = l_ui32EstimatedFrequency;
		}
		if (m_samplingFrequency != l_ui32EstimatedFrequency)
		{
			m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
					<< "Sampling rate [" << l_ui32EstimatedFrequency << "] suggested by chunk properties differs from stream-specified rate [" <<
					m_samplingFrequency << "]. There may be a problem with an upstream box. Trying to use the estimated rate.\n";
			m_samplingFrequency = l_ui32EstimatedFrequency;
		}

		//computes the number of buffer necessary to display the interval
		adjustNumberOfDisplayedBuffers(-1);

		m_pDrawable->init();

		m_bFirstBufferReceived = true;
	}

	if (!m_bChannelLookupTableInitialized)
	{
		fillChannelLookupTable();  //to retrieve the unrecognized electrode warning
		// The above call will fail if no electrode localisation data...
		// m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Unable to fill lookup table\n";
		//	return false;
	}
	else
	{
		//look for chanloc buffers recently received
		for (uint32_t i = 0; i < m_oChannelLocalisationStreamedCoords.size(); ++i)
		{
			//if a new set of coordinates was received
			if (m_oChannelLocalisationStreamedCoords[i].second) { onChannelLocalisationBufferReceived(i); }
		}
	}

	double* l_pBufferToWrite                      = nullptr;
	const uint64_t l_ui64NumberOfSamplesPerBuffer = m_pDimensionSizes[0] * m_pDimensionSizes[1];

	//if old buffers need to be removed
	if (m_oSampleBuffers.size() == m_nBufferToDisplay)
	{
		if (m_ovTotalDuration == 0) { m_ovTotalDuration = (m_oStartTime.back() - m_oStartTime.front()) + (m_oEndTime.back() - m_oStartTime.back()); }
		if (m_bufferStep == 0)
		{
			if (m_oStartTime.size() <= 1) { m_bufferStep = m_ovTotalDuration; }
			else { m_bufferStep = m_oStartTime[1] - m_oStartTime[0]; }
		}
		if (m_totalStep == 0) { m_totalStep = (m_oStartTime.back() - m_oStartTime.front()) + m_bufferStep; }

		//save first buffer pointer
		l_pBufferToWrite = m_oSampleBuffers.front();

		//pop first element from queues
		m_oSampleBuffers.pop_front();
		m_oStartTime.pop_front();
		m_oEndTime.pop_front();
		for (size_t c = 0; c < size_t(m_pDimensionSizes[0]); c++) { m_oLocalMinMaxValue[c].pop_front(); }
	}

	//do we need to allocate a new buffer?
	if (l_pBufferToWrite == nullptr) { l_pBufferToWrite = new double[size_t(l_ui64NumberOfSamplesPerBuffer)]; }

	//copy new buffer into internal buffer
	System::Memory::copy(l_pBufferToWrite, buffer, l_ui64NumberOfSamplesPerBuffer * sizeof(double));

	//push new buffer and its timestamps
	m_oSampleBuffers.push_back(l_pBufferToWrite);
	m_oStartTime.push_back(ui64StartTime);
	m_oEndTime.push_back(ui64EndTime);

	//compute and push min and max values of new buffer
	uint64_t l_ui64CurrentSample = 0;
	//for each channel
	for (size_t c = 0; c < size_t(m_pDimensionSizes[0]); c++)
	{
		double l_f64LocalMin = DBL_MAX;
		double l_f64LocalMax = -DBL_MAX;

		//for each sample
		for (uint64_t i = 0; i < m_pDimensionSizes[1]; i++, l_ui64CurrentSample++)
		{
			//get channel local min/max
			if (buffer[l_ui64CurrentSample] < l_f64LocalMin) { l_f64LocalMin = buffer[l_ui64CurrentSample]; }
			if (buffer[l_ui64CurrentSample] > l_f64LocalMax) { l_f64LocalMax = buffer[l_ui64CurrentSample]; }
		}

		//adds the minmax pair to the corresponding channel's list
		m_oLocalMinMaxValue[c].emplace_back(l_f64LocalMin, l_f64LocalMax);

		if (l_f64LocalMax > m_maximumValue) { m_maximumValue = l_f64LocalMax; }
		if (l_f64LocalMin < m_minimumValue) { m_minimumValue = l_f64LocalMin; }
	}

	//tells the drawable to redraw himself since the signal information has been updated
	if (m_bRedrawOnNewData) { m_pDrawable->redraw(); }
	return true;
}

bool CBufferDatabase::setSamplingFrequency(const uint32_t ui32SamplingFrequency)
{
	m_samplingFrequency = ui32SamplingFrequency;

	return true;
}

void CBufferDatabase::getDisplayedChannelLocalMeanValue(uint32_t /*ui32Channel*/, double& /*f64Mean*/) {}

void CBufferDatabase::getDisplayedChannelLocalMinMaxValue(const uint32_t ui32Channel, double& f64Min, double& f64Max)
{
	f64Min = +DBL_MAX;
	f64Max = -DBL_MAX;

	for (uint64_t i = 0; i < m_oLocalMinMaxValue[size_t(ui32Channel)].size(); ++i)
	{
		if (f64Min > m_oLocalMinMaxValue[size_t(ui32Channel)][size_t(i)].first) { f64Min = m_oLocalMinMaxValue[size_t(ui32Channel)][size_t(i)].first; }
		if (f64Max < m_oLocalMinMaxValue[size_t(ui32Channel)][size_t(i)].second) { f64Max = m_oLocalMinMaxValue[size_t(ui32Channel)][size_t(i)].second; }
	}
}

bool CBufferDatabase::isTimeInDisplayedInterval(const uint64_t& ui64Time) const
{
	if (m_oStartTime.empty()) { return false; }

	return ui64Time >= m_oStartTime.front() && ui64Time <= m_oEndTime.back();
}

bool CBufferDatabase::getIndexOfBufferStartingAtTime(const uint64_t& ui64Time, uint32_t& rIndex) const
{
	rIndex = 0;

	if (m_oSampleBuffers.empty() || ui64Time < m_oStartTime.front() || ui64Time > m_oStartTime.back()) { return false; }

	for (uint32_t i = 0; i < m_oStartTime.size(); ++i)
	{
		if (m_oStartTime[i] == ui64Time)
		{
			rIndex = i;
			return true;
		}
	}

	return false;
}

void CBufferDatabase::getDisplayedGlobalMinMaxValue(double& f64Min, double& f64Max)
{
	f64Min = +DBL_MAX;
	f64Max = -DBL_MAX;

	for (auto& pairs : m_oLocalMinMaxValue)
	{
		for (const auto& pair : pairs)
		{
			if (f64Min > pair.first) { f64Min = pair.first; }
			if (f64Max < pair.second) { f64Max = pair.second; }
		}
	}
}

uint64_t CBufferDatabase::getElectrodeCount() { return m_oChannelLocalisationLabels.size(); }

bool CBufferDatabase::getElectrodePosition(const uint32_t ui32ElectrodeIndex, double* pElectrodePosition)
{
	//TODO : add time parameter and look for coordinates closest to that time!
	if (ui32ElectrodeIndex < m_oChannelLocalisationLabels.size())
	{
		//if(m_bCartesianStreamedCoords == true)
		//{
		*pElectrodePosition       = *(m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * ui32ElectrodeIndex);
		*(pElectrodePosition + 1) = *(m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * ui32ElectrodeIndex + 1);
		*(pElectrodePosition + 2) = *(m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * ui32ElectrodeIndex + 2);
		//}
		return true;
	}
	return false;
}

bool CBufferDatabase::getElectrodePosition(const CString& rElectrodeLabel, double* pElectrodePosition)
{
	//TODO : add time parameter and look for coordinates closest to that time!
	for (uint32_t i = 0; i < m_oChannelLocalisationLabels.size(); ++i)
	{
		if (strcmp(rElectrodeLabel.toASCIIString(), m_oChannelLocalisationLabels[i].toASCIIString()) == 0)
		{
			//if(m_bCartesianStreamedCoords == true)
			//{
			*pElectrodePosition       = *(m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * i);
			*(pElectrodePosition + 1) = *(m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * i + 1);
			*(pElectrodePosition + 2) = *(m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * i + 2);
			//}
			return true;
		}
	}

	return false;
}

bool CBufferDatabase::getElectrodeLabel(const uint32_t ui32ElectrodeIndex, CString& rElectrodeLabel)
{
	if (ui32ElectrodeIndex >= m_oChannelLocalisationLabels.size()) { return false; }
	rElectrodeLabel = m_oChannelLocalisationLabels[ui32ElectrodeIndex].toASCIIString();
	return true;
}

bool CBufferDatabase::getChannelPosition(const uint32_t ui32ChannelIndex, double*& rChannelPosition)
{
	//TODO : add time parameter and look for coordinates closest to that time!
	if (ui32ChannelIndex >= 0 && ui32ChannelIndex < m_oChannelLookupIndices.size())
	{
		if (m_bCartesianStreamedCoords)
		{
			rChannelPosition = m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * m_oChannelLookupIndices[ui32ChannelIndex];
		}/*
		else
		{
			//TODO
		}*/
		return true;
	}

	return false;
}

bool CBufferDatabase::getChannelSphericalCoordinates(const uint32_t ui32ChannelIndex, double& rTheta, double& rPhi)
{
	//TODO : add time parameter and look for coordinates closest to that time!
	if (ui32ChannelIndex >= 0 && ui32ChannelIndex < m_oChannelLookupIndices.size())
	{
		if (m_bCartesianStreamedCoords)
		{
			//get cartesian coords
			double* l_pCoords = m_oChannelLocalisationStreamedCoords[0].first->getBuffer() + 3 * m_oChannelLookupIndices[ui32ChannelIndex];

			//convert to spherical coords
			return convertCartesianToSpherical(l_pCoords, rTheta, rPhi);
		}
		//streamed coordinates are spherical already
		//TODO
		return false;
	}
	return false;
}

bool CBufferDatabase::getChannelLabel(const uint32_t ui32ChannelIndex, CString& rChannelLabel)
{
	if (ui32ChannelIndex >= 0 && ui32ChannelIndex < m_oChannelLookupIndices.size())
	{
		rChannelLabel = m_oChannelLocalisationLabels[m_oChannelLookupIndices[ui32ChannelIndex]];
		return true;
	}
	rChannelLabel = "";
	return false;
}


void CBufferDatabase::setStimulation(const uint32_t /*ui32StimulationIndex*/, const uint64_t ui64StimulationIdentifier, const uint64_t ui64StimulationDate)
{
	// m_parentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Received new stimulation id:" << ui64StimulationIdentifier << " date:" << ui64StimulationDate << "\n";

	m_oStimulations.emplace_back(ui64StimulationDate, ui64StimulationIdentifier);

	if (!m_oStartTime.empty())
	{
		while (m_oStimulations.begin() != m_oStimulations.end() && m_oStimulations.begin()->first < m_oStartTime.front()) { m_oStimulations.pop_front(); }
	}
}

bool CBufferDatabase::fillChannelLookupTable()
{
	if (!m_bFirstBufferReceived || !m_bChannelLocalisationHeaderReceived) { return false; }

	bool res = true;

	//resize lookup array and initialize lookup indices to 0
	m_oChannelLookupIndices.resize(uint32_t(m_i64NbElectrodes), 0);

	//for all channels
	for (size_t i = 0; i < size_t(m_pDimensionSizes[0]); ++i)
	{
		//trim leading spaces
		size_t firstNonWhitespaceChar = 0;
		for (; firstNonWhitespaceChar < m_pDimensionLabels[0][i].size(); firstNonWhitespaceChar++)
		{
			if (isspace(m_pDimensionLabels[0][i][firstNonWhitespaceChar]) == 0) { break; }
		}

		//trim trailing spaces
		size_t lastNonWhitespaceChar = 0;
		if (!m_pDimensionLabels[0][i].empty())
		{
			for (lastNonWhitespaceChar = m_pDimensionLabels[0][i].size() - 1; lastNonWhitespaceChar >= 0; lastNonWhitespaceChar--)
			{
				if (isspace(m_pDimensionLabels[0][i][lastNonWhitespaceChar]) == 0) { break; }
			}
		}

		//look for label in channel localisation labels database
		bool l_bLabelRecognized = false;

		if (firstNonWhitespaceChar < lastNonWhitespaceChar)
		{
			std::string l_oChannelLabel(m_pDimensionLabels[0][i].substr(firstNonWhitespaceChar, lastNonWhitespaceChar - firstNonWhitespaceChar + 1));

			for (uint32_t j = 0; j < m_oChannelLocalisationLabels.size(); j++)
			{
				if (strcmp(l_oChannelLabel.c_str(), m_oChannelLocalisationLabels[j].toASCIIString()) == 0)
				{
					l_bLabelRecognized         = true;
					m_oChannelLookupIndices[i] = j;
					break;
				}
			}
		}

		//unrecognized electrode!
		if (!l_bLabelRecognized)
		{
			m_parentPlugin.getLogManager() << LogLevel_Warning
					<< "Unrecognized electrode name (index=" << uint32_t(i)
					<< ", name=" << m_pDimensionLabels[0][i].c_str()
					<< ")!\n";
			res = false;
		}
	}

	m_parentPlugin.getLogManager() << LogLevel_Trace << "Electrodes list : ";

	for (size_t i = 0; i < size_t(m_pDimensionSizes[0]); ++i)
	{
		m_parentPlugin.getLogManager() << CString(m_pDimensionLabels[0][i].c_str());
		if (i < m_pDimensionSizes[0] - 1) { m_parentPlugin.getLogManager() << ", "; }
		else { m_parentPlugin.getLogManager() << "\n"; }
	}

	if (res) { m_bChannelLookupTableInitialized = true; }

	return res;
}

bool CBufferDatabase::convertCartesianToSpherical(const double* pCartesianCoords, double& rTheta, double& rPhi) const
{
#define MY_THRESHOLD 1e-3
#define PI 3.1415926535

	const double l_f64RadToDeg = 180 / PI;

	//compute theta
	rTheta = acos(pCartesianCoords[2]) * l_f64RadToDeg;

	//compute phi so that it lies in [0, 360]
	if (fabs(pCartesianCoords[0]) < MY_THRESHOLD) { rPhi = (pCartesianCoords[1] > 0) ? 90 : 270; }
	else
	{
		rPhi = atan(pCartesianCoords[1] / pCartesianCoords[0]) * l_f64RadToDeg;

		if (pCartesianCoords[0] < 0) { rPhi += 180; }
		else if (pCartesianCoords[1] < 0) { rPhi += 360; }
	}

	return true;
}
