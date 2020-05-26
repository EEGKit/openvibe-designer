#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBE {
namespace Designer {
namespace Setting {

class CFloatSettingView final : public CAbstractSettingView
{
public:
	CFloatSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx);

	void getValue(CString& value) const override;
	void setValue(const CString& value) override;

	void adjustValue(double amount);
	void onChange();


private:
	GtkEntry* m_entry = nullptr;

	const Kernel::IKernelContext& m_kernelCtx;
	bool m_onValueSetting = false;
};

}  // namespace Setting
}  // namespace Designer
}  // namespace OpenViBE
