#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CConnectorEditor
	{
	public:

		CConnectorEditor(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, size_t connectorType, 
						 size_t connectorIndex, const char* sTitle, const char* sGUIFilename);
		virtual ~CConnectorEditor();
		virtual bool run();


		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IBox& m_box;
		size_t m_connectorType = 0;
		size_t m_connectorIdx = 0;
		const std::string m_sGUIFilename;
		const std::string m_sTitle;
		GtkEntry* m_ConnectorIdentifierEntry = nullptr;

	private:

		//		typedef size_t (OpenViBE::Kernel::IBox::*t_getConnectorCount)() const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorIdentifier)(size_t index, OpenViBE::CIdentifier& identifier) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorType)(size_t index, OpenViBE::CIdentifier& typeID) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_getConnectorName)(size_t index, OpenViBE::CString& name) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_setConnectorType)(size_t index, const OpenViBE::CIdentifier& typeID);
		typedef bool (OpenViBE::Kernel::IBox::*t_setConnectorName)(size_t index, const OpenViBE::CString& name);
		typedef bool (OpenViBE::Kernel::IBox::*t_isTypeSupported)(const OpenViBE::CIdentifier& typeID) const;
		typedef bool (OpenViBE::Kernel::IBox::*t_updateConnectorIdentifier)(size_t index, const OpenViBE::CIdentifier& newID);
	};
};
