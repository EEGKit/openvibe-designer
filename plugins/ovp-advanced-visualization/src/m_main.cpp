/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#include <ovp_license_checkout.h>

#include "m_defines.hpp"

// #define TARGET_Has_Experimental

#include "mTBoxAlgorithmStackedContinuousViz.hpp"
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

#include <system/Time.h>

#include <ctime>

#define OID OpenViBE::CIdentifier

namespace Mensia
{
	namespace AdvancedVisualization
	{
		// Prototype to create and release final renderer instances from the renderer API

		template < int iType, bool bStimulation=false >
		class TRendererProto
		{
		public:

			IRenderer* create(void)
			{
				return IRenderer::create(iType, bStimulation);
			}

			void release(IRenderer* pRenderer)
			{
				return IRenderer::release(pRenderer);
			}
		};

		// Type definitions of our OpenViBE boxes

		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_Bitmap, true > ,    TRulerPair < CRulerProgressV,    TRulerPair < TRulerAutoType < IRuler, TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, IRuler >,                   TRulerPair < CRulerLeftChannelNames, CRulerRightTexture > > > > Bitmap;
		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_Line, true > ,      TRulerPair < CRulerProgressV,    TRulerPair < TRulerAutoType < IRuler, TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, IRuler >,                   TRulerPair < CRulerLeftChannelNames, CRulerRightScale > > > > Oscilloscope;
		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_Bars, true > ,      TRulerPair < CRulerProgressV,    TRulerPair < TRulerAutoType < IRuler, TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, IRuler >,                   TRulerPair < CRulerLeftChannelNames, CRulerRightScale > > > > Bars;
		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_MultiLine, true > , TRulerPair < CRulerProgressV,    TRulerPair < TRulerAutoType < IRuler, TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, IRuler >,                   TRulerPair < TRulerPair < CRulerLeftChannelNames, CRulerLeftTexture>, CRulerRightMonoScale > > > > MOscilloscope;
		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_XYZPlot > > XYZPlot;
#if defined TARGET_Has_Experimental
		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_Flower > > Flower;
		typedef TBoxAlgorithmContinuousVizDesc < TRendererProto < IRenderer::RendererType_Mountain > , CRulerBottomTexture > Mountain;
#endif

		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Bitmap > ,             TRulerPair < CRulerERPProgress, TRulerPair < TRulerAutoType < CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerRightTexture > > > > IBitmap;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Line > ,               TRulerPair < CRulerERPProgress, TRulerPair < TRulerAutoType < CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerRightScale > > > > IOscilloscope;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Bars > ,               TRulerPair < CRulerERPProgress, TRulerPair < TRulerAutoType < CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerRightScale > > > > IBars;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_MultiLine > ,          TRulerPair < CRulerERPProgress, TRulerPair < TRulerAutoType < CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency >, TRulerPair < TRulerPair < CRulerLeftChannelNames, CRulerLeftTexture>, CRulerRightMonoScale > > > > MIOscilloscope;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_XYZPlot > > IXYZPlot;
#if defined TARGET_Has_Experimental
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Flower > > IFlower;
#endif

		typedef TBoxAlgorithmStackedContinuousVizDesc < false, true, TRendererProto < IRenderer::RendererType_Bitmap, true > , TRulerPair < TRulerAutoType < CRulerBottomERPCount, CRulerBottomERPTime, CRulerBottomFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerProgressH > > > SVBitmap;
		typedef TBoxAlgorithmStackedContinuousVizDesc < true,  true, TRendererProto < IRenderer::RendererType_Bitmap, true > , TRulerPair < TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, TRulerPair < TRulerAutoType < IRuler, IRuler, CRulerRightFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerProgressV > > > > SHBitmap;
#if defined TARGET_Has_Experimental
		typedef TBoxAlgorithmStackedContinuousVizDesc < false, false, TRendererProto < IRenderer::RendererType_Slice > , CRulerBottomTexture > StackedSlice;
#endif

		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_2DTopography > , CRulerBottomTexture > Topography2D;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_3DTopography > , CRulerBottomTexture > Topography3D;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Loreta > , CRulerBottomTexture, TBoxAlgorithmInstantLoretaViz > SLoreta;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Cube > , CRulerBottomTexture > Cubes;
#if defined TARGET_Has_Experimental
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Connectivity > , CRulerBottomTexture > Connectivity;
		typedef TBoxAlgorithmInstantVizDesc < TRendererProto < IRenderer::RendererType_Mountain > , CRulerBottomTexture > IMountain;
#endif
	}
}

