#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CSettingViewFactory final
		{
		public:
			CSettingViewFactory(const OpenViBE::CString& builderName, const OpenViBE::Kernel::IKernelContext& ctx)
				: m_builderName(builderName), m_kernelCtx(ctx) { }

			~CSettingViewFactory() = default;

			CAbstractSettingView* getSettingView(OpenViBE::Kernel::IBox& box, const size_t index);

		private:
			OpenViBE::CString m_builderName;
			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
