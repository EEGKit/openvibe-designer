#pragma once

#include <list>

#include "ovd_base.h"

namespace OpenViBE {
namespace Designer {
class CInterfacedScenario;

class CScenarioStateStack
{
public:
	CScenarioStateStack(const Kernel::IKernelContext& ctx, CInterfacedScenario& interfacedScenario, Kernel::IScenario& scenario);
	~CScenarioStateStack() { for (const auto& state : m_states) { delete state; } }

	bool isUndoPossible() { return m_currentState != m_states.begin(); }
	bool undo();
	bool isRedoPossible();
	bool redo();
	void dropLastState() { m_states.pop_back(); }

	bool snapshot();

private:
	bool restoreState(const IMemoryBuffer& state) const;
	bool dumpState(IMemoryBuffer& state) const;

protected:
	const Kernel::IKernelContext& m_kernelCtx;
	CInterfacedScenario& m_interfacedScenario;
	Kernel::IScenario& m_scenario;

	std::list<CMemoryBuffer*> m_states;
	std::list<CMemoryBuffer*>::iterator m_currentState;

	size_t m_nMaximumState = 0;
};
}  // namespace Designer
}  // namespace OpenViBE
