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

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::CIdentifier m_oPluginClassIdentifier = OV_UndefinedIdentifier;
		OpenViBE::CString m_sGUIFilename;

	private:
		const OpenViBE::Plugins::IPluginObjectDesc* m_pPluginObjectDescriptor = nullptr;

		CAboutPluginDialog();
	};
};
