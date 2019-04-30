#pragma once

#include "../../ovp_defines.h"

#include "ovpCBufferDatabase.h"

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
		class CTopographicMapDrawable : public CSignalDisplayDrawable
		{
		public:
			virtual ~CTopographicMapDrawable(){ }
			virtual OpenViBE::CMatrix* getSampleCoordinatesMatrix() = 0;
			virtual bool setSampleValuesMatrix(OpenViBE::IMatrix* pSampleValuesMatrix) = 0;
		};

		/**
		* This class is used to store information about the incoming signal stream. It can request a CSignalDisplayDrawable
		* object to redraw himself in case of some changes in its data.
		*/
		class CTopographicMapDatabase : public CBufferDatabase
		{
		public:
			CTopographicMapDatabase(
				OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& oPlugin,
				OpenViBE::Kernel::IAlgorithmProxy& rSphericalSplineInterpolation);
			~CTopographicMapDatabase();

			void setMatrixDimensionSize(
				const uint32_t ui32DimensionIndex,
				const uint32_t ui32DimensionSize);

			/**
			 * \brief Callback called upon channel localisation buffer reception
			 * \param uint32_t Index of newly received channel localisation buffer
			 * \return True if buffer data was correctly processed, false otherwise
			 */
			virtual bool onChannelLocalisationBufferReceived(
				uint32_t ui32ChannelLocalisationBufferIndex);

			bool setDelay(
				double f64Delay);

			/**
			 * \brief Set interpolation type
			 * Spline values (potentials) can be interpolated directly, but the spline laplacian (currents) may
			 * be used as well
			 * \sa OVP_TypeId_SphericalLinearInterpolationType enumeration
			 * \return True if interpolation type could be set, false otherwise
			 */
			bool setInterpolationType(
				uint64_t ui64InterpolationType);

			bool processValues();

			bool interpolateValues();

			//! Returns min/max interpolated values using the last buffer arrived (all channels taken into account)
			void getLastBufferInterpolatedMinMaxValue(
				double& f64Min,
				double& f64Max);

		private:
			/**
			 * \brief Looks for buffer whose timeframe contains time passed as parameter
			 * \param ui64Time [in] Time of buffer to be retrieved
			 * \param rBufferIndex [out] Index of buffer closest to time passed as parameter
			 * \return True if time passed as parameter lies within a buffer's timeframe, false otherwise
			 */
			bool getBufferIndexFromTime(
				uint64_t ui64Time,
				uint32_t& rBufferIndex);

			/**
			 * \brief Ensure electrode coordinates are normalized
			 * \return True if all electrode coordinates are normalized, false otherwise
			 */
			bool checkElectrodeCoordinates();

		private:
			//true until process() is called for the first time
			bool m_bFirstProcess;
			//spherical spline interpolation
			OpenViBE::Kernel::IAlgorithmProxy& m_rSphericalSplineInterpolation;
			//order of spherical spline used for interpolation - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder
			int64_t m_i64SplineOrder;
			/**
			 * \brief Type of interpolation
			 * \sa OVP_TypeId_SphericalLinearInterpolationType enumeration
			 */
			uint64_t m_ui64InterpolationType;
			//number of electrodes (see CBufferDatabase) - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount
			//int64_t m_i64NbElectrodes;
			//flag set to true once electrode coordinates have been initialized
			bool m_bElectrodeCoordsInitialized;
			//electrode cartesian coordinates, in normalized space (X right Y front Z up)
			OpenViBE::CMatrix m_oElectrodeCoords;
			//pointer to electrode coordinates matrix - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates
			OpenViBE::IMatrix* m_pElectrodeCoords;
			//electrode potentials
			OpenViBE::CMatrix m_oElectrodePotentials;
			//pointer to electrode potentials matrix - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues
			OpenViBE::IMatrix* m_pElectrodePotentials;
			//pointer to sample points coordinates matrix - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates
			OpenViBE::IMatrix* m_pSamplePointCoords;
			//minimum interpolated value
			OpenViBE::Kernel::TParameterHandler < double > m_oMinSamplePointValue;
			//maximum interpolated value
			OpenViBE::Kernel::TParameterHandler < double > m_oMaxSamplePointValue;
			//delay to apply to interpolated values
			uint64_t m_ui64Delay;
		};
	}
}

