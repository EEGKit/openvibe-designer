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
		std::function<unsigned int(unsigned int)> getAvailableValueMatrixCount;
		std::function<std::vector<float>(unsigned int)> popValueMatrix;
		std::function<bool()> dropBuffers;
	};

	struct SGUIBridge
	{
		std::function<void()> refreshStoppedEngine;
	};

	class CArchwayHandler final {
	public:
		CArchwayHandler(const OpenViBE::Kernel::IKernelContext& kernelContext);
		~CArchwayHandler();

		EngineInitialisationStatus initialize();
		bool uninitialize();

		bool reinitializeArchway();
		std::vector<SPipeline> getEnginePipelines() const;
		std::vector<SPipelineParameter> getPipelineParameters(unsigned int pipelineClassID) const;
		bool setPipelineParameterValue(unsigned int pipelineClassID, std::string const& parameterName, std::string const& parameterValue);
		std::string getPipelineScenarioPath(uint64_t pipelineID) const;

		bool startEngineWithPipeline(unsigned int pipelineClassID, bool isFastForward, bool shouldAcquireImpedance);
		bool loopEngine();
		bool stopEngine();

		bool isEngineStarted();
		bool writeArchwayConfigurationFile();

		std::map< std::string, std::string >& getPipelineSettings(unsigned int pipelineClassID) { return m_PipelineSettings[pipelineClassID]; }

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
		typedef void (*FPEnumerateAvailablePipelinesCallback)(unsigned int pipelineClassID, const char* pipelineDescription, void* userData);
		typedef void (*FPEnumeratePipelineParametersCallback)(unsigned int pipelineID, const char* parameterName, const char* parameterValue, void* userData) ;
		struct ArchwayAPI {
			unsigned int (*getLastError)();
			const char* (*getErrorString)(unsigned int errorCode);

			const char* (*getVersionDescription)();

			void (*getConfigurationParameterAsString)(const char* configurationParameter, char* outputBuffer, unsigned int bufferLength);
			bool (*getPipelineScenarioPath)(uint64_t pipelineID, char* messageBuffer, unsigned int bufferLength);

			bool (*initialize)(const char* login, const char* password, const char* applicationName, const char* configurationFilename);
			bool (*startAllAcquisitionDevices)();
			bool (*startImpedanceCheckOnAllAcquisitionDevices)();
			bool (*startEngine)();
			bool (*startEngineInFastForward)();
			bool (*stopEngine)();
			bool (*stopAllAcquisitionDevices)();
			bool (*uninitialize)();

			bool (*enumerateAvailablePipelines)(FPEnumerateAvailablePipelinesCallback callback, void* userData);
			unsigned int (*createPipeline)(unsigned int pipelineClassID, const char* profileName);
			bool (*releasePipeline)(unsigned int pipelineID);
			bool (*isPipelineRunning)(unsigned int pipelineID);
			bool (*isPipelineInErrorState)(unsigned int pipelineID);

			bool (*enumeratePipelineParameters)(unsigned int pipelineID, FPEnumeratePipelineParametersCallback callback, void* userData);
			bool (*setPipelineParameterAsString)(unsigned int pipelineID, const char* parameterName, const char* value);

			bool (*mainloop)();
			unsigned int (*getPendingValueCount)(unsigned int pipelineID, unsigned int matrixOutputIdx);
			unsigned int (*getPendingValueDimension)(unsigned int pipelineID, unsigned int matrixOutputIdx);
			bool (*getPendingValue)(unsigned int pipelineID, unsigned int matrixOutputIdx, float* value);
			unsigned int (*getPendingLogMessageCount)(unsigned int pipelineID);
			bool (*getPendingLogMessage)(unsigned int pipelineID, unsigned int* logLevel, char* messageBuffer, unsigned int bufferSize);

			bool (*dropPendingValues)(unsigned int pipelineID);
			bool (*dropPendingEvents)(unsigned int pipelineID);

			bool (*isInitialized)();
			bool (*isAcquiring)();
			bool (*isStarted)();
		};

		struct ArchwayAPI* m_Archway = nullptr;
		System::CDynamicModule m_ArchwayModule;
		const OpenViBE::Kernel::IKernelContext& m_KernelContext;

		// Current Configuration
		std::map< unsigned int, std::map< std::string, std::string > > m_PipelineSettings;

		static const std::string s_ArchwayConfigurationFile;
		static const std::string s_ArchwayPipelinesConfigurationFile;

		unsigned int m_RunningPipelineId = 0;
	};
}

#endif
