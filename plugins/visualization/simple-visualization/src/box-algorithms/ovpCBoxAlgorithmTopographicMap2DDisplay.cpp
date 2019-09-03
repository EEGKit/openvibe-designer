#include "ovpCBoxAlgorithmTopographicMap2DDisplay.h"
#include "../algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"
#include <cstdlib>
#include <cmath>
#include <memory.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace SimpleVisualization;

CBoxAlgorithmTopographicMap2DDisplay::CBoxAlgorithmTopographicMap2DDisplay() = default;

uint64_t CBoxAlgorithmTopographicMap2DDisplay::getClockFrequency() { return uint64_t(1LL) << 37; }

bool CBoxAlgorithmTopographicMap2DDisplay::initialize()

{
	m_bFirstBufferReceived = false;
	m_pDecoder             = new OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmTopographicMap2DDisplay>;
	m_pDecoder->initialize(*this, 0);

	m_pSphericalSplineInterpolation = &getAlgorithmManager().getAlgorithm(
		getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_SphericalSplineInterpolation));
	m_pSphericalSplineInterpolation->initialize();

	//create topographic map database
	m_pTopographicMapDatabase = new CTopographicMapDatabase(*this, *m_pSphericalSplineInterpolation);

	//retrieve settings
	CString l_sInterpolationModeSettingValue;
	getStaticBoxContext().getSettingValue(0, l_sInterpolationModeSettingValue);
	CString l_sDelaySettingValue;
	getStaticBoxContext().getSettingValue(1, l_sDelaySettingValue);

	//create topographic map view (handling GUI interaction)
	m_pTopographicMap2DView = new CTopographicMap2DView(*m_pTopographicMapDatabase,
														getTypeManager().getEnumerationEntryValueFromName(
															OVP_TypeId_SphericalLinearInterpolationType, l_sInterpolationModeSettingValue),
														strtod(l_sDelaySettingValue, nullptr));

	//have database notify us when new data is available
	m_pTopographicMapDatabase->setDrawable(m_pTopographicMap2DView);
	//ask not to be notified when new data is available (refresh is handled separately)
	m_pTopographicMapDatabase->setRedrawOnNewData(false);

	//send widget pointers to visualisation context for parenting
	GtkWidget* l_pWidget        = nullptr;
	GtkWidget* l_pToolbarWidget = nullptr;
	dynamic_cast<CTopographicMap2DView*>(m_pTopographicMap2DView)->getWidgets(l_pWidget, l_pToolbarWidget);

	if (!this->canCreatePluginObject(OVP_ClassId_Plugin_VisualizationContext))
	{
		this->getLogManager() << LogLevel_Error << "Visualization framework is not loaded" << "\n";
		return false;
	}

	m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(
		OVP_ClassId_Plugin_VisualizationContext));
	m_visualizationContext->setWidget(*this, l_pWidget);
	m_visualizationContext->setToolbar(*this, l_pToolbarWidget);

	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::uninitialize()

{
	if (m_pDecoder != nullptr)
	{
		m_pDecoder->uninitialize();
		delete m_pDecoder;
	}

	delete m_pTopographicMap2DView;
	m_pTopographicMap2DView = nullptr;
	delete m_pTopographicMapDatabase;
	m_pTopographicMapDatabase = nullptr;

	m_pSphericalSplineInterpolation->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_pSphericalSplineInterpolation);

	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::processInput(const uint32_t /*index*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::processClock(IMessageClock& /*messageClock*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::process()

{
	IDynamicBoxContext* context = getBoxAlgorithmContext()->getDynamicBoxContext();
	uint32_t i;

	//decode signal data
	for (i = 0; i < context->getInputChunkCount(0); ++i)
	{
		m_pDecoder->decode(i);
		if (m_pDecoder->isBufferReceived())
		{
			IMatrix* l_pInputMatrix = m_pDecoder->getOutputMatrix();

			//do we need to recopy this for each chunk?
			if (!m_bFirstBufferReceived)
			{
				m_pTopographicMapDatabase->setMatrixDimensionCount(l_pInputMatrix->getDimensionCount());
				for (uint32_t dimension = 0; dimension < l_pInputMatrix->getDimensionCount(); dimension++)
				{
					m_pTopographicMapDatabase->setMatrixDimensionSize(dimension, l_pInputMatrix->getDimensionSize(dimension));
					for (uint32_t entryIndex = 0; entryIndex < l_pInputMatrix->getDimensionSize(dimension); entryIndex++)
					{
						m_pTopographicMapDatabase->setMatrixDimensionLabel(dimension, entryIndex, l_pInputMatrix->getDimensionLabel(dimension, entryIndex));
					}
				}
				m_bFirstBufferReceived = true;
			}
			//

			if (!m_pTopographicMapDatabase->setMatrixBuffer(l_pInputMatrix->getBuffer(), context->getInputChunkStartTime(0, i),
															context->getInputChunkEndTime(0, i))) { return false; }
		}
	}

	//decode channel localisation data
	for (i = 0; i < context->getInputChunkCount(1); ++i)
	{
		const IMemoryBuffer* l_pBuf = context->getInputChunk(1, i);
		m_pTopographicMapDatabase->decodeChannelLocalisationMemoryBuffer(l_pBuf, context->getInputChunkStartTime(1, i), context->getInputChunkEndTime(1, i));
		context->markInputAsDeprecated(1, i);
	}

	const bool l_bProcessValues = m_pTopographicMapDatabase->processValues();

	//disable plugin upon errors
	return l_bProcessValues;
}
