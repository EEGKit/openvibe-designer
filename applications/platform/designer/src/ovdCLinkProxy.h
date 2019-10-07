#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CLinkProxy
	{
	public:

		CLinkProxy(const OpenViBE::Kernel::ILink& rLink);
		CLinkProxy(OpenViBE::Kernel::IScenario& scenario, const OpenViBE::CIdentifier& linkID);
		virtual ~CLinkProxy();

		operator OpenViBE::Kernel::ILink*() { return m_link; }
		operator const OpenViBE::Kernel::ILink*() { return m_constLink; }

		int getXSource() { return m_xSource; }
		int getYSource() { return m_ySource; }
		int getXTarget() { return m_xTarget; }
		int getYTarget() { return m_yTarget; }

		void setSource(int xSource, int ySource);
		void setTarget(int xTarget, int yTarget);

	protected:

		const OpenViBE::Kernel::ILink* m_constLink;
		OpenViBE::Kernel::ILink* m_link = nullptr;
		int m_xSource = 0;
		int m_ySource = 0;
		int m_xTarget = 0;
		int m_yTarget = 0;
	};
};
