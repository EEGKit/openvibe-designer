#pragma once

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gtk/gtk.h>

#include "ovpCTopographicMapDatabase.h"

#include <vector>

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		/**
		 * This class contains everything necessary to setup a GTK window and display
		 * a 2D topographic map
		 */
		class CTopographicMap2DView final : public CTopographicMapDrawable
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
			 * \param mapDatabase Datastore
			 * \param interpolation Interpolation mode
			 * \param delay Delay to apply to displayed data
			 */
			CTopographicMap2DView(CTopographicMapDatabase& mapDatabase, uint64_t interpolation, double delay);

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
			 * \param [in] matrix Pointer to matrix of sample points values
			 * \return True if values were successfully set, false otherwise
			 */
			bool setSampleValuesMatrix(OpenViBE::IMatrix* matrix) override;

			//@}

			/**
			 * \brief Get pointers to plugin main widget and (optional) toolbar widget
			 * \param [out] widget Pointer to main widget
			 * \param [out] toolbar Pointer to (optional) toolbar widget
			 */
			void getWidgets(GtkWidget*& widget, GtkWidget*& toolbar) const;

			/**
			 * \brief Get ID of current view
			 * \return ID of current view
			 */
			ETopographicMap2DView getCurrentView() const { return m_currentView; }

			/** \name Callbacks */
			//@{

			void resizeCB(size_t width, size_t height) { m_needResize = true; }
			void toggleElectrodesCB();
			void setProjectionCB(GtkWidget* widget);
			void setViewCB(GtkWidget* widget);
			void setInterpolationCB(GtkWidget* widget);
			void setDelayCB(const double delay) const { m_mapDatabase.setDelay(delay); }

			//@}

		private:
			//draw color palette
			void drawPalette(size_t x, size_t y, size_t width, size_t height) const;

			//draw face (ears, nose, neck)
			void drawFace(size_t x, size_t y, size_t width, size_t height) const;

			//draw head
			void drawHead() const;

			//draw RGB buffer
			void drawPotentials() const;

			//draw electrodes corresponding to visible channels as rings
			void drawElectrodes() const;

			/**
			 * \brief Get channel position in 2D
			 * \param index[in] Index of channel which position is to be retrieved
			 * \param x[out] X coordinate of channel location, if channel is visible
			 * \param y[out] Y coordinate of channel location, if channel is visible
			 * \return True if channel is visible in current view, false otherwise
			 */
			bool getChannel2DPosition(size_t index, gint& x, gint& y) const;

			//update RGB buffer with interpolated values
			void refreshPotentials();

			//draw a box in RGB buffer
			void drawBoxToBuffer(size_t x, size_t y, size_t width, size_t height, uint8_t red, uint8_t green, uint8_t blue) const;

			void enableElectrodeButtonSignals(bool enable);

			void enableProjectionButtonSignals(bool enable);

			void enableViewButtonSignals(bool enable);

			void enableInterpolationButtonSignals(bool enable);

			/**
			 * \brief Compute normalized coordinates of 2D samples
			 * \remarks This method should first be called with bComputeCoordinates = false, allowing caller
			 * to resize data structures appropriately, and then it may be called with bComputeCoordinates = true
			 * \param all If false, this method only computes the number of visible samples
			 * \return Number of visible samples (samples lying within the actual skull area)
			 */
			size_t computeSamplesNormalizedCoordinates(bool all);

			void resizeData();

			void redrawClipmask();

			double getThetaFromCartesianCoordinates(const std::array<double, 3>& cartesian) const;

			double getPhiFromCartesianCoordinates(const std::array<double, 3>& cartesian) const;

			bool compute2DCoordinates(double theta, double phi, size_t skullCenterX, size_t skullCenterY, gint& x, gint& y) const;

			//! The database that contains the information to use to draw the signals
			CTopographicMapDatabase& m_mapDatabase;

			//Maximum delay that can be applied to displayed data
			double m_maxDelay = 2.0;

			GtkBuilder* m_builderInterface = nullptr;

			GtkWidget* m_drawingArea   = nullptr;
			GdkBitmap* m_clipmask      = nullptr; //origin (m_skullX, m_skullY)
			size_t m_clipmaskWidth     = 0;
			size_t m_clipmaskHeight    = 0;
			GdkGC* m_clipmaskGC        = nullptr;
			GdkRegion* m_visibleRegion = nullptr; //reallocated whenever clipmask changes

			GdkColor m_bgColor;

			//! Active projection
			ETopographicMap2DProjection m_currentProjection = TopographicMap2DProjection_Radial;
			//! Projection radio buttons
			GtkRadioToolButton* m_axialProjectionButton  = nullptr;
			GtkRadioToolButton* m_radialProjectionButton = nullptr;

			//! Active view
			ETopographicMap2DView m_currentView = TopographicMap2DView_Top;
			//! View radio buttons
			GtkRadioToolButton* m_topViewButton   = nullptr;
			GtkRadioToolButton* m_leftViewButton  = nullptr;
			GtkRadioToolButton* m_rightViewButton = nullptr;
			GtkRadioToolButton* m_backViewButton  = nullptr;

			//! Interpolation type
			uint64_t m_currentInterpolation     = 0;
			GtkRadioToolButton* m_mapPotentials = nullptr;
			GtkRadioToolButton* m_mapCurrents   = nullptr;

			//! Electrodes toggle button
			GtkToggleToolButton* m_electrodesToggleButton = nullptr;
			//! Electrodes toggle state
			bool m_electrodesToggledOn = true;

			bool m_needResize = true;

			size_t m_gridSize = 0;
			size_t m_cellSize = 0;

			OpenViBE::CMatrix m_sampleCoordinatesMatrix;

			std::vector<size_t> m_sampleValues;
			std::vector<std::pair<size_t, size_t>> m_sample2DCoordinates; //in skull coords

			size_t m_minPaletteBarHeight = 10;
			size_t m_maxPaletteBarHeight = 30;

			size_t m_headWindowWidth  = 0;
			size_t m_headWindowHeight = 0;

			size_t m_paletteWindowWidth  = 0;
			size_t m_paletteWindowHeight = 0;

			size_t m_skullX        = 0;
			size_t m_skullY        = 0;
			size_t m_skullDiameter = 0;
			//angles relative to 3 o'clock position, CCW, in degrees
			float m_skullOutlineStartAngle = 0.0;
			float m_skullOutlineEndAngle   = 0.0;
			float m_skullFillStartAngle    = 0.0;
			float m_skullFillEndAngle      = 0.0;

			//determined from m_skullOutlineEndAngle
			size_t m_skullOutlineLeftPointX = 0;
			size_t m_skullOutlineLeftPointY = 0;
			//determined from m_skullOutlineStartAngle
			size_t m_skullOutlineRightPointX = 0;
			size_t m_skullOutlineRightPointY = 0;

			//determined from m_skullFillEndAngle
			size_t m_skullFillLeftPointX = 0;
			size_t m_skullFillLeftPointY = 0;
			//determined from m_skullFillStartAngle
			size_t m_skullFillRightPointX = 0;
			size_t m_skullFillRightPointY = 0;

			size_t m_skullFillBottomPointX = 0;
			size_t m_skullFillBottomPointY = 0;

			/////////////////////////////
			// TOP VIEW
			/////////////////////////////
			size_t m_noseY = 0;

			/////////////////////////////
			// BOTTOM VIEW
			/////////////////////////////
			size_t m_leftNeckX  = 0;
			size_t m_leftNeckY  = 0;
			size_t m_rightNeckX = 0;
			size_t m_rightNeckY = 0;

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
			size_t m_noseTopX    = 0; //A
			size_t m_noseTopY    = 0;
			size_t m_noseBumpX   = 0; //B
			size_t m_noseBumpY   = 0;
			size_t m_noseTipX    = 0; //C
			size_t m_noseTipY    = 0;
			size_t m_noseBaseX   = 0; //D
			size_t m_noseBaseY   = 0;
			size_t m_noseBottomX = 0; //E
			size_t m_noseBottomY = 0;

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
			std::vector<guchar> m_skullRGBBuffer;
			size_t m_rowStride = 0;
		};
	} // namespace SimpleVisualization
} // namespace OpenViBEPlugins
