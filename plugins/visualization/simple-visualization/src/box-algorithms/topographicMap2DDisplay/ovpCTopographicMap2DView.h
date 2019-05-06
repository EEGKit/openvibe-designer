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
			 * \param ui64Delay Delay to apply to displayed data
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
			void getWidgets(GtkWidget*& pWidget, GtkWidget*& pToolbarWidget);

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
			void setDelayCB(double f64Delay);

			//@}

		private:
			//draw color palette
			void drawPalette(uint32_t ui32X, uint32_t ui32Y, uint32_t ui32Width, uint32_t ui32Height);

			//draw face (ears, nose, neck)
			void drawFace(uint32_t ui32X, uint32_t ui32Y, uint32_t ui32Width, uint32_t ui32Height);

			//draw head
			void drawHead();

			//draw RGB buffer
			void drawPotentials();

			//draw electrodes corresponding to visible channels as rings
			void drawElectrodes();

			/**
			 * \brief Get channel position in 2D
			 * \param ui32ChannelIndex[in] Index of channel which position is to be retrieved
			 * \param l_i32ChannelX[out] X coordinate of channel location, if channel is visible
			 * \param l_i32ChannelY[out] Y coordinate of channel location, if channel is visible
			 * \return True if channel is visible in current view, false otherwise
			 */
			bool getChannel2DPosition(uint32_t ui32ChannelIndex, gint& l_i32ChannelX, gint& l_i32ChannelY);

			//update RGB buffer with interpolated values
			void refreshPotentials();

			//draw a box in RGB buffer
			void drawBoxToBuffer(uint32_t ui32X, uint32_t ui32Y, uint32_t ui32Width, uint32_t ui32Height, uint8_t ui8Red, uint8_t ui8Green, uint8_t ui8Blue);

			void enableElectrodeButtonSignals(bool bEnable);

			void enableProjectionButtonSignals(bool bEnable);

			void enableViewButtonSignals(bool bEnable);

			void enableInterpolationButtonSignals(bool bEnable);

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

			double getThetaFromCartesianCoordinates(const double* l_pCartesianCoords) const;

			double getPhiFromCartesianCoordinates(const double* l_pCartesianCoords) const;

			bool compute2DCoordinates(double f64Theta, double f64Phi, uint32_t ui32SkullCenterX, uint32_t ui32SkullCenterY, gint& rX, gint& rY) const;

			//! The database that contains the information to use to draw the signals
			CTopographicMapDatabase& m_rTopographicMapDatabase;

			//Maximum delay that can be applied to displayed data
			double m_f64MaxDelay = 2.0;

			GtkBuilder* m_pBuilderInterface = nullptr;

			GtkWidget* m_pDrawingArea = nullptr;
			GdkBitmap* m_pClipmask = nullptr; //origin (m_ui32SkullX, m_ui32SkullY)
			uint32_t m_ui32ClipmaskWidth = 0;
			uint32_t m_ui32ClipmaskHeight = 0;
			GdkGC* m_pClipmaskGC = nullptr;
			GdkRegion* m_pVisibleRegion = nullptr; //reallocated whenever clipmask changes

			GdkColor m_oBackgroundColor;

			//! Active projection
			ETopographicMap2DProjection m_ui32CurrentProjection = TopographicMap2DProjection_Radial;
			//! Projection radio buttons
			GtkRadioToolButton* m_pAxialProjectionButton = nullptr;
			GtkRadioToolButton* m_pRadialProjectionButton = nullptr;

			//! Active view
			ETopographicMap2DView m_ui32CurrentView = TopographicMap2DView_Top;
			//! View radio buttons
			GtkRadioToolButton* m_pTopViewButton = nullptr;
			GtkRadioToolButton* m_pLeftViewButton = nullptr;
			GtkRadioToolButton* m_pRightViewButton = nullptr;
			GtkRadioToolButton* m_pBackViewButton = nullptr;

			//! Interpolation type
			uint64_t m_ui64CurrentInterpolation = 0;
			GtkRadioToolButton* m_pMapPotentials = nullptr;
			GtkRadioToolButton* m_pMapCurrents = nullptr;

			//! Electrodes toggle button
			GtkToggleToolButton* m_pElectrodesToggleButton = nullptr;
			//! Electrodes toggle state
			bool m_bElectrodesToggledOn = true;

			bool m_bNeedResize = true;

			uint32_t m_ui32GridSize = 0;
			uint32_t m_ui32CellSize = 0;

			OpenViBE::CMatrix m_oSampleCoordinatesMatrix;

			std::vector<uint32_t> m_oSampleValues;
			std::vector<std::pair<uint32_t, uint32_t>> m_oSample2DCoordinates; //in skull coords

			uint32_t m_ui32MinPaletteBarHeight = 10;
			uint32_t m_ui32MaxPaletteBarHeight = 30;

			uint32_t m_ui32HeadWindowWidth = 0;
			uint32_t m_ui32HeadWindowHeight = 0;

			uint32_t m_ui32PaletteWindowWidth = 0;
			uint32_t m_ui32PaletteWindowHeight = 0;

			uint32_t m_ui32SkullX = 0;
			uint32_t m_ui32SkullY = 0;
			uint32_t m_ui32SkullDiameter = 0;
			//angles relative to 3 o'clock position, CCW, in degrees
			float m_f32SkullOutlineStartAngle = 0.0;
			float m_f32SkullOutlineEndAngle = 0.0;
			float m_f32SkullFillStartAngle = 0.0;
			float m_f32SkullFillEndAngle = 0.0;

			//determined from m_ui32SkullOutlineEndAngle
			uint32_t m_ui32SkullOutlineLeftPointX = 0;
			uint32_t m_ui32SkullOutlineLeftPointY = 0;
			//determined from m_ui32SkullOutlineStartAngle
			uint32_t m_ui32SkullOutlineRightPointX = 0;
			uint32_t m_ui32SkullOutlineRightPointY = 0;

			//determined from m_ui32SkullFillEndAngle
			uint32_t m_ui32SkullFillLeftPointX = 0;
			uint32_t m_ui32SkullFillLeftPointY = 0;
			//determined from m_ui32SkullFillStartAngle
			uint32_t m_ui32SkullFillRightPointX = 0;
			uint32_t m_ui32SkullFillRightPointY = 0;

			uint32_t m_ui32SkullFillBottomPointX = 0;
			uint32_t m_ui32SkullFillBottomPointY = 0;

			/////////////////////////////
			// TOP VIEW
			/////////////////////////////
			uint32_t m_ui32NoseY = 0;

			/////////////////////////////
			// BOTTOM VIEW
			/////////////////////////////
			uint32_t m_ui32LeftNeckX = 0;
			uint32_t m_ui32LeftNeckY = 0;
			uint32_t m_ui32RightNeckX = 0;
			uint32_t m_ui32RightNeckY = 0;

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
			uint32_t m_ui32NoseTopX = 0; //A
			uint32_t m_ui32NoseTopY = 0;
			uint32_t m_ui32NoseBumpX = 0; //B
			uint32_t m_ui32NoseBumpY = 0;
			uint32_t m_ui32NoseTipX = 0; //C
			uint32_t m_ui32NoseTipY = 0;
			uint32_t m_ui32NoseBaseX = 0; //D
			uint32_t m_ui32NoseBaseY = 0;
			uint32_t m_ui32NoseBottomX = 0; //E
			uint32_t m_ui32NoseBottomY = 0;

			/**
			 * \brief Main pixmap
			 * \remarks This pixmap is 32-bit aligned. Each row is m_ui32RowStride wide, and the pixmap has the height of the DrawingArea's
			 * window. It is pasted into the DrawingArea's window upon redraw
			 */
			//TODO
			//GdkPixmap* m_pPixmap;

			/**
			 * \brief Skull pixmap
			 * \remarks This pixmap is 32-bit aligned. Each row is m_ui32RowStride wide, and the pixmap has m_ui32SkullDiameter rows.
			 * It is pasted into the main pixmap everytime changes happen (window resizing, display options toggled on/off, etc)
			 */
			guchar* m_pSkullRGBBuffer = nullptr;
			uint32_t m_ui32RowStride = 0;
		};
	} // namespace SimpleVisualization;
}  // namespace OpenViBEPlugins;
