#pragma once

#include "ovd_base.h"

namespace OpenViBE
{
	namespace Designer
	{
	class CRenameDialog final
	{
	public:

		CRenameDialog(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CString& initialName, const OpenViBE::CString& defaultName,
					  const char* guiFilename)
			: m_kernelCtx(ctx), m_initialName(initialName), m_defaultName(defaultName), m_result(initialName), m_guiFilename(guiFilename) { }

		~CRenameDialog() = default;

		bool run();
		OpenViBE::CString getResult() const { return m_result; }

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::CString m_initialName;
		OpenViBE::CString m_defaultName;
		OpenViBE::CString m_result;
		OpenViBE::CString m_guiFilename;

	private:
		CRenameDialog() = delete;
	};
	}  // namespace Designer
}  // namespace OpenViBE
