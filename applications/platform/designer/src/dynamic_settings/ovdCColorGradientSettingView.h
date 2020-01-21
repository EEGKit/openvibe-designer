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
			double percent;
			GdkColor color;
			GtkColorButton* colorButton;
			GtkSpinButton* spinButton;
		} color_gradient_data_node_t;


		class CColorGradientSettingView final : public CAbstractSettingView
		{
		public:
			CColorGradientSettingView(OpenViBE::Kernel::IBox& box, const size_t index, OpenViBE::CString& builderName,
									  const OpenViBE::Kernel::IKernelContext& ctx);

			void getValue(OpenViBE::CString& value) const override;
			void setValue(const OpenViBE::CString& value) override;

			void configurePressed();

			void initializeGradient();
			void refreshColorGradient();
			void addColor();
			void removeColor();

			void spinChange(GtkSpinButton* button);
			void colorChange(GtkColorButton* button);

			void onChange();


		private:
			GtkEntry* m_entry = nullptr;
			const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
			OpenViBE::CString m_builderName;

			GtkWidget* m_dialog      = nullptr;
			GtkWidget* m_container   = nullptr;
			GtkWidget* m_drawingArea = nullptr;
			std::vector<color_gradient_data_node_t> m_colorGradient;
			std::map<GtkColorButton*, size_t> m_colorButtons;
			std::map<GtkSpinButton*, size_t> m_spinButtons;

			bool m_onValueSetting = false;
		};
	} // namespace Setting
} // namespace OpenViBEDesigner
