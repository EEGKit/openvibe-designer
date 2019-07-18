#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CFloatSettingView : public CAbstractSettingView
		{
		public:
			CFloatSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t index, OpenViBE::CString& rBuilderName, const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void adjustValue(double amount);
			void onChange();


		private:
			GtkEntry* m_entry = nullptr;

			const OpenViBE::Kernel::IKernelContext& m_kernelContext;
			bool m_onValueSetting = false;
		};
	}
}
