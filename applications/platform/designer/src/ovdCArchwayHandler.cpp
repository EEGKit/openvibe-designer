/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#if defined TARGET_HAS_LibArchway

#include "ovdCArchwayHandler.h"
#include <mensia/base/m_string.h>

#include <openvibe/ov_directories.h>
#include <openvibe/kernel/log/ovILogManager.h>
#include <openvibe/kernel/configuration/ovIConfigurationManager.h>
#include <fs/Files.h>

#include <ios>
#include <cassert>
#include <fstream>

using namespace Mensia;
using namespace OpenViBE::Kernel;
using namespace std;

const std::string CArchwayHandler::m_sArchwayConfigurationFile = (OpenViBE::Directories::getUserDataDir() + "/studio-archway.conf").toASCIIString();
const std::string CArchwayHandler::m_sArchwayPipelinesConfigurationFile = (OpenViBE::Directories::getUserDataDir() + "/studio-archway-pipeline-configuration.conf").toASCIIString();

std::string CArchwayHandler::getArchwayErrorString() const
{
	assert(m_pArchway);
	auto l_uiErrorCode = m_pArchway->getLastError();
	auto l_sErrorString = m_pArchway->getErrorString(l_uiErrorCode);

	std::stringstream l_sStream;
	l_sStream << std::hex << l_uiErrorCode;
	return "[0x" + l_sStream.str() + "] " + l_sErrorString;
}

CArchwayHandler::CArchwayHandler(const OpenViBE::Kernel::IKernelContext& rKernelContext)
    : m_sDeviceURL("simulator://"),
      m_pArchway(nullptr),
      m_rKernelContext(rKernelContext),
      m_uiRunningPipelineId(0)
{
}

EngineInitialisationStatus CArchwayHandler::initialize()
{
	m_pArchway = Engine::getMensiaEngineAPIByPointer();

	if (!m_pArchway)
	{
		return EngineInitialisationStatus::NotAvailable;
	}

	m_rKernelContext.getLogManager() << LogLevel_Trace << "Working with Archway version: " << m_pArchway->getVersionDescription() << "\n";

	// Now initialize the ArchwayBridge structure with closures that bridge to local Archway functions
	// this way we do not have to expose the Archway object or the C API

	m_oArchwayBridge.isStarted = [this](){
		return this->m_pArchway->isStarted();
	};
	m_oArchwayBridge.getAvailableValueMatrixCount = [this](unsigned int uiValueChannelId){
		return this->m_pArchway->getPendingValueCount(m_uiRunningPipelineId, uiValueChannelId);
	};

	// This function returns the last getPendingValue result as a vector
	// Such encapsulation enables us to avoid a call to getPendingValueDimension
	// from the client plugin.
	m_oArchwayBridge.popValueMatrix = [this](unsigned int uiValueChannelId){
		std::vector<float> m_vValueMatrix;
		auto l_uiValueChannelDimension = this->m_pArchway->getPendingValueDimension(m_uiRunningPipelineId, uiValueChannelId);
		m_vValueMatrix.resize(l_uiValueChannelDimension);
		if (l_uiValueChannelDimension > 0)
		{
			this->m_pArchway->getPendingValue(m_uiRunningPipelineId, uiValueChannelId, &m_vValueMatrix[0]);
		}
		return m_vValueMatrix;
	};

	// As archway buffers signals we have to expose this function to the client plugins
	m_oArchwayBridge.dropBuffers = [this](){
		m_pArchway->dropPendingEvents(0);
		m_pArchway->dropPendingValues(0);
		return true;
	};

	// We save the address of the structure in the heap memory as a string and save it in
	// the ConfigurationManager, this is because a box can not communicate directly with the Designer
	std::ostringstream l_ssArchwayBridgeAddress;
	l_ssArchwayBridgeAddress << static_cast<void const *>(&m_oArchwayBridge);
	m_rKernelContext.getConfigurationManager().createConfigurationToken("Designer_ArchwayBridgeAddress", l_ssArchwayBridgeAddress.str().c_str());

	this->loadPipelineConfigurations();

	// TODO_JL: Get the engine type from configuration
	if (!this->initializeArchway(EngineType::LAN))
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Failed to initialize the LAN engine " << this->getArchwayErrorString().c_str() << "\n";
		return EngineInitialisationStatus::Failure;
	}

	char l_sDeviceURL[2048];
	m_pArchway->getConfigurationParameterAsString("config.device[1]", l_sDeviceURL, sizeof(l_sDeviceURL));
	m_sDeviceURL = l_sDeviceURL;

	return EngineInitialisationStatus::Success;
}

bool CArchwayHandler::uninitialize()
{
	assert(m_pArchway);

	if (m_pArchway->isStarted())
	{
		m_pArchway->stop();
	}
	if (m_pArchway->isAcquiring())
	{
		m_pArchway->stopAllAcquisitionDevices();
	}
	if (m_pArchway->isInitialized())
	{
		m_pArchway->uninitialize();
	}

	this->savePipelineConfigurations();

	return true;
}

