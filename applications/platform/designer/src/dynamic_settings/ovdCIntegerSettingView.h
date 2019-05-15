#ifndef __OpenViBE_Designer_Setting_CIntegerSettingView_H__
#define __OpenViBE_Designer_Setting_CIntegerSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CIntegerSettingView : public CAbstractSettingView
		{
		public:
			CIntegerSettingView(OpenViBE::Kernel::IBox& rBox,
								uint32_t index,
								OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString &rValue) const override;
			void setValue(const OpenViBE::CString &rValue) override;

			void adjustValue(int amount);
			void onChange();


		private:
			::GtkEntry* m_pEntry;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			bool m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CIntegerSettingView_H__
