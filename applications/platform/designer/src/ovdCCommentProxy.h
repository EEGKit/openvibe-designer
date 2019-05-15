#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CCommentProxy
	{
	public:

		CCommentProxy(const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::Kernel::IComment& rComment);
		CCommentProxy(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IScenario& rScenario, const OpenViBE::CIdentifier& rCommentIdentifier);
		virtual ~CCommentProxy();

		operator OpenViBE::Kernel::IComment*() const { return m_pComment; }
		operator const OpenViBE::Kernel::IComment*() const { return m_pConstComment; }

		int getWidth(GtkWidget* widget) const;
		int getHeight(GtkWidget* widget) const;

		int getXCenter() const { return m_centerX; }
		int getYCenter() const { return m_centerY; }

		void setCenter(int centerX, int centerY);

		void apply();

		virtual const char* getLabel() const;

	protected:

		virtual void updateSize(GtkWidget* widget, const char* sText, int* pXSize, int* pYSize) const;

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		const OpenViBE::Kernel::IComment* m_pConstComment = nullptr;
		OpenViBE::Kernel::IComment* m_pComment = nullptr;
		bool m_bApplied = false;
		int m_centerX = 0;
		int m_centerY = 0;
		mutable std::string m_sLabel;
	};
};

