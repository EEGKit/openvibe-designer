#include "ovdCInterfacedObject.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;

CInterfacedObject::CInterfacedObject(const CIdentifier& identifier) : m_oIdentifier(identifier) { }

CInterfacedObject::CInterfacedObject(const CIdentifier& identifier, const uint32_t connectorType, const uint32_t connectorIndex)
	: m_oIdentifier(identifier), m_connectorType(connectorType), m_connectorIndex(connectorIndex) { }
