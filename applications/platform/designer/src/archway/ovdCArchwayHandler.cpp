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

const std::string CArchwayHandler::ARCHWAY_CONFIG_FILE           = (OpenViBE::Directories::getUserDataDir() + "/studio-archway.conf").toASCIIString();
const std::string CArchwayHandler::ARCHWAY_PIPELINES_CONFIG_FILE = (OpenViBE::Directories::getUserDataDir() + "/studio-archway-pipeline-configuration.conf").
		toASCIIString();

std::string CArchwayHandler::getArchwayErrorString() const
{
	assert(m_Archway);
	const auto errorCode   = m_archway->getLastError();
	const auto errorString = m_archway->getErrorString(errorCode);

	std::stringstream stream;
	stream << std::hex << errorCode;
	return "[0x" + stream.str() + "] " + errorString;
}


EEngineInitialisationStatus CArchwayHandler::initialize()
{
#if defined TARGET_OS_Windows
	m_archwayModule.loadFromPath("mensia-engine.dll", "initialize");
#elif defined TARGET_OS_Linux
	m_archwayModule.loadFromPath("libmensia-engine.so", "initialize");
#elif defined TARGET_OS_MacOS
	m_archwayModule.loadFromPath("libmensia-engine.dylib", "initialize");
#endif

	if (!m_archwayModule.isLoaded()) { return EEngineInitialisationStatus::NotAvailable; }
	m_archway = new struct ArchwayAPI();

	bool didLoad = true;
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getLastError", &m_archway->getLastError);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getErrorString", &m_archway->getErrorString);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getVersionDescription", &m_archway->getVersionDescription);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getConfigurationParameterAsString", &m_archway->getConfigurationParameterAsString);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getPipelineScenarioPath", &m_archway->getPipelineScenarioPath);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "initialize", &m_archway->initialize);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "startAllAcquisitionDevices", &m_archway->startAllAcquisitionDevices);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "startImpedanceCheckOnAllAcquisitionDevices", &m_archway->startImpedanceCheckOnAllAcquisitionDevices);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "startEngine", &m_archway->startEngine);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "startEngineInFastForward", &m_archway->startEngineInFastForward);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "stopEngine", &m_archway->stopEngine);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "stopAllAcquisitionDevices", &m_archway->stopAllAcquisitionDevices);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "uninitialize", &m_archway->uninitialize);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "enumerateAvailablePipelines", &m_archway->enumerateAvailablePipelines);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "createPipeline", &m_archway->createPipeline);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "releasePipeline", &m_archway->releasePipeline);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "isPipelineRunning", &m_archway->isPipelineRunning);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "isPipelineInErrorState", &m_archway->isPipelineInErrorState);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "enumeratePipelineParameters", &m_archway->enumeratePipelineParameters);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "setPipelineParameterAsString", &m_archway->setPipelineParameterAsString);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "mainloop", &m_archway->mainloop);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getPendingValueCount", &m_archway->getPendingValueCount);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getPendingValueDimension", &m_archway->getPendingValueDimension);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getPendingValue", &m_archway->getPendingValue);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getPendingLogMessageCount", &m_archway->getPendingLogMessageCount);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "getPendingLogMessage", &m_archway->getPendingLogMessage);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "dropPendingValues", &m_archway->dropPendingValues);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "dropPendingEvents", &m_archway->dropPendingEvents);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "isInitialized", &m_archway->isInitialized);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "isAcquiring", &m_archway->isAcquiring);
	didLoad &= System::CDynamicModuleSymbolLoader::getSymbol<>(m_archwayModule, "isStarted", &m_archway->isStarted);


	if (!didLoad)
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to load symbols from Archway library [" << m_archwayModule.getErrorDetails() << "]\n";
		delete m_archway;
		m_archway = nullptr;
		return EEngineInitialisationStatus::NotAvailable;
	}

	m_kernelCtx.getLogManager() << LogLevel_Trace << "Working with Archway version: " << m_archway->getVersionDescription() << "\n";

	// Now initialize the ArchwayBridge structure with closures that bridge to local Archway functions
	// this way we do not have to expose the Archway object or the C API

	m_ArchwayBridge.isStarted                    = [this]() { return this->m_archway->isStarted(); };
	m_ArchwayBridge.getAvailableValueMatrixCount = [this](size_t valueChannelId)
	{
		return this->m_archway->getPendingValueCount(m_RunningPipelineId, valueChannelId);
	};

	// This function returns the last getPendingValue result as a vector
	// Such encapsulation enables us to avoid a call to getPendingValueDimension
	// from the client plugin.
	m_ArchwayBridge.popValueMatrix = [this](const size_t valueChannelId)
	{
		std::vector<float> valueMatrix;
		const auto valueChannelDim = this->m_archway->getPendingValueDimension(m_RunningPipelineId, valueChannelId);
		valueMatrix.resize(valueChannelDim);
		if (valueChannelDim > 0)
		{
			if (!this->m_archway->getPendingValue(m_RunningPipelineId, valueChannelId, &valueMatrix[0]))
			{
				m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to get pending value " << this->getArchwayErrorString().c_str() << "\n";
			}
		}
		return valueMatrix;
	};

	// As archway buffers signals we have to expose this function to the client plugins
	m_ArchwayBridge.dropBuffers = [this]()
	{
		m_archway->dropPendingEvents(0);
		m_archway->dropPendingValues(0);
		return true;
	};

	// We save the address of the structure in the heap memory as a string and save it in
	// the ConfigurationManager, this is because a box can not communicate directly with the Designer
	std::ostringstream archwayBridgeAddress;
	archwayBridgeAddress << static_cast<void const*>(&m_ArchwayBridge);
	m_kernelCtx.getConfigurationManager().createConfigurationToken("Designer_ArchwayBridgeAddress", archwayBridgeAddress.str().c_str());

	if (!this->loadPipelineConfigurations()) { return EEngineInitialisationStatus::Failure; }

	m_EngineType = EEngineType::Local;

	// Get the starting engine type from configuration
	std::ifstream archwayConfigFile;
	FS::Files::openIFStream(archwayConfigFile, ARCHWAY_CONFIG_FILE.c_str());
	if (archwayConfigFile.good())
	{
		try
		{
			std::string line;
			while (std::getline(archwayConfigFile, line))
			{
				std::cmatch cm;
				if (std::regex_match(line.c_str(), cm, std::regex("config.server\\s*=\\s*'(lan|local)'")))
				{
					if (cm[1] == "lan") { m_EngineType = EEngineType::LAN; }
				}
				else if (std::regex_match(line.c_str(), cm, std::regex("config.device[1]\\s*=\\s*'([^']*)'"))) { m_DeviceURL = cm[1]; }
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
		return EEngineInitialisationStatus::Failure;
	}

	char deviceURL[2048];
	m_archway->getConfigurationParameterAsString("config.device[1]", deviceURL, sizeof(deviceURL));
	m_DeviceURL = deviceURL;

	return EEngineInitialisationStatus::Success;
}

bool CArchwayHandler::uninitialize()
{
	assert(m_Archway);

	if (m_archway->isStarted())
	{
		if (!m_archway->stopEngine())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to stop Engine " << this->getArchwayErrorString().c_str() << "\n";
		}
	}

	if (m_archway->isAcquiring())
	{
		if (!m_archway->stopAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to stop all acquisition devices " << this->getArchwayErrorString().c_str() << "\n";
		}
	}

	if (m_archway->isInitialized())
	{
		if (!m_archway->uninitialize())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to uninitialize Archway " << this->getArchwayErrorString().c_str() << "\n";
		}
	}

	delete m_archway;
	m_archway = nullptr;

	if (!this->savePipelineConfigurations()) { return false; }

	return true;
}

