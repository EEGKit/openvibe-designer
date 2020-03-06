#pragma once

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include "topographicMap2DDisplay/ovpCTopographicMapDatabase.h"
#include "topographicMap2DDisplay/ovpCTopographicMap2DView.h"

namespace OpenViBE
{
	namespace Plugins
	{
		namespace SimpleVisualization
		{
			class CBoxAlgorithmTopographicMap2DDisplay final : public Toolkit::TBoxAlgorithm<IBoxAlgorithm>
			{
			public:
				CBoxAlgorithmTopographicMap2DDisplay() = default;

				void release() override { delete this; }

				uint64_t getClockFrequency() override { return uint64_t(1LL) << 37; }
				bool initialize() override;
				bool uninitialize() override;
				bool processInput(const size_t index) override;
				bool processClock(Kernel::IMessageClock& messageClock) override;
				bool process() override;

				_IsDerivedFromClass_Final_(Toolkit::TBoxAlgorithm<IBoxAlgorithm>, OVP_ClassId_TopographicMap2DDisplay)

			protected:
				Toolkit::TStreamedMatrixDecoder<CBoxAlgorithmTopographicMap2DDisplay> m_decoder;

				VisualizationToolkit::IVisualizationContext* m_visualizationCtx = nullptr;
				Kernel::IAlgorithmProxy* m_interpolation                        = nullptr;
				CTopographicMapDatabase* m_database                             = nullptr;
				CSignalDisplayDrawable* m_view                                  = nullptr; //main object used for the display (contains all the GUI code)
				bool m_hasFirstBuffer                                           = false;
			};

			class CBoxAlgorithmTopographicMap2DDisplayDesc final : public IBoxAlgorithmDesc
			{
			public:

				void release() override { }

				CString getName() const override { return CString("2D topographic map"); }
				CString getAuthorName() const override { return CString("Vincent Delannoy"); }
				CString getAuthorCompanyName() const override { return CString("INRIA/IRISA"); }

				CString getShortDescription() const override { return CString("This box demonstrates how to perform spherical spline interpolation"); }

				CString getDetailedDescription() const override { return CString(""); }
				CString getCategory() const override { return CString("Visualization/Topography"); }
				CString getVersion() const override { return CString("2.0"); }
				CString getStockItemName() const override { return CString(GTK_STOCK_EXECUTE); }
				CString getSoftwareComponent() const override { return CString("openvibe-designer"); }
				CString getAddedSoftwareVersion() const override { return CString("0.0.0"); }
				CString getUpdatedSoftwareVersion() const override { return CString("0.0.0"); }

				CIdentifier getCreatedClass() const override { return OVP_ClassId_TopographicMap2DDisplay; }
				IPluginObject* create() override { return new CBoxAlgorithmTopographicMap2DDisplay(); }

				bool hasFunctionality(const EPluginFunctionality functionality) const override { return functionality == EPluginFunctionality::Visualization; }

				bool getBoxPrototype(Kernel::IBoxProto& prototype) const override
				{
					prototype.addSetting("Interpolation type", OVP_TypeId_SphericalLinearInterpolationType, "1");
					prototype.addSetting("Delay (in s)", OV_TypeId_Float, "0");
					prototype.addInput("Signal", OV_TypeId_StreamedMatrix);
					prototype.addInput("Channel localization", OV_TypeId_ChannelLocalisation);
					return true;
				}

				_IsDerivedFromClass_Final_(IBoxAlgorithmDesc, OVP_ClassId_TopographicMap2DDisplayDesc)
			};
		} // namespace SimpleVisualization
	}  // namespace Plugins
}  // namespace OpenViBE
