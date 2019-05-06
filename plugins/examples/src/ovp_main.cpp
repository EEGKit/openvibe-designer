#include "ovp_defines.h"

#include "box-algorithms/ovpCBoxAlgorithmModifiableSettings.h"

OVP_Declare_Begin()
		;
		rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_BoxAlgorithmFlag, OV_AttributeId_Box_FlagIsUnstable.toString(), OV_AttributeId_Box_FlagIsUnstable.toUInteger());

		OVP_Declare_New(OpenViBEPlugins::Examples::CBoxAlgorithmModifiableSettingsDesc);
OVP_Declare_End();
