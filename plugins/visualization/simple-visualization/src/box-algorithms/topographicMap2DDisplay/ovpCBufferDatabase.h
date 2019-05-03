#pragma once

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <vector>
#include <deque>
#include <queue>
#include <string>
#include <cfloat>

#include <iostream>

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		class CSignalDisplayDrawable;

		/**
		* Abtract class of objects than can be updated by a CBufferDatabase
		*/
		class CSignalDisplayDrawable
		{
		public:

			virtual ~CSignalDisplayDrawable() = default;
			virtual void init() = 0;
			virtual void redraw() = 0;
		};

		/**
		* This class is used to store information about the incoming signal stream. It can request a CSignalDisplayDrawable
		* object to redraw himself in case of some changes in its data.
		*/
		class CBufferDatabase
		{
		public:
			//! Number of channels
			int64_t m_i64NbElectrodes;

			//! Number of channels and number of samples per buffer
			uint64_t m_pDimensionSizes[2];

			//! Channel labels, buffer labels
			std::vector<std::string> m_pDimensionLabels[2];

			//! Flag set to true once first buffer is received
			bool m_bFirstBufferReceived;

			//! Sampling frequency of the incoming stream
			uint32_t m_ui32SamplingFrequency;

			//! double-linked list of pointers to the samples buffers of the current time window
			std::deque<double*> m_oSampleBuffers;

			//! stimulations to display. pair values are <date, stimcode>
			std::deque<std::pair<uint64_t, uint64_t>> m_oStimulations;

			//electrode spherical coordinates (in degrees)
			//OpenViBE::CMatrix m_oElectrodesSphericalCoords;

			//flag set to true once channel lookup indices are determined
			bool m_bChannelLookupTableInitialized;

			//indices of electrodes in channel localisation database
			std::vector<uint32_t> m_oChannelLookupIndices;

			//electrode labels (standardized)
			//std::vector<OpenViBE::CString> m_oElectrodesLabels;

			//! Number of buffer to display at the same time
			uint64_t m_ui64NumberOfBufferToDisplay;

			//! The global maximum value of the signal (up to now)
			double m_f64MaximumValue;

			//! The global minimum value of the signal (up to now)
			double m_f64MinimumValue;

			//! Double-linked list of the start times of the current buffers
			std::deque<uint64_t> m_oStartTime;

			//! Double-linked list of the end times of the current buffers
			std::deque<uint64_t> m_oEndTime;

			//! Duration to display in seconds
			double m_f64TotalDuration;

			/*! Duration to display in openvibe time units.
			Computed once every time the user changes the total duration to display,
			when the maximum number of buffers to store are received.*/
			uint64_t m_ui64TotalDuration;

			/*! Duration of a single buffer.
			Computed once, but not constant when sampling frequency is not a multiple of buffer size!*/
			uint64_t m_ui64BufferDuration;

			/*! Time step separating the start times of m_ui64NumberOfBufferToDisplay+1 buffers.
			Recomputed once every time the user changes the total duration to display,
			but not constant when sampling frequency is not a multiple of buffer size!*/
			uint64_t m_ui64TotalStep;

			/*! Time step separating the start times of 2 consecutive buffers.
			Computed once, but not constant when sampling frequency is not a multiple of buffer size!*/
			uint64_t m_ui64BufferStep;

			// When did the last inserted buffer end
			uint64_t m_ui64LastBufferEndTime;
			// Did we print a warning about noncontinuity?
			bool m_bWarningPrinted;

			//! Pointer to the drawable object to update (if needed)
			CSignalDisplayDrawable* m_pDrawable;

			std::vector<std::deque<std::pair<double, double>>> m_oLocalMinMaxValue;

			OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& m_oParentPlugin;

			bool m_bError;

			//! Redraws the associated SignalDisplayDrawable upon new data reception if true (default)
			bool m_bRedrawOnNewData;

		protected:
			/* \name Channel localisation */
			//@{
			//channel localisation decoder
			OpenViBE::Kernel::IAlgorithmProxy* m_pChannelLocalisationStreamDecoder;
			//flag set to true once channel localisation buffer is received
			bool m_bChannelLocalisationHeaderReceived;
			//dynamic channel localisation flag (e.g. localisation is constantly updated with MEG)
			bool m_bDynamicChannelLocalisation;
			//channel labels database
			std::vector<OpenViBE::CString> m_oChannelLocalisationLabels;
			//flag stating whether streamed coordinates are cartesian (as opposed to spherical)
			bool m_bCartesianStreamedCoords;
			//! double-linked list of streamed channel coordinates (if cartesian, expressed in normalized space (X right Y front Z up))
			std::deque<std::pair<OpenViBE::CMatrix*, bool>> m_oChannelLocalisationStreamedCoords;
			//! double-linked list of channel coordinates (spherical if streamed coords aere cartesian and vice versa)
			//std::deque<  std::pair<OpenViBE::CMatrix*, bool> > m_oChannelLocalisationAlternateCoords;
			//pointer to double linked list of cartesian coordinates
			//std::deque< std::pair<OpenViBE::CMatrix*, bool> > * m_pChannelLocalisationCartesianCoords;
			//pointer to double linked list of spherical coordinates
			//std::deque< std::pair<OpenViBE::CMatrix*, bool> > * m_pChannelLocalisationSphericalCoords;
			//! double-linked list of start/end times of channel coordinates
			std::deque<std::pair<uint64_t, uint64_t>> m_oChannelLocalisationTimes;
			//@}

			//! Redraw mode (shift or scan)
			OpenViBE::CIdentifier m_oDisplayMode;

		public:
			CBufferDatabase(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& oPlugin);

			virtual ~CBufferDatabase();

			/**
			 * \brief Decode a channel localisation memory buffer
			 * \param pMemoryBuffer Memory buffer to decode
			 * \param ui64StartTime Start time of memory buffer
			 * \param ui64EndTime End time of memory buffer
			 * \return True if memory buffer could be properly decoded, false otherwise
			 */
			virtual bool decodeChannelLocalisationMemoryBuffer(
				const OpenViBE::IMemoryBuffer* pMemoryBuffer,
				uint64_t ui64StartTime,
				uint64_t ui64EndTime);

			/**
			 * \brief Callback called upon channel localisation buffer reception
			 * \param uint32_t Index of newly received channel localisation buffer
			 * \return True if buffer data was correctly processed, false otherwise
			 */
			virtual bool onChannelLocalisationBufferReceived(
				uint32_t ui32ChannelLocalisationBufferIndex);

			/**
			 * \brief Sets the drawable object to update.
			 * \param pDrawable drawable object to update.
			 */
			virtual void setDrawable(
				CSignalDisplayDrawable* pDrawable);

			/**
			 * \brief Get error status
			 * \return Error status. If true, an error occurred.
			 */
			virtual bool getErrorStatus();

			/**
			 * \brief Determines whether first buffer has been received yet
			 * \return True if first buffer has been received already, false otherwise
			 */
			virtual bool isFirstBufferReceived();

			/**
			 * \brief Determines whether first channel localisation buffer has been processed yet
			 * When this condition is true, channel coordinates may be retrieved using the
			 * corresponding methods in this class.
			 * \return True if first chanloc buffer was processed
			 */
			virtual bool isFirstChannelLocalisationBufferProcessed();

			/**
			 * Compute the number of buffers needed to display the signal for a certain time period.
			 * \param f64NumberOfMsToDisplay the time window's width in seconds.
			 */
			virtual bool adjustNumberOfDisplayedBuffers(double f64NumberOfSecondsToDisplay);

			/**
			 * \brief Get time interval covered by data held in this object
			 * \return Time interval in ms
			 */
			virtual double getDisplayedTimeIntervalWidth() const;

			/**
			 * \brief Determine whether time passed in parameter lies in displayed data interval
			 * \param ui64Time Time to test
			 * \return True if time lies in displayed time interval, false otherwise
			 */
			virtual bool isTimeInDisplayedInterval(const uint64_t& ui64Time) const;

			/**
			 * \brief Get index of sample buffer which starts at a given time
			 * \param ui64Time[in] Start time of buffer
			 * \param rIndex[out] Buffer index
			 * \return True if buffer index could be determined, false otherwise
			 */
			virtual bool getIndexOfBufferStartingAtTime(const uint64_t& ui64Time, uint32_t& rIndex) const;

			//! Returns the min/max values currently displayed for the given channel
			virtual void getDisplayedChannelLocalMinMaxValue(uint32_t ui32Channel, double& f64Min, double& f64Max);
			//! Returns the min/max values currently displayed (all channels taken into account)
			virtual void getDisplayedGlobalMinMaxValue(double& f64Min, double& f64Max);

			virtual void getDisplayedChannelLocalMeanValue(uint32_t ui32Channel, double& f64Mean);

			//! Returns the min/max values of the last buffer arrived for the given channel
			virtual void getLastBufferChannelLocalMinMaxValue(uint32_t ui32Channel, double& f64Min, double& f64Max)
			{
				f64Min = m_oLocalMinMaxValue[ui32Channel].back().first;
				f64Max = m_oLocalMinMaxValue[ui32Channel].back().second;
			}

			//! Returns the min/max values of the last buffer arrived  (all channels taken into account)
			virtual void getLastBufferMinMaxValue(double& f64Min, double& f64Max)
			{
				f64Min = +DBL_MAX;
				f64Max = -DBL_MAX;

				for (auto& localValue : m_oLocalMinMaxValue)
				{
					f64Min = (localValue.back().first < f64Min) ? localValue.back().first : f64Min;
					f64Max = (localValue.back().second > f64Max) ? localValue.back().second : f64Max;
				}
			}

			/**
			 * \brief Get number of eletrodes in database
			 * \return Number of electrodes
			 */
			virtual uint64_t getElectrodeCount();

			/**
			 * \brief Get electrode normalized position
			 * \remarks Position expressed in normalized cartesian frame where X is right, Y front, Z up
			 * \param[in] ui32ElectrodeIndex Index of electrode in database whose position is to be retrieved
			 * \param[out] pElectrodePosition Pointer to an array of 3 floats where to store coordinates
			 * \return True if electrode position could be retrieved
			 */
			virtual bool getElectrodePosition(
				uint32_t ui32ElectrodeIndex,
				double* pElectrodePosition);

			/**
			 * \brief Get electrode normalized position
			 * \remarks Position expressed in normalized cartesian frame where X is right, Y front, Z up
			 * \param[in] rElectrodeLabel Label of electrode whose position is to be retrieved
			 * \param[out] pElectrodePosition Pointer to an array of 3 floats where to store coordinates
			 * \return True if electrode position could be retrieved
			 */
			virtual bool getElectrodePosition(
				const OpenViBE::CString& rElectrodeLabel,
				double* pElectrodePosition);

			/**
			 * \brief Get electrode label
			 * \param[in] ui32ElectrodeIndex Index of electrode in database whose label is to be retrieved
			 * \param[out] rElectrodeLabel Electrode label
			 * \return True if electrode label could be retrieved
			 */
			virtual bool getElectrodeLabel(
				uint32_t ui32ElectrodeIndex,
				OpenViBE::CString& rElectrodeLabel);

			/**
			 * \brief Get number of channels
			 * \return Number of channels
			 */
			virtual uint64_t getChannelCount() const;

			/**
			 * \brief Get channel normalized position
			 * \remarks Position expressed in normalized cartesian frame where X is right, Y front, Z up
			 * \param[in] ui32ChannelIndex Index of channel whose position is to be retrieved
			 * \param[out] rChannelPosition Reference on a double pointer
			 * \return True if channel position could be retrieved (rChannelPosition then points to an array of 3 floats)
			 */
			virtual bool getChannelPosition(
				uint32_t ui32ChannelIndex,
				double*& rChannelPosition);

			/**
			 * \brief Get channel spherical coordinates in degrees
			 * \param[in] ui32ChannelIndex Index of channel whose coordinates are to be retrieved
			 * \param[out] rTheta Reference on a float to be set with theta angle
			 * \param[out] rPhi Reference on a float to be set with phi angle
			 * \return True if channel coordinates could be retrieved
			 */
			virtual bool getChannelSphericalCoordinates(
				uint32_t ui32ChannelIndex,
				double& rTheta,
				double& rPhi);

			/**
			 * \brief Get channel label
			 * \param[in] ui32ChannelIndex Index of channel whose label is to be retrieved
			 * \param[out] rChannelLabel Channel label
			 * \return True if channel label could be retrieved
			 */
			virtual bool getChannelLabel(
				uint32_t ui32ChannelIndex,
				OpenViBE::CString& rChannelLabel);

			virtual void setMatrixDimensionCount(
				uint32_t ui32DimensionCount);
			virtual void setMatrixDimensionSize(
				uint32_t ui32DimensionIndex,
				uint32_t ui32DimensionSize);
			virtual void setMatrixDimensionLabel(
				uint32_t ui32DimensionIndex,
				uint32_t ui32DimensionEntryIndex,
				const char* sDimensionLabel);

			// Returns false on failure
			virtual bool setMatrixBuffer(
				const double* pBuffer,
				uint64_t ui64StartTime,
				uint64_t ui64EndTime);

			// Sets the sampling frequency. If this is not called, the frequency is estimated from the stream chunk properties.
			// Mainly used to force a warning if stream-specified rate differs from the chunk-estimated rate.
			virtual bool setSamplingFrequency(
				uint32_t ui32SamplingFrequency);

			virtual void setStimulationCount(
				uint32_t ui32StimulationCount);
			virtual void setStimulation(
				uint32_t ui32StimulationIndex,
				uint64_t ui64StimulationIdentifier,
				uint64_t ui64StimulationDate);

			/**
			 * \brief Set display mode
			 * \remarks Used by signal display and time ruler to determine how they should be updated
			 * \param oDisplayMode New display mode
			 */
			virtual void setDisplayMode(const OpenViBE::CIdentifier& oDisplayMode);

			/**
			 * \brief Get current display mode
			 * \return Current display mode
			 */
			virtual OpenViBE::CIdentifier getDisplayMode();

			/**
			 * \brief Set flag stating whether to redraw associated SignalDisplayDrawable objet when new data is available
			 * \param bSet Value to set flag with
			 */
			virtual void setRedrawOnNewData(
				bool bSet);

		protected:
			/**
			 * \brief Initialize table storing indices of electrodes in channel localisation database
			 * \return True if table could be initialized
			 */
			virtual bool fillChannelLookupTable();

			/**
			 * \brief Convert a cartesian coordinates triplet to spherical coordinates
			 * \param[in] pCartesianCoords Pointer to cartesian coordinates triplet
			 * \param[out] rTheta Equivalent theta angle
			 * \param[out] rPhi Equivalent phi angle
			 * \return True if coordinates were successfully converted
			 */
			bool convertCartesianToSpherical(
				const double* pCartesianCoords,
				double& rTheta,
				double& rPhi);
		};
	}  // namespace SimpleVisualization
} // namespace OpenViBEPlugins
