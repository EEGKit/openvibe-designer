#include "ovdCInterfacedObject.h"

using namespace OpenViBE;
using namespace OpenViBEDesigner;

CInterfacedObject::CInterfacedObject() { }

CInterfacedObject::CInterfacedObject(const CIdentifier& rIdentifier)
	: m_oIdentifier(rIdentifier) { }

CInterfacedObject::CInterfacedObject(const CIdentifier& rIdentifier, const uint32_t ui32ConnectorType, const uint32_t ui32ConnectorIndex)
	: m_oIdentifier(rIdentifier), m_ui32ConnectorType(ui32ConnectorType), m_ui32ConnectorIndex(ui32ConnectorIndex) { }
