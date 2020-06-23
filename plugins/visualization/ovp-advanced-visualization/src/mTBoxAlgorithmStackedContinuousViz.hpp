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

#include "mCBoxAlgorithmViz.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {

template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
class TBoxAlgorithmStackedContinuousViz : public CBoxAlgorithmViz
{
public:

	TBoxAlgorithmStackedContinuousViz(const CIdentifier& classID, const std::vector<int>& parameters);
	bool initialize() override;
	bool uninitialize() override;
	bool process() override;

	_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_ClassID)

	Toolkit::TStimulationDecoder<TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>>
	m_StimDecoder;
	Toolkit::TStreamedMatrixDecoder<TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>>
	m_MatrixDecoder;

	TRendererFactoryClass m_RendererFactory;
	std::vector<IRenderer*> m_Renderers;

protected:

	void draw() override;
};

class CBoxAlgorithmStackedContinuousVizListener final : public CBoxAlgorithmVizListener
{
public:

	explicit CBoxAlgorithmStackedContinuousVizListener(const std::vector<int>& parameters) : CBoxAlgorithmVizListener(parameters) { }

	bool onInputTypeChanged(Kernel::IBox& box, const size_t index) override
	{
		CIdentifier typeID = CIdentifier::undefined();
		box.getInputType(index, typeID);
		if (!this->getTypeManager().isDerivedFromStream(typeID, OV_TypeId_StreamedMatrix)) { box.setInputType(index, OV_TypeId_StreamedMatrix); }
		box.setInputType(1, OV_TypeId_Stimulations);
		return true;
	}
};

template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass = IRuler>
class TBoxAlgorithmStackedContinuousVizDesc : public CBoxAlgorithmVizDesc
{
public:

	TBoxAlgorithmStackedContinuousVizDesc(const CString& name, const CIdentifier& descClassID,
										  const CIdentifier& classID, const CString& addedSoftwareVersion,
										  const CString& updatedSoftwareVersion, const CParameterSet& parameterSet,
										  const CString& shortDesc, const CString& detailedDesc)
		: CBoxAlgorithmVizDesc(name, descClassID, classID, addedSoftwareVersion, updatedSoftwareVersion, parameterSet, shortDesc, detailedDesc) { }

	Plugins::IPluginObject* create() override
	{
		return new TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>(m_ClassID, m_Parameters);
	}

	Plugins::IBoxListener* createBoxListener() const override { return new CBoxAlgorithmStackedContinuousVizListener(m_Parameters); }

	CString getCategory() const override { return CString("Advanced Visualization/") + m_CategoryName; }

	_IsDerivedFromClass_Final_(Plugins::IBoxAlgorithmDesc, m_DescClassID)
};


template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::TBoxAlgorithmStackedContinuousViz(
	const CIdentifier& classID, const std::vector<int>& parameters) : CBoxAlgorithmViz(classID, parameters) { }

template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::initialize()

{
	const bool res = CBoxAlgorithmViz::initialize();

	m_MatrixDecoder.initialize(*this, 0);
	m_StimDecoder.initialize(*this, 1);

	m_RendererCtx->clear();
	m_RendererCtx->setTranslucency(float(m_Translucency));
	// m_rendererCtx->setTranslucency(m_nFlowerRing);
	m_RendererCtx->scaleBy(float(m_DataScale));
	m_RendererCtx->setPositiveOnly(m_IsPositive);
	m_RendererCtx->setAxisDisplay(m_ShowAxis);
	m_RendererCtx->setParentRendererContext(&getContext());

	m_SubRendererCtx->clear();
	m_SubRendererCtx->setParentRendererContext(m_RendererCtx);

	m_Ruler = new TRulerClass;
	m_Ruler->setRendererContext(m_RendererCtx);

	return res;
}

template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::uninitialize()

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

