#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CConnectorEditor
	{
	public:

		CConnectorEditor(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, uint32_t connectorType, 
						 uint32_t connectorIndex, const char* sTitle, const char* sGUIFilename);
		virtual ~CConnectorEditor();
		virtual bool run();


		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		uint32_t m_connectorType = 0;
		uint32_t m_connectorIndex = 0;
		const std::string m_sGUIFilename;
		const std::string m_sTitle;
		GtkEntry* m_ConnectorIdentifierEntry = nullptr;

	private:

		//		typedef uint32_t (OpenViBE::Kernel::IBox::*t_getConnectorCount)() const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorIdentifier)(uint32_t index, OpenViBE::CIdentifier& identifier) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorType)(uint32_t index, OpenViBE::CIdentifier& typeID) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorName)(uint32_t index, OpenViBE::CString& name) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_setConnectorType)(uint32_t index, const OpenViBE::CIdentifier& typeID);
		typedef bool (OpenViBE::Kernel::IBox::*t_setConnectorName)(uint32_t index, const OpenViBE::CString& name);
		typedef bool (OpenViBE::Kernel::IBox::*t_isTypeSupported)(const OpenViBE::CIdentifier& typeID) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_updateConnectorIdentifier)(uint32_t index, const OpenViBE::CIdentifier& newID);
	};
};
