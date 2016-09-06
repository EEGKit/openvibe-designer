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

#pragma once

#if defined TARGET_HAS_LibArchway

#include <mensia/engine.h>
#include <memory>
#include <openvibe/kernel/ovIKernelContext.h>
#include <gtk/gtk.h>

#include <vector>
#include <string>
#include <map>
#include <functional>

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

	// Replace this line when we stop supporting gcc 4.6.3 (Ubuntu 12.04)
	// class CArchwayHandler final {
	class CArchwayHandler {
	public:
		CArchwayHandler(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		~CArchwayHandler();

		EngineInitialisationStatus initialize();
		bool uninitialize();

		bool reinitializeArchway(EngineType eEngineType);
		std::vector<SPipeline> getEnginePipelines() const;
		std::vector<SPipelineParameter> getPipelineParameters(unsigned int uiPipelineClassId) const;
		bool setPipelineParameterValue(unsigned int uiPipelineClassId, std::string const& sParameterName, std::string const& sParameterValue);

		bool startEngineWithPipeline(unsigned int uiPipelineClassId);
		bool loopEngine();
		bool stopEngine();

		bool isEngineStarted();
		bool writeArchwayConfigurationFile();

		std::map< std::string, std::string >& getPipelineSettings(unsigned int uiPipelineClassId)
		{
			return m_vPipelineSettings[uiPipelineClassId];
		}

	public:
		SArchwayBridge m_oArchwayBridge;
		std::string m_sDeviceURL;

	private:
		bool initializeArchway(EngineType eEngineType);
		bool uninitializeArchway();
		bool savePipelineConfigurations();
		bool loadPipelineConfigurations();
		std::string getArchwayErrorString() const;

	private:
		Engine::MensiaEngineAPI* m_pArchway;
		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;

		// Current Configuration
		std::map< unsigned int, std::map< std::string, std::string > > m_vPipelineSettings;

		static const std::string m_sArchwayConfigurationFile;
		static const std::string m_sArchwayPipelinesConfigurationFile;

		unsigned int m_uiRunningPipelineId;
	};
}
#endif
