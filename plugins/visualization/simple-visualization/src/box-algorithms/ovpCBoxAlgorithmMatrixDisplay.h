#pragma once

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>
#include <gtk/gtk.h>
#include <map>
#include <string>

#define OVP_ClassId_BoxAlgorithm_MatrixDisplay     OpenViBE::CIdentifier(0x54F0796D, 0x3EDE2CC0)
#define OVP_ClassId_BoxAlgorithm_MatrixDisplayDesc OpenViBE::CIdentifier(0x63AB4BA7, 0x022C1524)


namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		class CBoxAlgorithmMatrixDisplay : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;
			bool processInput(uint32_t ui32InputIndex) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MatrixDisplay);

		protected:

			// we need an algorithm to decode the EBML stream (memory buffer) into a Streamed Matrix

			// for the TARGET
			OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmMatrixDisplay> m_MatrixDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pMatrixDecoder{};
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMatrix*> op_pMatrix;

			// Outputs: visualization in a gtk window
			GtkBuilder* m_pMainWidgetInterface{};
			GtkBuilder* m_pToolbarWidgetInterface{};
			GtkWidget* m_pMainWidget{};
			GtkWidget* m_pToolbarWidget{};

			std::vector<std::pair<GtkWidget*, GdkColor>> m_vEventBoxCache;
			std::vector<std::pair<GtkLabel*, std::string>> m_vLabelCache;

			std::vector<std::pair<GtkLabel*, std::string>> m_vRowLabelCache;
			std::vector<std::pair<GtkLabel*, std::string>> m_vColumnLabelCache;

			OpenViBE::CMatrix m_MatrixInterpolatedColorGardient;
			OpenViBE::CMatrix m_MatrixColorGradient;
			uint32_t m_GradientSteps{};
			double m_f64MaxValue{};
			double m_f64MinValue{};

			bool m_bSymetricMinMax{};
			bool m_bRealTimeMinMax{};

		public:

			bool m_bShowValues{};
			bool m_bShowColors{};

			virtual bool resetColors();
		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext{};
		};

		class CBoxAlgorithmMatrixDisplayDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Matrix Display"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Laurent Bonnet"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("INRIA/IRISA"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Display a streamed matrix"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("The streamed matrix can be visualized using a table of values and/or a color gradient."); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Visualization/Basic"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-select-color"); }
			OpenViBE::CString getSoftwareComponent() const override { return OpenViBE::CString("openvibe-designer"); }
			OpenViBE::CString getAddedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }
			OpenViBE::CString getUpdatedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_MatrixDisplay; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmMatrixDisplay; }

			bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const override { return functionalityIdentifier == OVD_Functionality_Visualization; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const override
			{
				rBoxAlgorithmPrototype.addSetting("Color gradient", OV_TypeId_ColorGradient, "0:2,36,58; 50:100,100,100; 100:83,17,20");
				rBoxAlgorithmPrototype.addSetting("Steps", OV_TypeId_Integer, "100");
				rBoxAlgorithmPrototype.addSetting("Symetric min/max", OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addSetting("Real time min/max", OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addInput("Matrix", OV_TypeId_StreamedMatrix);
				// rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_IsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MatrixDisplayDesc);
		};
	} // namespace SimpleVisualization;
}  // namespace OpenViBEPlugins;
