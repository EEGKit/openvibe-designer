#pragma once

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include "topographicMap2DDisplay/ovpCTopographicMapDatabase.h"
#include "topographicMap2DDisplay/ovpCTopographicMap2DView.h"

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		class CBoxAlgorithmTopographicMap2DDisplay : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			CBoxAlgorithmTopographicMap2DDisplay();

			void release() override { delete this; }

			uint64_t getClockFrequency() override;
			bool initialize() override;
			bool uninitialize() override;
			bool processInput(const uint32_t ui32InputIndex) override;
			bool processClock(OpenViBE::Kernel::IMessageClock& rMessageClock) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_TopographicMap2DDisplay)

		protected:
			OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmTopographicMap2DDisplay>* m_pDecoder = nullptr;
			bool m_bFirstBufferReceived                                                               = false;

			OpenViBE::Kernel::IAlgorithmProxy* m_pSphericalSplineInterpolation = nullptr;
			CTopographicMapDatabase* m_pTopographicMapDatabase                 = nullptr;
			CSignalDisplayDrawable* m_pTopographicMap2DView                    = nullptr; //main object used for the display (contains all the GUI code)
		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext = nullptr;
		};

		class CBoxAlgorithmTopographicMap2DDisplayDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("2D topographic map"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Vincent Delannoy"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("INRIA/IRISA"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("This box demonstrates how to perform spherical spline interpolation"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString(""); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Visualization/Topography"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("2.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString(GTK_STOCK_EXECUTE); }
			OpenViBE::CString getSoftwareComponent() const override { return OpenViBE::CString("openvibe-designer"); }
			OpenViBE::CString getAddedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }
			OpenViBE::CString getUpdatedSoftwareVersion() const override { return OpenViBE::CString("0.0.0"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_TopographicMap2DDisplay; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmTopographicMap2DDisplay(); }

			bool hasFunctionality(const OpenViBE::CIdentifier functionality) const override { return functionality == OVD_Functionality_Visualization; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const override
			{
				rPrototype.addSetting("Interpolation type", OVP_TypeId_SphericalLinearInterpolationType, "1");
				rPrototype.addSetting("Delay (in s)", OV_TypeId_Float, "0");
				rPrototype.addInput("Signal", OV_TypeId_StreamedMatrix);
				rPrototype.addInput("Channel localization", OV_TypeId_ChannelLocalisation);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_TopographicMap2DDisplayDesc)
		};
	} // namespace SimpleVisualization
} // namespace OpenViBEPlugins
