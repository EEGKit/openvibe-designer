#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <map>

namespace OpenViBE
{
	namespace Designer
	{
		namespace Setting
		{
			class CEnumerationSettingView final : public CAbstractSettingView
			{
			public:
				CEnumerationSettingView(Kernel::IBox& box, const size_t index, CString& builderName,
										const Kernel::IKernelContext& ctx, const CIdentifier& typeID);

				void getValue(CString& value) const override;
				void setValue(const CString& value) override;

				void onChange();


			private:
				GtkComboBox* m_comboBox = nullptr;
				CIdentifier m_typeID    = OV_UndefinedIdentifier;

				std::map<CString, size_t> m_entriesIdx;

				const Kernel::IKernelContext& m_kernelCtx;
				bool m_onValueSetting = false;
			};
		} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
