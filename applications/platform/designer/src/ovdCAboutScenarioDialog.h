#pragma once

#include "ovd_base.h"

namespace OpenViBE {
namespace Designer {

class CAboutScenarioDialog final
{
public:

	CAboutScenarioDialog(const Kernel::IKernelContext& ctx, Kernel::IScenario& scenario, const char* guiFilename)
		: m_kernelCtx(ctx), m_scenario(scenario), m_guiFilename(guiFilename) { }

	~CAboutScenarioDialog() = default;

	bool run();

protected:

	const Kernel::IKernelContext& m_kernelCtx;
	Kernel::IScenario& m_scenario;
	CString m_guiFilename;

	CAboutScenarioDialog() = delete;
};

}  // namespace Designer
}  // namespace OpenViBE
