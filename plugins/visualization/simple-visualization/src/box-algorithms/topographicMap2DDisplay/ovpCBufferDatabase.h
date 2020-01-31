#pragma once

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <cfloat>
#include <deque>
#include <string>
#include <vector>
#include <array>

namespace OpenViBE
{
	namespace Plugins
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
			int64_t m_NElectrodes = 0;

			//! Number of channels and number of samples per buffer
			std::array<size_t, 2> m_DimSizes;

			//! Channel labels, buffer labels
			std::vector<std::string> m_DimLabels[2];

			//! Flag set to true once first buffer is received
			bool m_HasFirstBuffer = false;

			//! Sampling frequency of the incoming stream
			size_t m_Sampling = 0;

			//! double-linked list of pointers to the samples buffers of the current time window
			std::deque<double*> m_SampleBuffers;

			//! stimulations to display. pair values are <date, stimcode>
			std::deque<std::pair<uint64_t, uint64_t>> m_Stimulations;

			//electrode spherical coordinates (in degrees)
			//CMatrix m_oElectrodesSphericalCoords;

			//flag set to true once channel lookup indices are determined
			bool m_ChannelLookupTableInitialized = false;

			//indices of electrodes in channel localisation database
			std::vector<size_t> m_ChannelLookupIndices;

			//electrode labels (standardized)
			//std::vector<CString> m_oElectrodesLabels;

			//! Number of buffer to display at the same time
			size_t m_NBufferToDisplay = 2;

			//! The global maximum value of the signal (up to now)
			double m_MaxValue = -DBL_MAX;

			//! The global minimum value of the signal (up to now)
			double m_MinValue = +DBL_MAX;

			//! Double-linked list of the start times of the current buffers
			std::deque<uint64_t> m_StartTime;

			//! Double-linked list of the end times of the current buffers
			std::deque<uint64_t> m_EndTime;

			//! Duration to display in seconds
			double m_TotalDuration = 0;

			/*! Duration to display in openvibe time units.
			Computed once every time the user changes the total duration to display,
			when the maximum number of buffers to store are received.*/
			uint64_t m_ovTotalDuration = 0;

			/*! Duration of a single buffer.
			Computed once, but not constant when sampling frequency is not a multiple of buffer size!*/
			uint64_t m_BufferDuration = 0;

			/*! Time step separating the start times of m_NBufferToDisplay+1 buffers.
			Recomputed once every time the user changes the total duration to display,
			but not constant when sampling frequency is not a multiple of buffer size!*/
			size_t m_TotalStep = 0;

			/*! Time step separating the start times of 2 consecutive buffers.
			Computed once, but not constant when sampling frequency is not a multiple of buffer size!*/
			size_t m_BufferStep = 0;

			// When did the last inserted buffer end
			uint64_t m_LastBufferEndTime = 0;
			// Did we print a warning about noncontinuity?
			bool m_WarningPrinted = false;

			//! Pointer to the drawable object to update (if needed)
			CSignalDisplayDrawable* m_Drawable = nullptr;

			std::vector<std::deque<std::pair<double, double>>> m_LocalMinMaxValue;

			Toolkit::TBoxAlgorithm<IBoxAlgorithm>& m_ParentPlugin;

			bool m_Error = false;

			//! Redraws the associated SignalDisplayDrawable upon new data reception if true (default)
			bool m_RedrawOnNewData = true;

		protected:
			/* \name Channel localisation */
			//@{
			//channel localisation decoder
			Kernel::IAlgorithmProxy* m_decoder = nullptr;
			//flag set to true once channel localisation buffer is received
			bool m_channelLocalisationHeaderReceived = false;
			//dynamic channel localisation flag (e.g. localisation is constantly updated with MEG)
			bool m_dynamicChannelLocalisation = false;
			//channel labels database
			std::vector<CString> m_channelLocalisationLabels;
			//flag stating whether streamed coordinates are cartesian (as opposed to spherical)
			bool m_cartesianCoords = false;
			//! double-linked list of streamed channel coordinates (if cartesian, expressed in normalized space (X right Y front Z up))
			std::deque<std::pair<CMatrix*, bool>> m_channelLocalisationCoords;
			//! double-linked list of channel coordinates (spherical if streamed coords aere cartesian and vice versa)
			//std::deque<  std::pair<CMatrix*, bool> > m_oChannelLocalisationAlternateCoords;
			//pointer to double linked list of cartesian coordinates
			//std::deque< std::pair<CMatrix*, bool> > * m_channelLocalisationCartesianCoords;
			//pointer to double linked list of spherical coordinates
			//std::deque< std::pair<CMatrix*, bool> > * m_channelLocalisationSphericalCoords;
			//! double-linked list of start/end times of channel coordinates
			std::deque<std::pair<uint64_t, uint64_t>> m_channelLocalisationTimes;
			//@}

