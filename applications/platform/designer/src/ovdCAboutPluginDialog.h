#pragma once

#include "ovd_base.h"

namespace OpenViBE
{
	namespace Designer
	{
		class CAboutPluginDialog final
		{
		public:

			CAboutPluginDialog(const Kernel::IKernelContext& ctx, const CIdentifier& pluginClassID, const char* guiFilename)
				: m_kernelCtx(ctx), m_pluginClassID(pluginClassID), m_guiFilename(guiFilename) { }

			CAboutPluginDialog(const Kernel::IKernelContext& ctx, const Plugins::IPluginObjectDesc* pod, const char* guiFilename)
				: m_kernelCtx(ctx), m_pluginClassID(OV_UndefinedIdentifier), m_guiFilename(guiFilename), m_pods(pod) { }

			~CAboutPluginDialog() = default;

			bool run();

		protected:

			const Kernel::IKernelContext& m_kernelCtx;
			CIdentifier m_pluginClassID = OV_UndefinedIdentifier;
			CString m_guiFilename;
			const Plugins::IPluginObjectDesc* m_pods = nullptr;

			CAboutPluginDialog() = delete;
		};
	}  // namespace Designer
}  // namespace OpenViBE
