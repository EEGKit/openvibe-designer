#ifdef MENSIA_DISTRIBUTION

#include "ovdCArchwayHandler.h"

#include <openvibe/ov_directories.h>
#include <openvibe/kernel/log/ovILogManager.h>
#include <openvibe/kernel/configuration/ovIConfigurationManager.h>
#include <fs/Files.h>

#include <ios>
#include <cassert>
#include <fstream>
#include <sstream>
#include <regex>

using namespace Mensia;
using namespace OpenViBE::Kernel;
using namespace std;

const std::string CArchwayHandler::s_ArchwayConfigurationFile = (OpenViBE::Directories::getUserDataDir() + "/studio-archway.conf").toASCIIString();
const std::string CArchwayHandler::s_ArchwayPipelinesConfigurationFile = (OpenViBE::Directories::getUserDataDir() + "/studio-archway-pipeline-configuration.conf").toASCIIString();

std::string CArchwayHandler::getArchwayErrorString() const
{
	assert(m_Archway);
	auto errorCode = m_Archway->getLastError();
	auto errorString = m_Archway->getErrorString(errorCode);

	std::stringstream stream;
	stream << std::hex << errorCode;
	return "[0x" + stream.str() + "] " + errorString;
}

CArchwayHandler::CArchwayHandler(const OpenViBE::Kernel::IKernelContext& ctx)
	: m_DeviceURL("simulator://"), m_Archway(nullptr), m_kernelCtx(ctx), m_RunningPipelineId(0) { }

EngineInitialisationStatus CArchwayHandler::initialize()
{
#if defined TARGET_OS_Windows
	m_ArchwayModule.loadFromPath("mensia-engine.dll", "initialize");
#elif defined TARGET_OS_Linux
	m_ArchwayModule.loadFromPath("libmensia-engine.so", "initialize");
#elif defined TARGET_OS_MacOS
	m_ArchwayModule.loadFromPath("libmensia-engine.dylib", "initialize");
#endif

	if (!m_ArchwayModule.isLoaded()) { return EngineInitialisationStatus::NotAvailable; }
	m_Archway = new struct ArchwayAPI();

	bool didLoad = true;
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getLastError", &m_Archway->getLastError);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getErrorString", &m_Archway->getErrorString);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getVersionDescription", &m_Archway->getVersionDescription);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getConfigurationParameterAsString", &m_Archway->getConfigurationParameterAsString);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getPipelineScenarioPath", &m_Archway->getPipelineScenarioPath);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "initialize", &m_Archway->initialize);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "startAllAcquisitionDevices", &m_Archway->startAllAcquisitionDevices);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "startImpedanceCheckOnAllAcquisitionDevices", &m_Archway->startImpedanceCheckOnAllAcquisitionDevices);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "startEngine", &m_Archway->startEngine);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "startEngineInFastForward", &m_Archway->startEngineInFastForward);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "stopEngine", &m_Archway->stopEngine);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "stopAllAcquisitionDevices", &m_Archway->stopAllAcquisitionDevices);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "uninitialize", &m_Archway->uninitialize);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "enumerateAvailablePipelines", &m_Archway->enumerateAvailablePipelines);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "createPipeline", &m_Archway->createPipeline);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "releasePipeline", &m_Archway->releasePipeline);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "isPipelineRunning", &m_Archway->isPipelineRunning);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "isPipelineInErrorState", &m_Archway->isPipelineInErrorState);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "enumeratePipelineParameters", &m_Archway->enumeratePipelineParameters);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "setPipelineParameterAsString", &m_Archway->setPipelineParameterAsString);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "mainloop", &m_Archway->mainloop);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getPendingValueCount", &m_Archway->getPendingValueCount);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getPendingValueDimension", &m_Archway->getPendingValueDimension);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getPendingValue", &m_Archway->getPendingValue);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getPendingLogMessageCount", &m_Archway->getPendingLogMessageCount);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "getPendingLogMessage", &m_Archway->getPendingLogMessage);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "dropPendingValues", &m_Archway->dropPendingValues);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "dropPendingEvents", &m_Archway->dropPendingEvents);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "isInitialized", &m_Archway->isInitialized);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "isAcquiring", &m_Archway->isAcquiring);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_ArchwayModule, "isStarted", &m_Archway->isStarted);


	if (!didLoad)
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to load symbols from Archway library [" << m_ArchwayModule.getErrorDetails() << "]\n";
		delete m_Archway;
		m_Archway = nullptr;
		return EngineInitialisationStatus::NotAvailable;
	}

	m_kernelCtx.getLogManager() << LogLevel_Trace << "Working with Archway version: " << m_Archway->getVersionDescription() << "\n";

	// Now initialize the ArchwayBridge structure with closures that bridge to local Archway functions
	// this way we do not have to expose the Archway object or the C API

	m_ArchwayBridge.isStarted = [this]() { return this->m_Archway->isStarted(); };
	m_ArchwayBridge.getAvailableValueMatrixCount = [this](uint32_t uiValueChannelId) {
		return this->m_Archway->getPendingValueCount(m_RunningPipelineId, uiValueChannelId);
	};

	// This function returns the last getPendingValue result as a vector
