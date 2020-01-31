#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBE
{
	namespace Designer
	{
		class CBoxProxy final
		{
		public:

			CBoxProxy(const Kernel::IKernelContext& ctx, Kernel::IScenario& scenario, const CIdentifier& boxID);
			~CBoxProxy() { if (!m_applied) { this->apply(); } }

			operator Kernel::IBox*() const { return m_box; }
			operator const Kernel::IBox*() const { return m_constBox; }

			int getWidth(GtkWidget* widget) const;
			int getHeight(GtkWidget* widget) const;

			int getXCenter() const { return m_centerX; }
			int getYCenter() const { return m_centerY; }
			void setCenter(int x, int y);

			void setBoxAlgorithmDescriptorOverride(const Plugins::IBoxAlgorithmDesc* pBoxAlgorithmDescriptor);

			void apply();

			const char* getLabel() const;
			const char* getStatusLabel() const;

			bool isBoxAlgorithmPluginPresent() const { return m_isBoxAlgorithmPresent; }
			bool isUpToDate() const { return !m_box->hasAttribute(OV_AttributeId_Box_ToBeUpdated); }
			bool hasPendingDeprecatedInterfacors() const { return m_box->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors); }
			bool isDeprecated() const { return m_isDeprecated; }
			static bool isUnstable() { return false; }
			bool isDisabled() const;
			bool isMetabox() const { return m_constBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox; }

		protected:

			void updateSize(GtkWidget* widget, const char* label, const char* status, int* xSize, int* ySize) const;

			const Kernel::IKernelContext& m_kernelCtx;
			const Plugins::IBoxAlgorithmDesc* m_boxAlgorithmDescOverride = nullptr;
			const Kernel::IBox* m_constBox                               = nullptr;
			Kernel::IBox* m_box                                          = nullptr;
			bool m_applied                                               = false;
			bool m_showOriginalNameWhenModified                          = false;
			int m_centerX                                                = 0;
			int m_centerY                                                = 0;
			mutable std::string m_label;
			mutable std::string m_status;
			bool m_isBoxAlgorithmPresent = false;
			bool m_isDeprecated          = false;
		};
	}  // namespace Designer
}  // namespace OpenViBE
