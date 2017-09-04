#pragma once

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include <vector>

#define OVP_ClassId_BoxAlgorithm_ModifiableSettings OpenViBE::CIdentifier(0x4AB0DD05, 0x32155D41)
#define OVP_ClassId_BoxAlgorithm_ModifiableSettingsDesc OpenViBE::CIdentifier(0x3808515D, 0x97C7F9B6)

namespace OpenViBEPlugins
{
	namespace Examples
	{
		/**
		 * \class CBoxAlgorithmModifiableSettings
		 * \author lmahe (Inria)
		 * \date Mon Oct 14 16:35:48 2013
		 * \brief The class CBoxAlgorithmModifiableSettings describes the box ModifiableSettings.
		 *
		 */
		class CBoxAlgorithmModifiableSettings : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }
			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool processClock(OpenViBE::CMessageClock& rMessageClock);
			virtual uint64_t getClockFrequency(void);
			
			virtual bool process(void);
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ModifiableSettings);

		protected:
			bool updateSettings(void);

			std::vector<OpenViBE::CString> m_SettingsValue;
		};

		/**
		 * \class CBoxAlgorithmModifiableSettingsDesc
		 * \author lmahe (Inria)
		 * \date Mon Oct 14 16:35:48 2013
		 * \brief Descriptor of the box ModifiableSettings.
		 *
		 */
		class CBoxAlgorithmModifiableSettingsDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Modifiable Settings example"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("lmahe"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Settings of this box are modifiable during playback. Values are displayed in log every 5 seconds"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("This box purpose is to test and demonstrate the modifiable settings feature.\n It has a setting of each type and all are modifiable during scenario playback.\n"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Examples/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString(""); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ModifiableSettings; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Examples::CBoxAlgorithmModifiableSettings; }
			
			virtual bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}


			virtual bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addSetting("Int",OV_TypeId_Integer,"1", true);
				rBoxAlgorithmPrototype.addSetting("Float",OV_TypeId_Float,"1.3", true);
				rBoxAlgorithmPrototype.addSetting("Bool",OV_TypeId_Boolean,"false", true);
				rBoxAlgorithmPrototype.addSetting("String",OV_TypeId_String,"string", true);
				rBoxAlgorithmPrototype.addSetting("filename",OV_TypeId_Filename, "somefile.txt", true);
				rBoxAlgorithmPrototype.addSetting("script",OV_TypeId_Script, "somescript.lua", true);
				rBoxAlgorithmPrototype.addSetting("color",OV_TypeId_Color, "20,65,90", true);
				rBoxAlgorithmPrototype.addSetting("colorgradient",OV_TypeId_ColorGradient, "0:0,0,0; 100:60,40,40", true);
				rBoxAlgorithmPrototype.addSetting("unit",OV_TypeId_MeasurementUnit, "V", true);
				rBoxAlgorithmPrototype.addSetting("factor",OV_TypeId_Factor, "1e-01", true);

				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);

				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ModifiableSettingsDesc);
		};
	};
};
