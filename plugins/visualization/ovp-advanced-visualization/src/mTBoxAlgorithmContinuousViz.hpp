///-------------------------------------------------------------------------------------------------
/// 
/// \file mTBoxAlgorithmContinuousViz.hpp
/// \copyright Copyright (C) 2022 Inria
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <https://www.gnu.org/licenses/>.
/// 
///-------------------------------------------------------------------------------------------------

#pragma once

#include "mCBoxAlgorithmViz.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {
template <class TRendererFactoryClass, class TRulerClass>
class TBoxAlgorithmContinuousViz final : public CBoxAlgorithmViz
{
public:
	TBoxAlgorithmContinuousViz(const CIdentifier& classID, const std::vector<int>& parameters);
	bool initialize() override;
	bool uninitialize() override;
	bool process() override;

	_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_ClassID)

	Toolkit::TStreamedMatrixDecoder<TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>> m_oMatrixDecoder;
	Toolkit::TStimulationDecoder<TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>> m_oStimulationDecoder;
	TRendererFactoryClass m_RendererFactory;
	IRenderer* m_Renderer = nullptr;

protected:
	void draw() override;
};

class CBoxAlgorithmContinuousVizListener final : public CBoxAlgorithmVizListener
{
public:
	explicit CBoxAlgorithmContinuousVizListener(const std::vector<int>& parameters) : CBoxAlgorithmVizListener(parameters) { }

	bool onInputTypeChanged(Kernel::IBox& box, const size_t index) override
	{
		CIdentifier typeID = CIdentifier::undefined();
		box.getInputType(index, typeID);
		if (!this->getTypeManager().isDerivedFromStream(typeID, OV_TypeId_StreamedMatrix)) { box.setInputType(index, OV_TypeId_StreamedMatrix); }
		box.setInputType(1, OV_TypeId_Stimulations);
		return true;
	}
};

template <class TRendererFactoryClass, class TRulerClass = IRuler>
class TBoxAlgorithmContinuousVizDesc final : public CBoxAlgorithmVizDesc
{
public:
	TBoxAlgorithmContinuousVizDesc(const CString& name, const CIdentifier& descClassID, const CIdentifier& classID,
								   const CString& addedSoftwareVersion, const CString& updatedSoftwareVersion,
								   const CParameterSet& parameterSet, const CString& shortDesc, const CString& detailedDesc)
		: CBoxAlgorithmVizDesc(name, descClassID, classID, addedSoftwareVersion, updatedSoftwareVersion, parameterSet, shortDesc, detailedDesc) { }

	Plugins::IPluginObject* create() override { return new TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>(m_ClassID, m_Parameters); }
	Plugins::IBoxListener* createBoxListener() const override { return new CBoxAlgorithmContinuousVizListener(m_Parameters); }
	CString getCategory() const override { return CString("Advanced Visualization/") + m_CategoryName; }

	_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_DescClassID)
};

template <class TRendererFactoryClass, class TRulerClass>
TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::TBoxAlgorithmContinuousViz(
	const CIdentifier& classID, const std::vector<int>& parameters) : CBoxAlgorithmViz(classID, parameters) { }

template <class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::initialize()
{
	const bool res = CBoxAlgorithmViz::initialize();

	m_oMatrixDecoder.initialize(*this, 0);
	m_oStimulationDecoder.initialize(*this, 1);

	m_Renderer = m_RendererFactory.create();

	m_Ruler = new TRulerClass;
	m_Ruler->setRendererContext(m_RendererCtx);
	m_Ruler->setRenderer(m_Renderer);

	return res;
}

template <class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::uninitialize()
{
	m_RendererFactory.release(m_Renderer);
	m_Renderer = nullptr;

	delete m_Ruler;
	m_Ruler = nullptr;

	m_oStimulationDecoder.uninitialize();
	m_oMatrixDecoder.uninitialize();

	return CBoxAlgorithmViz::uninitialize();
}