// Such encapsulation enables us to avoid a call to getPendingValueDimension
	// from the client plugin.
	m_ArchwayBridge.popValueMatrix = [this](uint32_t uiValueChannelId) {
		std::vector<float> m_vValueMatrix;
		auto l_uiValueChannelDimension = this->m_Archway->getPendingValueDimension(m_RunningPipelineId, uiValueChannelId);
		m_vValueMatrix.resize(l_uiValueChannelDimension);
		if (l_uiValueChannelDimension > 0)
		{
			if (!this->m_Archway->getPendingValue(m_RunningPipelineId, uiValueChannelId, &m_vValueMatrix[0]))
			{
				m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to get pending value " << this->getArchwayErrorString().c_str() << "\n";
			}
		}
		return m_vValueMatrix;
	};

	// As archway buffers signals we have to expose this function to the client plugins
	m_ArchwayBridge.dropBuffers = [this]() {
		m_Archway->dropPendingEvents(0);
		m_Archway->dropPendingValues(0);
		return true;
	};

	// We save the address of the structure in the heap memory as a string and save it in
	// the ConfigurationManager, this is because a box can not communicate directly with the Designer
	std::ostringstream l_ssArchwayBridgeAddress;
	l_ssArchwayBridgeAddress << static_cast<void const*>(&m_ArchwayBridge);
	m_kernelCtx.getConfigurationManager().createConfigurationToken("Designer_ArchwayBridgeAddress", l_ssArchwayBridgeAddress.str().c_str());

	if (!this->loadPipelineConfigurations()) { return EngineInitialisationStatus::Failure; }

	m_EngineType = EngineType::Local;

	// Get the starting engine type from configuration
	std::ifstream archwayConfigurationFile;
	FS::Files::openIFStream(archwayConfigurationFile, s_ArchwayConfigurationFile.c_str());
	if (archwayConfigurationFile.good())
	{
		try
		{
			std::string line;
			while (std::getline(archwayConfigurationFile, line))
			{
				std::cmatch cm;
				if (std::regex_match(line.c_str(), cm, std::regex("config.server\\s*=\\s*'(lan|local)'")))
				{
					if (cm[1] == "lan")
					{
						m_EngineType = EngineType::LAN;
					}
				}
				else if (std::regex_match(line.c_str(), cm, std::regex("config.device[1]\\s*=\\s*'([^']*)'")))
				{
					m_DeviceURL = cm[1];
				}
			}
		}
		catch (...)
		{
			// An exception will be thrown when using libc++ from ubuntu 14.04
			// in that case we do nothing and keep the defaults
		}
	}


	if (!this->initializeArchway())
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to initialize the engine " << this->getArchwayErrorString().c_str() << "\n";
		return EngineInitialisationStatus::Failure;
	}

	char deviceURL[2048];
	m_Archway->getConfigurationParameterAsString("config.device[1]", deviceURL, sizeof(deviceURL));
	m_DeviceURL = deviceURL;

	return EngineInitialisationStatus::Success;
}

bool CArchwayHandler::uninitialize()
{
	assert(m_Archway);

	if (m_Archway->isStarted())
	{
		if (!m_Archway->stopEngine())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to stop Engine " << this->getArchwayErrorString().c_str() << "\n";
		}
	}

	if (m_Archway->isAcquiring())
	{
		if (!m_Archway->stopAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to stop all acquisition devices " << this->getArchwayErrorString().c_str() << "\n";
		}
	}

	if (m_Archway->isInitialized())
	{
		if (!m_Archway->uninitialize())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to uninitialize Archway " << this->getArchwayErrorString().c_str() << "\n";
		}
	}

	delete m_Archway;
	m_Archway = nullptr;

	if (!this->savePipelineConfigurations()) { return false; }

	return true;
}

