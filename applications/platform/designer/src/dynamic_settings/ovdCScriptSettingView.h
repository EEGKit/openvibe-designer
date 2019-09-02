#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CScriptSettingView : public CAbstractSettingView
		{
		public:
			CScriptSettingView(OpenViBE::Kernel::IBox& box, const uint32_t index, OpenViBE::CString& rBuilderName,
							   const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void browse() const;
			void edit() const;

			void onChange();
#if defined TARGET_OS_Windows
			void onFocusLost();
#endif

		private:
			GtkEntry* m_entry = nullptr;

			const OpenViBE::Kernel::IKernelContext& m_kernelContext;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