template <class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::process()
{
	const Kernel::IBoxIO& boxContext = this->getDynamicBoxContext();
	for (size_t i = 0; i < boxContext.getInputChunkCount(0); ++i) {
		m_oMatrixDecoder.decode(i);

		CMatrix* matrix = m_oMatrixDecoder.getOutputMatrix();
		size_t nChannel = matrix->getDimensionSize(0);
		size_t nSample  = matrix->getDimensionSize(1);

		if (nChannel == 0) {
			this->getLogManager() << Kernel::LogLevel_Error << "Input stream " << i << " has 0 channels\n";
			return false;
		}

		if (matrix->getDimensionCount() == 1) {
			nChannel = matrix->getDimensionSize(0);
			nSample  = 1;
		}

		if (m_oMatrixDecoder.isHeaderReceived()) {
			GtkTreeIter gtkTreeIt;
			gtk_list_store_clear(m_ChannelListStore);

			m_Swaps.resize(size_t(nChannel));

			m_RendererCtx->clear();
			m_RendererCtx->setTranslucency(float(m_Translucency));
			m_RendererCtx->setFlowerRingCount(m_NFlowerRing);
			m_RendererCtx->setTimeScale(m_TimeScale);
			m_RendererCtx->setElementCount(m_NElement);
			m_RendererCtx->scaleBy(float(m_DataScale));
			m_RendererCtx->setAxisDisplay(m_ShowAxis);
			m_RendererCtx->setPositiveOnly(m_IsPositive);
			m_RendererCtx->setParentRendererContext(&getContext());
			m_RendererCtx->setTimeLocked(m_IsTimeLocked);
			m_RendererCtx->setXYZPlotDepth(m_XYZPlotHasDepth);

			gtk_tree_view_set_model(m_ChannelTreeView, nullptr);
			for (size_t j = 0; j < nChannel; ++j) {
				std::string name    = trim(matrix->getDimensionLabel(0, j));
				std::string subname = name;
				std::transform(name.begin(), name.end(), subname.begin(), tolower);
				const CVertex v = m_ChannelPositions[subname];

				if (name.empty()) { name = "Channel " + std::to_string(j + 1); }

				m_RendererCtx->addChannel(name, v.x, v.y, v.z);
				gtk_list_store_append(m_ChannelListStore, &gtkTreeIt);
				gtk_list_store_set(m_ChannelListStore, &gtkTreeIt, 0, j + 1, 1, name.c_str(), -1);
			}
			gtk_tree_view_set_model(m_ChannelTreeView, GTK_TREE_MODEL(m_ChannelListStore));
			gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_ChannelTreeView));

			m_Renderer->setChannelCount(nChannel);

			if (m_TypeID == OV_TypeId_Signal) { m_RendererCtx->setDataType(CRendererContext::EDataType::Signal); }
			else if (m_TypeID == OV_TypeId_Spectrum) { m_RendererCtx->setDataType(CRendererContext::EDataType::Spectrum); }
			else { m_RendererCtx->setDataType(CRendererContext::EDataType::Matrix); }

			if (nSample != 1) {
				//bool warned = false;
				if (m_TypeID == OV_TypeId_Spectrum) {
					//warned = true;
					this->getLogManager() << Kernel::LogLevel_Warning << "Input matrix has 'spectrum' type\n";
					this->getLogManager() << Kernel::LogLevel_Warning << "Such configuration is uncommon for a 'continous' kind of visualization !\n";
					this->getLogManager() << Kernel::LogLevel_Warning <<
							"You might want to consider the 'stacked' kind of visualization for time/frequency analysis for instance\n";
					this->getLogManager() << Kernel::LogLevel_Warning << "Please double check your scenario\n";
				}
				else {
					if (!m_RendererCtx->isTimeLocked()) {
						//warned = true;
						this->getLogManager() << Kernel::LogLevel_Warning << "Input matrix has " << nSample
								<< " elements and the box settings say the elements are independant with " << m_NElement << " elements to render\n";
						this->getLogManager() << Kernel::LogLevel_Warning <<
								"Such configuration is uncommon for a 'continous' kind of visualization !\n";
						this->getLogManager() << Kernel::LogLevel_Warning << "You might want either of the following alternative :\n";
						this->getLogManager() << Kernel::LogLevel_Warning << " - an 'instant' kind of visualization to highlight the " << m_NElement
								<< " elements of the matrix\n";
						this->getLogManager() << Kernel::LogLevel_Warning <<
								" - a 'time locked' kind of elements (thus the scenario must refresh the matrix on a regular basis)\n";
						this->getLogManager() << Kernel::LogLevel_Warning << "Please double check your scenario and box settings\n";
					}
				}
			}

			m_RebuildNeeded = true;
			m_RefreshNeeded = true;
			m_RedrawNeeded  = true;
		}
		if (m_oMatrixDecoder.isBufferReceived()) {
			m_Time1                 = m_Time2;
			m_Time2                 = boxContext.getInputChunkEndTime(0, i);
			const uint64_t duration = (m_Time2 - m_Time1) / nSample;
			if ((duration & ~0xf) != (m_RendererCtx->getSampleDuration() & ~0xf) && duration != 0) // 0xf mask avoids rounding errors
			{
				m_RendererCtx->setSampleDuration(duration);
			}
			m_RendererCtx->setSpectrumFrequencyRange(
				size_t((uint64_t(nSample) << 32) / (boxContext.getInputChunkEndTime(0, i) - boxContext.getInputChunkStartTime(0, i))));
			m_RendererCtx->setMinimumSpectrumFrequency(size_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_FrequencyBandMin))));
			m_RendererCtx->setMaximumSpectrumFrequency(size_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_FrequencyBandMax))));

			// Feed renderer with actual samples
			for (size_t j = 0; j < nSample; ++j) {
				for (size_t k = 0; k < nChannel; ++k) { m_Swaps[k] = float(matrix->getBuffer()[k * nSample + j]); }
				m_Renderer->feed(&m_Swaps[0]);
			}

			// Adjust feeding depending on theoretical dates
			if (m_RendererCtx->isTimeLocked() && m_RendererCtx->getSampleDuration()) {
				const auto nTheoreticalSample = size_t(m_Time2 / m_RendererCtx->getSampleDuration());
				if (nTheoreticalSample > m_Renderer->getHistoryCount()) { m_Renderer->prefeed(nTheoreticalSample - m_Renderer->getHistoryCount()); }
			}

			m_RefreshNeeded = true;
			m_RedrawNeeded  = true;
		}
	}

	const size_t nInput = this->getStaticBoxContext().getInputCount();
	if (nInput > 1) {
		for (size_t i = 0; i < boxContext.getInputChunkCount(1); ++i) {
			m_oStimulationDecoder.decode(i);
			if (m_oStimulationDecoder.isBufferReceived()) {
				const CStimulationSet* stimSet = m_oStimulationDecoder.getOutputStimulationSet();
				for (size_t j = 0; j < stimSet->size(); ++j) {
					m_Renderer->feed(stimSet->getDate(j), stimSet->getId(j));
					m_RedrawNeeded = true;
				}
			}
		}
	}

	size_t rendererSampleCount = 0;
	if (m_RendererCtx->isTimeLocked()) {
		if (0 != m_RendererCtx->getSampleDuration()) { rendererSampleCount = size_t(m_RendererCtx->getTimeScale() / m_RendererCtx->getSampleDuration()); }
	}
	else {
		rendererSampleCount = size_t(m_RendererCtx->getElementCount()); // *nSample;
	}

	if (rendererSampleCount != 0 && rendererSampleCount != m_Renderer->getSampleCount()) {
		m_Renderer->setSampleCount(rendererSampleCount);
		m_RebuildNeeded = true;
		m_RefreshNeeded = true;
		m_RedrawNeeded  = true;
	}

	if (m_RebuildNeeded) { m_Renderer->rebuild(*m_RendererCtx); }
	if (m_RefreshNeeded) { m_Renderer->refresh(*m_RendererCtx); }
	if (m_RedrawNeeded) { this->redraw(); }

	m_RebuildNeeded = false;
	m_RefreshNeeded = false;
	m_RedrawNeeded  = false;

	return true;
}

template <class TRendererFactoryClass, class TRulerClass>
void TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::draw()

{
	CBoxAlgorithmViz::preDraw();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glColor4f(m_Color.r, m_Color.g, m_Color.b, m_RendererCtx->getTranslucency());
	m_Renderer->render(*m_RendererCtx);
	glPopAttrib();

	CBoxAlgorithmViz::postDraw();
}
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