CArchwayHandler::~CArchwayHandler()
{
	if (m_Archway) { this->uninitialize(); }
}


bool CArchwayHandler::initializeArchway()
{
	assert(m_Archway);

	m_kernelCtx.getLogManager() << LogLevel_Trace << "Re-initializing engine in [" << (m_EngineType == EngineType::Local ? "Local" : "LAN") << "]" << "\n";
	return m_Archway->initialize("user", "pass", "neurort-studio", s_ArchwayConfigurationFile.c_str());
}

bool CArchwayHandler::uninitializeArchway()
{
	assert(m_Archway);

	if (m_Archway->isInitialized())
	{
		if (!m_Archway->uninitialize())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to uninitialize Archway " << this->getArchwayErrorString().c_str() << "\n";
			return false;
		}
	}

	return true;
}

bool CArchwayHandler::reinitializeArchway()
{
	assert(m_Archway);

	if (!this->uninitializeArchway())
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to uninitialize Engine " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}

	this->writeArchwayConfigurationFile();

	if (!this->initializeArchway())
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to initialize Engine " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}

	m_kernelCtx.getLogManager() << LogLevel_Info << "Archway re-initialized \n";
	return true;
}

bool CArchwayHandler::startEngineWithPipeline(uint32_t uiPipelineClassId, bool isFastForward, bool shouldAcquireImpedance)
{
	assert(m_Archway);

	if (m_Archway->isStarted())
	{
		m_kernelCtx.getLogManager() << LogLevel_Info << "Engine is already started\n";
		return true;
	}

	if (!m_Archway->isAcquiring())
	{
		if (shouldAcquireImpedance)
		{
			if (!m_Archway->startImpedanceCheckOnAllAcquisitionDevices())
			{
				m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to start the impedance acquisition" << this->getArchwayErrorString().c_str() << "\n";
				return false;
			}
		}
		else if (!m_Archway->startAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to start the data acquisition" << this->getArchwayErrorString().c_str() << "\n";
			return false;
		}
	}

	m_RunningPipelineId = m_Archway->createPipeline(uiPipelineClassId, "");

	if (m_RunningPipelineId == 0)
	{
		m_Archway->stopAllAcquisitionDevices();
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to create pipeline " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}

	if (m_PipelineSettings.count(uiPipelineClassId) != 0)
	{
		for (auto& parameter : m_PipelineSettings[uiPipelineClassId])
		{
			m_Archway->setPipelineParameterAsString(m_RunningPipelineId, parameter.first.c_str(), parameter.second.c_str());
		}
	}

	if (!(isFastForward ? m_Archway->startEngineInFastForward() : m_Archway->startEngine()))
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Engine failed to start " << this->getArchwayErrorString().c_str() << "\n";

		uint32_t pendingLogMessageCount = m_Archway->getPendingLogMessageCount(m_RunningPipelineId);

		for (uint32_t i = 0; i < pendingLogMessageCount; ++i)
		{
			uint32_t logLevel;
			char messageBuffer[2048];
			std::string logMessages;

			m_Archway->getPendingLogMessage(m_RunningPipelineId, &logLevel, messageBuffer, sizeof(messageBuffer));
			m_kernelCtx.getLogManager() << ELogLevel(logLevel) << messageBuffer;
		}

		if (!m_Archway->stopAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to stop all acquisition devices " << this->getArchwayErrorString().c_str() << "\n";
		}

		return false;
	}

	m_kernelCtx.getLogManager() << LogLevel_Info << "Engine Started\n";

	return true;
}

bool CArchwayHandler::stopEngine()
{
	assert(m_Archway);

	bool hasStopSucceeded = true;

	if (m_Archway->isStarted())
	{
		if (!m_Archway->stopEngine())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to stop Engine " << this->getArchwayErrorString().c_str() << "\n";
			hasStopSucceeded = false;
		}
	}

	if (m_Archway->isAcquiring())
	{
		if (!m_Archway->stopAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to all acquisition devices " << this->getArchwayErrorString().c_str() << "\n";
			hasStopSucceeded = false;
		}
	}

	if (m_RunningPipelineId != 0)
	{
		if (!m_Archway->releasePipeline(m_RunningPipelineId))
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to release pipeline [" << m_RunningPipelineId << "] " << this->getArchwayErrorString().c_str() << "\n";
			hasStopSucceeded = false;
		}

		m_RunningPipelineId = 0;
	}

	m_kernelCtx.getLogManager() << LogLevel_Info << "Engine Stopped\n";
	return hasStopSucceeded;
}

