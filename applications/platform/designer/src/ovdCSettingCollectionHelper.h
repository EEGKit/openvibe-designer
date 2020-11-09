#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBE {
namespace Designer {
class CSettingCollectionHelper final
{
public:

	CSettingCollectionHelper(const Kernel::IKernelContext& ctx, const char* guiFilename) : m_KernelCtx(ctx), m_GUIFilename(guiFilename) { }
	~CSettingCollectionHelper() = default;

	CString getSettingWidgetName(const CIdentifier& typeID) const;
	CString getSettingEntryWidgetName(const CIdentifier& typeID) const;

	CString getValue(const CIdentifier& typeID, GtkWidget* widget) const;
	static CString getValueBoolean(GtkWidget* widget);
	static CString getValueInteger(GtkWidget* widget);
	static CString getValueFloat(GtkWidget* widget);
	static CString getValueString(GtkWidget* widget);
	static CString getValueFilename(GtkWidget* widget);
	static CString getValueFoldername(GtkWidget* widget);
	static CString getValueScript(GtkWidget* widget);
	static CString getValueColor(GtkWidget* widget);
	static CString getValueColorGradient(GtkWidget* widget);
	static CString getValueEnumeration(const CIdentifier& typeID, GtkWidget* widget);
	static CString getValueBitMask(const CIdentifier& typeID, GtkWidget* widget);

	void setValue(const CIdentifier& typeID, GtkWidget* widget, const CString& value);
	void setValueBoolean(GtkWidget* widget, const CString& value);
	void setValueInteger(GtkWidget* widget, const CString& value);
	void setValueFloat(GtkWidget* widget, const CString& value);
	static void setValueString(GtkWidget* widget, const CString& value);
	void setValueFilename(GtkWidget* widget, const CString& value);
	void setValueFoldername(GtkWidget* widget, const CString& value);
	void setValueScript(GtkWidget* widget, const CString& value);
	void setValueColor(GtkWidget* widget, const CString& value);
	void setValueColorGradient(GtkWidget* widget, const CString& value);
	void setValueEnumeration(const CIdentifier& typeID, GtkWidget* widget, const CString& value) const;
	void setValueBitMask(const CIdentifier& typeID, GtkWidget* widget, const CString& value) const;

	const Kernel::IKernelContext& m_KernelCtx;
	CString m_GUIFilename;

private:
	CSettingCollectionHelper() = delete;
};
}  // namespace Designer
}  // namespace OpenViBE