CArchwayHandler::~CArchwayHandler() { if (m_archway) { this->uninitialize(); } }


bool CArchwayHandler::initializeArchway()
{
	assert(m_Archway);

	m_kernelCtx.getLogManager() << LogLevel_Trace << "Re-initializing engine in [" << (m_EngineType == EEngineType::Local ? "Local" : "LAN") << "]" << "\n";
	return m_archway->initialize("user", "pass", "neurort-studio", ARCHWAY_CONFIG_FILE.c_str());
}

bool CArchwayHandler::uninitializeArchway()
{
	assert(m_Archway);

	if (m_archway->isInitialized())
	{
		if (!m_archway->uninitialize())
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

bool CArchwayHandler::startEngineWithPipeline(const size_t pipelineClassID, const bool isFastForward, const bool shouldAcquireImpedance)
{
	assert(m_Archway);

	if (m_archway->isStarted())
	{
		m_kernelCtx.getLogManager() << LogLevel_Info << "Engine is already started\n";
		return true;
	}

	if (!m_archway->isAcquiring())
	{
		if (shouldAcquireImpedance)
		{
			if (!m_archway->startImpedanceCheckOnAllAcquisitionDevices())
			{
				m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to start the impedance acquisition" << this->getArchwayErrorString().c_str() << "\n";
				return false;
			}
		}
		else if (!m_archway->startAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to start the data acquisition" << this->getArchwayErrorString().c_str() << "\n";
			return false;
		}
	}

	m_RunningPipelineId = m_archway->createPipeline(pipelineClassID, "");

	if (m_RunningPipelineId == 0)
	{
		m_archway->stopAllAcquisitionDevices();
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to create pipeline " << this->getArchwayErrorString().c_str() << "\n";
		return false;
	}

	if (m_pipelineSettings.count(pipelineClassID) != 0)
	{
		for (auto& parameter : m_pipelineSettings[pipelineClassID])
		{
			m_archway->setPipelineParameterAsString(m_RunningPipelineId, parameter.first.c_str(), parameter.second.c_str());
		}
	}

	if (!(isFastForward ? m_archway->startEngineInFastForward() : m_archway->startEngine()))
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Engine failed to start " << this->getArchwayErrorString().c_str() << "\n";

		const size_t nPendingLogMessage = m_archway->getPendingLogMessageCount(m_RunningPipelineId);

		for (size_t i = 0; i < nPendingLogMessage; ++i)
		{
			size_t logLevel;
			char messageBuffer[2048];
			std::string logMessages;

			m_archway->getPendingLogMessage(m_RunningPipelineId, &logLevel, messageBuffer, sizeof(messageBuffer));
			m_kernelCtx.getLogManager() << ELogLevel(logLevel) << messageBuffer;
		}

		if (!m_archway->stopAllAcquisitionDevices())
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

	if (m_archway->isStarted())
	{
		if (!m_archway->stopEngine())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to stop Engine " << this->getArchwayErrorString().c_str() << "\n";
			hasStopSucceeded = false;
		}
	}

	if (m_archway->isAcquiring())
	{
		if (!m_archway->stopAllAcquisitionDevices())
		{
			m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to all acquisition devices " << this->getArchwayErrorString().c_str() << "\n";
			hasStopSucceeded = false;
		}
	}

	if (m_RunningPipelineId != 0)
	{
		if (!m_archway->releasePipeline(m_RunningPipelineId))
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

	const bool success = m_archway->mainloop();

	const bool isPipelineRunning      = m_archway->isPipelineRunning(m_RunningPipelineId);
	const bool isPipelineInErrorState = m_archway->isPipelineInErrorState(m_RunningPipelineId);

	if (!isPipelineRunning) { m_kernelCtx.getLogManager() << LogLevel_Info << "Pipeline [" << m_RunningPipelineId << "] is not running.\n"; }

	if (isPipelineInErrorState) { m_kernelCtx.getLogManager() << LogLevel_Error << "Pipeline [" << m_RunningPipelineId << "] is in error state.\n"; }

	const size_t nPendingLogMessage = m_archway->getPendingLogMessageCount(m_RunningPipelineId);

	for (size_t i = 0; i < nPendingLogMessage; ++i)
	{
		size_t logLevel;
		char messageBuffer[2048];
		std::string logMessages;

		m_archway->getPendingLogMessage(m_RunningPipelineId, &logLevel, messageBuffer, sizeof(messageBuffer));
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
	if (!m_archway) { return false; }
	return m_archway->isStarted();
}

namespace
{
	void enumerateEnginePipelinesCallback(const size_t pipelineID, const char* pipelineDescription, void* userData)
	{
		const auto callbackParameters = static_cast<pair<vector<SPipeline>*, map<size_t, map<string, string>>*>*>(userData);
		auto enginePipelines          = callbackParameters->first;
		const auto pipelineSettings   = callbackParameters->second;

		enginePipelines->push_back({ pipelineID, pipelineDescription, pipelineSettings->count(pipelineID) != 0 });
	}

	void enumeratePipelineParametersCallback(size_t/*pipelineID*/, const char* parameterName, const char* parameterValue, void* userData)
	{
		// This callback will go through the pipeline parameters one by one and push them into the
		// vector of SPipelineParameters which is passed as the first element of the data input pair
		// This callback receives the parameter's name and default value from Archway, which is why we pass
		// it the list of the _currently set_ parameters for the pipeline
		const auto parameters = static_cast<pair<vector<SPipelineParameter>*, map<string, string> const*>*>(userData);

		// Our output parameter is the list of pipeline parameters
		auto pipelineParameters = parameters->first;
		// Our additional input parameter is the map of currently set parameters for the pipeline
		// (note that if no parameters are set for the pipeline then this pointer will be null)
		const auto pipelineSettings = parameters->second;

		// Search for the currently set value of the currently handled parameter
		std::string str;

		if (pipelineSettings && pipelineSettings->count(parameterName) != 0) { str = pipelineSettings->at(parameterName); }

		pipelineParameters->push_back({ parameterName, parameterValue, str });
	}
}  // namespace

std::vector<SPipeline> CArchwayHandler::getEnginePipelines() const
{
	assert(m_Archway);

	std::vector<SPipeline> enginePipelines;
	auto callbackParameters = make_pair(&enginePipelines, &m_pipelineSettings);

	if (!m_archway->enumerateAvailablePipelines(enumerateEnginePipelinesCallback, &callbackParameters))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed enumerate the available pipelines " << this->getArchwayErrorString().c_str() << "\n";
	}

	return enginePipelines;
}

std::vector<SPipelineParameter> CArchwayHandler::getPipelineParameters(const size_t pipelineClassID) const
{
	assert(m_Archway);

	std::vector<SPipelineParameter> pipelineParameters;
	map<string, string> const* pipelineSettings = nullptr;

	if (m_pipelineSettings.count(pipelineClassID) != 0) { pipelineSettings = &m_pipelineSettings.at(pipelineClassID); }

	auto callbackParameters = std::make_pair(&pipelineParameters, pipelineSettings);

	const size_t pipelineID = m_archway->createPipeline(pipelineClassID, "");

	if (!m_archway->enumeratePipelineParameters(pipelineID, enumeratePipelineParametersCallback, &callbackParameters))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed enumerate the pipeline's parameters " << this->getArchwayErrorString().c_str() << "\n";
	}

	if (!m_archway->releasePipeline(pipelineID))
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Failed to release pipeline " << this->getArchwayErrorString().c_str() << "\n";
	}

	return pipelineParameters;
}

