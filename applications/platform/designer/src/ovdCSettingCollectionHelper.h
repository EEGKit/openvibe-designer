#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CSettingCollectionHelper
	{
	public:

		CSettingCollectionHelper(const OpenViBE::Kernel::IKernelContext& rKernelContext, const char* sGUIFilename);
		virtual ~CSettingCollectionHelper();

		OpenViBE::CString getSettingWidgetName(const OpenViBE::CIdentifier& rTypeIdentifier) const;
		OpenViBE::CString getSettingEntryWidgetName(const OpenViBE::CIdentifier& rTypeIdentifier) const;

		OpenViBE::CString getValue(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* widget) const;
		static OpenViBE::CString getValueBoolean(GtkWidget* widget);
		static OpenViBE::CString getValueInteger(GtkWidget* widget);
		static OpenViBE::CString getValueFloat(GtkWidget* widget);
		static OpenViBE::CString getValueString(GtkWidget* widget);
		static OpenViBE::CString getValueFilename(GtkWidget* widget);
		static OpenViBE::CString getValueFoldername(GtkWidget* widget);
		static OpenViBE::CString getValueScript(GtkWidget* widget);
		static OpenViBE::CString getValueColor(GtkWidget* widget);
		static OpenViBE::CString getValueColorGradient(GtkWidget* widget);
		static OpenViBE::CString getValueEnumeration(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* widget);
		static OpenViBE::CString getValueBitMask(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* widget);

		void setValue(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueBoolean(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueInteger(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueFloat(GtkWidget* widget, const OpenViBE::CString& rValue);
		static void setValueString(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueFilename(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueFoldername(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueScript(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueColor(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueColorGradient(GtkWidget* widget, const OpenViBE::CString& rValue);
		void setValueEnumeration(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* widget, const OpenViBE::CString& rValue) const;
		void setValueBitMask(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* widget, const OpenViBE::CString& rValue) const;

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::CString m_sGUIFilename;

	private:

		CSettingCollectionHelper();
	};
} // namespace OpenViBEDesigner
