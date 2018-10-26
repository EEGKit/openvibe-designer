#ifndef __OpenViBEDesigner_CBoxProxy_H__
#define __OpenViBEDesigner_CBoxProxy_H__

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CBoxProxy
	{
	public:

		CBoxProxy(
			const OpenViBE::Kernel::IKernelContext& rKernelContext,
			OpenViBE::Kernel::IScenario& rScenario,
			const OpenViBE::CIdentifier& rBoxIdentifier);
		virtual ~CBoxProxy(void);

		operator OpenViBE::Kernel::IBox* (void);
		operator const OpenViBE::Kernel::IBox* (void);

		OpenViBE::int32 getWidth(
			::GtkWidget* pWidget) const;
		OpenViBE::int32 getHeight(
			::GtkWidget* pWidget) const;

		OpenViBE::int32 getXCenter(void) const;
		OpenViBE::int32 getYCenter(void) const;

		void setCenter(
			OpenViBE::int32 i32XCenter,
			OpenViBE::int32 i32YCenter);

		void setBoxAlgorithmDescriptorOverride(const OpenViBE::Plugins::IBoxAlgorithmDesc* pBoxAlgorithmDescriptor);

		void apply(void);

		virtual const char* getLabel(void) const;
		virtual const char* getStatusLabel(void) const;

		bool isBoxAlgorithmPluginPresent(void) const;
		bool isUpToDate(void) const;
		bool hasPendingDeprecatedInterfacors(void) const;
		bool isDeprecated(void) const;
		bool isUnstable(void) const;
		bool isDisabled(void) const;
		bool isMetabox(void) const;

	protected:

		virtual void updateSize(
			::GtkWidget* pWidget,
			const char* sLabel,
			const char* sStatus,
			int* pXSize,
			int* pYSize) const;

	protected:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		const OpenViBE::Plugins::IBoxAlgorithmDesc* m_pBoxAlgorithmDescriptorOverride;
		const OpenViBE::Kernel::IBox* m_pConstBox;
		OpenViBE::Kernel::IBox* m_pBox;
		bool m_bApplied;
		bool m_bShowOriginalNameWhenModified;
		int m_iXCenter;
		int m_iYCenter;
		mutable std::string m_sLabel;
		mutable std::string m_sStatus;
		bool m_IsBoxAlgorithmPresent;
		bool m_IsDeprecated;
	};
};

#endif // __OpenViBEDesigner_CBoxProxy_H__
