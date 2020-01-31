#pragma once

#include "ovd_base.h"

namespace OpenViBE
{
	namespace Designer
	{
		class CCommentEditorDialog final
		{
		public:

			CCommentEditorDialog(const Kernel::IKernelContext& ctx, Kernel::IComment& comment, const char* guiFilename)
				: m_kernelCtx(ctx), m_comment(comment), m_guiFilename(guiFilename) { }

			~CCommentEditorDialog() = default;

			bool run();

			// Callback for text formatting
			void applyTagCB(const char* in, const char* out) const;

			// help formatting pango
			void infoCB() const { gtk_widget_show(m_infoDialog); }
		protected:

			const Kernel::IKernelContext& m_kernelCtx;
			Kernel::IComment& m_comment;
			CString m_guiFilename;

			GtkBuilder* m_interface = nullptr;
			GtkWidget* m_dialog     = nullptr;
			GtkWidget* m_infoDialog = nullptr;
			GtkWidget* m_desc       = nullptr;
			GtkTextBuffer* m_buffer = nullptr;

			CCommentEditorDialog() = delete;
		};
	}  // namespace Designer
}  // namespace OpenViBE
