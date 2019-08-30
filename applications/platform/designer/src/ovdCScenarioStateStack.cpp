#include "ovdCScenarioStateStack.h"

#include "ovdCInterfacedScenario.h"
#include <zlib.h>
#include <ovp_global_defines.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace OpenViBEToolkit;

CScenarioStateStack::CScenarioStateStack(const IKernelContext& kernelContext, CInterfacedScenario& interfacedScenario, IScenario& scenario)
	: m_kernelContext(kernelContext), m_InterfacedScenario(interfacedScenario), m_Scenario(scenario)
{
	m_CurrentState      = m_States.begin();
	m_MaximumStateCount = uint32_t(m_kernelContext.getConfigurationManager().expandAsUInteger("${Designer_UndoRedoStackSize}", 64));
}

CScenarioStateStack::~CScenarioStateStack()
{
	for (auto& state : m_States) { delete state; }
}

bool CScenarioStateStack::isUndoPossible()
{
	return m_CurrentState != m_States.begin();
}

bool CScenarioStateStack::undo()
{
	auto itState = m_CurrentState;
	if (itState == m_States.begin()) { return false; }

	--itState;

	m_CurrentState = itState;

	return this->restoreState(**m_CurrentState);
}

void CScenarioStateStack::dropLastState() { m_States.pop_back(); }

bool CScenarioStateStack::isRedoPossible()
{
	auto itState = m_CurrentState;
	if (itState == m_States.end()) { return false; }

	++itState;
	return itState != m_States.end();
}

bool CScenarioStateStack::redo()
{
	auto itState = m_CurrentState;
	if (itState == m_States.end()) { return false; }

	++itState;
	if (itState == m_States.end()) { return false; }

	m_CurrentState = itState;

	return this->restoreState(**m_CurrentState);
}

bool CScenarioStateStack::snapshot()
{
	auto* newState = new CMemoryBuffer();

	if (!this->dumpState(*newState))
	{
		delete newState;
		return false;
	}

	if (m_CurrentState != m_States.end()) { ++m_CurrentState; }

	while (m_CurrentState != m_States.end())
	{
		delete* m_CurrentState;
		m_CurrentState = m_States.erase(m_CurrentState);
	}

	if (m_MaximumStateCount != 0)
	{
		while (m_States.size() >= m_MaximumStateCount) { m_States.erase(m_States.begin()); }
	}

	m_States.push_back(newState);

	m_CurrentState = m_States.end();
	--m_CurrentState;

	return true;
}

