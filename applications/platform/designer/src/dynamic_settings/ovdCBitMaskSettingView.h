#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <vector>

namespace OpenViBE
{
	namespace Designer
	{
	namespace Setting
	{
		class CBitMaskSettingView final : public CAbstractSettingView
		{
		public:
			CBitMaskSettingView(OpenViBE::Kernel::IBox& box, const size_t index, OpenViBE::CString& builderName,
								const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CIdentifier& typeID);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void onChange();

		private:
			OpenViBE::CIdentifier m_typeID = OV_UndefinedIdentifier;
			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;

			std::vector<GtkToggleButton *> m_toggleButton;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
