#include "ovdCLinkProxy.h"
#include "ovdTAttributeHandler.h"

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;

CLinkProxy::CLinkProxy(const ILink& rLink)
	: m_pConstLink(&rLink)
{
	if (m_pConstLink)
	{
		const TAttributeHandler l_oAttributeHandler(*m_pConstLink);
		m_iXSource = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_XSourcePosition);
		m_iYSource = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_YSourcePosition);
		m_iXTarget = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_XTargetPosition);
		m_iYTarget = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_YTargetPosition);
	}
}

CLinkProxy::CLinkProxy(IScenario& rScenario, const CIdentifier& rLinkIdentifier)
	: m_pConstLink(rScenario.getLinkDetails(rLinkIdentifier))
	  , m_pLink(rScenario.getLinkDetails(rLinkIdentifier))
{
	if (m_pConstLink)
	{
		const TAttributeHandler l_oAttributeHandler(*m_pConstLink);
		m_iXSource = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_XSourcePosition);
		m_iYSource = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_YSourcePosition);
		m_iXTarget = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_XTargetPosition);
		m_iYTarget = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Link_YTargetPosition);
	}
}

CLinkProxy::~CLinkProxy()

{
	if (m_pLink)
	{
		TAttributeHandler l_oAttributeHandler(*m_pLink);

		if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XSourcePosition))
		{
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XSourcePosition, m_iXSource);
		}
		else
		{
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_XSourcePosition, m_iXSource);
		}

		if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
		{
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YSourcePosition, m_iYSource);
		}
		else
		{
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_YSourcePosition, m_iYSource);
		}

		if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XTargetPosition))
		{
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XTargetPosition, m_iXTarget);
		}
		else
		{
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_XTargetPosition, m_iXTarget);
		}

		if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
		{
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YTargetPosition, m_iYTarget);
		}
		else
		{
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_YTargetPosition, m_iYTarget);
		}
	}
}

void CLinkProxy::setSource(const int i32XSource, const int i32YSource)
{
	m_iXSource = i32XSource;
	m_iYSource = i32YSource;
}

void CLinkProxy::setTarget(const int i32XTarget, const int i32YTarget)
{
	m_iXTarget = i32XTarget;
	m_iYTarget = i32YTarget;
}
