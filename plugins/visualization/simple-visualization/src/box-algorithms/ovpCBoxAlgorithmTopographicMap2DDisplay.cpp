#include "ovpCBoxAlgorithmTopographicMap2DDisplay.h"
#include "../algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"
#include <cstdlib>

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Plugins;
using namespace SimpleVisualization;

bool CBoxAlgorithmTopographicMap2DDisplay::initialize()

{
	m_hasFirstBuffer = false;
	m_decoder.initialize(*this, 0);

	m_interpolation = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_SphericalSplineInterpolation));
	m_interpolation->initialize();

	//create topographic map database
	m_database = new CTopographicMapDatabase(*this, *m_interpolation);

	//retrieve settings
	CString interpolationValue;
	getStaticBoxContext().getSettingValue(0, interpolationValue);
	CString delayValue;
	getStaticBoxContext().getSettingValue(1, delayValue);

	//create topographic map view (handling GUI interaction)
	m_view = new CTopographicMap2DView(
		*m_database, EInterpolationType(getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_SphericalLinearInterpolationType, interpolationValue)),
		strtod(delayValue, nullptr));

	//have database notify us when new data is available
	m_database->setDrawable(m_view);
	//ask not to be notified when new data is available (refresh is handled separately)
	m_database->setRedrawOnNewData(false);

	//send widget pointers to visualisation context for parenting
	GtkWidget* widget        = nullptr;
	GtkWidget* toolbarWidget = nullptr;
	dynamic_cast<CTopographicMap2DView*>(m_view)->getWidgets(widget, toolbarWidget);

	if (!this->canCreatePluginObject(OVP_ClassId_Plugin_VisualizationCtx))
	{
		getLogManager() << LogLevel_Error << "Visualization framework is not loaded" << "\n";
		return false;
	}

	m_visualizationCtx = dynamic_cast<VisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationCtx));
	m_visualizationCtx->setWidget(*this, widget);
	m_visualizationCtx->setToolbar(*this, toolbarWidget);

	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::uninitialize()
{
	m_decoder.uninitialize();

	delete m_view;
	m_view = nullptr;
	delete m_database;
	m_database = nullptr;

	m_interpolation->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_interpolation);

	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::processInput(const size_t /*index*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::processClock(CMessage& /*msg*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

bool CBoxAlgorithmTopographicMap2DDisplay::process()

{
	Kernel::IBoxIO* context = getBoxAlgorithmContext()->getDynamicBoxContext();

	//decode signal data
	for (size_t i = 0; i < context->getInputChunkCount(0); ++i)
	{
		m_decoder.decode(i);
		if (m_decoder.isBufferReceived())
		{
			IMatrix* iMatrix = m_decoder.getOutputMatrix();

			//do we need to recopy this for each chunk?
			if (!m_hasFirstBuffer)
			{
				m_database->setMatrixDimensionCount(iMatrix->getDimensionCount());
				for (size_t dimension = 0; dimension < iMatrix->getDimensionCount(); ++dimension)
				{
					m_database->setMatrixDimensionSize(dimension, iMatrix->getDimensionSize(dimension));
					for (size_t entryIndex = 0; entryIndex < iMatrix->getDimensionSize(dimension); ++entryIndex)
					{
						m_database->setMatrixDimensionLabel(dimension, entryIndex, iMatrix->getDimensionLabel(dimension, entryIndex));
					}
				}
				m_hasFirstBuffer = true;
			}
			//

			if (!m_database->setMatrixBuffer(iMatrix->getBuffer(), context->getInputChunkStartTime(0, i), context->getInputChunkEndTime(0, i)))
			{
				return false;
			}
		}
	}

	//decode channel localisation data
	for (size_t i = 0; i < context->getInputChunkCount(1); ++i)
	{
		const IMemoryBuffer* buf = context->getInputChunk(1, i);
		m_database->decodeChannelLocalisationMemoryBuffer(buf, context->getInputChunkStartTime(1, i), context->getInputChunkEndTime(1, i));
		context->markInputAsDeprecated(1, i);
	}

	const bool processValues = m_database->processValues();

	//disable plugin upon errors
	return processValues;
}
