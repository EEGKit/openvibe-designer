#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CAboutScenarioDialog final
	{
	public:

		CAboutScenarioDialog(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IScenario& scenario, const char* guiFilename)
			: m_kernelCtx(ctx), m_scenario(scenario), m_guiFilename(guiFilename) { }

		~CAboutScenarioDialog() = default;

		bool run();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IScenario& m_scenario;
		OpenViBE::CString m_guiFilename;

		CAboutScenarioDialog() = delete;
	};
}  // namespace OpenViBEDesigner
