#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <vector>

namespace OpenViBE {
namespace Designer {
namespace Setting {
class CBitMaskSettingView final : public CAbstractSettingView
{
public:
	CBitMaskSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx, const CIdentifier& typeID);

	void getValue(CString& value) const override;
	void setValue(const CString& value) override;

	void onChange();

private:
	CIdentifier m_typeID = OV_UndefinedIdentifier;
	const Kernel::IKernelContext& m_kernelCtx;

	std::vector<GtkToggleButton*> m_toggleButton;
	bool m_onValueSetting = false;
};
}  // namespace Setting
}  // namespace Designer
}  // namespace OpenViBE
