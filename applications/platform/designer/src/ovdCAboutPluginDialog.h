#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CAboutPluginDialog final
	{
	public:

		CAboutPluginDialog(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CIdentifier& pluginClassID, const char* guiFilename)
			: m_kernelCtx(ctx), m_pluginClassID(pluginClassID), m_guiFilename(guiFilename) { }

		CAboutPluginDialog(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::Plugins::IPluginObjectDesc* pod, const char* guiFilename)
			: m_kernelCtx(ctx), m_pluginClassID(OV_UndefinedIdentifier), m_guiFilename(guiFilename), m_pods(pod) { }

		~CAboutPluginDialog() = default;

		bool run();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::CIdentifier m_pluginClassID = OV_UndefinedIdentifier;
		OpenViBE::CString m_guiFilename;
		const OpenViBE::Plugins::IPluginObjectDesc* m_pods = nullptr;

		CAboutPluginDialog() = delete;
	};
}  // namespace OpenViBEDesigner
