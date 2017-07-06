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
		std::function<void()> resfreshStoppedEngine;
	};

	// Replace this line when we stop supporting gcc 4.6.3 (Ubuntu 12.04)
	// class CArchwayHandler final {
	class CArchwayHandler {
	public:
		CArchwayHandler(const OpenViBE::Kernel::IKernelContext& kernelContext);
		~CArchwayHandler();

		EngineInitialisationStatus initialize();
		bool uninitialize();

		bool reinitializeArchway(EngineType engineType);
		std::vector<SPipeline> getEnginePipelines() const;
		std::vector<SPipelineParameter> getPipelineParameters(unsigned int pipelineClassId) const;
		bool setPipelineParameterValue(unsigned int pipelineClassId, std::string const& parameterName, std::string const& parameterValue);

		bool startEngineWithPipeline(unsigned int pipelineClassId, bool isFastForward);
		bool loopEngine();
		bool stopEngine();

		bool isEngineStarted();
		bool writeArchwayConfigurationFile();

		std::map< std::string, std::string >& getPipelineSettings(unsigned int pipelineClassId)
		{
			return m_PipelineSettings[pipelineClassId];
		}

	public:
		SArchwayBridge m_oArchwayBridge;
		SGUIBridge m_guiBridge;
		std::string m_sDeviceURL;

	private:
		bool initializeArchway(EngineType eEngineType);
		bool uninitializeArchway();
		bool savePipelineConfigurations();
		bool loadPipelineConfigurations();
		std::string getArchwayErrorString() const;

	private:
		typedef void (*FPEnumerateAvailablePipelinesCallback)(unsigned int pipelineClassId, const char* pipelineDescription, void* userData);
		typedef void (*FPEnumeratePipelineParametersCallback)(unsigned int pipelineId, const char* parameterName, const char* parameterValue, void* userData) ;
		struct ArchwayAPI {
			unsigned int (*getLastError)(void);
			const char* (*getErrorString)(unsigned int errorCode);

			const char* (*getVersionDescription)(void);

			void (*getConfigurationParameterAsString)(const char* configurationParameter, char* outputBuffer, unsigned int bufferLength);

			bool (*initialize)(const char* login, const char* password, const char* applicationName, const char* configurationFilename);
			bool (*startAllAcquisitionDevices)(void);
			bool (*startEngine)(void);
			bool (*startEngineInFastForward)(void);
			bool (*stopEngine)(void);
			bool (*stopAllAcquisitionDevices)(void);
			bool (*uninitialize)(void);

			bool (*enumerateAvailablePipelines)(FPEnumerateAvailablePipelinesCallback callback, void* userData);
			unsigned int (*createPipeline)(unsigned int pipelineClassId, const char* profileName);
			bool (*releasePipeline)(unsigned int pipelineId);
			bool (*isPipelineRunning)(unsigned int pipelineId);
			bool (*isPipelineInErrorState)(unsigned int pipelineId);

			bool (*enumeratePipelineParameters)(unsigned int pipelineId, FPEnumeratePipelineParametersCallback callback, void* userData);
			bool (*setPipelineParameterAsString)(unsigned int pipelineId, const char* parameterName, const char* value);

			bool (*mainloop)(void);
			unsigned int (*getPendingValueCount)(unsigned int pipelineId, unsigned int matrixOutputIndex);
			unsigned int (*getPendingValueDimension)(unsigned int pipelineId, unsigned int matrixOutputIndex);
			bool (*getPendingValue)(unsigned int pipelineId, unsigned int matrixOutputIndex, float* value);
			unsigned int (*getPendingLogMessageCount)(unsigned int pipelineId);
			bool (*getPendingLogMessage)(unsigned int pipelineId, unsigned int* logLevel, char* messageBuffer, unsigned int bufferSize);

			bool (*dropPendingValues)(unsigned int pipelineId);
			bool (*dropPendingEvents)(unsigned int pipelineId);

			bool (*isInitialized)(void);
			bool (*isAcquiring)(void);
			bool (*isStarted)(void);
		};

		struct ArchwayAPI* m_Archway;
		System::CDynamicModule m_ArchwayModule;
		const OpenViBE::Kernel::IKernelContext& m_KernelContext;

		// Current Configuration
		std::map< unsigned int, std::map< std::string, std::string > > m_PipelineSettings;

		static const std::string s_ArchwayConfigurationFile;
		static const std::string s_ArchwayPipelinesConfigurationFile;

		unsigned int m_RunningPipelineId;
	};
}

#endif