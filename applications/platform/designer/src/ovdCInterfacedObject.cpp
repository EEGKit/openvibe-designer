#include "ovdCInterfacedObject.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;

CInterfacedObject::CInterfacedObject(const CIdentifier& identifier) : m_id(identifier) { }

CInterfacedObject::CInterfacedObject(const CIdentifier& identifier, const uint32_t connectorType, const uint32_t connectorIndex)
	: m_id(identifier), m_connectorType(connectorType), m_connectorIdx(connectorIndex) { }
