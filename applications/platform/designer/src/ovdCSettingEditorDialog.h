#pragma once

#include "ovdCSettingCollectionHelper.h"

#include <string>
#include <map>

namespace OpenViBE {
namespace Designer {
class CSettingEditorDialog final
{
public:

	CSettingEditorDialog(const Kernel::IKernelContext& ctx, Kernel::IBox& box, const size_t index, const char* title,
						 const char* guiFilename, const char* guiSettingsFilename)
		: m_kernelCtx(ctx), m_box(box), m_helper(ctx, guiFilename), m_settingIdx(index), m_guiFilename(guiFilename),
		  m_guiSettingsFilename(guiSettingsFilename), m_title(title) { }

	~CSettingEditorDialog() = default;

	bool run();
	void typeChangedCB();

protected:

	const Kernel::IKernelContext& m_kernelCtx;
	Kernel::IBox& m_box;
	CSettingCollectionHelper m_helper;
	size_t m_settingIdx = 0;
	CString m_guiFilename;
	CString m_guiSettingsFilename;
	std::string m_title;
	GtkWidget* m_table        = nullptr;
	GtkWidget* m_type         = nullptr;
	GtkWidget* m_defaultValue = nullptr;
	std::map<std::string, CIdentifier> m_settingTypes;
};
}  // namespace Designer
}  // namespace OpenViBE
