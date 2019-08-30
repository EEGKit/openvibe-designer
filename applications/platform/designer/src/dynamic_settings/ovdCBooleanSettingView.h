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
			CBooleanSettingView(OpenViBE::Kernel::IBox& box, const uint32_t index, OpenViBE::CString& rBuilderName);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void toggleButtonClick();
			void onChange();


		private:
			GtkToggleButton* m_toggle = nullptr;
			GtkEntry* m_entry         = nullptr;
			bool m_onValueSetting     = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
