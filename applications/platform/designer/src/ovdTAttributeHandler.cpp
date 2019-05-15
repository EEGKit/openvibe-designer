#include "ovdTAttributeHandler.h"

#include <cstdlib>

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;

bool TAttributeHandler::removeAttribute(const CIdentifier& rAttributeIdentifier)
{
	if (!m_pAttributable) { return false; }
	return m_pAttributable->removeAttribute(rAttributeIdentifier);
}

bool TAttributeHandler::removeAllAttributes()

{
	if (!m_pAttributable) { return false; }
	return m_pAttributable->removeAllAttributes();
}

namespace OpenViBEDesigner
{
	template <>
	bool TAttributeHandler::addAttribute(const CIdentifier& rAttributeIdentifier, const int& rValue) const
	{
		if (!m_pAttributable) { return false; }
		char l_sValue[1024];
		sprintf(l_sValue, "%i", rValue);
		return m_pAttributable->addAttribute(rAttributeIdentifier, l_sValue);
	}

	template <>
	bool TAttributeHandler::addAttribute(const CIdentifier& rAttributeIdentifier, const bool& rValue) const
	{
		if (!m_pAttributable) { return false; }

		return m_pAttributable->addAttribute(rAttributeIdentifier, (rValue ? "true" : "false"));
	}

	template <>
	int TAttributeHandler::getAttributeValue<int>(const CIdentifier& rAttributeIdentifier) const
	{
		return strtol(m_pConstAttributable->getAttributeValue(rAttributeIdentifier), nullptr, 10);
	}

	template <>
	bool TAttributeHandler::getAttributeValue<bool>(const CIdentifier& rAttributeIdentifier) const
	{
		bool retval = false;
		const CString l_sAttributeValue(m_pConstAttributable->getAttributeValue(rAttributeIdentifier));
		if (l_sAttributeValue == CString("true"))
		{
			retval = true;
		}

		return retval;
	}

	template <>
	bool TAttributeHandler::setAttributeValue(const CIdentifier& rAttributeIdentifier, const int& rValue)
	{
		if (!m_pAttributable) { return false; }
		char l_sValue[1024];
		sprintf(l_sValue, "%i", rValue);
		return m_pAttributable->setAttributeValue(rAttributeIdentifier, l_sValue);
	}

	template <>
	bool TAttributeHandler::setAttributeValue(const CIdentifier& rAttributeIdentifier, const bool& rValue)
	{
		if (!m_pAttributable) { return false; }

		return m_pAttributable->setAttributeValue(rAttributeIdentifier, (rValue ? "true" : "false"));
	}
} // namespace OpenViBEDesigner