std::string CArchwayHandler::getPipelineScenarioPath(size_t pipelineClassID) const
{
	assert(m_Archway);

	char messageBuffer[2048];
	std::string logMessages;

	if (!m_archway->getPipelineScenarioPath(pipelineClassID, messageBuffer, sizeof(messageBuffer))) { return ""; }
	return std::string(messageBuffer);
}


bool CArchwayHandler::setPipelineParameterValue(const size_t pipelineClassID, std::string const& parameterName, std::string const& parameterValue)
{
	if (m_pipelineSettings.find(pipelineClassID) == m_pipelineSettings.end())
	{
		m_kernelCtx.getLogManager() << LogLevel_Error << "Failed to set the value [" << parameterValue << "] to the parameter [" << parameterName << "]. The pipeline class id is invalid.\n";
		return false;
	}

	if (parameterValue != "") { m_pipelineSettings[pipelineClassID][parameterName] = parameterValue; }
	else
	{
		if (m_pipelineSettings.count(pipelineClassID) != 0)
		{
			if (m_pipelineSettings[pipelineClassID].count(parameterName) != 0)
			{
				m_pipelineSettings[pipelineClassID].erase(m_pipelineSettings[pipelineClassID].find(parameterName));

				if (m_pipelineSettings[pipelineClassID].size() == 0) { m_pipelineSettings.erase(m_pipelineSettings.find(pipelineClassID)); }
			}
		}
	}

	// m_kernelCtx.getLogManager() << LogLevel_Info << "Set [" << uiPipelineClassId << "/" << sParameterName << "] to [" << sParameterValue << "]\n";
	return true;
}