			//! Redraw mode (shift or scan)
			CIdentifier m_displayMode = Scan;

		public:
			explicit CBufferDatabase(Toolkit::TBoxAlgorithm<IBoxAlgorithm>& plugin);

			virtual ~CBufferDatabase();

			/**
			 * \brief Decode a channel localisation memory buffer
			 * \param buffer Memory buffer to decode
			 * \param startTime Start time of memory buffer
			 * \param endTime End time of memory buffer
			 * \return True if memory buffer could be properly decoded, false otherwise
			 */
			virtual bool decodeChannelLocalisationMemoryBuffer(const IMemoryBuffer* buffer, uint64_t startTime, uint64_t endTime);

			/**
			 * \brief Callback called upon channel localisation buffer reception
			 * \param index Index of newly received channel localisation buffer
			 * \return True if buffer data was correctly processed, false otherwise
			 */
			virtual bool onChannelLocalisationBufferReceived(const size_t index);

			/**
			 * \brief Sets the drawable object to update.
			 * \param drawable drawable object to update.
			 */
			virtual void setDrawable(CSignalDisplayDrawable* drawable) { m_Drawable = drawable; }

			/**
			 * \brief Get error status
			 * \return Error status. If true, an error occurred.
			 */
			virtual bool getErrorStatus() { return m_Error; }

			/**
			 * \brief Determines whether first buffer has been received yet
			 * \return True if first buffer has been received already, false otherwise
			 */
			virtual bool hasFirstBuffer() { return m_HasFirstBuffer; }

			/**
			 * \brief Determines whether first channel localisation buffer has been processed yet
			 * When this condition is true, channel coordinates may be retrieved using the
			 * corresponding methods in this class.
			 * \return True if first chanloc buffer was processed
			 */
			virtual bool isFirstChannelLocalisationBufferProcessed();

			/**
			 * Compute the number of buffers needed to display the signal for a certain time period.
			 * \param time the time window's width in seconds.
			 */
			virtual bool adjustNumberOfDisplayedBuffers(const double time);

			/**
			 * \brief Get time interval covered by data held in this object
			 * \return Time interval in ms
			 */
			virtual double getDisplayedTimeIntervalWidth() const;

			/**
			 * \brief Determine whether time passed in parameter lies in displayed data interval
			 * \param time Time to test
			 * \return True if time lies in displayed time interval, false otherwise
			 */
			virtual bool isTimeInDisplayedInterval(const uint64_t& time) const;

			/**
			 * \brief Get index of sample buffer which starts at a given time
			 * \param time[in] Start time of buffer
			 * \param index[out] Buffer index
			 * \return True if buffer index could be determined, false otherwise
			 */
			virtual bool getIndexOfBufferStartingAtTime(const uint64_t& time, size_t& index) const;

			//! Returns the min/max values currently displayed for the given channel
			virtual void getDisplayedChannelLocalMinMaxValue(const size_t channel, double& min, double& max);
			//! Returns the min/max values currently displayed (all channels taken into account)
			virtual void getDisplayedGlobalMinMaxValue(double& min, double& max);

			virtual void getDisplayedChannelLocalMeanValue(size_t /*channel*/, double& /*mean*/) { }

			//! Returns the min/max values of the last buffer arrived for the given channel
			virtual void getLastBufferChannelLocalMinMaxValue(const size_t channel, double& min, double& max);

			//! Returns the min/max values of the last buffer arrived  (all channels taken into account)
			virtual void getLastBufferMinMaxValue(double& min, double& max);

			/**
			 * \brief Get number of eletrodes in database
			 * \return Number of electrodes
			 */
			virtual size_t getElectrodeCount() { return m_channelLocalisationLabels.size(); }

