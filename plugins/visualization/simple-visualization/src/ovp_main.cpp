#include "ovp_defines.h"

//#include "algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"
//#include "algorithms/ovpCAlgorithmLevelMeasure.h"

//Presentation
//#include "box-algorithms/ovpCGrazVisualization.h"
//#include "box-algorithms/ovpCBoxAlgorithmP300Spellervisualization.h"
//#include "box-algorithms/ovpCBoxAlgorithmP300MagicCardvisualization.h"
//#include "box-algorithms/ovpCBoxAlgorithmP300IdentifierCardvisualization.h"
#include "box-algorithms/ovpCDisplayCueImage.h"

//2D plugins
//#include "box-algorithms/ovpCSignalDisplay.h"
//#include "box-algorithms/ovpCTimeFrequencyMapDisplay.h"
//#include "box-algorithms/ovpCPowerSpectrumDisplay.h"
//#include "box-algorithms/ovpCTopographicMap2DDisplay.h"
//#include "box-algorithms/ovpCBoxAlgorithmLevelMeasure.h"
//#include "box-algorithms/ovpCBoxAlgorithmClassifierAccuracyMeasure.h"
#include "box-algorithms/ovpCBoxAlgorithmMatrixDisplay.h"
//3D plugins
//#include "box-algorithms/ovpCSimple3DDisplay.h"
//#include "box-algorithms/ovpCTopographicMap3DDisplay.h"
//#include "box-algorithms/ovpCVoxelDisplay.h"

#include "box-algorithms/ovpCKeyboardStimulator.h"

OVP_Declare_Begin()

//	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SphericalLinearInterpolationType, "Spherical linear interpolation type");
//	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline (potentials)", OVP_TypeId_SphericalLinearInterpolationType_Spline);
//	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline laplacian (currents)",  OVP_TypeId_SphericalLinearInterpolationType_Laplacian);

//	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SignalDisplayMode, "Signal display mode");
//	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayMode, "Scroll", OVP_TypeId_SignalDisplayMode_Scroll.toUInteger());
//	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayMode, "Scan",  OVP_TypeId_SignalDisplayMode_Scan.toUInteger());

//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CAlgorithmLevelMeasureDesc)
//	OVP_Declare_New(OpenViBEPlugins::Test::CAlgorithmSphericalSplineInterpolationDesc)

//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CGrazVisualizationDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CBoxAlgorithmP300SpellervisualizationDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CBoxAlgorithmP300MagicCardvisualizationDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CDisplayCueImageDesc)

//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CSignalDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CTimeFrequencyMapDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CPowerSpectrumDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CTopographicMap2DDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CBoxAlgorithmLevelMeasureDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CBoxAlgorithmClassifierAccuracyMeasureDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmMatrixDisplayDesc)

//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CSimple3DDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CTopographicMap3DDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CVoxelDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::Simplevisualization::CBoxAlgorithmP300IdentifierCardvisualizationDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmMatrixDisplayDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CKeyboardStimulatorDesc)

OVP_Declare_End()
