#pragma once

#include <list>

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CInterfacedScenario;

	class CScenarioStateStack
	{
	public:

		CScenarioStateStack(const OpenViBE::Kernel::IKernelContext& ctx, CInterfacedScenario& interfacedScenario, OpenViBE::Kernel::IScenario& scenario);
		virtual ~CScenarioStateStack() { for (auto& state : m_states) { delete state; } }

		virtual bool isUndoPossible() { return m_currentState != m_states.begin(); }
		virtual bool undo();
		virtual bool isRedoPossible();
		virtual bool redo();
		void dropLastState() { m_states.pop_back(); }

		virtual bool snapshot();

	private:

		virtual bool restoreState(const OpenViBE::IMemoryBuffer& state);
		virtual bool dumpState(OpenViBE::IMemoryBuffer& state);

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		CInterfacedScenario& m_interfacedScenario;
		OpenViBE::Kernel::IScenario& m_scenario;

		std::list<OpenViBE::CMemoryBuffer*> m_states;
		std::list<OpenViBE::CMemoryBuffer*>::iterator m_currentState;

		size_t m_nMaximumState = 0;
	};
}
