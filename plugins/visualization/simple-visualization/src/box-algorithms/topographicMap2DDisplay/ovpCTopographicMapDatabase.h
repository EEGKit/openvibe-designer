#pragma once

#include "../../ovp_defines.h"

#include "ovpCBufferDatabase.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

namespace OpenViBE
{
	namespace Plugins
	{
		namespace SimpleVisualization
		{
			class CTopographicMapDrawable : public CSignalDisplayDrawable
			{
			public:
				~CTopographicMapDrawable() override = default;
				virtual CMatrix* getSampleCoordinatesMatrix() = 0;
				virtual bool setSampleValuesMatrix(IMatrix* pSampleValuesMatrix) = 0;
			};

			/**
			* This class is used to store information about the incoming signal stream. It can request a CSignalDisplayDrawable
			* object to redraw himself in case of some changes in its data.
			*/
			class CTopographicMapDatabase : public CBufferDatabase
			{
			public:
				CTopographicMapDatabase(Toolkit::TBoxAlgorithm<IBoxAlgorithm>& plugin, Kernel::IAlgorithmProxy& interpolation);
				~CTopographicMapDatabase() override = default;

				void setMatrixDimensionSize(const size_t index, const size_t size) override;

				/**
				 * \brief Callback called upon channel localisation buffer reception
				 * \param bufferIndex Index of newly received channel localisation buffer
				 * \return True if buffer data was correctly processed, false otherwise
				 */
				bool onChannelLocalisationBufferReceived(const size_t bufferIndex) override;

				bool setDelay(double delay);

				/**
				 * \brief Set interpolation type
				 * Spline values (potentials) can be interpolated directly, but the spline laplacian (currents) may
				 * be used as well
				 * \sa OVP_TypeId_SphericalLinearInterpolationType enumeration
				 * \return True if interpolation type could be set, false otherwise
				 */
				bool setInterpolationType(uint64_t type);

				bool processValues();

				bool interpolateValues();

				//! Returns min/max interpolated values using the last buffer arrived (all channels taken into account)
				void getLastBufferInterpolatedMinMaxValue(double& min, double& max) const;

			private:
				/**
				 * \brief Looks for buffer whose timeframe contains time passed as parameter
				 * \param time [in] Time of buffer to be retrieved
				 * \param bufferIndex [out] Index of buffer closest to time passed as parameter
				 * \return True if time passed as parameter lies within a buffer's timeframe, false otherwise
				 */
				bool getBufferIndexFromTime(uint64_t time, size_t& bufferIndex);

				/**
				 * \brief Ensure electrode coordinates are normalized
				 * \return True if all electrode coordinates are normalized, false otherwise
				 */
				bool checkElectrodeCoordinates();

				//true until process() is called for the first time
				bool m_firstProcess = true;
				//spherical spline interpolation
				Kernel::IAlgorithmProxy& m_interpolation;
				//order of spherical spline used for interpolation - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder
				int64_t m_splineOrder = 4;
				/**
				 * \brief Type of interpolation
				 * \sa OVP_TypeId_SphericalLinearInterpolationType enumeration
				 */
				uint64_t m_interpolationType = Spline;
				//number of electrodes (see CBufferDatabase) - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount
				//int64_t m_NElectrodes = 0;
				//flag set to true once electrode coordinates have been initialized
				bool m_electrodeCoordsInitialized = false;
				//electrode cartesian coordinates, in normalized space (X right Y front Z up)
				CMatrix m_electrodeCoords;
				//pointer to electrode coordinates matrix - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates
				IMatrix* m_pElectrodeCoords = nullptr;
				//electrode potentials
				CMatrix m_electrodePotentials;
				//pointer to electrode potentials matrix - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues
				IMatrix* m_pElectrodePotentials = nullptr;
				//pointer to sample points coordinates matrix - mapped to OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates
				IMatrix* m_samplePointCoords = nullptr;
				//minimum interpolated value
				Kernel::TParameterHandler<double> m_minSamplePointValue;
				//maximum interpolated value
				Kernel::TParameterHandler<double> m_maxSamplePointValue;
				//delay to apply to interpolated values
				uint64_t m_delay = 0;
			};
		} // namespace SimpleVisualization
	}  // namespace Plugins
}  // namespace OpenViBE
