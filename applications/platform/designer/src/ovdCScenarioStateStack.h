#pragma once

#include <list>

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CInterfacedScenario;

	class CScenarioStateStack
	{
	public:

		CScenarioStateStack(const OpenViBE::Kernel::IKernelContext& kernelContext, OpenViBEDesigner::CInterfacedScenario& interfacedScenario, OpenViBE::Kernel::IScenario& scenario);
		virtual ~CScenarioStateStack(void);

		virtual bool isUndoPossible(void);
		virtual bool undo(void);
		virtual bool isRedoPossible(void);
		virtual bool redo(void);

		virtual bool snapshot(void);

	private:

		virtual bool restoreState(const OpenViBE::IMemoryBuffer& state);
		virtual bool dumpState(OpenViBE::IMemoryBuffer& state);

	protected:

		const OpenViBE::Kernel::IKernelContext& m_KernelContext;
		OpenViBEDesigner::CInterfacedScenario& m_InterfacedScenario;
		OpenViBE::Kernel::IScenario& m_Scenario;

		std::list<OpenViBE::CMemoryBuffer*> m_States;
		std::list<OpenViBE::CMemoryBuffer*>::iterator m_CurrentState;

		OpenViBE::uint32 m_MaximumStateCount;
	};
}

