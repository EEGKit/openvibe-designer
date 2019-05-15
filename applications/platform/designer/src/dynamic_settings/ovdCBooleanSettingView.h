#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CBooleanSettingView : public CAbstractSettingView
		{
		public:
			CBooleanSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t index, OpenViBE::CString &rBuilderName);

			void getValue(OpenViBE::CString& rValue) const override;
			void setValue(const OpenViBE::CString& rValue) override;

			void toggleButtonClick();
			void onChange();


		private:
			::GtkToggleButton* m_pToggle;
			::GtkEntry* m_pEntry;
			bool m_bOnValueSetting;
		};
	}

}

