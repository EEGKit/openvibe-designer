#include "ovdTAttributeHandler.h"

#include <cstdlib>

namespace OpenViBE {
namespace Designer {

bool TAttributeHandler::removeAttribute(const CIdentifier& id) const
{
	if (!m_attributable) { return false; }
	return m_attributable->removeAttribute(id);
}

bool TAttributeHandler::removeAllAttributes() const

{
	if (!m_attributable) { return false; }
	return m_attributable->removeAllAttributes();
}

template <>
bool TAttributeHandler::addAttribute(const CIdentifier& id, const int& value) const
{
	if (!m_attributable) { return false; }
	const std::string str = std::to_string(value);	// pass directly  std::to_string().c_str() with int value can return anything
	return m_attributable->addAttribute(id, str.c_str());
}

template <>
bool TAttributeHandler::addAttribute(const CIdentifier& id, const bool& value) const
{
	if (!m_attributable) { return false; }

	return m_attributable->addAttribute(id, (value ? "true" : "false"));
}

template <>
int TAttributeHandler::getAttributeValue<int>(const CIdentifier& id) const { return strtol(m_constAttributable->getAttributeValue(id), nullptr, 10); }

template <>
bool TAttributeHandler::getAttributeValue<bool>(const CIdentifier& id) const
{
	bool res = false;
	const CString value(m_constAttributable->getAttributeValue(id));
	if (value == CString("true")) { res = true; }

	return res;
}

template <>
bool TAttributeHandler::setAttributeValue(const CIdentifier& id, const int& value)
{
	if (!m_attributable) { return false; }
	const std::string str = std::to_string(value);	// pass directly  std::to_string().c_str() with int value can return anything
	return m_attributable->setAttributeValue(id, str.c_str());
}

template <>
bool TAttributeHandler::setAttributeValue(const CIdentifier& id, const bool& value)
{
	if (!m_attributable) { return false; }
	return m_attributable->setAttributeValue(id, (value ? "true" : "false"));
}
}  // namespace Designer
}  // namespace OpenViBE
