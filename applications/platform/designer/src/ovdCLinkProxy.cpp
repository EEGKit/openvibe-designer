#include "ovdCLinkProxy.h"
#include "ovdTAttributeHandler.h"

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;

CLinkProxy::CLinkProxy(const ILink& rLink)
	: m_constLink(&rLink)
{
	if (m_constLink)
	{
		const TAttributeHandler handler(*m_constLink);
		m_xSource = handler.getAttributeValue<int>(OV_AttributeId_Link_XSourcePosition);
		m_ySource = handler.getAttributeValue<int>(OV_AttributeId_Link_YSourcePosition);
		m_xTarget = handler.getAttributeValue<int>(OV_AttributeId_Link_XTargetPosition);
		m_yTarget = handler.getAttributeValue<int>(OV_AttributeId_Link_YTargetPosition);
	}
}

CLinkProxy::CLinkProxy(IScenario& scenario, const CIdentifier& linkID)
	: m_constLink(scenario.getLinkDetails(linkID))
	  , m_link(scenario.getLinkDetails(linkID))
{
	if (m_constLink)
	{
		const TAttributeHandler handler(*m_constLink);
		m_xSource = handler.getAttributeValue<int>(OV_AttributeId_Link_XSourcePosition);
		m_ySource = handler.getAttributeValue<int>(OV_AttributeId_Link_YSourcePosition);
		m_xTarget = handler.getAttributeValue<int>(OV_AttributeId_Link_XTargetPosition);
		m_yTarget = handler.getAttributeValue<int>(OV_AttributeId_Link_YTargetPosition);
	}
}

CLinkProxy::~CLinkProxy()
{
	if (m_link)
	{
		TAttributeHandler handler(*m_link);

		if (handler.hasAttribute(OV_AttributeId_Link_XSourcePosition))
		{
			handler.setAttributeValue<int>(OV_AttributeId_Link_XSourcePosition, m_xSource);
		}
		else { handler.addAttribute<int>(OV_AttributeId_Link_XSourcePosition, m_xSource); }

		if (handler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
		{
			handler.setAttributeValue<int>(OV_AttributeId_Link_YSourcePosition, m_ySource);
		}
		else { handler.addAttribute<int>(OV_AttributeId_Link_YSourcePosition, m_ySource); }

		if (handler.hasAttribute(OV_AttributeId_Link_XTargetPosition))
		{
			handler.setAttributeValue<int>(OV_AttributeId_Link_XTargetPosition, m_xTarget);
		}
		else { handler.addAttribute<int>(OV_AttributeId_Link_XTargetPosition, m_xTarget); }

		if (handler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
		{
			handler.setAttributeValue<int>(OV_AttributeId_Link_YTargetPosition, m_yTarget);
		}
		else { handler.addAttribute<int>(OV_AttributeId_Link_YTargetPosition, m_yTarget); }
	}
}

void CLinkProxy::setSource(const int xSource, const int ySource)
{
	m_xSource = xSource;
	m_ySource = ySource;
}

void CLinkProxy::setTarget(const int xTarget, const int yTarget)
{
	m_xTarget = xTarget;
	m_yTarget = yTarget;
}
