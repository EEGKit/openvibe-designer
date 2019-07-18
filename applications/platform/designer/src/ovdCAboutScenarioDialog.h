#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CAboutScenarioDialog
	{
	public:

		CAboutScenarioDialog(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IScenario& rScenario, const char* sGUIFilename);
		virtual ~CAboutScenarioDialog();

		bool run();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::Kernel::IScenario& m_rScenario;
		OpenViBE::CString m_sGUIFilename;

	private:

		CAboutScenarioDialog();
	};
};
