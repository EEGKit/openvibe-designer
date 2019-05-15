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
		template <class TRendererFactoryClass, class TRulerClass>
		class TBoxAlgorithmContinuousViz : public CBoxAlgorithmViz
		{
		public:

			TBoxAlgorithmContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter);
			bool initialize() override;
			bool uninitialize() override;
			bool process() override;

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId);

			OpenViBEToolkit::TStreamedMatrixDecoder<TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>> m_oMatrixDecoder;
			OpenViBEToolkit::TStimulationDecoder<TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>> m_oStimulationDecoder;
			TRendererFactoryClass m_oRendererFactory;
			IRenderer* m_pRenderer;

		protected:

			void draw() override;
		};

		class CBoxAlgorithmContinuousVizListener : public CBoxAlgorithmVizListener
		{
		public:

			explicit CBoxAlgorithmContinuousVizListener(const std::vector<int>& vParameter)
				: CBoxAlgorithmVizListener(vParameter) { }

			bool onInputTypeChanged(IBox& rBox, const uint32_t index) override
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(index, l_oTypeIdentifier);
				if (!this->getTypeManager().isDerivedFromStream(l_oTypeIdentifier, OV_TypeId_StreamedMatrix))
				{
					rBox.setInputType(index, OV_TypeId_StreamedMatrix);
				}
				rBox.setInputType(1, OV_TypeId_Stimulations);
				return true;
			}
		};

		template <class TRendererFactoryClass, class TRulerClass = IRuler>
		class TBoxAlgorithmContinuousVizDesc : public CBoxAlgorithmVizDesc
		{
		public:

			TBoxAlgorithmContinuousVizDesc(const OpenViBE::CString& sName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, 
										   const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion,
										   const CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription, const OpenViBE::CString& sDetailedDescription)
				: CBoxAlgorithmVizDesc(sName, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription, sDetailedDescription) { }

			OpenViBE::Plugins::IPluginObject* create() override
			{
				return new TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter);
			}

			OpenViBE::Plugins::IBoxListener* createBoxListener() const override { return new CBoxAlgorithmContinuousVizListener(m_vParameter); }

			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Advanced Visualization/") + m_sCategoryName; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId);
		};

		template <class TRendererFactoryClass, class TRulerClass>
		TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::TBoxAlgorithmContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter)
			: CBoxAlgorithmViz(rClassId, vParameter), m_pRenderer(nullptr) { }

		template <class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::initialize()

		{
			const bool l_bResult = CBoxAlgorithmViz::initialize();

			m_oMatrixDecoder.initialize(*this, 0);
			m_oStimulationDecoder.initialize(*this, 1);

			m_pRenderer = m_oRendererFactory.create();

			m_pRuler = new TRulerClass;
			m_pRuler->setRendererContext(m_pRendererContext);
			m_pRuler->setRenderer(m_pRenderer);

			return l_bResult;
		}

		template <class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::uninitialize()

		{
			m_oRendererFactory.release(m_pRenderer);
			m_pRenderer = nullptr;

			delete m_pRuler;
			m_pRuler = nullptr;

			m_oStimulationDecoder.uninitialize();
			m_oMatrixDecoder.uninitialize();

			return CBoxAlgorithmViz::uninitialize();
		}

		template <class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::process()

		{
			const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
			IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();
			uint32_t i, j;

			for (i = 0; i < l_rDynamicBoxContext.getInputChunkCount(0); ++i)
			{
				m_oMatrixDecoder.decode(i);

				OpenViBE::IMatrix* l_pMatrix = m_oMatrixDecoder.getOutputMatrix();
				uint32_t channelCount = l_pMatrix->getDimensionSize(0);
				uint32_t sampleCount = l_pMatrix->getDimensionSize(1);

				if (channelCount == 0)
				{
					this->getLogManager() << LogLevel_Error << "Input stream " << static_cast<uint32_t>(i) << " has 0 channels\n";
					return false;
				}

				if (l_pMatrix->getDimensionCount() == 1)
				{
					channelCount = l_pMatrix->getDimensionSize(0);
					sampleCount = 1;
				}

				if (m_oMatrixDecoder.isHeaderReceived())
				{
					GtkTreeIter l_oGtkTreeIterator;
					gtk_list_store_clear(m_pChannelListStore);

					m_vSwap.resize(size_t(channelCount));

					m_pRendererContext->clear();
					m_pRendererContext->setTranslucency(float(m_translucency));
					m_pRendererContext->setFlowerRingCount(m_flowerRingCount);
					m_pRendererContext->setTimeScale(m_timeScale);
					m_pRendererContext->setElementCount(m_elementCount);
					m_pRendererContext->scaleBy(float(m_f64DataScale));
					m_pRendererContext->setAxisDisplay(m_bShowAxis);
					m_pRendererContext->setPositiveOnly(m_bIsPositive);
					m_pRendererContext->setParentRendererContext(&getContext());
					m_pRendererContext->setTimeLocked(m_bIsTimeLocked);
					m_pRendererContext->setXYZPlotDepth(m_bXYZPlotHasDepth);

					gtk_tree_view_set_model(m_pChannelTreeView, nullptr);
					for (j = 0; j < channelCount; j++)
					{
						std::string l_sName = trim(l_pMatrix->getDimensionLabel(0, j));
						std::string l_sSubname = l_sName;
						std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), tolower);
						const CVertex v = m_vChannelLocalisation[l_sSubname];

						if (l_sName.empty())
						{
							char l_sIndexedChannelName[1024];
							sprintf(l_sIndexedChannelName, "Channel %u", j + 1);
							l_sName = l_sIndexedChannelName;
						}

						m_pRendererContext->addChannel(l_sName, v.x, v.y, v.z);
						gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
						gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, j + 1, 1, l_sName.c_str(), -1);
					}
					gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
					gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_pChannelTreeView));

					m_pRenderer->setChannelCount(channelCount);

					if (m_oTypeIdentifier == OV_TypeId_Signal)
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Signal);
					}
					else if (m_oTypeIdentifier == OV_TypeId_Spectrum)
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Spectrum);
					}
					else
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Matrix);
					}

					if (sampleCount != 1)
					{
						bool l_bWarned = false;
						if (m_oTypeIdentifier == OV_TypeId_Spectrum)
						{
							l_bWarned = true;
							this->getLogManager() << LogLevel_Warning << "Input matrix has 'spectrum' type\n";
							this->getLogManager() << LogLevel_Warning << "Such configuration is uncommon for a 'continous' kind of visualization !\n";
							this->getLogManager() << LogLevel_Warning << "You might want to consider the 'stacked' kind of visualization for time/frequency analysis for instance\n";
							this->getLogManager() << LogLevel_Warning << "Please double check your scenario\n";
						}
						else
						{
							if (!m_pRendererContext->isTimeLocked())
							{
								l_bWarned = true;
								this->getLogManager() << LogLevel_Warning << "Input matrix has " << static_cast<uint32_t>(sampleCount) << " elements and the box settings say the elements are independant with " << uint64_t(m_elementCount) << " elements to render\n";
								this->getLogManager() << LogLevel_Warning << "Such configuration is uncommon for a 'continous' kind of visualization !\n";
								this->getLogManager() << LogLevel_Warning << "You might want either of the following alternative :\n";
								this->getLogManager() << LogLevel_Warning << " - an 'instant' kind of visualization to highlight the " << uint64_t(m_elementCount) << " elements of the matrix\n";
								this->getLogManager() << LogLevel_Warning << " - a 'time locked' kind of elements (thus the scenario must refresh the matrix on a regular basis)\n";
								this->getLogManager() << LogLevel_Warning << "Please double check your scenario and box settings\n";
							}
						}
					}

					m_bRebuildNeeded = true;
					m_bRefreshNeeded = true;
					m_bRedrawNeeded = true;
				}
				if (m_oMatrixDecoder.isBufferReceived())
				{
					m_time1 = m_time2;
					m_time2 = l_rDynamicBoxContext.getInputChunkEndTime(0, i);
					const uint64_t l_ui64SampleDuration = (m_time2 - m_time1) / sampleCount;
					if ((l_ui64SampleDuration & ~0xf) != (m_pRendererContext->getSampleDuration() & ~0xf) && l_ui64SampleDuration != 0) // 0xf mask avoids rounding errors
					{
						m_pRendererContext->setSampleDuration(l_ui64SampleDuration);
					}
					m_pRendererContext->setSpectrumFrequencyRange(uint32_t((uint64_t(sampleCount) << 32) / (l_rDynamicBoxContext.getInputChunkEndTime(0, i) - l_rDynamicBoxContext.getInputChunkStartTime(0, i))));
					m_pRendererContext->setMinimumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin))));
					m_pRendererContext->setMaximumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax))));

					// Feed renderer with actual samples
					for (j = 0; j < sampleCount; j++)
					{
						for (uint32_t k = 0; k < channelCount; k++)
						{
							m_vSwap[k] = float(l_pMatrix->getBuffer()[k * sampleCount + j]);
						}
						m_pRenderer->feed(&m_vSwap[0]);
					}

					// Adjust feeding depending on theoretical dates
					if (m_pRendererContext->isTimeLocked() && m_pRendererContext->getSampleDuration())
					{
						auto l_ui32TheoreticalSampleCount = uint32_t(m_time2 / m_pRendererContext->getSampleDuration());
						if (l_ui32TheoreticalSampleCount > m_pRenderer->getHistoryCount())
						{
							m_pRenderer->prefeed(l_ui32TheoreticalSampleCount - m_pRenderer->getHistoryCount());
						}
					}

					m_bRefreshNeeded = true;
					m_bRedrawNeeded = true;
				}
			}

			if (l_rStaticBoxContext.getInputCount() > 1)
			{
				for (i = 0; i < l_rDynamicBoxContext.getInputChunkCount(1); ++i)
				{
					m_oStimulationDecoder.decode(i);
					if (m_oStimulationDecoder.isBufferReceived())
					{
						OpenViBE::IStimulationSet* l_pStimulationSet = m_oStimulationDecoder.getOutputStimulationSet();
						for (j = 0; j < l_pStimulationSet->getStimulationCount(); j++)
						{
							m_pRenderer->feed(l_pStimulationSet->getStimulationDate(j), l_pStimulationSet->getStimulationIdentifier(j));
							m_bRedrawNeeded = true;
						}
					}
				}
			}

			uint32_t l_ui32RendererSampleCount = 0;
			if (m_pRendererContext->isTimeLocked())
			{
				if (0 != m_pRendererContext->getSampleDuration())
				{
					l_ui32RendererSampleCount = uint32_t(m_pRendererContext->getTimeScale() / m_pRendererContext->getSampleDuration());
				}
			}
			else
			{
				l_ui32RendererSampleCount = static_cast<uint32_t>(m_pRendererContext->getElementCount()); // *l_ui32SampleCount;
			}

			if (l_ui32RendererSampleCount != 0 && l_ui32RendererSampleCount != m_pRenderer->getSampleCount())
			{
				m_pRenderer->setSampleCount(l_ui32RendererSampleCount);
				m_bRebuildNeeded = true;
				m_bRefreshNeeded = true;
				m_bRedrawNeeded = true;
			}

			if (m_bRebuildNeeded) m_pRenderer->rebuild(*m_pRendererContext);
			if (m_bRefreshNeeded) m_pRenderer->refresh(*m_pRendererContext);
			if (m_bRedrawNeeded) { this->redraw(); }

			m_bRebuildNeeded = false;
			m_bRefreshNeeded = false;
			m_bRedrawNeeded = false;

			return true;
		}

		template <class TRendererFactoryClass, class TRulerClass>
		void TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::draw()

		{
			CBoxAlgorithmViz::preDraw();

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
			m_pRenderer->render(*m_pRendererContext);
			glPopAttrib();

			CBoxAlgorithmViz::postDraw();
		}
	}  // namespace AdvancedVisualization
} // namespace Mensia