CArchwayHandler::~CArchwayHandler()
{
	this->uninitialize();
}


bool CArchwayHandler::initializeArchway(EngineType eEngineType)
{
	assert(m_pArchway);

	m_rKernelContext.getLogManager() << LogLevel_Trace << "Re-initializing engine in [" << (eEngineType == EngineType::Local ? "Local" : "LAN") << "]" << "\n";
	return m_pArchway->initialize("user", "pass", "neurort-studio", m_sArchwayConfigurationFile.c_str());
}

bool CArchwayHandler::uninitializeArchway()
{
	assert(m_pArchway);

	if (m_pArchway->isInitialized())
	{
		m_pArchway->uninitialize();
	}

	return true;
}

bool CArchwayHandler::reinitializeArchway(EngineType eEngineType)
{
	assert(m_pArchway);

	if (!this->uninitializeArchway())
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Failed to uninitialize Engine " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}
	if (!this->initializeArchway(eEngineType))
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Failed to initialize Engine " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}
	m_rKernelContext.getLogManager() << LogLevel_Info << "Archway re-initialized \n";
	return true;
}

bool CArchwayHandler::startEngineWithPipeline(unsigned int uiPipelineClassId)
{
	assert(m_pArchway);

	if (m_pArchway->isStarted())
	{
		m_rKernelContext.getLogManager() << LogLevel_Info << "Engine is already started\n";
		return true;
	}

	if (!m_pArchway->isAcquiring())
	{
		if (!m_pArchway->startAllAcquisitionDevices())
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to start the acquisition" << this->getArchwayErrorString().c_str() << "\n";
			return false;
		}
	}

	m_uiRunningPipelineId = m_pArchway->createPipeline(uiPipelineClassId, "");
	if (m_uiRunningPipelineId == 0)
	{
		m_pArchway->stopAllAcquisitionDevices();
		m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to create pipeline " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}

	if (m_vPipelineSettings.count(uiPipelineClassId) != 0)
	{
		for (auto& parameter: m_vPipelineSettings[uiPipelineClassId])
		{
			m_pArchway->setPipelineParameterAsString(m_uiRunningPipelineId, parameter.first.c_str(), parameter.second.c_str());
		}
	}

	if (!m_pArchway->start())
	{
		m_pArchway->stopAllAcquisitionDevices();
		m_rKernelContext.getLogManager() << LogLevel_Error << "Engine failed to start " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}

	m_rKernelContext.getLogManager() << LogLevel_Info << "Engine Started\n";

	return true;
}

bool CArchwayHandler::stopEngine()
{
	assert(m_pArchway);

	if (m_pArchway->isStarted())
	{
		m_pArchway->stop();
	}

	if (m_pArchway->isAcquiring())
	{
		m_pArchway->stopAllAcquisitionDevices();
	}

	if (m_uiRunningPipelineId != 0)
	{
		m_pArchway->releasePipeline(m_uiRunningPipelineId);
		m_uiRunningPipelineId = 0;
	}

	m_rKernelContext.getLogManager() << LogLevel_Info << "Engine Stopped\n";
	return true;
}

bool CArchwayHandler::loopEngine()
{
	assert(m_pArchway);

	return m_pArchway->mainloop();
}

bool CArchwayHandler::isEngineStarted()
{
	assert(m_pArchway);

	return m_pArchway->isStarted();
}

namespace
{
	void enumerateEnginePipelinesCallback(unsigned int uiPipelineId, const char* sPipelineDescription, void* pUserData)
	{
		auto l_vCallbackParameters = static_cast< pair< vector<SPipeline>*, map< unsigned int, map< string, string > >* >* >(pUserData);
		auto l_vEnginePipelines = l_vCallbackParameters->first;
		auto l_vPipelineSettings = l_vCallbackParameters->second;

		l_vEnginePipelines->push_back({
		                                uiPipelineId,
		                                sPipelineDescription,
		                                l_vPipelineSettings->count(uiPipelineId) != 0
		                            });
	}

	void enumeratePipelineParametersCallback(unsigned int uiPipelineId, const char* sParameterName, const char* sParameterValue, void* pUserData)
	{
		(void)uiPipelineId;

		// This callback will go through the pipeline parameters one by one and push them into the
		// vector of SPipelineParameters which is passed as the first element of the pUserData input pair
		// This callback receives the parameter's name and default value from Archway, which is why we pass
		// it the list of the _currently set_ parameters for the pipeline
		auto vCallbackParameters = static_cast< pair< vector<SPipelineParameter>*, map<string, string> const* >* >(pUserData);

		// Our output parameter is the list of pipeline parameters
		auto vPipelineParameters = vCallbackParameters->first;
		// Our additional input parameter is the map of currently set parameters for the pipeline
		// (note that if no parameters are set for the pipeline then this pointer will be null)
		auto vPipelineSettings = vCallbackParameters->second;

		// Search for the currently set value of the currently handled parameter
		std::string l_sCurrentParameterValue = "";
		if (vPipelineSettings && vPipelineSettings->count(sParameterName) != 0)
		{
			l_sCurrentParameterValue = vPipelineSettings->at(sParameterName);
		}

		vPipelineParameters->push_back({
		                                   sParameterName,
		                                   sParameterValue,
		                                   l_sCurrentParameterValue
		                               });
	}
}