bool CArchwayHandler::writeArchwayConfigurationFile()
{
	std::ofstream file;
	FS::Files::openOFStream(file, ARCHWAY_CONFIG_FILE.c_str());

	if (!file.good())
	{
		file.close();
		return false;
	}

	file << "config.server = '" << (m_EngineType == EEngineType::LAN ? "lan" : "local") << "'\n";
	file << "config.device[1] = '" << m_DeviceURL << "'\n";
	file.close();

	return true;
}

bool CArchwayHandler::savePipelineConfigurations()
{
	std::ofstream file;
	FS::Files::openOFStream(file, ARCHWAY_PIPELINES_CONFIG_FILE.c_str());

	if (!file.good())
	{
		m_kernelCtx.getLogManager() << LogLevel_Warning << "Cannot open file for writing\n";
		return false;
	}

	for (auto& pipeline : m_pipelineSettings)
	{
		for (auto& setting : pipeline.second) { file << pipeline.first << "\t" << setting.first << "\t" << setting.second << "\n"; }
	}

	file.close();

	return true;
}

bool CArchwayHandler::loadPipelineConfigurations()
{
	std::ifstream file;
	FS::Files::openIFStream(file, ARCHWAY_PIPELINES_CONFIG_FILE.c_str());

	if (!file.good())
	{
		m_kernelCtx.getLogManager() << LogLevel_Trace << "Cannot open Engine Pipeline Configuration file for reading\n";
		return false;
	}

	while (!file.eof())
	{
		size_t pipelineClassID;
		file >> pipelineClassID;

		std::string parameterName;
		// The first one simply trashes the tab after the pipeline Id
		getline(file, parameterName, '\t');
		getline(file, parameterName, '\t');

		std::string parameterValue;
		getline(file, parameterValue, '\n');

		if (parameterValue != "")
		{
			m_pipelineSettings[pipelineClassID];
			m_pipelineSettings[pipelineClassID][parameterName] = parameterValue;
		}
	}

	file.close();

	return true;
}

#endif
