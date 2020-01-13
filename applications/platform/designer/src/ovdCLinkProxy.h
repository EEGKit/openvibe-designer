#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CLinkProxy final
	{
	public:

		explicit CLinkProxy(const OpenViBE::Kernel::ILink& link);
		CLinkProxy(OpenViBE::Kernel::IScenario& scenario, const OpenViBE::CIdentifier& linkID);
		~CLinkProxy();

		operator OpenViBE::Kernel::ILink*() const { return m_link; }
		operator const OpenViBE::Kernel::ILink*() const { return m_constLink; }

		int getXSource() const { return m_xSrc; }
		int getYSource() const { return m_ySrc; }
		int getXTarget() const { return m_xDst; }
		int getYTarget() const { return m_yDst; }

		void setSource(int x, int y);
		void setTarget(int x, int y);

	protected:

		const OpenViBE::Kernel::ILink* m_constLink;
		OpenViBE::Kernel::ILink* m_link = nullptr;
		int m_xSrc                      = 0;
		int m_ySrc                      = 0;
		int m_xDst                      = 0;
		int m_yDst                      = 0;
	};
}  // namespace OpenViBEDesigner
