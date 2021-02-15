#include "ovpCBoxAlgorithmModifiableSettings.h"

namespace OpenViBE {
namespace Plugins {
namespace Examples {
//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmModifiableSettings::processClock(Kernel::CMessageClock& /*msg*/)
{
	updateSettings();
	//print settings values
	for (size_t i = 0; i < m_SettingsValue.size(); ++i)
	{
		this->getLogManager() << Kernel::LogLevel_Info << "Setting " << i << " value is " << m_SettingsValue[i] << "\n";
	}
	this->getLogManager() << Kernel::LogLevel_Info << "\n";

	return true;
}

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmModifiableSettings::updateSettings()
{
	m_SettingsValue.clear();
	const size_t nSetting = this->getStaticBoxContext().getSettingCount();
	for (size_t i = 0; i < nSetting; ++i)
	{
		CString value = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		m_SettingsValue.push_back(value);
	}
	return true;
}

}  // namespace Examples
}  // namespace Plugins
}  // namespace OpenViBE
