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
		class CBoxAlgorithmMatrixDisplay final : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;
			bool processInput(const uint32_t index) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MatrixDisplay)

		protected:

			// we need an algorithm to decode the EBML stream (memory buffer) into a Streamed Matrix

			// for the TARGET
			OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmMatrixDisplay> m_MatrixDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_matrixDecoder{};
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
			uint32_t m_GradientSteps = 0;
			double m_f64MaxValue     = 0;
			double m_f64MinValue     = 0;

			bool m_bSymetricMinMax = false;
			bool m_bRealTimeMinMax = false;

		public:

			bool m_bShowValues = false;
			bool m_bShowColors = false;

			virtual bool resetColors();
		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext{};
		};

		class CBoxAlgorithmMatrixDisplayDesc final : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Matrix Display"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Laurent Bonnet"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("INRIA/IRISA"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Display a streamed matrix"); }

			OpenViBE::CString getDetailedDescription() const override
			{
				return OpenViBE::CString("The streamed matrix can be visualized using a table of values and/or a color gradient.");
			}

			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Visualization/Basic"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-select-color"); }
			OpenViBE::CString getSoftwareComponent() const override { return OpenViBE::CString("openvibe-designer"); }
			OpenViBE::CString getAddedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }
			OpenViBE::CString getUpdatedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_MatrixDisplay; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmMatrixDisplay; }

			bool hasFunctionality(const OpenViBE::CIdentifier functionality) const override { return functionality == OVD_Functionality_Visualization; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& prototype) const override
			{
				prototype.addSetting("Color gradient", OV_TypeId_ColorGradient, "0:2,36,58; 50:100,100,100; 100:83,17,20");
				prototype.addSetting("Steps", OV_TypeId_Integer, "100");
				prototype.addSetting("Symetric min/max", OV_TypeId_Boolean, "false");
				prototype.addSetting("Real time min/max", OV_TypeId_Boolean, "false");
				prototype.addInput("Matrix", OV_TypeId_StreamedMatrix);
				// prototype.addFlag   (OpenViBE::Kernel::BoxFlag_IsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MatrixDisplayDesc)
		};
	} // namespace SimpleVisualization
} // namespace OpenViBEPlugins
