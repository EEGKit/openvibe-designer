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
			CStringSettingView(OpenViBE::Kernel::IBox& rBox, const uint32_t index, OpenViBE::CString& rBuilderName);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void onChange();

		private:
			GtkEntry* m_entry     = nullptr;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
