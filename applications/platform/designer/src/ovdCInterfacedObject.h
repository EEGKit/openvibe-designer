#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CInterfacedObject
	{
	public:

		CInterfacedObject() = default;
		CInterfacedObject(const OpenViBE::CIdentifier& identifier);
		CInterfacedObject(const OpenViBE::CIdentifier& identifier, uint32_t connectorType, uint32_t connectorIndex);

		OpenViBE::CIdentifier m_oIdentifier;
		uint32_t m_connectorType = 0;
		uint32_t m_connectorIndex = 0;
	};
};
