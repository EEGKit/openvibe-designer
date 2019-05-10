#pragma once

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gtk/gtk.h>

#include "ovpCTopographicMapDatabase.h"

#include <vector>
#include <string>

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		/**
		 * This class contains everything necessary to setup a GTK window and display
		 * a 2D topographic map
		 */
		class CTopographicMap2DView : public CTopographicMapDrawable
		{
		public:
			enum ETopographicMap2DProjection
			{
				TopographicMap2DProjection_Axial,
				TopographicMap2DProjection_Radial,
				TopographicMap2DProjection_NumProjection
			};

			enum ETopographicMap2DView
			{
				TopographicMap2DView_Top,
				TopographicMap2DView_Left,
				TopographicMap2DView_Right,
				TopographicMap2DView_Back
			};

			/**
			 * \brief Constructor
			 * \param rTopographicMapDatabase Datastore
			 * \param ui64DefaultInterpolation Interpolation mode
			 * \param f64Delay Delay to apply to displayed data
			 */
			CTopographicMap2DView(CTopographicMapDatabase& rTopographicMapDatabase, uint64_t ui64DefaultInterpolation, double f64Delay);

			/**
			 * \brief Destructor
			 */
			~CTopographicMap2DView() override;

			/** \name CSignalDisplayDrawable implementation */
			//@{

			/**
			 * \brief Initialize widgets
			 */
			void init() override;

			/**
			 * \brief Redraw map
			 */
			void redraw() override;

			//@}

			/** \name CTopographicMapDrawable implementation */
			//@{

			/**
			 * \brief Get matrix of sample points coordinates (places where to interpolate values)
			 * \return Pointer to matrix of sample points coordinates
			 */
			OpenViBE::CMatrix* getSampleCoordinatesMatrix() override;

			/**
			 * \brief Set matrix of sample points values (values interpolated at places specified in sample coordinates matrix)
			 * \param [in] pSampleValuesMatrix Pointer to matrix of sample points values
			 * \return True if values were successfully set, false otherwise
			 */
			bool setSampleValuesMatrix(OpenViBE::IMatrix* pSampleValuesMatrix) override;

			//@}

			/**
			 * \brief Get pointers to plugin main widget and (optional) toolbar widget
			 * \param [out] pWidget Pointer to main widget
			 * \param [out] pToolbarWidget Pointer to (optional) toolbar widget
			 */
			void getWidgets(GtkWidget*& pWidget, GtkWidget*& pToolbarWidget) const;

			/**
			 * \brief Get ID of current view
			 * \return ID of current view
			 */
			ETopographicMap2DView getCurrentView();

			/** \name Callbacks */
			//@{

			void resizeCB(uint32_t ui32Width, uint32_t ui32Height);
			void toggleElectrodesCB();
			void setProjectionCB(GtkWidget* pWidget);
			void setViewCB(GtkWidget* pWidget);
			void setInterpolationCB(GtkWidget* pWidget);
			void setDelayCB(double f64Delay) const;

			//@}

		private:
			//draw color palette
			void drawPalette(uint32_t ui32X, uint32_t ui32Y, uint32_t ui32Width, uint32_t ui32Height) const;

			//draw face (ears, nose, neck)
			void drawFace(uint32_t ui32X, uint32_t ui32Y, uint32_t ui32Width, uint32_t ui32Height) const;

			//draw head
			void drawHead();

			//draw RGB buffer
			void drawPotentials() const;

			//draw electrodes corresponding to visible channels as rings
			void drawElectrodes();

			/**
			 * \brief Get channel position in 2D
			 * \param ui32ChannelIndex[in] Index of channel which position is to be retrieved
			 * \param l_i32ChannelX[out] X coordinate of channel location, if channel is visible
			 * \param l_i32ChannelY[out] Y coordinate of channel location, if channel is visible
			 * \return True if channel is visible in current view, false otherwise
			 */
			bool getChannel2DPosition(uint32_t ui32ChannelIndex, gint& l_i32ChannelX, gint& l_i32ChannelY) const;

			//update RGB buffer with interpolated values
			void refreshPotentials();

			//draw a box in RGB buffer
			void drawBoxToBuffer(uint32_t ui32X, uint32_t ui32Y, uint32_t ui32Width, uint32_t ui32Height, uint8_t ui8Red, uint8_t ui8Green, uint8_t ui8Blue) const;

			void enableElectrodeButtonSignals(bool enable);

			void enableProjectionButtonSignals(bool enable);

			void enableViewButtonSignals(bool enable);

			void enableInterpolationButtonSignals(bool enable);

			/**
			 * \brief Compute normalized coordinates of 2D samples
			 * \remarks This method should first be called with bComputeCoordinates = false, allowing caller
			 * to resize data structures appropriately, and then it may be called with bComputeCoordinates = true
			 * \param bComputeCoordinates If false, this method only computes the number of visible samples
			 * \return Number of visible samples (samples lying within the actual skull area)
			 */
			uint32_t computeSamplesNormalizedCoordinates(bool bComputeCoordinates);

			void resizeData();

			void redrawClipmask();

			double getThetaFromCartesianCoordinates(const double* pCartesianCoords) const;

			double getPhiFromCartesianCoordinates(const double* pCartesianCoords) const;

			bool compute2DCoordinates(double f64Theta, double f64Phi, uint32_t ui32SkullCenterX, uint32_t ui32SkullCenterY, gint& rX, gint& rY) const;

			//! The database that contains the information to use to draw the signals
			CTopographicMapDatabase& m_topographicMapDatabase;

			//Maximum delay that can be applied to displayed data
			double m_maxDelay = 2.0;

			GtkBuilder* m_builderInterface = nullptr;

			GtkWidget* m_drawingArea = nullptr;
			GdkBitmap* m_clipmask = nullptr; //origin (m_skullX, m_skullY)
			uint32_t m_clipmaskWidth = 0;
			uint32_t m_clipmaskHeight = 0;
			GdkGC* m_clipmaskGC = nullptr;
			GdkRegion* m_visibleRegion = nullptr; //reallocated whenever clipmask changes

			GdkColor m_backgroundColor;

			//! Active projection
			ETopographicMap2DProjection m_currentProjection = TopographicMap2DProjection_Radial;
			//! Projection radio buttons
			GtkRadioToolButton* m_axialProjectionButton = nullptr;
			GtkRadioToolButton* m_radialProjectionButton = nullptr;

			//! Active view
			ETopographicMap2DView m_currentView = TopographicMap2DView_Top;
			//! View radio buttons
			GtkRadioToolButton* m_topViewButton = nullptr;
			GtkRadioToolButton* m_leftViewButton = nullptr;
			GtkRadioToolButton* m_rightViewButton = nullptr;
			GtkRadioToolButton* m_backViewButton = nullptr;

			//! Interpolation type
			uint64_t m_currentInterpolation = 0;
			GtkRadioToolButton* m_mapPotentials = nullptr;
			GtkRadioToolButton* m_mapCurrents = nullptr;

			//! Electrodes toggle button
			GtkToggleToolButton* m_electrodesToggleButton = nullptr;
			//! Electrodes toggle state
			bool m_electrodesToggledOn = true;

			bool m_needResize = true;

			uint32_t m_gridSize = 0;
			uint32_t m_cellSize = 0;

			OpenViBE::CMatrix m_sampleCoordinatesMatrix;

			std::vector<uint32_t> m_sampleValues;
			std::vector<std::pair<uint32_t, uint32_t>> m_sample2DCoordinates; //in skull coords

			uint32_t m_minPaletteBarHeight = 10;
			uint32_t m_maxPaletteBarHeight = 30;

			uint32_t m_headWindowWidth = 0;
			uint32_t m_headWindowHeight = 0;

			uint32_t m_paletteWindowWidth = 0;
			uint32_t m_paletteWindowHeight = 0;

			uint32_t m_skullX = 0;
			uint32_t m_skullY = 0;
			uint32_t m_skullDiameter = 0;
			//angles relative to 3 o'clock position, CCW, in degrees
			float m_skullOutlineStartAngle = 0.0;
			float m_skullOutlineEndAngle = 0.0;
			float m_skullFillStartAngle = 0.0;
			float m_skullFillEndAngle = 0.0;

			//determined from m_ui32SkullOutlineEndAngle
			uint32_t m_skullOutlineLeftPointX = 0;
			uint32_t m_skullOutlineLeftPointY = 0;
			//determined from m_ui32SkullOutlineStartAngle
			uint32_t m_skullOutlineRightPointX = 0;
			uint32_t m_skullOutlineRightPointY = 0;

			//determined from m_ui32SkullFillEndAngle
			uint32_t m_skullFillLeftPointX = 0;
			uint32_t m_skullFillLeftPointY = 0;
			//determined from m_ui32SkullFillStartAngle
			uint32_t m_skullFillRightPointX = 0;
			uint32_t m_skullFillRightPointY = 0;

			uint32_t m_skullFillBottomPointX = 0;
			uint32_t m_skullFillBottomPointY = 0;

			/////////////////////////////
			// TOP VIEW
			/////////////////////////////
			uint32_t m_noseY = 0;

			/////////////////////////////
			// BOTTOM VIEW
			/////////////////////////////
			uint32_t m_leftNeckX = 0;
			uint32_t m_leftNeckY = 0;
			uint32_t m_rightNeckX = 0;
			uint32_t m_rightNeckY = 0;

			//////////////////////////////////
			// LEFT/RIGHT VIEWS
			//////////////////////////////////
			/*
				+ A
			   /
			  /
			 /
			+ B
			| C
			+----+ D
				 |
				 + E
			*/
			uint32_t m_noseTopX = 0; //A
			uint32_t m_noseTopY = 0;
			uint32_t m_noseBumpX = 0; //B
			uint32_t m_noseBumpY = 0;
			uint32_t m_noseTipX = 0; //C
			uint32_t m_noseTipY = 0;
			uint32_t m_noseBaseX = 0; //D
			uint32_t m_noseBaseY = 0;
			uint32_t m_noseBottomX = 0; //E
			uint32_t m_noseBottomY = 0;

			/**
			 * \brief Main pixmap
			 * \remarks This pixmap is 32-bit aligned. Each row is m_rowStride wide, and the pixmap has the height of the DrawingArea's
			 * window. It is pasted into the DrawingArea's window upon redraw
			 */
			//TODO
			//GdkPixmap* m_pPixmap;

			/**
			 * \brief Skull pixmap
			 * \remarks This pixmap is 32-bit aligned. Each row is m_rowStride wide, and the pixmap has m_skullDiameter rows.
			 * It is pasted into the main pixmap everytime changes happen (window resizing, display options toggled on/off, etc)
			 */
			guchar* m_skullRGBBuffer = nullptr;
			uint32_t m_rowStride = 0;
		};
	} // namespace SimpleVisualization;
}  // namespace OpenViBEPlugins;