bool CArchwayHandler::loopEngine()
{
	assert(m_Archway);

	bool success = m_Archway->mainloop();

	bool isPipelineRunning = m_Archway->isPipelineRunning(m_RunningPipelineId);
	bool isPipelineInErrorState = m_Archway->isPipelineInErrorState(m_RunningPipelineId);

	if (!isPipelineRunning)
	{
		m_kernelCtx.getLogManager() << LogLevel_Info << "Pipeline [" << m_RunningPipelineId << "] is not running.\n";
	}

	if (isPipelineInErrorState)
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Pipeline [" << m_RunningPipelineId << "] is in error state.\n";
	}

	uint32_t pendingLogMessageCount = m_Archway->getPendingLogMessageCount(m_RunningPipelineId);

	for (uint32_t i = 0; i < pendingLogMessageCount; ++i)
	{
		uint32_t logLevel;
		char messageBuffer[2048];
		std::string logMessages;

		m_Archway->getPendingLogMessage(m_RunningPipelineId, &logLevel, messageBuffer, sizeof(messageBuffer));
		m_kernelCtx.getLogManager() << ELogLevel(logLevel) << messageBuffer;
	}

	if (!isPipelineRunning || isPipelineInErrorState)
	{
		if (!this->stopEngine()) { return false; }

		m_GUIBridge.refreshStoppedEngine();

		return success && !isPipelineInErrorState;
	}

	return success;
}

bool CArchwayHandler::isEngineStarted()
{
	if (!m_Archway) { return false; }

	return m_Archway->isStarted();
}

namespace
{
	void enumerateEnginePipelinesCallback(uint32_t pipelineID, const char* pipelineDescription, void* userData)
	{
		auto callbackParameters = static_cast<pair< vector<SPipeline>*, map< uint32_t, map< string, string > >* >*>(userData);
		auto enginePipelines = callbackParameters->first;
		auto pipelineSettings = callbackParameters->second;

		enginePipelines->push_back({ pipelineID, pipelineDescription, pipelineSettings->count(pipelineID) != 0 });
	}

	void enumeratePipelineParametersCallback(uint32_t/*pipelineID*/, const char* parameterName, const char* parameterValue, void* userData)
	{
		// This callback will go through the pipeline parameters one by one and push them into the
// vector of SPipelineParameters which is passed as the first element of the data input pair
// This callback receives the parameter's name and default value from Archway, which is why we pass
		// it the list of the _currently set_ parameters for the pipeline
		auto vCallbackParameters = static_cast<pair< vector<SPipelineParameter>*, map<string, string> const* >*>(userData);

		// Our output parameter is the list of pipeline parameters
		auto vPipelineParameters = vCallbackParameters->first;
		// Our additional input parameter is the map of currently set parameters for the pipeline
		// (note that if no parameters are set for the pipeline then this pointer will be null)
		auto vPipelineSettings = vCallbackParameters->second;

		// Search for the currently set value of the currently handled parameter
		std::string l_sCurrentParameterValue = "";

		if (vPipelineSettings && vPipelineSettings->count(parameterName) != 0)
		{
			l_sCurrentParameterValue = vPipelineSettings->at(parameterName);
		}

		vPipelineParameters->push_back({
			parameterName,
			parameterValue,
			l_sCurrentParameterValue
			});
	}
}

std::vector<SPipeline> CArchwayHandler::getEnginePipelines() const
{
	assert(m_Archway);

	std::vector<SPipeline> enginePipelines;
	auto callbackParameters = make_pair(&enginePipelines, &m_PipelineSettings);

	if (!m_Archway->enumerateAvailablePipelines(enumerateEnginePipelinesCallback, &callbackParameters))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed enumerate the available pipelines " << this->getArchwayErrorString().c_str() << "\n";
	}

	return enginePipelines;
}

