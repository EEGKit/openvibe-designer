#pragma once

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <string>
#include <map>
#include <vector>

namespace OpenViBE
{
	namespace Designer
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
			CColorGradientSettingView(Kernel::IBox& box, const size_t index, CString& builderName, const Kernel::IKernelContext& ctx);

			void getValue(CString& value) const override;
			void setValue(const CString& value) override;

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
			const Kernel::IKernelContext& m_kernelCtx;
			CString m_builderName;

			GtkWidget* m_dialog      = nullptr;
			GtkWidget* m_container   = nullptr;
			GtkWidget* m_drawingArea = nullptr;
			std::vector<color_gradient_data_node_t> m_colorGradient;
			std::map<GtkColorButton*, size_t> m_colorButtons;
			std::map<GtkSpinButton*, size_t> m_spinButtons;

			bool m_onValueSetting = false;
		};
	} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
