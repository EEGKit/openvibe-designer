#include "ovpCBoxAlgorithmModifiableSettings.h"

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace Examples;

bool CBoxAlgorithmModifiableSettings::initialize() { return true; }
/*******************************************************************************/

bool CBoxAlgorithmModifiableSettings::uninitialize() { return true; }
/*******************************************************************************/


uint64_t CBoxAlgorithmModifiableSettings::getClockFrequency()

{
	// 4Hz
	return 0x1ULL << 30;
}

bool CBoxAlgorithmModifiableSettings::processClock(IMessageClock& /* messageClock */)
{
	updateSettings();
	//print settings values
	for (size_t i = 0; i < m_SettingsValue.size(); ++i)
	{
		this->getLogManager() << LogLevel_Info << "Setting " << i << " value is " << m_SettingsValue[i] << "\n";
	}
	this->getLogManager() << LogLevel_Info << "\n";

	return true;
}

/*******************************************************************************/

bool CBoxAlgorithmModifiableSettings::updateSettings()
{
	m_SettingsValue.clear();
	const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
	for (uint32_t i = 0; i < l_rStaticBoxContext.getSettingCount(); ++i)
	{
		CString l_sSettingValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		m_SettingsValue.push_back(l_sSettingValue);
	}
	return true;
}


bool CBoxAlgorithmModifiableSettings::process() { return true; }
