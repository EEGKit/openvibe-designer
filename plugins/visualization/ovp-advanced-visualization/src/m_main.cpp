/*********************************************************************
 * Software License Agreement (AGPL-3 License)
 *
 * OpenViBE Designer
 * Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
 * Copyright (C) Inria, 2015-2017,V1.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "m_defines.hpp"

// #define TARGET_Has_Experimental

#include "mTBoxAlgorithmStackedContinuousViz.hpp"
#include "mTBoxAlgorithmStackedInstantViz.hpp"
#include "mTBoxAlgorithmContinuousViz.hpp"
#include "mTBoxAlgorithmInstantViz.hpp"
#include "mTBoxAlgorithmInstantLoretaViz.hpp"

#include "ruler/mTRulerAutoType.hpp"
#include "ruler/mTRulerPair.hpp"
#include "ruler/mTRulerConditionalPair.hpp"
#include "ruler/mCRulerConditionIsTimeLocked.hpp"

#include "ruler/mCRulerProgress.hpp"
#include "ruler/mCRulerProgressH.hpp"
#include "ruler/mCRulerProgressV.hpp"
#include "ruler/mCRulerERPProgress.hpp"

#include "ruler/mCRulerBottomCount.hpp"
#include "ruler/mCRulerBottomERPCount.hpp"
#include "ruler/mCRulerBottomERPTime.hpp"
#include "ruler/mCRulerBottomFrequency.hpp"
#include "ruler/mCRulerBottomPercent.hpp"
#include "ruler/mCRulerBottomTexture.hpp"
#include "ruler/mCRulerBottomTime.hpp"

#include "ruler/mCRulerLeftChannelNames.hpp"
#include "ruler/mCRulerLeftTexture.hpp"

#include "ruler/mCRulerRightCount.hpp"
#include "ruler/mCRulerRightFrequency.hpp"
#include "ruler/mCRulerRightMonoScale.hpp"
#include "ruler/mCRulerRightTexture.hpp"
#include "ruler/mCRulerRightScale.hpp"
#include "ruler/mCRulerRightLabels.hpp"

#include <system/ovCTime.h>

#include <ctime>

namespace OpenViBE {
namespace AdvancedVisualization {
// Prototype to create and release final renderer instances from the renderer API

template <ERendererType TType, bool TStimulation = false>
class TRendererProto
{
public:
	static IRenderer* create() { return IRenderer::create(TType, TStimulation); }
	static void release(IRenderer* renderer) { return IRenderer::release(renderer); }
};

// Type definitions of our OpenViBE boxes

typedef TBoxAlgorithmContinuousVizDesc<TRendererProto<ERendererType::Bitmap, true>, TRulerPair<
										   CRulerProgressV, TRulerPair<
											   TRulerAutoType<IRuler, TRulerConditionalPair<CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked>,
															  IRuler>, TRulerPair<CRulerLeftChannelNames, CRulerRightTexture>>>> bitmap_t;
typedef TBoxAlgorithmContinuousVizDesc<TRendererProto<ERendererType::Line, true>, TRulerPair<
										   CRulerProgressV, TRulerPair<
											   TRulerAutoType<IRuler, TRulerConditionalPair<CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked>,
															  IRuler>, TRulerPair<CRulerLeftChannelNames, CRulerRightScale>>>> oscilloscope_t;
typedef TBoxAlgorithmContinuousVizDesc<TRendererProto<ERendererType::Bars, true>, TRulerPair<
										   CRulerProgressV, TRulerPair<
											   TRulerAutoType<IRuler, TRulerConditionalPair<CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked>,
															  IRuler>, TRulerPair<CRulerLeftChannelNames, CRulerRightScale>>>> bars_t;
typedef TBoxAlgorithmContinuousVizDesc<TRendererProto<ERendererType::MultiLine, true>, TRulerPair<
										   CRulerProgressV, TRulerPair<
											   TRulerAutoType<IRuler, TRulerConditionalPair<CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked>,
															  IRuler>, TRulerPair<
												   TRulerPair<CRulerLeftChannelNames, CRulerLeftTexture>, CRulerRightMonoScale>>>> MOscilloscope;
typedef TBoxAlgorithmContinuousVizDesc<TRendererProto<ERendererType::XYZPlot>> xyz_plot_t;

typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Bitmap>, TRulerPair<
										CRulerERPProgress, TRulerPair<
											TRulerAutoType<CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency>, TRulerPair<
												CRulerLeftChannelNames, CRulerRightTexture>>>> i_bitmap_t;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Line>, TRulerPair<
										CRulerERPProgress, TRulerPair<
											TRulerAutoType<CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency>, TRulerPair<
												CRulerLeftChannelNames, CRulerRightScale>>>> i_oscilloscope_t;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Bars>, TRulerPair<
										CRulerERPProgress, TRulerPair<
											TRulerAutoType<CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency>, TRulerPair<
												CRulerLeftChannelNames, CRulerRightScale>>>> i_bars_t;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::MultiLine>, TRulerPair<
										CRulerERPProgress, TRulerPair<
											TRulerAutoType<CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency>, TRulerPair<
												TRulerPair<CRulerLeftChannelNames, CRulerLeftTexture>, CRulerRightMonoScale>>>> MIOscilloscope;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::XYZPlot>> i_xyz_plot_t;

typedef TBoxAlgorithmStackedContinuousVizDesc<false, true, TRendererProto<ERendererType::Bitmap, true>, TRulerPair<
												  TRulerAutoType<CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency>, TRulerPair<
													  CRulerLeftChannelNames, CRulerProgressH>>> sv_bitmap_t;
typedef TBoxAlgorithmStackedContinuousVizDesc<true, true, TRendererProto<ERendererType::Bitmap, true>, TRulerPair<
												  TRulerConditionalPair<CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked>, TRulerPair<
													  TRulerAutoType<IRuler, IRuler, CRulerRightFrequency>, TRulerPair<CRulerLeftChannelNames, CRulerProgressV>>
											  >> sh_bitmap_t;
typedef TBoxAlgorithmStackedInstantVizDesc<true, TRendererProto<ERendererType::Bitmap, true>, TRulerPair<
											   CRulerBottomERPTime, TRulerPair<CRulerRightLabels<1>, TRulerPair<CRulerLeftChannelNames, CRulerProgressV>>>>
si_bitmap_t;

typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Topography2D>, CRulerBottomTexture> topography_2d_t;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Topography3D>, CRulerBottomTexture> topography_3d_t;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Loreta>, CRulerBottomTexture, TBoxAlgorithmInstantLoretaViz> loreta_t;
typedef TBoxAlgorithmInstantVizDesc<TRendererProto<ERendererType::Cube>, CRulerBottomTexture> cubes_t;

OVP_Declare_Begin()
	context.getTypeManager().registerEnumerationType(OVP_TypeId_TemporalCoherence, "Temporal Coherence");
	context.getTypeManager().registerEnumerationEntry(OVP_TypeId_TemporalCoherence, "Time Locked", size_t(ETemporalCoherence::TimeLocked));
	context.getTypeManager().registerEnumerationEntry(OVP_TypeId_TemporalCoherence, "Independant", size_t(ETemporalCoherence::Independant));

	OVP_Declare_New(bitmap_t("Continuous bitmap_t", CIdentifier(0x0BE99978487D3DC6), CIdentifier(0x9E1CD34586569E7E), "0.0.0", "0.0.0",
		CParameterSet(I_Matrix,I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_ColorGradient,
			P_None),
		"Displays the input matrices as a map of colored tiles, or bitmap, continuously.", ""))

	OVP_Declare_New(oscilloscope_t("Continuous oscilloscope_t", CIdentifier(0x6D63896D86685134), CIdentifier(0x0842BCD1D53C1C89), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataPositive, S_DataScale, S_Caption,
			S_Translucency, S_Color, P_None),
		"Displays the input matrices as a series of curves.\nChannels are vertically distributed.",""))

	OVP_Declare_New(bars_t("Continuous bars_t", CIdentifier(0xE6B2C8A8B43DE2C3), CIdentifier(0x24510F3EFCBE83A2), "0.0.0", "0.0.0",
		CParameterSet(I_Matrix,I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataPositive, S_DataScale, S_Caption,
			S_Translucency,S_ColorGradient, P_None),
		"Displays the input matrices as a series of colored bars.\nChannels are vertically distributed.",""))

	OVP_Declare_New(MOscilloscope("Continuous Multi oscilloscope_t", CIdentifier(0xD4C4DCEE6F5D47E1), CIdentifier(0xA927FB20C4089F99), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataPositive, S_DataScale,
			S_Caption, S_Translucency, S_ColorGradient, P_None),
		"Displays the input matrices as a series of curves.\nAll channels are rendered on the same vertical axis, with different colors.", ""))

	OVP_Declare_New(xyz_plot_t("Continuous XYZ Plot", CIdentifier(0xAAFD16CF57DC7138), CIdentifier(0xDDA9A9D9D04F2C14), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_Translucency, S_ShowAxis,S_XYZPlotHasDepth,
			S_ColorGradient, F_FixedChannelOrder, F_FixedChannelSelection, P_None),
		"Displays the input matrices as a series of points in 2D or 3D space, continuously.", ""))


	OVP_Declare_New(i_bitmap_t("Instant bitmap_t", CIdentifier(0x942CDA76A437E83E), CIdentifier(0xF862765845247823), "0.0.0", "0.0.0",
		CParameterSet(I_Spectrum, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, P_None),
		"Displays each and every input matrix as a map of colored tiles, or bitmap, instantly.", ""))

	OVP_Declare_New(i_oscilloscope_t("Instant oscilloscope_t", CIdentifier(0x1207B96D19A805B4), CIdentifier(0x997B654D0CDB3102), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, S_ChannelLocalisation, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_Color, F_CanAddInput, P_None),
		"Displays each and every input matrix as a series of curves, instantly.\nChannels are vertically distributed.", ""))

	OVP_Declare_New(i_bars_t("Instant bars_t", CIdentifier(0xD401BE1F25F01E4E), CIdentifier(0xECB4608196DA0D49), "0.0.0", "0.0.0",
		CParameterSet(I_Spectrum,S_ChannelLocalisation, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, P_None),
		"Displays each and every input matrix as a series of colored bars, instantly.\nChannels are vertically distributed.", ""))

	OVP_Declare_New(MIOscilloscope("Instant Multi oscilloscope_t", CIdentifier(0x4F142FE87C7773E3), CIdentifier(0x072B75A9C28D588A), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, S_ChannelLocalisation, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, P_None),
		"Displays each and every input matrix as a series of curves, instantly.\nAll channels are rendered on the same vertical axis, with different colors.",
		""))

	OVP_Declare_New(i_xyz_plot_t("Instant XYZ Plot", CIdentifier(0x1353BED819EC86A5), CIdentifier(0xDB19374A41A22DD3), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, S_DataScale, S_Caption, S_Translucency, S_ShowAxis, S_XYZPlotHasDepth, S_ColorGradient,F_FixedChannelOrder,
			F_FixedChannelSelection, P_None),
		"Displays the input matrices as a series of points in 2D or 3D space, instantly.", ""))


	OVP_Declare_New(sv_bitmap_t("Stacked bitmap_t (Vertical)", CIdentifier(0x93F400CF9F6C5AFD), CIdentifier(0x9926A761BA82D233), "0.0.0", "0.0.0",
		CParameterSet(I_Matrix, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_ColorGradient
			,P_None),
		"Displays the input matrices as a map of colored tiles, or bitmap, continuously.\nAll the bitmaps are stacked vertically, starting from the bottom edge of the window"
		, ""))

	OVP_Declare_New(sh_bitmap_t("Stacked bitmap_t (Horizontal)", CIdentifier(0x6B22DC653AB0EC0F), CIdentifier(0x7B0DDB65FDC51488), "0.0.0", "0.0.0",
		CParameterSet(I_Matrix, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_ColorGradient
			, P_None),
		"Displays the input matrices as a map of colored tiles, or bitmap, continuously.\nAll the bitmaps are stacked horizontally, starting from the left edge of the window"
		, ""))

	OVP_Declare_New(si_bitmap_t("Instant bitmap_t (3D Stream)", CIdentifier(0x0C61E7632A5C3178), CIdentifier(0xC3CC8B43EE985C1D), "0.1.0", "0.1.0",
		CParameterSet(I_TimeFrequency, I_Stimulations, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, P_None),
		"Displays the input matrices as a map of colored tiles, or bitmap, continuously.", ""))

	OVP_Declare_New(topography_2d_t("2D Topography", CIdentifier(0x0A0C12A7E695F3FB), CIdentifier(0x7C3A05B8C45386F8), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, F_FixedChannelOrder, P_None),
		"The input is mapped to a 2 dimensional plane model of the scalp surface, using a color gradient and interpolation.", ""))

	OVP_Declare_New(topography_3d_t("3D Topography", CIdentifier(0xA3F1CF20DEC477F3), CIdentifier(0xC709EA84B577D910), "0.0.0", "0.0.0",
		CParameterSet(I_Signal,S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, F_FixedChannelOrder, P_None),
		"The input is mapped to a 3D model of the scalp, using a color gradient and interpolation.", ""))

	OVP_Declare_New(cubes_t("3D cubes_t", CIdentifier(0x6307469DF938EF27), CIdentifier(0xE028305D4ACF9D1A), "0.0.0", "0.0.0",
		CParameterSet(I_Signal,S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, F_FixedChannelOrder, P_None),
		"The input is mapped to a 3D representation of the EEG setup where electrodes are symbolized with cubes,\nusing a color gradient and interpolation.", ""
	))

	OVP_Declare_New(loreta_t("3D Tomographic Visualization", CIdentifier(0xD5F62E7685E0D97C), CIdentifier(0xFD0826035C47E922), "0.0.0", "0.0.0",
		CParameterSet(I_Signal, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, F_FixedChannelOrder, P_None),
		"Displays sources activity by mapping it to voxels in a 3D model of the scalp.", ""))

OVP_Declare_End()

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
