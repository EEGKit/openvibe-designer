#ifndef __OpenViBE_Designer_Setting_CEnumerationSettingView_H__
#define __OpenViBE_Designer_Setting_CEnumerationSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <map>

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CEnumerationSettingView : public CAbstractSettingView
		{
		public:
			CEnumerationSettingView(OpenViBE::Kernel::IBox& rBox,
								uint32_t index,
								OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext,
									const OpenViBE::CIdentifier &rTypeIdentifier);

			void getValue(OpenViBE::CString &rValue) const override;
			void setValue(const OpenViBE::CString &rValue) override;

			void onChange();


		private:
			::GtkComboBox* m_pComboBox;
			OpenViBE::CIdentifier m_oTypeIdentifier;
			bool p;

			std::map < OpenViBE::CString, uint64_t > m_mEntriesIndex;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			bool m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CEnumerationSettingView_H__
