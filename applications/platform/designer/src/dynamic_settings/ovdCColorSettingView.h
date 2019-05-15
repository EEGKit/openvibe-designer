#ifndef __OpenViBE_Designer_Setting_CColorSettingView_H__
#define __OpenViBE_Designer_Setting_CColorSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CColorSettingView : public CAbstractSettingView
		{
		public:
			CColorSettingView(OpenViBE::Kernel::IBox& rBox,
								uint32_t index,
								OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString &rValue) const override;
			void setValue(const OpenViBE::CString &rValue) override;

			void selectColor();
			void onChange();

		private:
			::GtkEntry* m_pEntry;
			::GtkColorButton *m_pButton;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			bool m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CColorSettingView_H__
