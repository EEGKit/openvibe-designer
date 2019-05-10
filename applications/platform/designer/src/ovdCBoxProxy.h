#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CBoxProxy
	{
	public:

		CBoxProxy(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IScenario& rScenario, const OpenViBE::CIdentifier& rBoxIdentifier);
		virtual ~CBoxProxy();

		operator OpenViBE::Kernel::IBox*();
		operator const OpenViBE::Kernel::IBox*();

		int getWidth(GtkWidget* pWidget) const;
		int getHeight(GtkWidget* pWidget) const;

		int getXCenter() const;
		int getYCenter() const;

		void setCenter(int i32XCenter, int i32YCenter);

		void setBoxAlgorithmDescriptorOverride(const OpenViBE::Plugins::IBoxAlgorithmDesc* pBoxAlgorithmDescriptor);

		void apply();

		virtual const char* getLabel() const;
		virtual const char* getStatusLabel() const;

		bool isBoxAlgorithmPluginPresent() const;
		bool isUpToDate() const;
		bool hasPendingDeprecatedInterfacors() const;
		bool isDeprecated() const;
		bool isUnstable() const;
		bool isDisabled() const;
		bool isMetabox() const;

	protected:

		virtual void updateSize(GtkWidget* pWidget, const char* sLabel, const char* sStatus, int* pXSize, int* pYSize) const;

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		const OpenViBE::Plugins::IBoxAlgorithmDesc* m_pBoxAlgorithmDescriptorOverride = nullptr;
		const OpenViBE::Kernel::IBox* m_pConstBox = nullptr;
		OpenViBE::Kernel::IBox* m_pBox = nullptr;
		bool m_bApplied = false;
		bool m_bShowOriginalNameWhenModified = false;
		int m_iXCenter = 0;
		int m_iYCenter = 0;
		mutable std::string m_sLabel;
		mutable std::string m_sStatus;
		bool m_IsBoxAlgorithmPresent = false;
		bool m_IsDeprecated = false;
	};
};