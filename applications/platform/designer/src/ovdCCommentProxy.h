#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CCommentProxy final
	{
	public:

		CCommentProxy(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::Kernel::IComment& comment);
		CCommentProxy(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IScenario& scenario, const OpenViBE::CIdentifier& commentID);
		~CCommentProxy() { if (!m_applied) { this->apply(); } }

		operator OpenViBE::Kernel::IComment*() const { return m_comment; }
		operator const OpenViBE::Kernel::IComment*() const { return m_constComment; }

		int getWidth(GtkWidget* widget) const;
		int getHeight(GtkWidget* widget) const;

		int getXCenter() const { return m_centerX; }
		int getYCenter() const { return m_centerY; }

		void setCenter(int centerX, int centerY);

		void apply();

		const char* getLabel() const;

	protected:

		static void updateSize(GtkWidget* widget, const char* text, int* xSize, int* ySize);

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		const OpenViBE::Kernel::IComment* m_constComment = nullptr;
		OpenViBE::Kernel::IComment* m_comment            = nullptr;
		bool m_applied                                   = false;
		int m_centerX                                    = 0;
		int m_centerY                                    = 0;
		mutable std::string m_label;
	};
} // namespace OpenViBEDesigner
