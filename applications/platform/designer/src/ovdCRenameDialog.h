#pragma once

#include "ovd_base.h"

namespace OpenViBE
{
	namespace Designer
	{
		class CRenameDialog final
		{
		public:

			CRenameDialog(const Kernel::IKernelContext& ctx, const CString& initialName, const CString& defaultName, const char* guiFilename)
				: m_kernelCtx(ctx), m_initialName(initialName), m_defaultName(defaultName), m_result(initialName), m_guiFilename(guiFilename) { }

			~CRenameDialog() = default;

			bool run();
			CString getResult() const { return m_result; }

		protected:

			const Kernel::IKernelContext& m_kernelCtx;
			CString m_initialName;
			CString m_defaultName;
			CString m_result;
			CString m_guiFilename;

		private:
			CRenameDialog() = delete;
		};
	}  // namespace Designer
}  // namespace OpenViBE
