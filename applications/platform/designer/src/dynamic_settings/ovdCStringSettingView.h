#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBE {
namespace Designer {
namespace Setting {
class CStringSettingView final : public CAbstractSettingView
{
public:
	CStringSettingView(Kernel::IBox& box, const size_t index, CString& builderName);

	void getValue(CString& value) const override;
	void setValue(const CString& value) override;

	void onChange();

private:
	GtkEntry* m_entry     = nullptr;
	bool m_onValueSetting = false;
};
}  // namespace Setting
}  // namespace Designer
}  // namespace OpenViBE
