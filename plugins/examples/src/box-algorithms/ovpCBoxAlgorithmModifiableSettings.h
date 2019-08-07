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
		class CBoxAlgorithmModifiableSettings : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			void release() override { delete this; }
			bool initialize() override;
			bool uninitialize() override;
			bool processClock(OpenViBE::CMessageClock& rMessageClock) override;
			uint64_t getClockFrequency() override;

			bool process() override;
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ModifiableSettings)

		protected:
			bool updateSettings();

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

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Modifiable Settings example"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("lmahe"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("Inria"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Settings of this box are modifiable during playback. Values are displayed in log every 5 seconds"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("This box purpose is to test and demonstrate the modifiable settings feature.\n It has a setting of each type and all are modifiable during scenario playback.\n"); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Examples/Basic"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString(""); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_ModifiableSettings; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmModifiableSettings; }

			bool hasFunctionality(const OpenViBE::CIdentifier functionalityIdentifier) const override { return functionalityIdentifier == OVD_Functionality_Visualization; }


			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const override
			{
				rBoxAlgorithmPrototype.addSetting("Int", OV_TypeId_Integer, "1", true);
				rBoxAlgorithmPrototype.addSetting("Float", OV_TypeId_Float, "1.3", true);
				rBoxAlgorithmPrototype.addSetting("Bool", OV_TypeId_Boolean, "false", true);
				rBoxAlgorithmPrototype.addSetting("String", OV_TypeId_String, "string", true);
				rBoxAlgorithmPrototype.addSetting("filename", OV_TypeId_Filename, "somefile.txt", true);
				rBoxAlgorithmPrototype.addSetting("script", OV_TypeId_Script, "somescript.lua", true);
				rBoxAlgorithmPrototype.addSetting("color", OV_TypeId_Color, "20,65,90", true);
				rBoxAlgorithmPrototype.addSetting("colorgradient", OV_TypeId_ColorGradient, "0:0,0,0; 100:60,40,40", true);
				rBoxAlgorithmPrototype.addSetting("unit", OV_TypeId_MeasurementUnit, "V", true);
				rBoxAlgorithmPrototype.addSetting("factor", OV_TypeId_Factor, "1e-01", true);

				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ModifiableSettingsDesc)
		};
	} // namespace Examples
} // namespace OpenViBEPlugins
