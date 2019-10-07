#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CSettingCollectionHelper
	{
	public:

		CSettingCollectionHelper(const OpenViBE::Kernel::IKernelContext& ctx, const char* guiFilename);
		virtual ~CSettingCollectionHelper();

		OpenViBE::CString getSettingWidgetName(const OpenViBE::CIdentifier& typeID) const;
		OpenViBE::CString getSettingEntryWidgetName(const OpenViBE::CIdentifier& typeID) const;

		OpenViBE::CString getValue(const OpenViBE::CIdentifier& typeID, GtkWidget* widget) const;
		static OpenViBE::CString getValueBoolean(GtkWidget* widget);
		static OpenViBE::CString getValueInteger(GtkWidget* widget);
		static OpenViBE::CString getValueFloat(GtkWidget* widget);
		static OpenViBE::CString getValueString(GtkWidget* widget);
		static OpenViBE::CString getValueFilename(GtkWidget* widget);
		static OpenViBE::CString getValueFoldername(GtkWidget* widget);
		static OpenViBE::CString getValueScript(GtkWidget* widget);
		static OpenViBE::CString getValueColor(GtkWidget* widget);
		static OpenViBE::CString getValueColorGradient(GtkWidget* widget);
		static OpenViBE::CString getValueEnumeration(const OpenViBE::CIdentifier& typeID, GtkWidget* widget);
		static OpenViBE::CString getValueBitMask(const OpenViBE::CIdentifier& typeID, GtkWidget* widget);

		void setValue(const OpenViBE::CIdentifier& typeID, GtkWidget* widget, const OpenViBE::CString& value);
		void setValueBoolean(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueInteger(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueFloat(GtkWidget* widget, const OpenViBE::CString& value);
		static void setValueString(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueFilename(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueFoldername(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueScript(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueColor(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueColorGradient(GtkWidget* widget, const OpenViBE::CString& value);
		void setValueEnumeration(const OpenViBE::CIdentifier& typeID, GtkWidget* widget, const OpenViBE::CString& value) const;
		void setValueBitMask(const OpenViBE::CIdentifier& typeID, GtkWidget* widget, const OpenViBE::CString& value) const;

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::CString m_sGUIFilename;

	private:

		CSettingCollectionHelper() = delete;
	};
} // namespace OpenViBEDesigner
