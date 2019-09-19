#pragma once

#ifdef MENSIA_DISTRIBUTION

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>

#include <openvibe/kernel/ovIKernelContext.h>
#include <gtk/gtk.h>
#include <system/ovCDynamicModule.h>

namespace Mensia {

	enum class EngineType
	{
		Local,
		LAN
	};

	enum class EngineInitialisationStatus
	{
		Success,
		Failure,
		NotAvailable
	};

	struct SPipelineParameter
	{
		std::string name;
		std::string defaultValue;
		std::string value;
	};

	struct SPipeline
	{
		unsigned long long id;
		std::string description;
		bool isConfigured;
	};

	struct SArchwayBridge
	{
		std::function<bool()> isStarted;
		std::function<uint32_t(uint32_t)> getAvailableValueMatrixCount;
		std::function<std::vector<float>(uint32_t)> popValueMatrix;
		std::function<bool()> dropBuffers;
	};

	struct SGUIBridge
	{
		std::function<void()> refreshStoppedEngine;
	};

	class CArchwayHandler final {
	public:
		CArchwayHandler(const OpenViBE::Kernel::IKernelContext& ctx);
		~CArchwayHandler();

		EngineInitialisationStatus initialize();
		bool uninitialize();

		bool reinitializeArchway();
		std::vector<SPipeline> getEnginePipelines() const;
		std::vector<SPipelineParameter> getPipelineParameters(uint32_t pipelineClassID) const;
		bool setPipelineParameterValue(uint32_t pipelineClassID, std::string const& parameterName, std::string const& parameterValue);
		std::string getPipelineScenarioPath(uint64_t pipelineID) const;

		bool startEngineWithPipeline(uint32_t pipelineClassID, bool isFastForward, bool shouldAcquireImpedance);
		bool loopEngine();
		bool stopEngine();

		bool isEngineStarted();
		bool writeArchwayConfigurationFile();

		std::map< std::string, std::string >& getPipelineSettings(uint32_t pipelineClassID) { return m_PipelineSettings[pipelineClassID]; }

	public:
		SArchwayBridge m_ArchwayBridge;
		SGUIBridge m_GUIBridge;
		std::string m_DeviceURL;
		EngineType m_EngineType;

	private:
		bool initializeArchway();
		bool uninitializeArchway();
		bool savePipelineConfigurations();
		bool loadPipelineConfigurations();
		std::string getArchwayErrorString() const;

	private:
		typedef void (*FPEnumerateAvailablePipelinesCallback)(uint32_t pipelineClassID, const char* pipelineDescription, void* userData);
		typedef void (*FPEnumeratePipelineParametersCallback)(uint32_t pipelineID, const char* parameterName, const char* parameterValue, void* userData) ;
		struct ArchwayAPI {
			uint32_t (*getLastError)();
			const char* (*getErrorString)(uint32_t errorCode);

			const char* (*getVersionDescription)();

			void (*getConfigurationParameterAsString)(const char* configurationParameter, char* outputBuffer, uint32_t bufferLength);
			bool (*getPipelineScenarioPath)(uint64_t pipelineID, char* messageBuffer, uint32_t bufferLength);

			bool (*initialize)(const char* login, const char* password, const char* applicationName, const char* configurationFilename);
			bool (*startAllAcquisitionDevices)();
			bool (*startImpedanceCheckOnAllAcquisitionDevices)();
			bool (*startEngine)();
			bool (*startEngineInFastForward)();
			bool (*stopEngine)();
			bool (*stopAllAcquisitionDevices)();
			bool (*uninitialize)();

			bool (*enumerateAvailablePipelines)(FPEnumerateAvailablePipelinesCallback callback, void* userData);
			uint32_t (*createPipeline)(uint32_t pipelineClassID, const char* profileName);
			bool (*releasePipeline)(uint32_t pipelineID);
			bool (*isPipelineRunning)(uint32_t pipelineID);
			bool (*isPipelineInErrorState)(uint32_t pipelineID);

			bool (*enumeratePipelineParameters)(uint32_t pipelineID, FPEnumeratePipelineParametersCallback callback, void* userData);
			bool (*setPipelineParameterAsString)(uint32_t pipelineID, const char* parameterName, const char* value);

			bool (*mainloop)();
			uint32_t (*getPendingValueCount)(uint32_t pipelineID, uint32_t matrixOutputIdx);
			uint32_t (*getPendingValueDimension)(uint32_t pipelineID, uint32_t matrixOutputIdx);
			bool (*getPendingValue)(uint32_t pipelineID, uint32_t matrixOutputIdx, float* value);
			uint32_t (*getPendingLogMessageCount)(uint32_t pipelineID);
			bool (*getPendingLogMessage)(uint32_t pipelineID, uint32_t* logLevel, char* messageBuffer, uint32_t bufferSize);

			bool (*dropPendingValues)(uint32_t pipelineID);
			bool (*dropPendingEvents)(uint32_t pipelineID);

			bool (*isInitialized)();
			bool (*isAcquiring)();
			bool (*isStarted)();
		};

		struct ArchwayAPI* m_Archway = nullptr;
		System::CDynamicModule m_ArchwayModule;
		const OpenViBE::Kernel::IKernelContext& m_KernelContext;

		// Current Configuration
		std::map< uint32_t, std::map< std::string, std::string > > m_PipelineSettings;

		static const std::string s_ArchwayConfigurationFile;
		static const std::string s_ArchwayPipelinesConfigurationFile;

		uint32_t m_RunningPipelineId = 0;
	};
}

#endif
