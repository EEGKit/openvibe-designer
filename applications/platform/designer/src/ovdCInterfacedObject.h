#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CInterfacedObject
	{
	public:

		CInterfacedObject();
		CInterfacedObject(const OpenViBE::CIdentifier& rIdentifier);
		CInterfacedObject(const OpenViBE::CIdentifier& rIdentifier, uint32_t ui32ConnectorType, uint32_t ui32ConnectorIndex);

		OpenViBE::CIdentifier m_oIdentifier;
		uint32_t m_ui32ConnectorType = 0;
		uint32_t m_ui32ConnectorIndex = 0;
	};
};
