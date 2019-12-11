#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <map>

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CEnumerationSettingView final : public CAbstractSettingView
		{
		public:
			CEnumerationSettingView(OpenViBE::Kernel::IBox& box, const size_t index, OpenViBE::CString& builderName,
									const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CIdentifier& typeID);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void onChange();


		private:
			GtkComboBox* m_comboBox        = nullptr;
			OpenViBE::CIdentifier m_typeID = OV_UndefinedIdentifier;

			std::map<OpenViBE::CString, size_t> m_entriesIdx;

			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
			bool m_onValueSetting = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
