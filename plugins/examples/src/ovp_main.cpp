#include "ovp_defines.h"

#include "box-algorithms/ovpCBoxAlgorithmModifiableSettings.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Plugins;

OVP_Declare_Begin()
	context.getTypeManager().registerEnumerationEntry(OV_TypeId_BoxAlgorithmFlag, OV_AttributeId_Box_FlagIsUnstable.toString(),
													  OV_AttributeId_Box_FlagIsUnstable.id());

	OVP_Declare_New(Examples::CBoxAlgorithmModifiableSettingsDesc);
OVP_Declare_End()
