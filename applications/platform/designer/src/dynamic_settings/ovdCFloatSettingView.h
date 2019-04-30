#ifndef __OpenViBE_Designer_Setting_CFloatSettingView_H__
#define __OpenViBE_Designer_Setting_CFloatSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CFloatSettingView : public CAbstractSettingView
		{
		public:
			CFloatSettingView(OpenViBE::Kernel::IBox& rBox,
								uint32_t ui32Index,
								OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void adjustValue(double amount);
			void onChange();




		private:
			::GtkEntry* m_pEntry;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			bool m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CFloatSettingView_H__
