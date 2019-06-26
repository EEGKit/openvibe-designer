#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CColorSettingView : public CAbstractSettingView
		{
		public:
			CColorSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t index, OpenViBE::CString& rBuilderName, const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void selectColor();
			void onChange();

		private:
			GtkEntry* m_entry = nullptr;
			GtkColorButton* m_button = nullptr;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			bool m_onValueSetting = false;
		};
	}
}
