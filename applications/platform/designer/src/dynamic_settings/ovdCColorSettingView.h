#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBE
{
	namespace Designer
	{
		namespace Setting
		{
			class CColorSettingView final : public CAbstractSettingView
			{
			public:
				CColorSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx);

				void getValue(CString& value) const override;
				void setValue(const CString& value) override;

				void selectColor();
				void onChange();

			private:
				GtkEntry* m_entry        = nullptr;
				GtkColorButton* m_button = nullptr;

				const Kernel::IKernelContext& m_kernelCtx;
				bool m_onValueSetting = false;
			};
		} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
