#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <string>
#include <map>
#include <vector>

namespace OpenViBEDesigner
{
	namespace Setting
	{
		typedef struct
		{
			double fPercent;
			GdkColor oColor;
			GtkColorButton* pColorButton;
			GtkSpinButton* pSpinButton;
		} SColorGradientDataNode;


		class CColorGradientSettingView : public CAbstractSettingView
		{
		public:
			CColorGradientSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t index, OpenViBE::CString& rBuilderName,
									  const OpenViBE::Kernel::IKernelContext& rKernelContext);

			void getValue(OpenViBE::CString& rValue) const override;
			void setValue(const OpenViBE::CString& rValue) override;

			void configurePressed();

			void initializeGradient();
			void refreshColorGradient();
			void addColor();
			void removeColor();

			void spinChange(GtkSpinButton* button);
			void colorChange(GtkColorButton* button);

			void onChange();


		private:
			GtkEntry* m_pEntry = nullptr;
			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			OpenViBE::CString m_sBuilderName;

			GtkWidget* pDialog = nullptr;
			GtkWidget* pContainer = nullptr;
			GtkWidget* pDrawingArea = nullptr;
			std::vector<SColorGradientDataNode> vColorGradient;
			std::map<GtkColorButton*, uint32_t> vColorButtonMap;
			std::map<GtkSpinButton*, uint32_t> vSpinButtonMap;

			bool m_bOnValueSetting = false;
		};
	}
}
