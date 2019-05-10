#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CRenameDialog
	{
	public:

		CRenameDialog(const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::CString& rInitialName, const OpenViBE::CString& rDefaultName, const char* sGUIFilename);
		virtual ~CRenameDialog();

		bool run();
		OpenViBE::CString getResult() { return m_sResult; }

	protected:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::CString m_sInitialName;
		OpenViBE::CString m_sDefaultName;
		OpenViBE::CString m_sResult;
		OpenViBE::CString m_sGUIFilename;

	private:

		CRenameDialog();
	};
};