OVP_Declare_Begin()

#if 1
	std::string l_sError1;
	std::string l_sError2;
	if(!Mensia::License::licenseCheckOut("neurort-studio", "2.0", "viz", l_sError1))
	{
		if(!Mensia::License::licenseCheckOut("neurort-engine", "2.0", "viz", l_sError2))
		{
			rPluginModuleContext.getLogManager() << OpenViBE::Kernel::LogLevel_ImportantWarning << "Could not checkout license :\n" << OpenViBE::CString(l_sError1.c_str());
			rPluginModuleContext.getLogManager() << OpenViBE::Kernel::LogLevel_ImportantWarning << "Could not checkout license :\n" << OpenViBE::CString(l_sError2.c_str());
			rPluginModuleContext.getLogManager() << OpenViBE::Kernel::LogLevel_ImportantWarning << "Please get in touch with license@mensiatech.com\n";
			return false;
		}
	}
#endif

#if 0
OID()
OID()
OID()
OID()
OID(0x063786E2D2FC39D5)
#endif

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_TemporalCoherence, "Temporal Coherence");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_TemporalCoherence, "Time Locked", OVP_TypeId_TemporalCoherence_TimeLocked.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_TemporalCoherence, "Independant", OVP_TypeId_TemporalCoherence_Independant.toUInteger());

	using namespace Mensia::AdvancedVisualization;

	OVP_Declare_New(Mensia::AdvancedVisualization::Bitmap        ("Continuous Bitmap",                    OID(0x0BE99978487D3DC6), OID(0x9E1CD34586569E7E), "2.0.0.0", "2.2.0.0", CParameterSet(I_Matrix, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_ColorGradient, P_None),                                        OpenViBE::CString("Displays the input matrices as a map of colored tiles, or bitmap, continuously."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::Oscilloscope  ("Continuous Oscilloscope",              OID(0x6D63896D86685134), OID(0x0842BCD1D53C1C89), "2.0.0.0", "2.2.0.0", CParameterSet(I_Signal, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_Color, P_None),                OpenViBE::CString("Displays the input matrices as a series of curves.\nChannels are vertically distributed."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::Bars          ("Continuous Bars",                      OID(0xE6B2C8A8B43DE2C3), OID(0x24510F3EFCBE83A2), "2.0.0.0", "2.2.0.0", CParameterSet(I_Matrix, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, P_None),        OpenViBE::CString("Displays the input matrices as a series of colored bars.\nChannels are vertically distributed."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::MOscilloscope ("Continuous Multi Oscilloscope",        OID(0xD4C4DCEE6F5D47E1), OID(0xA927FB20C4089F99), "2.0.0.0", "2.2.0.0", CParameterSet(I_Signal, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, P_None),        OpenViBE::CString("Displays the input matrices as a series of curves.\nAll channels are rendered on the same vertical axis, with different colors."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::XYZPlot       ("Continuous XYZ Plot",                  OID(0xAAFD16CF57DC7138), OID(0xDDA9A9D9D04F2C14), "2.4.0.0", "2.4.0.0", CParameterSet(I_Signal, /*S_ChannelLocalisation, S_DataPositive, */S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_Translucency, S_ShowAxis, S_XYZPlotHasDepth, S_ColorGradient, F_FixedChannelOrder, F_FixedChannelSelection, P_None), OpenViBE::CString("Displays the input matrices as a series of points in 2D or 3D space, continuously."), OpenViBE::CString("")))
#if defined TARGET_Has_Experimental
	OVP_Declare_New(Mensia::AdvancedVisualization::Flower        ("Continuous Flower",                    OID(0x1CB8CFFBC2864860), OID(0xDA407F0B276FB27C), "experimental", "experimental", CParameterSet(I_Signal, /*S_ChannelLocalisation, S_DataPositive, */S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_FlowerRingCount, S_Translucency, S_ColorGradient, F_Unstable, P_None), OpenViBE::CString("TODO"), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::Mountain      ("Continuous Mountain",                  OID(0x390CEC93464DCD39), OID(0x80EDBCC16CB1CE73), "experimental", "experimental", CParameterSet(I_Signal, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_Translucency, S_DataPositive, S_ColorGradient, F_Unstable, P_None), OpenViBE::CString("TODO"), OpenViBE::CString("")))
#endif

	OVP_Declare_New(Mensia::AdvancedVisualization::IBitmap       ("Instant Bitmap",                       OID(0x942CDA76A437E83E), OID(0xF862765845247823), "2.0.0.0", "2.0.0.0", CParameterSet(I_Spectrum, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, P_None),                                                                          OpenViBE::CString("Displays each and every input matrix as a map of colored tiles, or bitmap, instantly."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::IOscilloscope ("Instant Oscilloscope",                 OID(0x1207B96D19A805B4), OID(0x997B654D0CDB3102), "2.0.0.0", "2.0.0.0", CParameterSet(I_Signal, S_ChannelLocalisation, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_Color, F_CanAddInput, P_None),                                   OpenViBE::CString("Displays each and every input matrix as a series of curves, instantly.\nChannels are vertically distributed."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::IBars         ("Instant Bars",                         OID(0xD401BE1F25F01E4E), OID(0xECB4608196DA0D49), "2.0.0.0", "2.0.0.0", CParameterSet(I_Spectrum, S_ChannelLocalisation, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, P_None),                                          OpenViBE::CString("Displays each and every input matrix as a series of colored bars, instantly.\nChannels are vertically distributed."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::MIOscilloscope("Instant Multi Oscilloscope",           OID(0x4F142FE87C7773E3), OID(0x072B75A9C28D588A), "2.0.0.0", "2.0.0.0", CParameterSet(I_Signal, S_ChannelLocalisation, S_DataPositive, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, P_None),                                          OpenViBE::CString("Displays each and every input matrix as a series of curves, instantly.\nAll channels are rendered on the same vertical axis, with different colors."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::IXYZPlot      ("Instant XYZ Plot",                     OID(0x1353BED819EC86A5), OID(0xDB19374A41A22DD3), "2.4.0.0", "2.4.0.0", CParameterSet(I_Signal, /*S_ChannelLocalisation, S_DataPositive, */S_DataScale, S_Caption, S_Translucency, S_ShowAxis, S_XYZPlotHasDepth, S_ColorGradient, F_FixedChannelOrder, F_FixedChannelSelection, /*F_CanAddInput, */P_None), OpenViBE::CString("Displays the input matrices as a series of points in 2D or 3D space, instantly."), OpenViBE::CString("")))
#if defined TARGET_Has_Experimental
	OVP_Declare_New(Mensia::AdvancedVisualization::IFlower       ("Instant Flower",                       OID(0xD203F5AF4FEF6671), OID(0x51FED5D6CFE84307), "experimental", "experimental", CParameterSet(I_Signal, /*S_ChannelLocalisation, S_DataPositive, */S_DataScale, S_Caption, S_FlowerRingCount, S_Translucency, S_ColorGradient, /*F_CanAddInput, */F_Unstable, P_None), OpenViBE::CString("TODO"), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::IMountain     ("Instant Mountain",                     OID(0x2AF5BEF3E505C8FC), OID(0xDCDB2E6B32EEA55A), "experimental", "experimental", CParameterSet(I_Signal, S_ChannelLocalisation, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, /*F_CanAddInput, */F_Unstable, P_None), OpenViBE::CString("TODO"), OpenViBE::CString("")))
#endif

	OVP_Declare_New(Mensia::AdvancedVisualization::SVBitmap      ("Stacked Bitmap (Vertical)",            OID(0x93F400CF9F6C5AFD), OID(0x9926A761BA82D233), "2.0.0.0", "2.2.0.0", CParameterSet(I_Matrix, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_ColorGradient, P_None), OpenViBE::CString("Displays the input matrices as a map of colored tiles, or bitmap, continuously.\nAll the bitmaps are stacked vertically, starting from the bottom edge of the window"), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::SHBitmap      ("Stacked Bitmap (Horizontal)",          OID(0x6B22DC653AB0EC0F), OID(0x7B0DDB65FDC51488), "2.0.0.0", "2.2.0.0", CParameterSet(I_Matrix, I_Stimulations, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_ColorGradient, P_None), OpenViBE::CString("Displays the input matrices as a map of colored tiles, or bitmap, continuously.\nAll the bitmaps are stacked horizontally, starting from the left edge of the window"), OpenViBE::CString("")))
#if defined TARGET_Has_Experimental
	OVP_Declare_New(Mensia::AdvancedVisualization::StackedSlice  ("Stacked Slice",                        OID(0x52B9CEEC453FA529), OID(0x339E3C89A506172B), "experimental", "experimental", CParameterSet(I_Matrix, S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, F_Unstable, P_None), OpenViBE::CString("TODO"), OpenViBE::CString("")))
#endif

	OVP_Declare_New(Mensia::AdvancedVisualization::Topography2D  ("2D Topography",                        OID(0x0A0C12A7E695F3FB), OID(0x7C3A05B8C45386F8), "2.0.0.0", "2.0.0.0", CParameterSet(I_Signal, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, F_FixedChannelOrder, P_None), OpenViBE::CString("The input is mapped to a 2 dimensional plane model of the scalp surface, using a color gradient and interpolation."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::Topography3D  ("3D Topography",                        OID(0xA3F1CF20DEC477F3), OID(0xC709EA84B577D910), "2.0.0.0", "2.0.0.0", CParameterSet(I_Signal, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, F_FixedChannelOrder, P_None), OpenViBE::CString("The input is mapped to a 3D model of the scalp, using a color gradient and interpolation."), OpenViBE::CString("")))
	OVP_Declare_New(Mensia::AdvancedVisualization::Cubes         ("3D Cubes",                             OID(0x6307469DF938EF27), OID(0xE028305D4ACF9D1A), "2.0.0.0", "2.0.0.0", CParameterSet(I_Signal, S_ChannelLocalisation, S_DataScale, S_Caption, S_ColorGradient, F_FixedChannelOrder, P_None), OpenViBE::CString("The input is mapped to a 3D representation of the EEG setup where electrodes are symbolized with cubes,\nusing a color gradient and interpolation."), OpenViBE::CString("")))
#if defined TARGET_Has_Experimental
	OVP_Declare_New(Mensia::AdvancedVisualization::Connectivity  ("3D Channel Link",                      OID(0xF16C8EB9E3749A93), OID(0x89D9A77C3A2DD8ED), "experimental", "experimental", CParameterSet(I_Covariance, S_ChannelLocalisation, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, F_FixedChannelOrder, F_Unstable, P_None), OpenViBE::CString("TODO"), OpenViBE::CString("")))
#endif
	OVP_Declare_New(Mensia::AdvancedVisualization::SLoreta       ("3D Tomographic Visualization",         OID(0xD5F62E7685E0D97C), OID(0xFD0826035C47E922), "2.0.0.0", "2.2.0.0", CParameterSet(I_Signal, S_DataScale, S_Caption, S_Translucency, S_ColorGradient, F_FixedChannelOrder, P_None), OpenViBE::CString("Displays sources activity by mapping it to voxels in a 3D model of the scalp."), OpenViBE::CString("")))

OVP_Declare_End()
