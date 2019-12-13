#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CInterfacedObject
	{
	public:

		CInterfacedObject() = default;
		explicit CInterfacedObject(const OpenViBE::CIdentifier& identifier) : m_ID(identifier) { }

		CInterfacedObject(const OpenViBE::CIdentifier& identifier, const size_t connectorType, const size_t connectorIndex)
			: m_ID(identifier), m_ConnectorType(connectorType), m_ConnectorIdx(connectorIndex) { }

		OpenViBE::CIdentifier m_ID = OV_UndefinedIdentifier;
		size_t m_ConnectorType     = 0;
		size_t m_ConnectorIdx      = 0;
	};
} // namespace OpenViBEDesigner
