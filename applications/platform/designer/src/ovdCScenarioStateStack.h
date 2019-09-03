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
		virtual ~CScenarioStateStack();

		virtual bool isUndoPossible();
		virtual bool undo();
		virtual bool isRedoPossible();
		virtual bool redo();
		void dropLastState();

		virtual bool snapshot();

	private:

		virtual bool restoreState(const OpenViBE::IMemoryBuffer& state);
		virtual bool dumpState(OpenViBE::IMemoryBuffer& state);

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		CInterfacedScenario& m_InterfacedScenario;
		OpenViBE::Kernel::IScenario& m_Scenario;

		std::list<OpenViBE::CMemoryBuffer*> m_States;
		std::list<OpenViBE::CMemoryBuffer*>::iterator m_CurrentState;

		uint32_t m_MaximumStateCount = 0;
	};
}
