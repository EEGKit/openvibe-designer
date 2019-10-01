#pragma once

#include "ovdCSettingCollectionHelper.h"

#include <string>
#include <map>

namespace OpenViBEDesigner
{
	class CSettingEditorDialog
	{
	public:

		CSettingEditorDialog(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, uint32_t settingIndex, const char* sTitle, const char* sGUIFilename, const char* sGUISettingsFilename);
		virtual ~CSettingEditorDialog();
		virtual bool run();

		virtual void typeChangedCB();

	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::Kernel::IBox& m_box;
		CSettingCollectionHelper m_oHelper;
		uint32_t m_ui32SettingIndex = 0;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;
		std::string m_sTitle;
		GtkWidget* m_pTable = nullptr;
		GtkWidget* m_pType = nullptr;
		GtkWidget* m_pDefaultValue = nullptr;
		std::map<std::string, OpenViBE::CIdentifier> m_vSettingTypes;
	};
};
