#pragma once

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
			CEnumerationSettingView(OpenViBE::Kernel::IBox& rBox, const uint32_t index, OpenViBE::CString& rBuilderName,
									const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::CIdentifier& rTypeIdentifier);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void onChange();


		private:
			GtkComboBox* m_comboBox                 = nullptr;
			OpenViBE::CIdentifier m_oTypeIdentifier = OV_UndefinedIdentifier;
			bool p                                  = false;

			std::map<OpenViBE::CString, uint64_t> m_entriesIndex;

			const OpenViBE::Kernel::IKernelContext& m_kernelContext;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
