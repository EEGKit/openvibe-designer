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
			CBitMaskSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t index, OpenViBE::CString& rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::CIdentifier& rTypeIdentifier);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void onChange();

		private:
			OpenViBE::CIdentifier m_oTypeIdentifier;
			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;

			std::vector<GtkToggleButton *> m_toggleButton;
			bool m_onValueSetting = false;
		};
	}
}
