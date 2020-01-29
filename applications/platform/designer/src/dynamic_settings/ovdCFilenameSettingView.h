#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBE
{
	namespace Designer
	{
	namespace Setting
	{
		class CFilenameSettingView final : public CAbstractSettingView
		{
		public:
			CFilenameSettingView(OpenViBE::Kernel::IBox& box, const size_t index, OpenViBE::CString& builderName,
								 const OpenViBE::Kernel::IKernelContext& ctx);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void browse() const;
			void onChange();

#if defined TARGET_OS_Windows
			void onFocusLost();
#endif

		private:
			GtkEntry* m_entry = nullptr;

			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
