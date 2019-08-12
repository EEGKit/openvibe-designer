#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CBoxProxy
	{
	public:

		CBoxProxy(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IScenario& rScenario, const OpenViBE::CIdentifier& boxIdentifier);
		virtual ~CBoxProxy() { if (!m_bApplied) { this->apply(); } }

		operator OpenViBE::Kernel::IBox* () const { return m_pBox; }
		operator const OpenViBE::Kernel::IBox* () const { return m_pConstBox; }

		int getWidth(GtkWidget* widget) const;
		int getHeight(GtkWidget* widget) const;

		int getXCenter() const { return m_centerX; }
		int getYCenter() const { return m_centerY; }
		void setCenter(int centerX, int centerY);

		void setBoxAlgorithmDescriptorOverride(const OpenViBE::Plugins::IBoxAlgorithmDesc* pBoxAlgorithmDescriptor);

		void apply();

		virtual const char* getLabel() const;
		virtual const char* getStatusLabel() const;

		bool isBoxAlgorithmPluginPresent() const { return m_IsBoxAlgorithmPresent; }
		bool isUpToDate() const { return !m_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated); }
		bool hasPendingDeprecatedInterfacors() const { return m_pBox->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors); }
		bool isDeprecated() const { return m_IsDeprecated; }
		bool isUnstable() const { return false; }
		bool isDisabled() const;
		bool isMetabox() const { return m_pConstBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox; }

	protected:

		virtual void updateSize(GtkWidget* widget, const char* sLabel, const char* sStatus, int* pXSize, int* pYSize) const;

		const OpenViBE::Kernel::IKernelContext& m_kernelContext;
		const OpenViBE::Plugins::IBoxAlgorithmDesc* m_pBoxAlgorithmDescriptorOverride = nullptr;
		const OpenViBE::Kernel::IBox* m_pConstBox = nullptr;
		OpenViBE::Kernel::IBox* m_pBox = nullptr;
		bool m_bApplied = false;
		bool m_bShowOriginalNameWhenModified = false;
		int m_centerX = 0;
		int m_centerY = 0;
		mutable std::string m_sLabel;
		mutable std::string m_sStatus;
		bool m_IsBoxAlgorithmPresent = false;
		bool m_IsDeprecated = false;
	};
};