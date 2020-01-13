#pragma once

#include "ovdCSettingCollectionHelper.h"

#include <string>
#include <map>

namespace OpenViBEDesigner
{
	class CSettingEditorDialog final
	{
	public:

		CSettingEditorDialog(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, size_t settingIndex, const char* title,
							 const char* guiFilename, const char* guiSettingsFilename)
			: m_kernelCtx(ctx), m_box(box), m_helper(ctx, guiFilename), m_settingIdx(settingIndex), m_guiFilename(guiFilename),
			  m_guiSettingsFilename(guiSettingsFilename), m_title(title) { }

		~CSettingEditorDialog() = default;

		bool run();
		void typeChangedCB();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IBox& m_box;
		CSettingCollectionHelper m_helper;
		size_t m_settingIdx = 0;
		OpenViBE::CString m_guiFilename;
		OpenViBE::CString m_guiSettingsFilename;
		std::string m_title;
		GtkWidget* m_table        = nullptr;
		GtkWidget* m_type         = nullptr;
		GtkWidget* m_defaultValue = nullptr;
		std::map<std::string, OpenViBE::CIdentifier> m_settingTypes;
	};
}  // namespace OpenViBEDesigner
