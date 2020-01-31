#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBE
{
	namespace Designer
	{
		namespace Setting
		{
			class CSettingViewFactory final
			{
			public:
				CSettingViewFactory(const CString& builderName, const Kernel::IKernelContext& ctx)
					: m_builderName(builderName), m_kernelCtx(ctx) { }

				~CSettingViewFactory() = default;

				CAbstractSettingView* getSettingView(Kernel::IBox& box, const size_t index);

			private:
				CString m_builderName;
				const Kernel::IKernelContext& m_kernelCtx;
			};
		} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
