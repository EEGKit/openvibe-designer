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
		std::function<size_t(size_t)> getAvailableValueMatrixCount;
		std::function<std::vector<float>(size_t)> popValueMatrix;
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
		std::vector<SPipelineParameter> getPipelineParameters(size_t pipelineClassID) const;
		bool setPipelineParameterValue(size_t pipelineClassID, std::string const& parameterName, std::string const& parameterValue);
		std::string getPipelineScenarioPath(uint64_t pipelineID) const;

		bool startEngineWithPipeline(size_t pipelineClassID, bool isFastForward, bool shouldAcquireImpedance);
		bool loopEngine();
		bool stopEngine();

		bool isEngineStarted();
		bool writeArchwayConfigurationFile();

		std::map< std::string, std::string >& getPipelineSettings(size_t pipelineClassID) { return m_PipelineSettings[pipelineClassID]; }

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
		typedef void (*FPEnumerateAvailablePipelinesCallback)(size_t pipelineClassID, const char* pipelineDescription, void* userData);
		typedef void (*FPEnumeratePipelineParametersCallback)(size_t pipelineID, const char* parameterName, const char* parameterValue, void* userData) ;
		struct ArchwayAPI {
			size_t (*getLastError)();
			const char* (*getErrorString)(size_t errorCode);

			const char* (*getVersionDescription)();

			void (*getConfigurationParameterAsString)(const char* configurationParameter, char* outputBuffer, size_t bufferLength);
			bool (*getPipelineScenarioPath)(uint64_t pipelineID, char* messageBuffer, size_t bufferLength);

			bool (*initialize)(const char* login, const char* password, const char* applicationName, const char* configFilename);
			bool (*startAllAcquisitionDevices)();
			bool (*startImpedanceCheckOnAllAcquisitionDevices)();
			bool (*startEngine)();
			bool (*startEngineInFastForward)();
			bool (*stopEngine)();
			bool (*stopAllAcquisitionDevices)();
			bool (*uninitialize)();

			bool (*enumerateAvailablePipelines)(FPEnumerateAvailablePipelinesCallback callback, void* userData);
			size_t (*createPipeline)(size_t pipelineClassID, const char* profileName);
			bool (*releasePipeline)(size_t pipelineID);
			bool (*isPipelineRunning)(size_t pipelineID);
			bool (*isPipelineInErrorState)(size_t pipelineID);

			bool (*enumeratePipelineParameters)(size_t pipelineID, FPEnumeratePipelineParametersCallback callback, void* userData);
			bool (*setPipelineParameterAsString)(size_t pipelineID, const char* parameterName, const char* value);

			bool (*mainloop)();
			size_t (*getPendingValueCount)(size_t pipelineID, size_t matrixOutputIdx);
			size_t (*getPendingValueDimension)(size_t pipelineID, size_t matrixOutputIdx);
			bool (*getPendingValue)(size_t pipelineID, size_t matrixOutputIdx, float* value);
			size_t (*getPendingLogMessageCount)(size_t pipelineID);
			bool (*getPendingLogMessage)(size_t pipelineID, size_t* logLevel, char* messageBuffer, size_t bufferSize);

			bool (*dropPendingValues)(size_t pipelineID);
			bool (*dropPendingEvents)(size_t pipelineID);

			bool (*isInitialized)();
			bool (*isAcquiring)();
			bool (*isStarted)();
		};

		struct ArchwayAPI* m_Archway = nullptr;
		System::CDynamicModule m_ArchwayModule;
		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;

		// Current Configuration
		std::map< size_t, std::map< std::string, std::string > > m_PipelineSettings;

		static const std::string s_ArchwayConfigurationFile;
		static const std::string s_ArchwayPipelinesConfigurationFile;

		size_t m_RunningPipelineId = 0;
	};
}

#endif
