#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CIntegerSettingView final : public CAbstractSettingView
		{
		public:
			CIntegerSettingView(OpenViBE::Kernel::IBox& box, const size_t index, OpenViBE::CString& builderName,
								const OpenViBE::Kernel::IKernelContext& ctx);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void adjustValue(int amount);
			void onChange();


		private:
			GtkEntry* m_entry = nullptr;

			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
