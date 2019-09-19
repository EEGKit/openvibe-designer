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

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		class TBoxAlgorithmStackedContinuousViz : public CBoxAlgorithmViz
		{
		public:

			TBoxAlgorithmStackedContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter);
			bool initialize() override;
			bool uninitialize() override;
			bool process() override;

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId)

			OpenViBEToolkit::TStimulationDecoder<TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>>
			m_oStimulationDecoder;
			OpenViBEToolkit::TStreamedMatrixDecoder<TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>>
			m_oMatrixDecoder;

			TRendererFactoryClass m_oRendererFactory;
			std::vector<IRenderer*> m_vRenderer;

		protected:

			void draw() override;
		};

		class CBoxAlgorithmStackedContinuousVizListener final : public CBoxAlgorithmVizListener
		{
		public:

			explicit CBoxAlgorithmStackedContinuousVizListener(const std::vector<int>& vParameter)
				: CBoxAlgorithmVizListener(vParameter) { }

			bool onInputTypeChanged(OpenViBE::Kernel::IBox& box, const uint32_t index) override
			{
				OpenViBE::CIdentifier typeID = OV_UndefinedIdentifier;
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

			TBoxAlgorithmStackedContinuousVizDesc(const OpenViBE::CString& name, const OpenViBE::CIdentifier& rDescClassId,
												  const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion,
												  const OpenViBE::CString& sUpdatedSoftwareVersion, const CParameterSet& rParameterSet,
												  const OpenViBE::CString& sShortDescription, const OpenViBE::CString& sDetailedDescription)
				: CBoxAlgorithmVizDesc(name, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription,
									   sDetailedDescription) { }

			OpenViBE::Plugins::IPluginObject* create() override
			{
				return new TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter);
			}

			OpenViBE::Plugins::IBoxListener* createBoxListener() const override { return new CBoxAlgorithmStackedContinuousVizListener(m_vParameter); }

			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Advanced Visualization/") + m_sCategoryName; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId)
		};


		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::TBoxAlgorithmStackedContinuousViz(
			const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter)
			: CBoxAlgorithmViz(rClassId, vParameter) { }

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::initialize()

		{
			const bool l_bResult = CBoxAlgorithmViz::initialize();

			m_oMatrixDecoder.initialize(*this, 0);
			m_oStimulationDecoder.initialize(*this, 1);

			m_pRendererContext->clear();
			m_pRendererContext->setTranslucency(float(m_translucency));
			// m_pRendererContext->setTranslucency(m_nFlowerRing);
			m_pRendererContext->scaleBy(float(m_f64DataScale));
			m_pRendererContext->setPositiveOnly(m_bIsPositive);
			m_pRendererContext->setAxisDisplay(m_bShowAxis);
			m_pRendererContext->setParentRendererContext(&getContext());

			m_pSubRendererContext->clear();
			m_pSubRendererContext->setParentRendererContext(m_pRendererContext);

			m_pRuler = new TRulerClass;
			m_pRuler->setRendererContext(m_pRendererContext);

			return l_bResult;
		}

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::uninitialize()

		{
			for (uint32_t i = 0; i < m_vRenderer.size(); ++i) { m_oRendererFactory.release(m_vRenderer[i]); }
			m_vRenderer.clear();

			IRendererContext::release(m_pSubRendererContext);
			m_pSubRendererContext = nullptr;

			IRendererContext::release(m_pRendererContext);
			m_pRendererContext = nullptr;

			delete m_pRuler;
			m_pRuler = nullptr;

			m_oStimulationDecoder.uninitialize();
			m_oMatrixDecoder.uninitialize();

			return CBoxAlgorithmViz::uninitialize();
		}

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::process()
		{
			OpenViBE::Kernel::IBoxIO& boxContext = this->getDynamicBoxContext();
			const uint32_t nInput                = this->getStaticBoxContext().getInputCount();
			size_t i, j;

			for (i = 0; i < boxContext.getInputChunkCount(0); ++i)
			{
				m_oMatrixDecoder.decode(uint32_t(i));

				OpenViBE::IMatrix* l_pMatrix = m_oMatrixDecoder.getOutputMatrix();
				uint32_t l_ui32ChannelCount  = l_pMatrix->getDimensionSize(0);
				uint32_t nSample         = l_pMatrix->getDimensionSize(1);

				if (l_ui32ChannelCount == 0)
				{
					this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Input stream " << uint32_t(i) << " has 0 channels\n";
					return false;
				}

				if (l_pMatrix->getDimensionCount() == 1)
				{
					l_ui32ChannelCount = l_pMatrix->getDimensionSize(0);
					nSample        = 1;
				}

				if (m_oMatrixDecoder.isHeaderReceived())
				{
					GtkTreeIter l_oGtkTreeIterator;
					gtk_list_store_clear(m_pChannelListStore);

					m_vSwap.resize(nSample);

					for (j = 0; j < m_vRenderer.size(); j++) { m_oRendererFactory.release(m_vRenderer[j]); }
					m_vRenderer.clear();
					m_vRenderer.resize(l_ui32ChannelCount);

					m_pSubRendererContext->clear();
					m_pSubRendererContext->setParentRendererContext(m_pRendererContext);
					m_pSubRendererContext->setTimeLocked(m_bIsTimeLocked);
					m_pSubRendererContext->setStackCount(l_ui32ChannelCount);
					for (j = 0; j < nSample; j++)
					{
						std::string l_sName    = trim(l_pMatrix->getDimensionLabel(1, uint32_t(j)));
						std::string l_sSubname = l_sName;
						std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), tolower);
						CVertex v = m_vChannelLocalisation[l_sSubname];
						m_pSubRendererContext->addChannel(l_sName, v.x, v.y, v.z);
					}

					m_pRendererContext->clear();
					m_pRendererContext->setTranslucency(float(m_translucency));
					m_pRendererContext->setTimeScale(m_timeScale);
					m_pRendererContext->setElementCount(m_nElement);
					m_pRendererContext->scaleBy(float(m_f64DataScale));
					m_pRendererContext->setParentRendererContext(&getContext());
					m_pRendererContext->setTimeLocked(m_bIsTimeLocked);
					m_pRendererContext->setXYZPlotDepth(m_bXYZPlotHasDepth);

					gtk_tree_view_set_model(m_pChannelTreeView, nullptr);
					for (j = 0; j < l_ui32ChannelCount; j++)
					{
						std::string l_sName    = trim(l_pMatrix->getDimensionLabel(0, uint32_t(j)));
						std::string l_sSubname = l_sName;
						std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), tolower);
						const CVertex v = m_vChannelLocalisation[l_sSubname];

						if (l_sName.empty())
						{
							char l_sIndexedChannelName[1024];
							sprintf(l_sIndexedChannelName, "Channel %llu", j + 1);
							l_sName = l_sIndexedChannelName;
						}

						m_vRenderer[j] = m_oRendererFactory.create();
						m_vRenderer[j]->setChannelCount(nSample);
						m_vRenderer[j]->setSampleCount(uint32_t(m_nElement)); // $$$
						//				m_vRenderer[j]->setSampleCount(uint32_t(m_f64TimeScale)); // $$$

						m_pRendererContext->addChannel(l_sName, v.x, v.y, v.z);
						//				m_pRendererContext->addChannel(sanitize(l_pMatrix->getDimensionLabel(0, j)));
						gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
						gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, j + 1, 1, l_sName.c_str(), -1);
					}
					gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
					gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_pChannelTreeView));

					if (m_oTypeIdentifier == OV_TypeId_Signal)
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Signal);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_Signal);
					}
					else if (m_oTypeIdentifier == OV_TypeId_Spectrum)
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Spectrum);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_Spectrum);
					}
					else
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Matrix);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_Matrix);
					}

					m_pRuler->setRenderer(l_ui32ChannelCount ? m_vRenderer[0] : nullptr);

					m_bRebuildNeeded = true;
					m_bRefreshNeeded = true;
					m_bRedrawNeeded  = true;
				}
				if (m_oMatrixDecoder.isBufferReceived())
				{
					m_time1                                 = m_time2;
					m_time2                                 = boxContext.getInputChunkEndTime(0, uint32_t(i));
					const uint64_t l_ui64InterChunkDuration = m_time2 - m_time1;
					const uint64_t l_ui64ChunkDuration      = (boxContext.getInputChunkEndTime(0, uint32_t(i)) - boxContext.getInputChunkStartTime(
																   0, uint32_t(i)));
					const uint64_t l_ui64SampleDuration = l_ui64ChunkDuration / m_nElement;
					if (m_pRendererContext->isTimeLocked())
					{
						if ((l_ui64InterChunkDuration & ~0xf) != (m_pRendererContext->getSampleDuration() & ~0xf) && l_ui64InterChunkDuration != 0
						) // 0xf mask avoids rounding errors
						{
							m_pSubRendererContext->setSampleDuration(l_ui64InterChunkDuration);
							m_pRendererContext->setSampleDuration(l_ui64InterChunkDuration);
						}
					}
					else
					{
						m_pSubRendererContext->setSampleDuration(l_ui64SampleDuration);
						m_pRendererContext->setSampleDuration(l_ui64SampleDuration);
					}

					m_pRendererContext->setSpectrumFrequencyRange(uint32_t((uint64_t(nSample) << 32) / l_ui64ChunkDuration));
					m_pRendererContext->setMinimumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin))));
					m_pRendererContext->setMaximumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax))));

					for (j = 0; j < l_ui32ChannelCount; j++)
					{
						// Feed renderer with actual samples
						for (size_t k = 0; k < nSample; k++) { m_vSwap[nSample - k - 1] = float(l_pMatrix->getBuffer()[j * nSample + k]); }
						m_vRenderer[j]->feed(&m_vSwap[0]);

						// Adjust feeding depending on theoretical dates
						if (m_pRendererContext->isTimeLocked() && m_pRendererContext->getSampleDuration())
						{
							const auto l_ui32TheoreticalSampleCount = uint32_t(m_time2 / m_pRendererContext->getSampleDuration());
							if (l_ui32TheoreticalSampleCount > m_vRenderer[j]->getHistoryCount())
							{
								m_vRenderer[j]->prefeed(l_ui32TheoreticalSampleCount - m_vRenderer[j]->getHistoryCount());
							}
						}
					}

					m_bRefreshNeeded = true;
					m_bRedrawNeeded  = true;
				}
			}

			if (nInput > 1)
			{
				for (i = 0; i < boxContext.getInputChunkCount(1); ++i)
				{
					m_oStimulationDecoder.decode(uint32_t(i));
					if (m_oStimulationDecoder.isBufferReceived())
					{
						OpenViBE::IStimulationSet* stimulationSet = m_oStimulationDecoder.getOutputStimulationSet();
						for (j = 0; j < stimulationSet->getStimulationCount(); j++)
						{
							m_vRenderer[0]->feed(stimulationSet->getStimulationDate(j), stimulationSet->getStimulationIdentifier(j));
							m_bRedrawNeeded = true;
						}
					}
				}
			}

			uint32_t rendererSampleCount = 0;
			if (m_pRendererContext->isTimeLocked())
			{
				if (0 != m_pRendererContext->getSampleDuration())
				{
					rendererSampleCount = uint32_t(m_pRendererContext->getTimeScale() / m_pRendererContext->getSampleDuration());
				}
			}
			else { rendererSampleCount = uint32_t(m_pRendererContext->getElementCount()); }

			if (rendererSampleCount != 0)
			{
				for (j = 0; j < m_vRenderer.size(); j++)
				{
					if (rendererSampleCount != m_vRenderer[j]->getSampleCount())
					{
						m_vRenderer[j]->setSampleCount(rendererSampleCount);
						m_bRebuildNeeded = true;
						m_bRefreshNeeded = true;
						m_bRedrawNeeded  = true;
					}
				}
			}

			if (m_bRebuildNeeded) { for (auto& renderer : m_vRenderer) { renderer->rebuild(*m_pSubRendererContext); } }
			if (m_bRefreshNeeded) { for (auto& renderer : m_vRenderer) { renderer->refresh(*m_pSubRendererContext); } }
			if (m_bRedrawNeeded) { this->redraw(); }

			m_bRebuildNeeded = false;
			m_bRefreshNeeded = false;
			m_bRedrawNeeded  = false;

			return true;
		}

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		void TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::draw()

		{
			CBoxAlgorithmViz::preDraw();

			if (m_pRendererContext->getSelectedCount() != 0)
			{
				glPushMatrix();
				glScalef(1, 1.f / m_pRendererContext->getSelectedCount(), 1);
				for (uint32_t i = 0; i < m_pRendererContext->getSelectedCount(); ++i)
				{
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					glPushMatrix();
					glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
					glTranslatef(0, m_pRendererContext->getSelectedCount() - i - 1.f, 0);
					if (!bHorizontalStack)
					{
						glScalef(1, -1, 1);
						glRotatef(-90, 0, 0, 1);
					}
					m_pSubRendererContext->setAspect(m_pRendererContext->getAspect());
					m_pSubRendererContext->setStackCount(m_pRendererContext->getSelectedCount());
					m_pSubRendererContext->setStackIndex(i);
					m_vRenderer[m_pRendererContext->getSelected(i)]->render(*m_pSubRendererContext);
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
	} // namespace AdvancedVisualization
} // namespace Mensia