template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::process()
{
	Kernel::IBoxIO& boxContext = this->getDynamicBoxContext();
	const size_t nInput        = this->getStaticBoxContext().getInputCount();
	size_t i, j;

	for (i = 0; i < boxContext.getInputChunkCount(0); ++i)
	{
		m_MatrixDecoder.decode(size_t(i));

		IMatrix* matrix = m_MatrixDecoder.getOutputMatrix();
		size_t nChannel = matrix->getDimensionSize(0);
		size_t nSample  = matrix->getDimensionSize(1);

		if (nChannel == 0)
		{
			getLogManager() << Kernel::LogLevel_Error << "Input stream " << i << " has 0 channels\n";
			return false;
		}

		if (matrix->getDimensionCount() == 1)
		{
			nChannel = matrix->getDimensionSize(0);
			nSample  = 1;
		}

		if (m_MatrixDecoder.isHeaderReceived())
		{
			GtkTreeIter gtkTreeIter;
			gtk_list_store_clear(m_ChannelListStore);

			m_Swaps.resize(nSample);

			for (j = 0; j < m_Renderers.size(); ++j) { m_RendererFactory.release(m_Renderers[j]); }
			m_Renderers.clear();
			m_Renderers.resize(nChannel);

			m_SubRendererCtx->clear();
			m_SubRendererCtx->setParentRendererContext(m_RendererCtx);
			m_SubRendererCtx->setTimeLocked(m_IsTimeLocked);
			m_SubRendererCtx->setStackCount(nChannel);
			for (j = 0; j < nSample; ++j)
			{
				std::string name    = trim(matrix->getDimensionLabel(1, size_t(j)));
				std::string subname = name;
				std::transform(name.begin(), name.end(), subname.begin(), tolower);
				const CVertex v = m_ChannelPositions[subname];
				m_SubRendererCtx->addChannel(name, v.x, v.y, v.z);
			}

			m_RendererCtx->clear();
			m_RendererCtx->setTranslucency(float(m_Translucency));
			m_RendererCtx->setTimeScale(m_TimeScale);
			m_RendererCtx->setElementCount(m_NElement);
			m_RendererCtx->scaleBy(float(m_DataScale));
			m_RendererCtx->setParentRendererContext(&getContext());
			m_RendererCtx->setTimeLocked(m_IsTimeLocked);
			m_RendererCtx->setXYZPlotDepth(m_XYZPlotHasDepth);

			gtk_tree_view_set_model(m_ChannelTreeView, nullptr);
			for (j = 0; j < nChannel; ++j)
			{
				std::string name    = trim(matrix->getDimensionLabel(0, size_t(j)));
				std::string subname = name;
				std::transform(name.begin(), name.end(), subname.begin(), tolower);
				const CVertex v = m_ChannelPositions[subname];

				if (name.empty()) { name = "Channel " + std::to_string(j + 1); }

				m_Renderers[j] = m_RendererFactory.create();
				m_Renderers[j]->setChannelCount(nSample);
				m_Renderers[j]->setSampleCount(size_t(m_NElement)); // $$$
				// m_Renderers[j]->setSampleCount(size_t(m_TimeScaleW)); // $$$

				m_RendererCtx->addChannel(name, v.x, v.y, v.z);
				// m_rendererCtx->addChannel(sanitize(matrix->getDimensionLabel(0, j)));
				gtk_list_store_append(m_ChannelListStore, &gtkTreeIter);
				gtk_list_store_set(m_ChannelListStore, &gtkTreeIter, 0, j + 1, 1, name.c_str(), -1);
			}
			gtk_tree_view_set_model(m_ChannelTreeView, GTK_TREE_MODEL(m_ChannelListStore));
			gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_ChannelTreeView));

			if (m_TypeID == OV_TypeId_Signal)
			{
				m_RendererCtx->setDataType(CRendererContext::EDataType::Signal);
				m_SubRendererCtx->setDataType(CRendererContext::EDataType::Signal);
			}
			else if (m_TypeID == OV_TypeId_Spectrum)
			{
				m_RendererCtx->setDataType(CRendererContext::EDataType::Spectrum);
				m_SubRendererCtx->setDataType(CRendererContext::EDataType::Spectrum);
			}
			else
			{
				m_RendererCtx->setDataType(CRendererContext::EDataType::Matrix);
				m_SubRendererCtx->setDataType(CRendererContext::EDataType::Matrix);
			}

			m_Ruler->setRenderer(nChannel ? m_Renderers[0] : nullptr);

			m_RebuildNeeded = true;
			m_RefreshNeeded = true;
			m_RedrawNeeded  = true;
		}
		if (m_MatrixDecoder.isBufferReceived())
		{
			m_Time1                        = m_Time2;
			m_Time2                        = boxContext.getInputChunkEndTime(0, size_t(i));
			const CTime interChunkDuration = m_Time2 - m_Time1;
			const CTime chunkDuration      = boxContext.getInputChunkEndTime(0, size_t(i)) - boxContext.getInputChunkStartTime(0, size_t(i));
			const CTime sampleDuration     = chunkDuration.time() / m_NElement;
			if (m_RendererCtx->isTimeLocked())
			{
				if ((interChunkDuration.time() & ~0xf) != (m_RendererCtx->getSampleDuration() & ~0xf) && interChunkDuration != 0
				) // 0xf mask avoids rounding errors
				{
					m_SubRendererCtx->setSampleDuration(interChunkDuration.time());
					m_RendererCtx->setSampleDuration(interChunkDuration.time());
				}
			}
			else
			{
				m_SubRendererCtx->setSampleDuration(sampleDuration.time());
				m_RendererCtx->setSampleDuration(sampleDuration.time());
			}

			m_RendererCtx->setSpectrumFrequencyRange(size_t((uint64_t(nSample) << 32) / chunkDuration.time()));
			m_RendererCtx->setMinimumSpectrumFrequency(size_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_FrequencyBandMin))));
			m_RendererCtx->setMaximumSpectrumFrequency(size_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_FrequencyBandMax))));

			for (j = 0; j < nChannel; ++j)
			{
				// Feed renderer with actual samples
				for (size_t k = 0; k < nSample; ++k) { m_Swaps[nSample - k - 1] = float(matrix->getBuffer()[j * nSample + k]); }
				m_Renderers[j]->feed(&m_Swaps[0]);

				// Adjust feeding depending on theoretical dates
				if (m_RendererCtx->isTimeLocked() && m_RendererCtx->getSampleDuration())
				{
					const auto nTheoreticalSample = size_t(m_Time2.time() / m_RendererCtx->getSampleDuration());
					if (nTheoreticalSample > m_Renderers[j]->getHistoryCount())
					{
						m_Renderers[j]->prefeed(nTheoreticalSample - m_Renderers[j]->getHistoryCount());
					}
				}
			}

			m_RefreshNeeded = true;
			m_RedrawNeeded  = true;
		}
	}

	if (nInput > 1)
	{
		for (i = 0; i < boxContext.getInputChunkCount(1); ++i)
		{
			m_StimDecoder.decode(size_t(i));
			if (m_StimDecoder.isBufferReceived())
			{
				CStimulationSet& set = *m_StimDecoder.getOutputStimulationSet();
				for (j = 0; j < set.size(); ++j)
				{
					m_Renderers[0]->feed(set[j].m_Date.time(), set[j].m_ID);
					m_RedrawNeeded = true;
				}
			}
		}
	}

	size_t rendererSampleCount = 0;
	if (m_RendererCtx->isTimeLocked())
	{
		if (0 != m_RendererCtx->getSampleDuration()) { rendererSampleCount = size_t(m_RendererCtx->getTimeScale() / m_RendererCtx->getSampleDuration()); }
	}
	else { rendererSampleCount = size_t(m_RendererCtx->getElementCount()); }

	if (rendererSampleCount != 0)
	{
		for (j = 0; j < m_Renderers.size(); ++j)
		{
			if (rendererSampleCount != m_Renderers[j]->getSampleCount())
			{
				m_Renderers[j]->setSampleCount(rendererSampleCount);
				m_RebuildNeeded = true;
				m_RefreshNeeded = true;
				m_RedrawNeeded  = true;
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

template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
void TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::draw()

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
			if (!bHorizontalStack)
			{
				glScalef(1, -1, 1);
				glRotatef(-90, 0, 0, 1);
			}
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
