#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBE {
namespace Designer {
class CConnectorEditor final
{
public:

	CConnectorEditor(const Kernel::IKernelContext& ctx, Kernel::IBox& box, const size_t type, const size_t index, const char* title, const char* guiFilename)
		: m_Box(box), m_kernelCtx(ctx), m_type(type), m_index(index), m_guiFilename(guiFilename), m_title(title ? title : "") { }

	~CConnectorEditor() = default;
	bool run();

	Kernel::IBox& m_Box;
	GtkEntry* m_IDEntry = nullptr;

protected:
	const Kernel::IKernelContext& m_kernelCtx;
	size_t m_type  = 0;
	size_t m_index = 0;
	const std::string m_guiFilename;
	const std::string m_title;

	typedef bool (Kernel::IBox::*get_identifier_t)(size_t index, CIdentifier& identifier) const;
	typedef bool (Kernel::IBox::*get_type_t)(size_t index, CIdentifier& typeID) const;
	typedef bool (Kernel::IBox::*get_name_t)(size_t index, CString& name) const;
	typedef bool (Kernel::IBox::*set_type_t)(size_t index, const CIdentifier& typeID);
	typedef bool (Kernel::IBox::*set_name_t)(size_t index, const CString& name);
	typedef bool (Kernel::IBox::*is_type_supported_t)(const CIdentifier& typeID) const;
	typedef bool (Kernel::IBox::*update_identifier_t)(size_t index, const CIdentifier& newID);
};
}  //namespace Designer
}  //namespace OpenViBE
