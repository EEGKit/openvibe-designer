#pragma once

#include "ovd_base.h"

namespace OpenViBE {
namespace Designer {
class CInterfacedObject
{
public:
	CInterfacedObject() = default;
	explicit CInterfacedObject(const CIdentifier& identifier) : m_ID(identifier) { }

	CInterfacedObject(const CIdentifier& identifier, const size_t connectorType, const size_t connectorIndex)
		: m_ID(identifier), m_ConnectorType(connectorType), m_ConnectorIdx(connectorIndex) { }

	CIdentifier m_ID       = CIdentifier::undefined();
	size_t m_ConnectorType = 0;
	size_t m_ConnectorIdx  = 0;
};
}  // namespace Designer
}  // namespace OpenViBE
