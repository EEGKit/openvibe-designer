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

		operator OpenViBE::Kernel::IComment*();
		operator const OpenViBE::Kernel::IComment*();

		int getWidth(GtkWidget* pWidget) const;
		int getHeight(GtkWidget* pWidget) const;

		int getXCenter() const;
		int getYCenter() const;

		void setCenter(int i32XCenter, int i32YCenter);

		void apply();

		virtual const char* getLabel() const;

	protected:

		virtual void updateSize(GtkWidget* pWidget, const char* sText, int* pXSize, int* pYSize) const;

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		const OpenViBE::Kernel::IComment* m_pConstComment = nullptr;
		OpenViBE::Kernel::IComment* m_pComment = nullptr;
		bool m_bApplied = false;
		int m_iXCenter = 0;
		int m_iYCenter = 0;
		mutable std::string m_sLabel;
	};
};

