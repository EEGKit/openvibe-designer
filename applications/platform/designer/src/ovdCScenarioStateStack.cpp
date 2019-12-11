#include "ovdCScenarioStateStack.h"

#include "ovdCInterfacedScenario.h"
#include <zlib.h>
#include <ovp_global_defines.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace OpenViBEToolkit;

CScenarioStateStack::CScenarioStateStack(const IKernelContext& ctx, CInterfacedScenario& interfacedScenario, IScenario& scenario)
	: m_kernelCtx(ctx), m_interfacedScenario(interfacedScenario), m_scenario(scenario)
{
	m_currentState  = m_states.begin();
	m_nMaximumState = size_t(m_kernelCtx.getConfigurationManager().expandAsUInteger("${Designer_UndoRedoStackSize}", 64));
}

bool CScenarioStateStack::undo()
{
	auto it = m_currentState;
	if (it == m_states.begin()) { return false; }

	--it;

	m_currentState = it;

	return this->restoreState(**m_currentState);
}

bool CScenarioStateStack::isRedoPossible()
{
	auto it = m_currentState;
	if (it == m_states.end()) { return false; }

	++it;
	return it != m_states.end();
}

bool CScenarioStateStack::redo()
{
	auto it = m_currentState;
	if (it == m_states.end()) { return false; }

	++it;
	if (it == m_states.end()) { return false; }

	m_currentState = it;

	return this->restoreState(**m_currentState);
}

bool CScenarioStateStack::snapshot()
{
	CMemoryBuffer* newState = new CMemoryBuffer();

	if (!this->dumpState(*newState))
	{
		delete newState;
		return false;
	}

	if (m_currentState != m_states.end()) { ++m_currentState; }

	while (m_currentState != m_states.end())
	{
		delete*m_currentState;
		m_currentState = m_states.erase(m_currentState);
	}

	if (m_nMaximumState != 0) { while (m_states.size() >= m_nMaximumState) { m_states.erase(m_states.begin()); } }

	m_states.push_back(newState);

	m_currentState = m_states.end();
	--m_currentState;

	return true;
}

bool CScenarioStateStack::restoreState(const IMemoryBuffer& state)
{
	CMemoryBuffer uncompressedBuffer;

	if (state.getSize() == 0) { return false; }

	const CIdentifier importerID = m_kernelCtx.getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
	if (importerID == OV_UndefinedIdentifier) { return false; }

	IAlgorithmProxy* importer = &m_kernelCtx.getAlgorithmManager().getAlgorithm(importerID);
	if (!importer) { return false; }

	const uLongf srcSize = uLongf(state.getSize()) - sizeof(uLongf);
	Bytef* srcBuffer     = const_cast<Bytef*>(state.getDirectPointer());

	uLongf dstSize = *(uLongf*)(state.getDirectPointer() + state.getSize() - sizeof(uLongf));
	uncompressedBuffer.setSize(dstSize, true);
	Bytef* dstBuffer = static_cast<Bytef*>(uncompressedBuffer.getDirectPointer());

	if (uncompress(dstBuffer, &dstSize, srcBuffer, srcSize) != Z_OK) { return false; }

	importer->initialize();

	TParameterHandler<const IMemoryBuffer*> ip_buffer(importer->getInputParameter(OV_Algorithm_ScenarioImporter_InputParameterId_MemoryBuffer));
	TParameterHandler<IScenario*> op_scenario(importer->getOutputParameter(OV_Algorithm_ScenarioImporter_OutputParameterId_Scenario));

	m_scenario.clear();

	ip_buffer   = &uncompressedBuffer;
	op_scenario = &m_scenario;

	importer->process();
	importer->uninitialize();
	m_kernelCtx.getAlgorithmManager().releaseAlgorithm(*importer);

	// Find the VisualizationTree metadata
	IMetadata* treeMetadata = nullptr;
	CIdentifier metadataID  = OV_UndefinedIdentifier;
	while ((metadataID = m_scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
	{
		treeMetadata = m_scenario.getMetadataDetails(metadataID);
		if (treeMetadata && treeMetadata->getType() == OVVIZ_MetadataIdentifier_VisualizationTree) { break; }
	}

	OpenViBEVisualizationToolkit::IVisualizationTree* visualizationTree = m_interfacedScenario.m_pVisualizationTree;
	if (treeMetadata && visualizationTree) { visualizationTree->deserialize(treeMetadata->getData()); }

	return true;
}

bool CScenarioStateStack::dumpState(IMemoryBuffer& state)
{
	CMemoryBuffer uncompressedBuffer;
	CMemoryBuffer compressedBuffer;

	// Update the scenario metadata according to the current state of the visualization tree

	// Remove all VisualizationTree type metadata
	CIdentifier oldTreeMetadataID = OV_UndefinedIdentifier;
	CIdentifier metadataID        = OV_UndefinedIdentifier;
	while ((metadataID = m_scenario.getNextMetadataIdentifier(metadataID)) != OV_UndefinedIdentifier)
	{
		if (m_scenario.getMetadataDetails(metadataID)->getType() == OVVIZ_MetadataIdentifier_VisualizationTree)
		{
			oldTreeMetadataID = metadataID;
			m_scenario.removeMetadata(metadataID);
			metadataID = OV_UndefinedIdentifier;
		}
	}

	// Insert new metadata
	m_scenario.addMetadata(metadataID, oldTreeMetadataID);
	m_scenario.getMetadataDetails(metadataID)->setType(OVVIZ_MetadataIdentifier_VisualizationTree);
	m_scenario.getMetadataDetails(metadataID)->setData(m_interfacedScenario.m_pVisualizationTree->serialize());

	const CIdentifier exporterID = m_kernelCtx.getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_XMLScenarioExporter);

	if (exporterID == OV_UndefinedIdentifier) { return false; }

	IAlgorithmProxy* exporter = &m_kernelCtx.getAlgorithmManager().getAlgorithm(exporterID);
	if (!exporter) { return false; }

	exporter->initialize();

	TParameterHandler<const IScenario*> ip_scenario(exporter->getInputParameter(OV_Algorithm_ScenarioExporter_InputParameterId_Scenario));
	TParameterHandler<IMemoryBuffer*> op_buffer(exporter->getOutputParameter(OV_Algorithm_ScenarioExporter_OutputParameterId_MemoryBuffer));

	ip_scenario = &m_scenario;
	op_buffer   = &uncompressedBuffer;

	exporter->process();
	exporter->uninitialize();
	m_kernelCtx.getAlgorithmManager().releaseAlgorithm(*exporter);

	uLongf srcSize   = uLongf(uncompressedBuffer.getSize());
	Bytef* srcBuffer = static_cast<Bytef*>(uncompressedBuffer.getDirectPointer());

	compressedBuffer.setSize(12 + uint64_t(srcSize * 1.1), true);

	uLongf dstSize   = uLongf(compressedBuffer.getSize());
	Bytef* dstBuffer = static_cast<Bytef*>(compressedBuffer.getDirectPointer());

	if (compress(dstBuffer, &dstSize, srcBuffer, srcSize) != Z_OK) { return false; }

	state.setSize(0, true);
	state.append(compressedBuffer.getDirectPointer(), dstSize);
	state.append(reinterpret_cast<const uint8_t*>(&srcSize), sizeof(uLongf));

	return true;
}
