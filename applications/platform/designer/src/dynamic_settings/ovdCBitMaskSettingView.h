#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <vector>

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CBitMaskSettingView : public CAbstractSettingView
		{
		public:
			CBitMaskSettingView(OpenViBE::Kernel::IBox& box, const uint32_t index, OpenViBE::CString& rBuilderName,
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
} // namespace OpenViBEDesigner