std::vector<SPipelineParameter> CArchwayHandler::getPipelineParameters(uint32_t pipelineClassID) const
{
	assert(m_Archway);

	std::vector<SPipelineParameter> pipelineParameters;
	map<string, string> const* pipelineSettings = nullptr;

	if (m_PipelineSettings.count(pipelineClassID) != 0)
	{
		pipelineSettings = &m_PipelineSettings.at(pipelineClassID);
	}

	auto callbackParameters = std::make_pair(&pipelineParameters, pipelineSettings);

	uint32_t pipelineID = m_Archway->createPipeline(pipelineClassID, "");

	if (!m_Archway->enumeratePipelineParameters(pipelineID, enumeratePipelineParametersCallback, &callbackParameters))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed enumerate the pipeline's parameters " << this->getArchwayErrorString().c_str() << "\n";
	}

	if (!m_Archway->releasePipeline(pipelineID))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to release pipeline " << this->getArchwayErrorString().c_str() << "\n";
	}

	return pipelineParameters;
}

std::string CArchwayHandler::getPipelineScenarioPath(uint64_t pipelineClassID) const
{
	assert(m_Archway);

	char messageBuffer[2048];
	std::string logMessages;

	if (!m_Archway->getPipelineScenarioPath(pipelineClassID, messageBuffer, sizeof(messageBuffer))) { return ""; }
	return std::string(messageBuffer);
}


bool CArchwayHandler::setPipelineParameterValue(uint32_t pipelineClassID, std::string const& parameterName, std::string const& parameterValue)
{
	if (m_PipelineSettings.find(pipelineClassID) == m_PipelineSettings.end())
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to set the value [" << parameterValue.c_str() << "] to the parameter [" << parameterName.c_str() << "]. The pipeline class id is invalid.\n";
		return false;
	}

	if (parameterValue != "")
	{
		m_PipelineSettings[pipelineClassID][parameterName] = parameterValue;
	}
	else
	{
		if (m_PipelineSettings.count(pipelineClassID) != 0)
		{
			if (m_PipelineSettings[pipelineClassID].count(parameterName) != 0)
			{
				m_PipelineSettings[pipelineClassID].erase(m_PipelineSettings[pipelineClassID].find(parameterName));

				if (m_PipelineSettings[pipelineClassID].size() == 0) { m_PipelineSettings.erase(m_PipelineSettings.find(pipelineClassID)); }
			}
		}
	}

	//	m_kernelContext.getLogManager() << LogLevel_Info << "Set [" << uiPipelineClassId << "/" << sParameterName.c_str() << "] to [" << sParameterValue.c_str() << "]\n";
	return true;
}

bool CArchwayHandler::writeArchwayConfigurationFile()
{
	std::ofstream file;
	FS::Files::openOFStream(file, s_ArchwayConfigurationFile.c_str());

	if (!file.good())
	{
		file.close();
		return false;
	}

	file << "config.server = '" << (m_EngineType == EngineType::LAN ? "lan" : "local") << "'\n";
	file << "config.device[1] = '" << m_DeviceURL << "'\n";
	file.close();

	return true;
}

bool CArchwayHandler::savePipelineConfigurations()
{
	std::ofstream file;
	FS::Files::openOFStream(file, s_ArchwayPipelinesConfigurationFile.c_str());

	if (!file.good())
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Cannot open file for writing\n";
		return false;
	}

	for (auto& pipeline : m_PipelineSettings)
	{
		for (auto& setting : pipeline.second)
		{
			file << pipeline.first << "\t" << setting.first << "\t" << setting.second << "\n";
		}
	}

	file.close();

	return true;
}

bool CArchwayHandler::loadPipelineConfigurations()
{
	std::ifstream file;
	FS::Files::openIFStream(file, s_ArchwayPipelinesConfigurationFile.c_str());

	if (!file.good())
	{
		m_kernelCtx.getLogManager() << LogLevel_Trace << "Cannot open Engine Pipeline Configuration file for reading\n";
		return false;
	}

	while (!file.eof())
	{
		uint32_t pipelineClassID;
		file >> pipelineClassID;

		std::string parameterName;
		// The first one simply trashes the tab after the pipeline Id
		getline(file, parameterName, '\t');
		getline(file, parameterName, '\t');

		std::string parameterValue;
		getline(file, parameterValue, '\n');

		if (parameterValue != "")
		{
			m_PipelineSettings[pipelineClassID];
			m_PipelineSettings[pipelineClassID][parameterName] = parameterValue;
		}
	}

	file.close();

	return true;
}

#endif
