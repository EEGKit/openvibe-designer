#pragma once

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>
#include <gtk/gtk.h>
#include <map>
#include <string>


namespace OpenViBE
{
	namespace Plugins
	{
		namespace SimpleVisualization
		{
			class CBoxAlgorithmMatrixDisplay final : public Toolkit::TBoxAlgorithm<IBoxAlgorithm>
			{
			public:

				void release() override { delete this; }

				bool initialize() override;
				bool uninitialize() override;
				bool processInput(const size_t index) override;
				bool process() override;

				_IsDerivedFromClass_Final_(Toolkit::TBoxAlgorithm<IBoxAlgorithm>, OVP_ClassId_BoxAlgorithm_MatrixDisplay)

			protected:

				// we need an algorithm to decode the EBML stream (memory buffer) into a Streamed Matrix

				// for the TARGET
				Toolkit::TStreamedMatrixDecoder<CBoxAlgorithmMatrixDisplay> m_decoder;
				Kernel::IAlgorithmProxy* iMatrix = nullptr;
				Kernel::TParameterHandler<const IMemoryBuffer*> ip_buffer;
				Kernel::TParameterHandler<IMatrix*> op_matrix;

				// Outputs: visualization in a gtk window
				GtkBuilder* m_mainWidgetInterface    = nullptr;
				GtkBuilder* m_toolbarWidgetInterface = nullptr;
				GtkWidget* m_mainWidget              = nullptr;
				GtkWidget* m_toolbarWidget           = nullptr;

				std::vector<std::pair<GtkWidget*, GdkColor>> m_eventBoxCache;
				std::vector<std::pair<GtkLabel*, std::string>> m_labelCache;
				std::vector<std::pair<GtkLabel*, std::string>> m_rowLabelCache;
				std::vector<std::pair<GtkLabel*, std::string>> m_columnLabelCache;

				CMatrix m_interpolatedColorGardient;
				CMatrix m_colorGradient;
				size_t m_gradientSteps = 0;
				double m_max           = 0;
				double m_min           = 0;

				bool m_symetricMinMax = false;
				bool m_realTimeMinMax = false;

				VisualizationToolkit::IVisualizationContext* m_visualizationCtx{};

			public:

				bool m_ShowValues = false;
				bool m_ShowColors = false;

				bool resetColors();
			};

			class CBoxAlgorithmMatrixDisplayDesc final : public IBoxAlgorithmDesc
			{
			public:

				void release() override { }

				CString getName() const override { return CString("Matrix Display"); }
				CString getAuthorName() const override { return CString("Laurent Bonnet"); }
				CString getAuthorCompanyName() const override { return CString("INRIA/IRISA"); }
				CString getShortDescription() const override { return CString("Display a streamed matrix"); }

				CString getDetailedDescription() const override
				{
					return CString("The streamed matrix can be visualized using a table of values and/or a color gradient.");
				}

				CString getCategory() const override { return CString("Visualization/Basic"); }
				CString getVersion() const override { return CString("1.0"); }
				CString getStockItemName() const override { return CString("gtk-select-color"); }
				CString getSoftwareComponent() const override { return CString("openvibe-designer"); }
				CString getAddedSoftwareVersion() const override { return CString("0.0.0"); }
				CString getUpdatedSoftwareVersion() const override { return CString("0.0.0"); }

				CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_MatrixDisplay; }
				IPluginObject* create() override { return new CBoxAlgorithmMatrixDisplay; }

				bool hasFunctionality(const EPluginFunctionality functionality) const override { return functionality == EPluginFunctionality::Visualization; }

				bool getBoxPrototype(Kernel::IBoxProto& prototype) const override
				{
					prototype.addSetting("Color gradient", OV_TypeId_ColorGradient, "0:2,36,58; 50:100,100,100; 100:83,17,20");
					prototype.addSetting("Steps", OV_TypeId_Integer, "100");
					prototype.addSetting("Symetric min/max", OV_TypeId_Boolean, "false");
					prototype.addSetting("Real time min/max", OV_TypeId_Boolean, "false");
					prototype.addInput("Matrix", OV_TypeId_StreamedMatrix);
					// prototype.addFlag   (Kernel::BoxFlag_IsUnstable);

					return true;
				}

				_IsDerivedFromClass_Final_(IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MatrixDisplayDesc)
			};
		} // namespace SimpleVisualization
	}  // namespace Plugins
}  // namespace OpenViBE
