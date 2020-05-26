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
	virtual ~CScenarioStateStack() { for (auto& state : m_states) { delete state; } }

	virtual bool isUndoPossible() { return m_currentState != m_states.begin(); }
	virtual bool undo();
	virtual bool isRedoPossible();
	virtual bool redo();
	void dropLastState() { m_states.pop_back(); }

	virtual bool snapshot();

private:

	virtual bool restoreState(const IMemoryBuffer& state);
	virtual bool dumpState(IMemoryBuffer& state);

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
