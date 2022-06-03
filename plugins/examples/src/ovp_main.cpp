#include "ovp_defines.h"

#include "box-algorithms/ovpCBoxAlgorithmModifiableSettings.h"

OVP_Declare_Begin()
	context.getTypeManager().registerEnumerationEntry(OV_TypeId_BoxAlgorithmFlag, OV_AttributeId_Box_FlagIsUnstable.toString(),
													  OV_AttributeId_Box_FlagIsUnstable.id());

	OVP_Declare_New(OpenViBE::Plugins::Examples::CBoxAlgorithmModifiableSettingsDesc)
OVP_Declare_End()