bool CScenarioStateStack::restoreState(const IMemoryBuffer& state)
{
	CMemoryBuffer uncompressedMemoryBuffer;

	if (state.getSize() == 0) { return false; }

	const CIdentifier importerIdentifier = m_kernelContext.getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	if (importerIdentifier == OV_UndefinedIdentifier) { return false; }

	IAlgorithmProxy* importer = &m_kernelContext.getAlgorithmManager().getAlgorithm(importerIdentifier);
	if (!importer) { return false; }

	const uLongf sourceSize = uLongf(state.getSize()) - sizeof(uLongf);
	auto* sourceBuffer      = const_cast<Bytef*>(state.getDirectPointer());

	uLongf destinationSize = *(uLongf*)(state.getDirectPointer() + state.getSize() - sizeof(uLongf));
	uncompressedMemoryBuffer.setSize(destinationSize, true);
	auto* destinationBuffer = static_cast<Bytef*>(uncompressedMemoryBuffer.getDirectPointer());

	if (uncompress(destinationBuffer, &destinationSize, sourceBuffer, sourceSize) != Z_OK) { return false; }

	importer->initialize();

	TParameterHandler<const IMemoryBuffer*> ip_MemoryBuffer(importer->getInputParameter(OV_Algorithm_ScenarioImporter_InputParameterId_MemoryBuffer));
	TParameterHandler<IScenario*> op_Scenario(importer->getOutputParameter(OV_Algorithm_ScenarioImporter_OutputParameterId_Scenario));

	m_Scenario.clear();

	ip_MemoryBuffer = &uncompressedMemoryBuffer;
	op_Scenario     = &m_Scenario;

	importer->process();
	importer->uninitialize();
	m_kernelContext.getAlgorithmManager().releaseAlgorithm(*importer);

	// Find the VisualizationTree metadata
	IMetadata* visualizationTreeMetadata = nullptr;
	CIdentifier metadataID       = OV_UndefinedIdentifier;
	while ((metadataID = m_Scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
	{
		visualizationTreeMetadata = m_Scenario.getMetadataDetails(metadataID);
		if (visualizationTreeMetadata && visualizationTreeMetadata->getType() == OVVIZ_MetadataIdentifier_VisualizationTree) { break; }
	}

	OpenViBEVisualizationToolkit::IVisualizationTree* visualizationTree = m_InterfacedScenario.m_pVisualizationTree;
	if (visualizationTreeMetadata && visualizationTree) { visualizationTree->deserialize(visualizationTreeMetadata->getData()); }

	return true;
}

bool CScenarioStateStack::dumpState(IMemoryBuffer& state)
{
	CMemoryBuffer uncompressedMemoryBuffer;
	CMemoryBuffer compressedMemoryBuffer;

	// Update the scenario metadata according to the current state of the visualization tree

	// Remove all VisualizationTree type metadata
	CIdentifier oldVisualizationTreeMetadataIdentifier = OV_UndefinedIdentifier;
	CIdentifier metadataID                     = OV_UndefinedIdentifier;
	while ((metadataID = m_Scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
	{
		if (m_Scenario.getMetadataDetails(metadataID)->getType() == OVVIZ_MetadataIdentifier_VisualizationTree)
		{
			oldVisualizationTreeMetadataIdentifier = metadataID;
			m_Scenario.removeMetadata(metadataID);
			metadataID = OV_UndefinedIdentifier;
		}
	}

	// Insert new metadata
	m_Scenario.addMetadata(metadataID, oldVisualizationTreeMetadataIdentifier);
	m_Scenario.getMetadataDetails(metadataID)->setType(OVVIZ_MetadataIdentifier_VisualizationTree);
	m_Scenario.getMetadataDetails(metadataID)->setData(m_InterfacedScenario.m_pVisualizationTree->serialize());

	const CIdentifier exporterIdentifier = m_kernelContext.getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_XMLScenarioExporter);

	if (exporterIdentifier == OV_UndefinedIdentifier) { return false; }

	IAlgorithmProxy* exporter = &m_kernelContext.getAlgorithmManager().getAlgorithm(exporterIdentifier);
	if (!exporter) { return false; }

	exporter->initialize();

	TParameterHandler<const IScenario*> ip_Scenario(exporter->getInputParameter(OV_Algorithm_ScenarioExporter_InputParameterId_Scenario));
	TParameterHandler<IMemoryBuffer*> op_MemoryBuffer(exporter->getOutputParameter(OV_Algorithm_ScenarioExporter_OutputParameterId_MemoryBuffer));

	ip_Scenario     = &m_Scenario;
	op_MemoryBuffer = &uncompressedMemoryBuffer;

	exporter->process();
	exporter->uninitialize();
	m_kernelContext.getAlgorithmManager().releaseAlgorithm(*exporter);

	auto sourceSize    = uLongf(uncompressedMemoryBuffer.getSize());
	auto* sourceBuffer = static_cast<Bytef*>(uncompressedMemoryBuffer.getDirectPointer());

	compressedMemoryBuffer.setSize(12 + uint64_t(sourceSize * 1.1), true);

	auto destinationSize    = uLongf(compressedMemoryBuffer.getSize());
	auto* destinationBuffer = static_cast<Bytef*>(compressedMemoryBuffer.getDirectPointer());

	if (compress(destinationBuffer, &destinationSize, sourceBuffer, sourceSize) != Z_OK) { return false; }

	state.setSize(0, true);
	state.append(compressedMemoryBuffer.getDirectPointer(), destinationSize);
	state.append(reinterpret_cast<const uint8_t*>(& sourceSize), sizeof(uLongf));

	return true;
}