			/**
			 * \brief Get electrode normalized position
			 * \remarks Position expressed in normalized cartesian frame where X is right, Y front, Z up
			 * \param[in] index Index of electrode in database whose position is to be retrieved
			 * \param[out] position Pointer to an array of 3 floats where to store coordinates
			 * \return True if electrode position could be retrieved
			 */
			virtual bool getElectrodePosition(const size_t index, double* position);

			/**
			 * \brief Get electrode normalized position
			 * \remarks Position expressed in normalized cartesian frame where X is right, Y front, Z up
			 * \param[in] label Label of electrode whose position is to be retrieved
			 * \param[out] position Pointer to an array of 3 floats where to store coordinates
			 * \return True if electrode position could be retrieved
			 */
			virtual bool getElectrodePosition(const CString& label, double* position);

			/**
			 * \brief Get electrode label
			 * \param[in] index Index of electrode in database whose label is to be retrieved
			 * \param[out] label Electrode label
			 * \return True if electrode label could be retrieved
			 */
			virtual bool getElectrodeLabel(const size_t index, CString& label);

			/**
			 * \brief Get number of channels
			 * \return Number of channels
			 */
			virtual size_t getChannelCount() const;

			/**
			 * \brief Get channel normalized position
			 * \remarks Position expressed in normalized cartesian frame where X is right, Y front, Z up
			 * \param[in] index Index of channel whose position is to be retrieved
			 * \param[out] position Reference on a double pointer
			 * \return True if channel position could be retrieved (rChannelPosition then points to an array of 3 floats)
			 */
			virtual bool getChannelPosition(const size_t index, double*& position);

			/**
			 * \brief Get channel spherical coordinates in degrees
			 * \param[in] index Index of channel whose coordinates are to be retrieved
			 * \param[out] theta Reference on a float to be set with theta angle
			 * \param[out] phi Reference on a float to be set with phi angle
			 * \return True if channel coordinates could be retrieved
			 */
			virtual bool getChannelSphericalCoordinates(const size_t index, double& theta, double& phi);

			/**
			 * \brief Get channel label
			 * \param[in] index Index of channel whose label is to be retrieved
			 * \param[out] label Channel label
			 * \return True if channel label could be retrieved
			 */
			virtual bool getChannelLabel(const size_t index, CString& label);

			virtual void setMatrixDimensionCount(const size_t n);
			virtual void setMatrixDimensionSize(const size_t index, const size_t size);
			virtual void setMatrixDimensionLabel(const size_t idx1, const size_t idx2, const char* label);

			// Returns false on failure
			virtual bool setMatrixBuffer(const double* buffer, const uint64_t startTime, const uint64_t endTime);

			// Sets the sampling frequency. If this is not called, the frequency is estimated from the stream chunk properties.
			// Mainly used to force a warning if stream-specified rate differs from the chunk-estimated rate.
			virtual bool setSampling(const size_t sampling);

			virtual void setStimulationCount(const size_t /*n*/) { }
			virtual void setStimulation(const size_t index, const uint64_t id, const uint64_t date);

			/**
			 * \brief Set display mode
			 * \remarks Used by signal display and time ruler to determine how they should be updated
			 * \param mode New display mode
			 */
			virtual void setDisplayMode(const CIdentifier& mode) { m_displayMode = mode; }

			/**
			 * \brief Get current display mode
			 * \return Current display mode
			 */
			virtual CIdentifier getDisplayMode() { return m_displayMode; }

			/**
			 * \brief Set flag stating whether to redraw associated SignalDisplayDrawable objet when new data is available
			 * \param redraw Value to set flag with
			 */
			virtual void setRedrawOnNewData(const bool redraw) { m_RedrawOnNewData = redraw; }

		protected:
			/**
			 * \brief Initialize table storing indices of electrodes in channel localisation database
			 * \return True if table could be initialized
			 */
			virtual bool fillChannelLookupTable();

			/**
			 * \brief Convert a cartesian coordinates triplet to spherical coordinates
			 * \param[in] cartesian Pointer to cartesian coordinates triplet
			 * \param[out] theta Equivalent theta angle
			 * \param[out] phi Equivalent phi angle
			 * \return True if coordinates were successfully converted
			 */
			bool convertCartesianToSpherical(const double* cartesian, double& theta, double& phi) const;
		};
	} // namespace SimpleVisualization
	}  // namespace Plugins
}  // namespace OpenViBE
