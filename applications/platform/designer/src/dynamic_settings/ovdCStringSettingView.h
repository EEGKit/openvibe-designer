#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CStringSettingView : public CAbstractSettingView
		{
		public:
			CStringSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index, OpenViBE::CString &rBuilderName);

			void getValue(OpenViBE::CString& rValue) const override;
			void setValue(const OpenViBE::CString& rValue) override;

			void onChange();

		private:
			::GtkEntry* m_pEntry = nullptr;
			bool m_bOnValueSetting = false;
		};
	}
}
