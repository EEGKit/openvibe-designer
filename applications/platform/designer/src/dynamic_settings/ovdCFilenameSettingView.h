#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CFilenameSettingView : public CAbstractSettingView
		{
		public:
			CFilenameSettingView(OpenViBE::Kernel::IBox& rBox,
								 uint32_t ui32Index,
								 OpenViBE::CString &rBuilderName,
								 const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void browse();
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
