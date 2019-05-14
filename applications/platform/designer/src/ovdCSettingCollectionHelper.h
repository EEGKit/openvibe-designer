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

		OpenViBE::CString getValue(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* pWidget);
		static OpenViBE::CString getValueBoolean(GtkWidget* pWidget);
		static OpenViBE::CString getValueInteger(GtkWidget* pWidget);
		static OpenViBE::CString getValueFloat(GtkWidget* pWidget);
		static OpenViBE::CString getValueString(GtkWidget* pWidget);
		static OpenViBE::CString getValueFilename(GtkWidget* pWidget);
		static OpenViBE::CString getValueFoldername(GtkWidget* pWidget);
		static OpenViBE::CString getValueScript(GtkWidget* pWidget);
		static OpenViBE::CString getValueColor(GtkWidget* pWidget);
		static OpenViBE::CString getValueColorGradient(GtkWidget* pWidget);
		static OpenViBE::CString getValueEnumeration(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* pWidget);
		static OpenViBE::CString getValueBitMask(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* pWidget);

		void setValue(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueBoolean(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueInteger(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueFloat(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		static void setValueString(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueFilename(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueFoldername(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueScript(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueColor(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueColorGradient(GtkWidget* pWidget, const OpenViBE::CString& rValue);
		void setValueEnumeration(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* pWidget, const OpenViBE::CString& rValue) const;
		void setValueBitMask(const OpenViBE::CIdentifier& rTypeIdentifier, GtkWidget* pWidget, const OpenViBE::CString& rValue);

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::CString m_sGUIFilename;

	private:

		CSettingCollectionHelper();
	};
};
