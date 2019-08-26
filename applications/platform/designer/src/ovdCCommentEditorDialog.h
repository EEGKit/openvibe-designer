#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CCommentEditorDialog
	{
	public:

		CCommentEditorDialog(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IComment& rComment, const char* sGUIFilename);
		virtual ~CCommentEditorDialog() = default;

		bool run();

		// Callback for text formatting
		void applyTagCB(const char* sTagIn, const char* sTagOut) const;

		// help formatting pango
		void infoCB() const { gtk_widget_show(m_pInfoDialog); }
	protected:

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		OpenViBE::Kernel::IComment& m_rComment;
		OpenViBE::CString m_sGUIFilename;

	private:

		CCommentEditorDialog();

		GtkBuilder* m_pInterface{};
		GtkWidget* m_pDialog{};
		GtkWidget* m_pInfoDialog{};
		GtkWidget* m_pDescription{};
		GtkTextBuffer* m_pDescriptionBuffer{};
	};
} // namespace OpenViBEDesigner
