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

using namespace OpenViBE::Kernel;

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		class TBoxAlgorithmStackedInstantViz : public CBoxAlgorithmViz
		{
		public:

			TBoxAlgorithmStackedInstantViz(const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter);
			bool initialize() override;
			bool uninitialize() override;
			bool process() override;

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId)

			OpenViBEToolkit::TStimulationDecoder<TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>> m_oStimulationDecoder;
			OpenViBEToolkit::TStreamedMatrixDecoder<TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>> m_oMatrixDecoder;

			TRendererFactoryClass m_oRendererFactory;
			std::vector<IRenderer*> m_vRenderer;

		protected:

			void draw() override;
		};

		class CBoxAlgorithmStackedInstantVizListener final : public CBoxAlgorithmVizListener
		{
		public:

			CBoxAlgorithmStackedInstantVizListener(const std::vector<int>& vParameter) : CBoxAlgorithmVizListener(vParameter) { }

			bool onInputTypeChanged(IBox& box, const uint32_t index) override
			{
				OpenViBE::CIdentifier typeID = OV_UndefinedIdentifier;
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

			TBoxAlgorithmStackedInstantVizDesc(const OpenViBE::CString& name, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId,
											   const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion,
											   const CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription,
											   const OpenViBE::CString& sDetailedDescription)
				: CBoxAlgorithmVizDesc(name, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription,
									   sDetailedDescription) { }

			OpenViBE::Plugins::IPluginObject* create() override
			{
				return new TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter);
			}

			OpenViBE::Plugins::IBoxListener* createBoxListener() const override { return new CBoxAlgorithmStackedInstantVizListener(m_vParameter); }

			OpenViBE::CString getCategory() const override { return OpenViBE::CString("Advanced Visualization/") + m_sCategoryName; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId)
		};


		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::TBoxAlgorithmStackedInstantViz(
			const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter)
			: CBoxAlgorithmViz(rClassId, vParameter) { }

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::initialize()

		{
			const bool res = CBoxAlgorithmViz::initialize();

			m_oMatrixDecoder.initialize(*this, 0);
			m_oStimulationDecoder.initialize(*this, 1);

			m_pRendererContext->clear();
			m_pRendererContext->setTranslucency(float(m_translucency));
			m_pRendererContext->scaleBy(float(m_f64DataScale));
			m_pRendererContext->setPositiveOnly(true);
			m_pRendererContext->setAxisDisplay(m_bShowAxis);
			m_pRendererContext->setParentRendererContext(&getContext());

			m_pSubRendererContext->clear();
			m_pSubRendererContext->setParentRendererContext(m_pRendererContext);

			m_pRuler = new TRulerClass;
			m_pRuler->setRendererContext(m_pRendererContext);

			OpenViBE::CMatrix gradientMatrix;
			OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(gradientMatrix, m_sColorGradient);
			for (uint32_t step = 0; step < gradientMatrix.getDimensionSize(1); ++step)
			{
				const double currentStepValue            = gradientMatrix.getBuffer()[4 * step + 0];
				gradientMatrix.getBuffer()[4 * step + 0] = (currentStepValue / 100.0) * 50.0 + 50.0;
			}
			OpenViBEVisualizationToolkit::Tools::ColorGradient::format(m_sColorGradient, gradientMatrix);

			return res;
		}

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::uninitialize()

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

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::process()

		{
			const IBox& staticBoxContext = this->getStaticBoxContext();
			IBoxIO& dynamicBoxContext    = this->getDynamicBoxContext();

			for (uint32_t chunk = 0; chunk < dynamicBoxContext.getInputChunkCount(0); ++chunk)
			{
				m_oMatrixDecoder.decode(chunk);

				OpenViBE::IMatrix* inputMatrix = m_oMatrixDecoder.getOutputMatrix();
				const uint32_t nChannel    = inputMatrix->getDimensionSize(0);

				if (nChannel == 0)
				{
					this->getLogManager() << LogLevel_Error << "Input stream " << uint32_t(chunk) << " has 0 channels\n";
					return false;
				}

				if (m_oMatrixDecoder.isHeaderReceived())
				{
					for (auto renderer : m_vRenderer) { m_oRendererFactory.release(renderer); }
					m_vRenderer.clear();
					m_vRenderer.resize(nChannel);

					m_pSubRendererContext->clear();
					m_pSubRendererContext->setParentRendererContext(m_pRendererContext);
					m_pSubRendererContext->setTimeLocked(false);
					m_pSubRendererContext->setStackCount(nChannel);
					m_pSubRendererContext->setPositiveOnly(true);


					m_pRendererContext->clear();
					m_pRendererContext->setTranslucency(float(m_translucency));
					m_pRendererContext->setTimeScale(1);
					m_pRendererContext->scaleBy(float(m_f64DataScale));
					m_pRendererContext->setParentRendererContext(&getContext());
					m_pRendererContext->setTimeLocked(false);
					m_pRendererContext->setXYZPlotDepth(false);
					m_pRendererContext->setPositiveOnly(true);

					if (m_typeID == OV_TypeId_TimeFrequency)
					{
						GtkTreeIter l_oGtkTreeIterator;
						gtk_list_store_clear(m_pChannelListStore);

						const uint32_t frequencyCount = inputMatrix->getDimensionSize(1);
						const uint32_t nSample    = inputMatrix->getDimensionSize(2);

						// I do not know what this is for...
						for (uint32_t frequency = 0; frequency < frequencyCount; ++frequency)
						{
							try
							{
								const double frequencyValue = std::stod(inputMatrix->getDimensionLabel(1, frequency), nullptr);
								const int stringSize        = snprintf(nullptr, 0, "%.2f", frequencyValue) + 1;
								if (stringSize > 0)
								{
									std::unique_ptr<char[]> buffer(new char[stringSize]);
									snprintf(buffer.get(), size_t(stringSize), "%.2f", frequencyValue);
									m_pRendererContext->setDimensionLabel(1, frequencyCount - frequency - 1, buffer.get());
								}
							}
							catch (...) { m_pRendererContext->setDimensionLabel(1, frequencyCount - frequency - 1, "NaN"); }
							m_pSubRendererContext->addChannel("", 0, 0, 0);
						}


						m_pRendererContext->setDataType(IRendererContext::DataType_TimeFrequency);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_TimeFrequency);

						m_pRendererContext->setElementCount(nSample);
						m_pSubRendererContext->setElementCount(nSample);
						gtk_tree_view_set_model(m_pChannelTreeView, nullptr);

						for (uint32_t channel = 0; channel < nChannel; ++channel)
						{
							std::string channelName          = trim(inputMatrix->getDimensionLabel(0, channel));
							std::string lowercaseChannelName = channelName;
							std::transform(channelName.begin(), channelName.end(), lowercaseChannelName.begin(), tolower);
							const CVertex v = m_vChannelLocalisation[lowercaseChannelName];

							if (channelName.empty())
							{
								char indexedChannelName[1024];
								sprintf(indexedChannelName, "Channel %u", channel + 1);
								channelName = indexedChannelName;
							}

							m_vRenderer[channel] = m_oRendererFactory.create();

							// The channels in the sub-renderer are the frequencies in the spectrum
							m_vRenderer[channel]->setChannelCount(frequencyCount);
							m_vRenderer[channel]->setSampleCount(nSample);

							m_pRendererContext->addChannel(channelName, v.x, v.y, v.z);
							gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
							gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, channel + 1, 1, channelName.c_str(), -1);
						}
						gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
						gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_pChannelTreeView));
					}
					else
					{
						this->getLogManager() << LogLevel_Error << "Input stream type is not supported\n";
						return false;
					}

					m_pRuler->setRenderer(nChannel ? m_vRenderer[0] : nullptr);

					m_bRebuildNeeded = true;
					m_bRefreshNeeded = true;
					m_bRedrawNeeded  = true;
				}

				if (m_oMatrixDecoder.isBufferReceived())
				{
					if (m_typeID == OV_TypeId_TimeFrequency)
					{
						m_time1 = m_time2;
						m_time2 = dynamicBoxContext.getInputChunkEndTime(0, chunk);

						const uint32_t frequencyCount = inputMatrix->getDimensionSize(1);
						const uint32_t nSample    = inputMatrix->getDimensionSize(2);

						const uint64_t chunkDuration  = dynamicBoxContext.getInputChunkEndTime(0, chunk) - dynamicBoxContext.getInputChunkStartTime(0, chunk);
						const uint64_t sampleDuration = chunkDuration / nSample;

						m_pSubRendererContext->setSampleDuration(sampleDuration);
						m_pRendererContext->setSampleDuration(sampleDuration);

						for (uint32_t channel = 0; channel < nChannel; ++channel)
						{
							// Feed renderer with actual samples
							for (uint32_t sample = 0; sample < nSample; ++sample)
							{
								m_vSwap.resize(frequencyCount);
								for (uint32_t frequency = 0; frequency < frequencyCount; ++frequency)
								{
									m_vSwap[frequencyCount - frequency - 1] = float(
										inputMatrix->getBuffer()[sample + frequency * nSample + channel * nSample * frequencyCount]);
								}
								m_vRenderer[channel]->feed(&m_vSwap[0]);
							}
						}

						m_bRefreshNeeded = true;
						m_bRedrawNeeded  = true;
					}
				}
			}

			if (staticBoxContext.getInputCount() > 1)
			{
				for (uint32_t i = 0; i < dynamicBoxContext.getInputChunkCount(1); ++i)
				{
					m_oStimulationDecoder.decode(i);
					if (m_oStimulationDecoder.isBufferReceived())
					{
						OpenViBE::IStimulationSet* stimSet = m_oStimulationDecoder.getOutputStimulationSet();
						for (size_t j = 0; j < stimSet->getStimulationCount(); ++j)
						{
							m_vRenderer[0]->feed(stimSet->getStimulationDate(j), stimSet->getStimulationIdentifier(j));
							m_bRedrawNeeded = true;
						}
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

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		void TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::draw()

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
