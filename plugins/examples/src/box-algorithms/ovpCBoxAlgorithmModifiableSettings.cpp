#include "ovpCBoxAlgorithmModifiableSettings.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Plugins;

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
	const size_t nSetting = this->getStaticBoxContext().getSettingCount();
	for (size_t i = 0; i < nSetting; ++i)
	{
		CString value = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		m_SettingsValue.push_back(value);
	}
	return true;
}


bool CBoxAlgorithmModifiableSettings::process() { return true; }
