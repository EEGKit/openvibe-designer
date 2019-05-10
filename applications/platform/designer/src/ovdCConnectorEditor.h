#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CConnectorEditor
	{
	public:

		CConnectorEditor(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox, uint32_t ui32ConnectorType, uint32_t ui32ConnectorIndex, const char* sTitle, const char* sGUIFilename);
		virtual ~CConnectorEditor();
		virtual bool run();


		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		uint32_t m_ui32ConnectorType = 0;
		uint32_t m_ui32ConnectorIndex = 0;
		const std::string m_sGUIFilename;
		const std::string m_sTitle;
		GtkEntry* m_ConnectorIdentifierEntry = nullptr;

	private:

		//		typedef uint32_t (OpenViBE::Kernel::IBox::*t_getConnectorCount)() const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorIdentifier)(uint32_t inputIndex, OpenViBE::CIdentifier& identifier) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorType)(uint32_t ui32InputIndex, OpenViBE::CIdentifier& rTypeIdentifier) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorName)(uint32_t ui32InputIndex, OpenViBE::CString& rName) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_setConnectorType)(uint32_t ui32InputIndex, const OpenViBE::CIdentifier& rTypeIdentifier);
		typedef bool (OpenViBE::Kernel::IBox::*t_setConnectorName)(uint32_t ui32InputIndex, const OpenViBE::CString& rName);
		typedef bool (OpenViBE::Kernel::IBox::*t_isTypeSupported)(const OpenViBE::CIdentifier& rTypeIdentifier) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_updateConnectorIdentifier)(uint32_t inputIndex, const OpenViBE::CIdentifier& newIdentifier);
	};
};