std::vector<SPipeline> CArchwayHandler::getEnginePipelines() const
{
	assert(m_pArchway);

	std::vector<SPipeline> l_vEnginePipelines;
	auto l_oCallbackParameters = make_pair(&l_vEnginePipelines, &m_vPipelineSettings);

	m_pArchway->enumerateAvailablePipelines(enumerateEnginePipelinesCallback, &l_oCallbackParameters);

	return l_vEnginePipelines;
}

std::vector<SPipelineParameter> CArchwayHandler::getPipelineParameters(unsigned int uiPipelineClassId) const
{
	assert(m_pArchway);

	std::vector<SPipelineParameter> l_vPipelineParameters;
	map<string, string> const* l_pPipelineSettings = nullptr;

	if (m_vPipelineSettings.count(uiPipelineClassId) != 0)
	{
		l_pPipelineSettings = &m_vPipelineSettings.at(uiPipelineClassId);
	}

	auto l_oCallbackParameters = std::make_pair(&l_vPipelineParameters, l_pPipelineSettings);

	unsigned int uiPipelineId = m_pArchway->createPipeline(uiPipelineClassId, "");
	m_pArchway->enumeratePipelineParameters(uiPipelineId, enumeratePipelineParametersCallback, &l_oCallbackParameters);
	m_pArchway->releasePipeline(uiPipelineId);

	return l_vPipelineParameters;
}

bool CArchwayHandler::setPipelineParameterValue(unsigned int uiPipelineClassId, std::string const& sParameterName, std::string const& sParameterValue)
{
	// TODO_JL verify that the parameter name is legal
	if (sParameterValue != "")
	{
		m_vPipelineSettings[uiPipelineClassId][sParameterName] = sParameterValue;
	}
	else
	{
		if (m_vPipelineSettings.count(uiPipelineClassId) != 0)
		{
			if (m_vPipelineSettings[uiPipelineClassId].count(sParameterName) != 0)
			{
				m_vPipelineSettings[uiPipelineClassId].erase(m_vPipelineSettings[uiPipelineClassId].find(sParameterName));
				if (m_vPipelineSettings[uiPipelineClassId].size() == 0)
				{
					m_vPipelineSettings.erase(m_vPipelineSettings.find(uiPipelineClassId));
				}
			}
		}
	}

//	m_rKernelContext.getLogManager() << LogLevel_Info << "Set [" << uiPipelineClassId << "/" << sParameterName.c_str() << "] to [" << sParameterValue.c_str() << "]\n";
	return true;
}

bool CArchwayHandler::writeArchwayConfigurationFile()
{
	std::ofstream l_oFile;
	FS::Files::openOFStream(l_oFile, m_sArchwayConfigurationFile.c_str());
	if (!l_oFile.good())
	{
		l_oFile.close();
		return false;
	}
	l_oFile << "config.server = 'lan'\n";
	l_oFile << "config.device[1] = '" << m_sDeviceURL << "'\n";
	l_oFile.close();

	return true;
}

bool CArchwayHandler::savePipelineConfigurations()
{
	std::ofstream l_oFile;
	FS::Files::openOFStream(l_oFile, m_sArchwayPipelinesConfigurationFile.c_str());

	if (!l_oFile.good())
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Cannot open file for writing\n";
		return false;
	}

	for (auto& pipeline : m_vPipelineSettings)
	{
		for (auto& setting : pipeline.second)
		{
			l_oFile << pipeline.first << "\t" << setting.first << "\t" << setting.second << "\n";
		}
	}

	l_oFile.close();

	return true;
}

bool CArchwayHandler::loadPipelineConfigurations()
{
	std::ifstream l_oFile;
	FS::Files::openIFStream(l_oFile, m_sArchwayPipelinesConfigurationFile.c_str());

	if (!l_oFile.good())
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Cannot open Engine Pipeline Configuration file for reading\n";
		return false;
	}

	while (!l_oFile.eof())
	{
		unsigned int l_uiPipelineClassId;
		l_oFile >> l_uiPipelineClassId;

		std::string l_sParameterName;
		// The first one simply trashes the tab after the pipeline Id
		getline(l_oFile, l_sParameterName, '\t');
		getline(l_oFile, l_sParameterName, '\t');

		std::string l_sParameterValue;
		getline(l_oFile, l_sParameterValue, '\n');

		if (l_sParameterValue != "")
		{
			m_vPipelineSettings[l_uiPipelineClassId];
			m_vPipelineSettings[l_uiPipelineClassId][l_sParameterName] = l_sParameterValue;
		}
	}

	l_oFile.close();

	return true;
}

#endif
