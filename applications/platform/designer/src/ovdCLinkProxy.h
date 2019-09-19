#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CLinkProxy
	{
	public:

		CLinkProxy(const OpenViBE::Kernel::ILink& rLink);
		CLinkProxy(OpenViBE::Kernel::IScenario& scenario, const OpenViBE::CIdentifier& rLinkIdentifier);
		virtual ~CLinkProxy();

		operator OpenViBE::Kernel::ILink*() { return m_pLink; }
		operator const OpenViBE::Kernel::ILink*() { return m_pConstLink; }

		int getXSource() { return m_iXSource; }
		int getYSource() { return m_iYSource; }
		int getXTarget() { return m_iXTarget; }
		int getYTarget() { return m_iYTarget; }

		void setSource(int i32XSource, int i32YSource);
		void setTarget(int i32XTarget, int i32YTarget);

	protected:

		const OpenViBE::Kernel::ILink* m_pConstLink;
		OpenViBE::Kernel::ILink* m_pLink = nullptr;
		int m_iXSource = 0;
		int m_iYSource = 0;
		int m_iXTarget = 0;
		int m_iYTarget = 0;
	};
};
