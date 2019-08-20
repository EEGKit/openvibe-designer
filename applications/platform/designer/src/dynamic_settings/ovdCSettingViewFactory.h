#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CSettingViewFactory
		{
		public:
			CSettingViewFactory(const OpenViBE::CString& rBuilderName, const OpenViBE::Kernel::IKernelContext& rKernelContext);
			virtual ~CSettingViewFactory() = default;

			CAbstractSettingView* getSettingView(OpenViBE::Kernel::IBox& rBox, const uint32_t index);

		private:
			OpenViBE::CString m_builderName;
			const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
