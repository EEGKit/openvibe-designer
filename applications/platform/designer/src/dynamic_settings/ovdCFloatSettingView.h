#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBE
{
	namespace Designer
	{
	namespace Setting
	{
		class CFloatSettingView final : public CAbstractSettingView
		{
		public:
			CFloatSettingView(OpenViBE::Kernel::IBox& box, const size_t index, OpenViBE::CString& builderName,
							  const OpenViBE::Kernel::IKernelContext& ctx);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void adjustValue(double amount);
			void onChange();


		private:
			GtkEntry* m_entry = nullptr;

			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
