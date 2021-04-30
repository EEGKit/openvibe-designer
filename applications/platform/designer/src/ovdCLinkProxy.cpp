#include "ovdCLinkProxy.h"
#include "ovdTAttributeHandler.h"
#include "../../../../../sdk/openvibe/include/openvibe/ov_defines.h"

namespace OpenViBE {
namespace Designer {

CLinkProxy::CLinkProxy(const Kernel::ILink& link) : m_constLink(&link)
{
	if (m_constLink)
	{
		const TAttributeHandler handler(*m_constLink);
		m_xSrc = handler.getAttributeValue<int>(OV_AttributeId_Link_XSrc);
		m_ySrc = handler.getAttributeValue<int>(OV_AttributeId_Link_YSrc);
		m_xDst = handler.getAttributeValue<int>(OV_AttributeId_Link_XDst);
		m_yDst = handler.getAttributeValue<int>(OV_AttributeId_Link_YDst);
	}
}

CLinkProxy::CLinkProxy(Kernel::IScenario& scenario, const CIdentifier& linkID)
	: m_constLink(scenario.getLinkDetails(linkID)), m_link(scenario.getLinkDetails(linkID))
{
	if (m_constLink)
	{
		const TAttributeHandler handler(*m_constLink);
		m_xSrc = handler.getAttributeValue<int>(OV_AttributeId_Link_XSrc);
		m_ySrc = handler.getAttributeValue<int>(OV_AttributeId_Link_YSrc);
		m_xDst = handler.getAttributeValue<int>(OV_AttributeId_Link_XDst);
		m_yDst = handler.getAttributeValue<int>(OV_AttributeId_Link_YDst);
	}
}

CLinkProxy::~CLinkProxy()
{
	if (m_link)
	{
		TAttributeHandler handler(*m_link);

		if (handler.hasAttribute(OV_AttributeId_Link_XSrc)) { handler.setAttributeValue<int>(OV_AttributeId_Link_XSrc, m_xSrc); }
		else { handler.addAttribute<int>(OV_AttributeId_Link_XSrc, m_xSrc); }

		if (handler.hasAttribute(OV_AttributeId_Link_YSrc)) { handler.setAttributeValue<int>(OV_AttributeId_Link_YSrc, m_ySrc); }
		else { handler.addAttribute<int>(OV_AttributeId_Link_YSrc, m_ySrc); }

		if (handler.hasAttribute(OV_AttributeId_Link_XDst)) { handler.setAttributeValue<int>(OV_AttributeId_Link_XDst, m_xDst); }
		else { handler.addAttribute<int>(OV_AttributeId_Link_XDst, m_xDst); }

		if (handler.hasAttribute(OV_AttributeId_Link_YDst)) { handler.setAttributeValue<int>(OV_AttributeId_Link_YDst, m_yDst); }
		else { handler.addAttribute<int>(OV_AttributeId_Link_YDst, m_yDst); }
	}
}

void CLinkProxy::setSource(const int x, const int y)
{
	m_xSrc = x;
	m_ySrc = y;
}

void CLinkProxy::setTarget(const int x, const int y)
{
	m_xDst = x;
	m_yDst = y;
}

}  // namespace Designer
}  // namespace OpenViBE
