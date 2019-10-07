#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CAboutScenarioDialog
	{
	public:

		CAboutScenarioDialog(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IScenario& scenario, const char* sGUIFilename);
		virtual ~CAboutScenarioDialog();

		bool run();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IScenario& m_rScenario;
		OpenViBE::CString m_sGUIFilename;

	private:

		CAboutScenarioDialog();
	};
};
