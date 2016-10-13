#pragma once

#define OV_TypeId_Color OpenViBE::CIdentifier(0x7F45A2A9, 0x7DB12219)
#define OV_TypeId_ColorGradient OpenViBE::CIdentifier(0x3D3C7C7F, 0xEF0E7129)

#define OVP_ClassId_Plugin_VisualizationContext OpenViBE::CIdentifier(0x05A7171D, 0x78E4FE3C)
#define OVVTK_MetadataIdentifier_VisualizationTree OpenViBE::CIdentifier(0x3BCCE5D2, 0x43F2D968)

//___________________________________________________________________//
//                                                                   //
// API Definition                                                    //
//___________________________________________________________________//
//                                                                   //

// Taken from
// - http://people.redhat.com/drepper/dsohowto.pdf
// - http://www.nedprod.com/programs/gccvisibility.html
#if defined OVVTK_Shared
	#if defined TARGET_OS_Windows
		#define OVVTK_API_Export __declspec(dllexport)
		#define OVVTK_API_Import __declspec(dllimport)
	#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		#define OVVTK_API_Export __attribute__((visibility("default")))
		#define OVVTK_API_Import __attribute__((visibility("default")))
	#else
		#define OVVTK_API_Export
		#define OVVTK_API_Import
	#endif
#else
	#define OVVTK_API_Export
	#define OVVTK_API_Import
#endif

#if defined OVVTK_Exports
	#define OVVTK_API OVVTK_API_Export
#else
	#define OVVTK_API OVVTK_API_Import
#endif

