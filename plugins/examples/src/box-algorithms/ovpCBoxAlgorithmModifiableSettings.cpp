#include "ovpCBoxAlgorithmModifiableSettings.h"
#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Examples;

bool CBoxAlgorithmModifiableSettings::initialize(void)
{	
	return true;
}
/*******************************************************************************/

bool CBoxAlgorithmModifiableSettings::uninitialize(void)
{
	return true;
}
/*******************************************************************************/


uint64_t CBoxAlgorithmModifiableSettings::getClockFrequency(void)
{
	// 4Hz
	return 0x1ULL << 30;
}

bool CBoxAlgorithmModifiableSettings::processClock(OpenViBE::Kernel::IMessageClock& /* rMessageClock */)
{
	updateSettings();
	//print settings values
	for(size_t i=0; i < m_SettingsValue.size(); ++i)
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
	// const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
	for(uint32_t i=0; i < l_rStaticBoxContext.getSettingCount(); ++i)
	{
		CString l_sSettingValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		m_SettingsValue.push_back(l_sSettingValue);
	}
	return true;
}


bool CBoxAlgorithmModifiableSettings::process(void)
{
	return true;
}
