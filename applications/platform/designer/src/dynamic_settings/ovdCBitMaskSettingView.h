#ifndef __OpenViBE_Designer_Setting_CBitMaskSettingView_H__
#define __OpenViBE_Designer_Setting_CBitMaskSettingView_H__

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
			CBitMaskSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t index, OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::CIdentifier &rTypeIdentifier);

			void getValue(OpenViBE::CString &rValue) const override;
			void setValue(const OpenViBE::CString &rValue) override;

			void onChange();

		private:
			OpenViBE::CIdentifier m_oTypeIdentifier;
			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;

			std::vector < ::GtkToggleButton *> m_vToggleButton;
			bool m_bOnValueSetting = false;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CBitMaskSettingView_H__
