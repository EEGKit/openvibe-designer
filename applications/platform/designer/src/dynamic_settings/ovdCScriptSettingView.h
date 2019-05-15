#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CScriptSettingView : public CAbstractSettingView
		{
		public:
			CScriptSettingView(OpenViBE::Kernel::IBox& rBox,
								 uint32_t index,
								 OpenViBE::CString &rBuilderName,
								 const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString &rValue) const override;
			void setValue(const OpenViBE::CString &rValue) override;

			void browse() const;
			void edit() const;

			void onChange();
#if defined TARGET_OS_Windows
			void onFocusLost();
#endif

		private:
			::GtkEntry* m_pEntry;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			bool m_bOnValueSetting;
		};
	}
}
