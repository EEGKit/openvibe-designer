#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CAboutPluginDialog
	{
	public:

		CAboutPluginDialog(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CIdentifier& rPluginClassIdentifier, const char* sGUIFilename);
		CAboutPluginDialog(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::Plugins::IPluginObjectDesc* pPluginObjectDesc, const char* sGUIFilename);
		virtual ~CAboutPluginDialog();

		bool run();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::CIdentifier m_oPluginClassID = OV_UndefinedIdentifier;
		OpenViBE::CString m_sGUIFilename;

	private:
		const OpenViBE::Plugins::IPluginObjectDesc* m_pPluginObjectDescriptor = nullptr;

		CAboutPluginDialog();
	};
};
