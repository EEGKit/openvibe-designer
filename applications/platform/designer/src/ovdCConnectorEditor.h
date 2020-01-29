#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBE
{
	namespace Designer
	{
	class CConnectorEditor final
	{
	public:

		CConnectorEditor(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, const size_t type, const size_t index, const char* title,
						 const char* guiFilename)
			: m_Box(box), m_kernelCtx(ctx), m_type(type), m_index(index), m_guiFilename(guiFilename), m_title(title ? title : "") { }

		~CConnectorEditor() = default;
		bool run();

		OpenViBE::Kernel::IBox& m_Box;
		GtkEntry* m_IDEntry = nullptr;

	protected:
		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		size_t m_type  = 0;
		size_t m_index = 0;
		const std::string m_guiFilename;
		const std::string m_title;

		typedef bool (OpenViBE::Kernel::IBox::*get_identifier_t)(size_t index, OpenViBE::CIdentifier& identifier) const;
		typedef bool (OpenViBE::Kernel::IBox::*get_type_t)(size_t index, OpenViBE::CIdentifier& typeID) const;
		typedef bool (OpenViBE::Kernel::IBox::*get_name_t)(size_t index, OpenViBE::CString& name) const;
		typedef bool (OpenViBE::Kernel::IBox::*set_type_t)(size_t index, const OpenViBE::CIdentifier& typeID);
		typedef bool (OpenViBE::Kernel::IBox::*set_name_t)(size_t index, const OpenViBE::CString& name);
		typedef bool (OpenViBE::Kernel::IBox::*is_type_supported_t)(const OpenViBE::CIdentifier& typeID) const;
		typedef bool (OpenViBE::Kernel::IBox::*update_identifier_t)(size_t index, const OpenViBE::CIdentifier& newID);
	};
	}  //namespace Designer
}  //namespace OpenViBE
