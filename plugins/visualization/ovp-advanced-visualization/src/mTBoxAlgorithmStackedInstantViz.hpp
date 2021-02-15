/*********************************************************************
 * Software License Agreement (AGPL-3 License)
 *
 * OpenViBE Designer
 * Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
 * Copyright (C) Inria, 2015-2017,V1.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>

#include "mCBoxAlgorithmViz.hpp"

#if defined TARGET_OS_Windows
#define snprintf _snprintf
#endif

namespace OpenViBE {
namespace AdvancedVisualization {
template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
class TBoxAlgorithmStackedInstantViz : public CBoxAlgorithmViz
{
public:

	TBoxAlgorithmStackedInstantViz(const CIdentifier& classId, const std::vector<int>& parameters);
	bool initialize() override;
	bool uninitialize() override;
	bool process() override;

	_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_ClassID)

	Toolkit::TStimulationDecoder<TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>> m_StimDecoder;
	Toolkit::TStreamedMatrixDecoder<TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>> m_MatrixDecoder;

	TRendererFactoryClass m_RendererFactory;
	std::vector<IRenderer*> m_Renderers;

protected:

	void draw() override;
};

class CBoxAlgorithmStackedInstantVizListener final : public CBoxAlgorithmVizListener
{
public:

	CBoxAlgorithmStackedInstantVizListener(const std::vector<int>& parameters) : CBoxAlgorithmVizListener(parameters) { }

	bool onInputTypeChanged(Kernel::IBox& box, const size_t index) override
	{
		CIdentifier typeID = CIdentifier::undefined();
		box.getInputType(index, typeID);
		if (!this->getTypeManager().isDerivedFromStream(typeID, OV_TypeId_TimeFrequency)) { box.setInputType(index, OV_TypeId_TimeFrequency); }
		box.setInputType(1, OV_TypeId_Stimulations);
		return true;
	}
};

template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass = IRuler>
class TBoxAlgorithmStackedInstantVizDesc : public CBoxAlgorithmVizDesc
{
public:

	TBoxAlgorithmStackedInstantVizDesc(const CString& name, const CIdentifier& descClassID, const CIdentifier& classID,
									   const CString& addedSoftwareVersion, const CString& updatedSoftwareVersion,
									   const CParameterSet& parameterSet, const CString& shortDesc, const CString& detailedDesc)
		: CBoxAlgorithmVizDesc(name, descClassID, classID, addedSoftwareVersion, updatedSoftwareVersion, parameterSet, shortDesc, detailedDesc) { }

	Plugins::IPluginObject* create() override
	{
		return new TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>(m_ClassID, m_Parameters);
	}

	Plugins::IBoxListener* createBoxListener() const override { return new CBoxAlgorithmStackedInstantVizListener(m_Parameters); }

	CString getCategory() const override { return CString("Advanced Visualization/") + m_CategoryName; }

	_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_DescClassID)
};


template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::TBoxAlgorithmStackedInstantViz(
	const CIdentifier& classId, const std::vector<int>& parameters)
	: CBoxAlgorithmViz(classId, parameters) { }

template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::initialize()

{
	const bool res = CBoxAlgorithmViz::initialize();

	m_MatrixDecoder.initialize(*this, 0);
	m_StimDecoder.initialize(*this, 1);

	m_RendererCtx->clear();
	m_RendererCtx->setTranslucency(float(m_Translucency));
	m_RendererCtx->scaleBy(float(m_DataScale));
	m_RendererCtx->setPositiveOnly(true);
	m_RendererCtx->setAxisDisplay(m_ShowAxis);
	m_RendererCtx->setParentRendererContext(&getContext());

	m_SubRendererCtx->clear();
	m_SubRendererCtx->setParentRendererContext(m_RendererCtx);

	m_Ruler = new TRulerClass;
	m_Ruler->setRendererContext(m_RendererCtx);

	CMatrix gradientMatrix;
	VisualizationToolkit::ColorGradient::parse(gradientMatrix, m_ColorGradient);
	for (size_t step = 0; step < gradientMatrix.getDimensionSize(1); ++step)
	{
		const double currentStepValue            = gradientMatrix.getBuffer()[4 * step + 0];
		gradientMatrix.getBuffer()[4 * step + 0] = (currentStepValue / 100.0) * 50.0 + 50.0;
	}
	VisualizationToolkit::ColorGradient::format(m_ColorGradient, gradientMatrix);

	return res;
}

template <bool TDrawBorders, class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmStackedInstantViz<TDrawBorders, TRendererFactoryClass, TRulerClass>::uninitialize()

{
	for (size_t i = 0; i < m_Renderers.size(); ++i) { m_RendererFactory.release(m_Renderers[i]); }
	m_Renderers.clear();

	delete m_SubRendererCtx;
	m_SubRendererCtx = nullptr;

	delete m_RendererCtx;
	m_RendererCtx = nullptr;

	delete m_Ruler;
	m_Ruler = nullptr;

	m_StimDecoder.uninitialize();
	m_MatrixDecoder.uninitialize();

	return CBoxAlgorithmViz::uninitialize();
}

template <bool TDrawBorders, class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmStackedInstantViz<TDrawBorders, TRendererFactoryClass, TRulerClass>::process()

{
	const Kernel::IBox& staticBoxContext = this->getStaticBoxContext();
	Kernel::IBoxIO& dynamicBoxContext    = this->getDynamicBoxContext();

	for (size_t chunk = 0; chunk < dynamicBoxContext.getInputChunkCount(0); ++chunk)
	{
		m_MatrixDecoder.decode(chunk);

		CMatrix* inputMatrix  = m_MatrixDecoder.getOutputMatrix();
		const size_t nChannel = inputMatrix->getDimensionSize(0);

		if (nChannel == 0)
		{
			this->getLogManager() << Kernel::LogLevel_Error << "Input stream " << chunk << " has 0 channels\n";
			return false;
		}

		if (m_MatrixDecoder.isHeaderReceived())
		{
			for (auto renderer : m_Renderers) { m_RendererFactory.release(renderer); }
			m_Renderers.clear();
			m_Renderers.resize(nChannel);

			m_SubRendererCtx->clear();
			m_SubRendererCtx->setParentRendererContext(m_RendererCtx);
			m_SubRendererCtx->setTimeLocked(false);
			m_SubRendererCtx->setStackCount(nChannel);
			m_SubRendererCtx->setPositiveOnly(true);


			m_RendererCtx->clear();
			m_RendererCtx->setTranslucency(float(m_Translucency));
			m_RendererCtx->setTimeScale(1);
			m_RendererCtx->scaleBy(float(m_DataScale));
			m_RendererCtx->setParentRendererContext(&getContext());
			m_RendererCtx->setTimeLocked(false);
			m_RendererCtx->setXYZPlotDepth(false);
			m_RendererCtx->setPositiveOnly(true);

			if (m_TypeID == OV_TypeId_TimeFrequency)
			{
				GtkTreeIter gtkTreeIter;
				gtk_list_store_clear(m_ChannelListStore);

				const size_t frequencyCount = inputMatrix->getDimensionSize(1);
				const size_t nSample        = inputMatrix->getDimensionSize(2);

				// I do not know what this is for...
				for (size_t frequency = 0; frequency < frequencyCount; ++frequency)
				{
					try
					{
						const double frequencyValue = std::stod(inputMatrix->getDimensionLabel(1, frequency), nullptr);
						const int stringSize        = snprintf(nullptr, 0, "%.2f", frequencyValue) + 1;
						if (stringSize > 0)
						{
							std::unique_ptr<char[]> buffer(new char[stringSize]);
							snprintf(buffer.get(), size_t(stringSize), "%.2f", frequencyValue);
							m_RendererCtx->setDimensionLabel(1, frequencyCount - frequency - 1, buffer.get());
						}
					}
					catch (...) { m_RendererCtx->setDimensionLabel(1, frequencyCount - frequency - 1, "NaN"); }
					m_SubRendererCtx->addChannel("", 0, 0, 0);
				}


				m_RendererCtx->setDataType(CRendererContext::EDataType::TimeFrequency);
				m_SubRendererCtx->setDataType(CRendererContext::EDataType::TimeFrequency);

				m_RendererCtx->setElementCount(nSample);
				m_SubRendererCtx->setElementCount(nSample);
				gtk_tree_view_set_model(m_ChannelTreeView, nullptr);

				for (size_t channel = 0; channel < nChannel; ++channel)
				{
					std::string channelName          = trim(inputMatrix->getDimensionLabel(0, channel));
					std::string lowercaseChannelName = channelName;
					std::transform(channelName.begin(), channelName.end(), lowercaseChannelName.begin(), tolower);
					const CVertex v = m_ChannelPositions[lowercaseChannelName];

					if (channelName.empty()) { channelName = "Channel " + std::to_string(channel + 1); }

					m_Renderers[channel] = m_RendererFactory.create();

					// The channels in the sub-renderer are the frequencies in the spectrum
					m_Renderers[channel]->setChannelCount(frequencyCount);
					m_Renderers[channel]->setSampleCount(nSample);

					m_RendererCtx->addChannel(channelName, v.x, v.y, v.z);
					gtk_list_store_append(m_ChannelListStore, &gtkTreeIter);
					gtk_list_store_set(m_ChannelListStore, &gtkTreeIter, 0, channel + 1, 1, channelName.c_str(), -1);
				}
				gtk_tree_view_set_model(m_ChannelTreeView, GTK_TREE_MODEL(m_ChannelListStore));
				gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_ChannelTreeView));
			}
			else
			{
				this->getLogManager() << Kernel::LogLevel_Error << "Input stream type is not supported\n";
				return false;
			}

			m_Ruler->setRenderer(nChannel ? m_Renderers[0] : nullptr);

			m_RebuildNeeded = true;
			m_RefreshNeeded = true;
			m_RedrawNeeded  = true;
		}

		if (m_MatrixDecoder.isBufferReceived())
		{
			if (m_TypeID == OV_TypeId_TimeFrequency)
			{
				m_Time1 = m_Time2;
				m_Time2 = dynamicBoxContext.getInputChunkEndTime(0, chunk);

				const size_t frequencyCount = inputMatrix->getDimensionSize(1);
				const size_t nSample        = inputMatrix->getDimensionSize(2);

				const uint64_t chunkDuration  = dynamicBoxContext.getInputChunkEndTime(0, chunk) - dynamicBoxContext.getInputChunkStartTime(0, chunk);
				const uint64_t sampleDuration = chunkDuration / nSample;

				m_SubRendererCtx->setSampleDuration(sampleDuration);
				m_RendererCtx->setSampleDuration(sampleDuration);

				for (size_t channel = 0; channel < nChannel; ++channel)
				{
					// Feed renderer with actual samples
					for (size_t sample = 0; sample < nSample; ++sample)
					{
						m_Swaps.resize(frequencyCount);
						for (size_t frequency = 0; frequency < frequencyCount; ++frequency)
						{
							m_Swaps[frequencyCount - frequency - 1] = float(
								inputMatrix->getBuffer()[sample + frequency * nSample + channel * nSample * frequencyCount]);
						}
						m_Renderers[channel]->feed(&m_Swaps[0]);
					}
				}

				m_RefreshNeeded = true;
				m_RedrawNeeded  = true;
			}
		}
	}

	if (staticBoxContext.getInputCount() > 1)
	{
		for (size_t i = 0; i < dynamicBoxContext.getInputChunkCount(1); ++i)
		{
			m_StimDecoder.decode(i);
			if (m_StimDecoder.isBufferReceived())
			{
				IStimulationSet* stimSet = m_StimDecoder.getOutputStimulationSet();
				for (size_t j = 0; j < stimSet->getStimulationCount(); ++j)
				{
					m_Renderers[0]->feed(stimSet->getStimulationDate(j), stimSet->getStimulationIdentifier(j));
					m_RedrawNeeded = true;
				}
			}
		}
	}

	if (m_RebuildNeeded) { for (auto& renderer : m_Renderers) { renderer->rebuild(*m_SubRendererCtx); } }
	if (m_RefreshNeeded) { for (auto& renderer : m_Renderers) { renderer->refresh(*m_SubRendererCtx); } }
	if (m_RedrawNeeded) { this->redraw(); }

	m_RebuildNeeded = false;
	m_RefreshNeeded = false;
	m_RedrawNeeded  = false;

	return true;
}

template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
void TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::draw()

{
	CBoxAlgorithmViz::preDraw();

	if (m_RendererCtx->getSelectedCount() != 0)
	{
		glPushMatrix();
		glScalef(1, 1.F / m_RendererCtx->getSelectedCount(), 1);
		for (size_t i = 0; i < m_RendererCtx->getSelectedCount(); ++i)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
			glColor4f(m_Color.r, m_Color.g, m_Color.b, m_RendererCtx->getTranslucency());
			glTranslatef(0, m_RendererCtx->getSelectedCount() - i - 1.F, 0);

			m_SubRendererCtx->setAspect(m_RendererCtx->getAspect());
			m_SubRendererCtx->setStackCount(m_RendererCtx->getSelectedCount());
			m_SubRendererCtx->setStackIndex(i);
			m_Renderers[m_RendererCtx->getSelected(i)]->render(*m_SubRendererCtx);
			if (bDrawBorders)
			{
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_BLEND);
				glColor3f(0, 0, 0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(0, 0);
				glVertex2f(1, 0);
				glVertex2f(1, 1);
				glVertex2f(0, 1);
				glEnd();
			}
			glPopMatrix();
			glPopAttrib();
		}
		glPopMatrix();
	}

	CBoxAlgorithmViz::postDraw();
}
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
