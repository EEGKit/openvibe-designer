#include "ovdTAttributeHandler.h"

#include <cstdlib>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;

bool TAttributeHandler::removeAttribute(const CIdentifier& attributeID)
{
	if (!m_pAttributable) { return false; }
	return m_pAttributable->removeAttribute(attributeID);
}

bool TAttributeHandler::removeAllAttributes()

{
	if (!m_pAttributable) { return false; }
	return m_pAttributable->removeAllAttributes();
}

namespace OpenViBEDesigner
{
	template <>
	bool TAttributeHandler::addAttribute(const CIdentifier& attributeID, const int& rValue) const
	{
		if (!m_pAttributable) { return false; }
		char l_sValue[1024];
		sprintf(l_sValue, "%i", rValue);
		return m_pAttributable->addAttribute(attributeID, l_sValue);
	}

	template <>
	bool TAttributeHandler::addAttribute(const CIdentifier& attributeID, const bool& rValue) const
	{
		if (!m_pAttributable) { return false; }

		return m_pAttributable->addAttribute(attributeID, (rValue ? "true" : "false"));
	}

	template <>
	int TAttributeHandler::getAttributeValue<int>(const CIdentifier& attributeID) const
	{
		return strtol(m_pConstAttributable->getAttributeValue(attributeID), nullptr, 10);
	}

	template <>
	bool TAttributeHandler::getAttributeValue<bool>(const CIdentifier& attributeID) const
	{
		bool retval = false;
		const CString l_sAttributeValue(m_pConstAttributable->getAttributeValue(attributeID));
		if (l_sAttributeValue == CString("true")) { retval = true; }

		return retval;
	}

	template <>
	bool TAttributeHandler::setAttributeValue(const CIdentifier& attributeID, const int& rValue)
	{
		if (!m_pAttributable) { return false; }
		char l_sValue[1024];
		sprintf(l_sValue, "%i", rValue);
		return m_pAttributable->setAttributeValue(attributeID, l_sValue);
	}

	template <>
	bool TAttributeHandler::setAttributeValue(const CIdentifier& attributeID, const bool& rValue)
	{
		if (!m_pAttributable) { return false; }

		return m_pAttributable->setAttributeValue(attributeID, (rValue ? "true" : "false"));
	}
} // namespace OpenViBEDesigner
