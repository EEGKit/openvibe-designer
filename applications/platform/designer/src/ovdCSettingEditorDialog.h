#pragma once

#include "ovdCSettingCollectionHelper.h"

#include <string>
#include <map>

namespace OpenViBEDesigner
{
	class CSettingEditorDialog
	{
	public:

		CSettingEditorDialog(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, uint32_t settingIndex, const char* title, const char* guiFilename, const char* guiSettingsFilename);
		virtual ~CSettingEditorDialog();
		virtual bool run();

		virtual void typeChangedCB();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IBox& m_box;
		CSettingCollectionHelper m_oHelper;
		uint32_t m_settingIdx = 0;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;
		std::string m_sTitle;
		GtkWidget* m_table = nullptr;
		GtkWidget* m_type = nullptr;
		GtkWidget* m_defaultValue = nullptr;
		std::map<std::string, OpenViBE::CIdentifier> m_vSettingTypes;
	};
};




