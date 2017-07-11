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

			TBoxAlgorithmStackedInstantViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter);
			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool process(void);

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId);

			OpenViBEToolkit::TStimulationDecoder < TBoxAlgorithmStackedInstantViz < bDrawBorders, TRendererFactoryClass, TRulerClass > > m_oStimulationDecoder;
			OpenViBEToolkit::TStreamedMatrixDecoder < TBoxAlgorithmStackedInstantViz < bDrawBorders, TRendererFactoryClass, TRulerClass > > m_oMatrixDecoder;

			TRendererFactoryClass m_oRendererFactory;
			std::vector < IRenderer* > m_vRenderer;

		protected:

			virtual void draw(void);
		};

		class CBoxAlgorithmStackedInstantVizListener : public CBoxAlgorithmVizListener
		{
		public:

			CBoxAlgorithmStackedInstantVizListener(const std::vector < int >& vParameter)
				:CBoxAlgorithmVizListener(vParameter)
			{
			}

			virtual bool onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index)
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(ui32Index, l_oTypeIdentifier);
				if(!this->getTypeManager().isDerivedFromStream(l_oTypeIdentifier, OV_TypeId_TimeFrequency))
				{
					rBox.setInputType(ui32Index, OV_TypeId_TimeFrequency);
				}
				rBox.setInputType(1, OV_TypeId_Stimulations);
				return true;
			}
		};

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass=IRuler>
		class TBoxAlgorithmStackedInstantVizDesc : public CBoxAlgorithmVizDesc
		{
		public:

			TBoxAlgorithmStackedInstantVizDesc(const OpenViBE::CString& sName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, const Mensia::AdvancedVisualization::CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription, const OpenViBE::CString& sDetailedDescription)
				:CBoxAlgorithmVizDesc(sName, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription, sDetailedDescription)
			{
			}

			virtual OpenViBE::Plugins::IPluginObject* create(void)
			{
				return new Mensia::AdvancedVisualization::TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter);
			}

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const
			{
				return new CBoxAlgorithmStackedInstantVizListener(m_vParameter);
			}

			virtual OpenViBE::CString getCategory(void) const
			{
				return OpenViBE::CString("Advanced Visualization/")+m_sCategoryName;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId);
		};


		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::TBoxAlgorithmStackedInstantViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
			:CBoxAlgorithmViz(rClassId, vParameter)
		{
		}

		template < bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::initialize(void)
		{
			bool l_bResult=CBoxAlgorithmViz::initialize();

			m_oMatrixDecoder.initialize(*this, 0);
			m_oStimulationDecoder.initialize(*this, 1);

			m_pRendererContext->clear();
			m_pRendererContext->setTranslucency(float(m_f64Translucency));
			m_pRendererContext->scaleBy(float(m_f64DataScale));
			m_pRendererContext->setPositiveOnly(true);
			m_pRendererContext->setAxisDisplay(m_bShowAxis);
			m_pRendererContext->setParentRendererContext(&getContext());

			m_pSubRendererContext->clear();
			m_pSubRendererContext->setParentRendererContext(m_pRendererContext);

			m_pRuler=new TRulerClass;
			m_pRuler->setRendererContext(m_pRendererContext);

			OpenViBE::CMatrix gradientMatrix;
			OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(gradientMatrix, m_sColorGradient);
			for (unsigned int step = 0; step < gradientMatrix.getDimensionSize(1); ++step)
			{
				double currentStepValue = gradientMatrix.getBuffer()[4 * step + 0];
				gradientMatrix.getBuffer()[4 * step + 0] = (currentStepValue / 100.0) * 50.0 + 50.0;
			}
			OpenViBEVisualizationToolkit::Tools::ColorGradient::format(m_sColorGradient, gradientMatrix);

			return l_bResult;
		}

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::uninitialize(void)
		{
			for(uint32_t i=0; i<m_vRenderer.size(); i++)
			{
				m_oRendererFactory.release(m_vRenderer[i]);
			}
			m_vRenderer.clear();

			IRendererContext::release(m_pSubRendererContext);
			m_pSubRendererContext=NULL;

			IRendererContext::release(m_pRendererContext);
			m_pRendererContext=NULL;

			delete m_pRuler;
			m_pRuler=NULL;

			m_oStimulationDecoder.uninitialize();
			m_oMatrixDecoder.uninitialize();

			return CBoxAlgorithmViz::uninitialize();
		}

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::process(void)
		{
			const OpenViBE::Kernel::IBox& staticBoxContext=this->getStaticBoxContext();
			OpenViBE::Kernel::IBoxIO& dynamicBoxContext=this->getDynamicBoxContext();
			uint32_t i, j;

			for(uint32_t chunk = 0; chunk < dynamicBoxContext.getInputChunkCount(0); chunk++)
			{
				m_oMatrixDecoder.decode(chunk);

				OpenViBE::IMatrix* inputMatrix = m_oMatrixDecoder.getOutputMatrix();
				uint32_t channelCount = inputMatrix->getDimensionSize(0);

				if (channelCount == 0)
				{
					this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Input stream " << static_cast<OpenViBE::uint32>(chunk) << " has 0 channels\n";
					return false;
				}

				if (m_oMatrixDecoder.isHeaderReceived())
				{
					for (auto renderer : m_vRenderer)
					{
						m_oRendererFactory.release(renderer);
					}
					m_vRenderer.clear();
					m_vRenderer.resize(channelCount);

					m_pSubRendererContext->clear();
					m_pSubRendererContext->setParentRendererContext(m_pRendererContext);
					m_pSubRendererContext->setTimeLocked(false);
					m_pSubRendererContext->setStackCount(channelCount);
					m_pSubRendererContext->setPositiveOnly(true);


					m_pRendererContext->clear();
					m_pRendererContext->setTranslucency(float(m_f64Translucency));
					m_pRendererContext->setTimeScale(1);
					m_pRendererContext->scaleBy(float(m_f64DataScale));
					m_pRendererContext->setParentRendererContext(&getContext());
					m_pRendererContext->setTimeLocked(false);
					m_pRendererContext->setXYZPlotDepth(false);
					m_pRendererContext->setPositiveOnly(true);

					if (m_oTypeIdentifier == OV_TypeId_TimeFrequency)
					{
						::GtkTreeIter l_oGtkTreeIterator;
						::gtk_list_store_clear(m_pChannelListStore);

						uint32_t frequencyCount = inputMatrix->getDimensionSize(1);
						uint32_t sampleCount = inputMatrix->getDimensionSize(2);

						// I do not know what this is for...
						for (uint32_t frequency = 0; frequency < frequencyCount; frequency++)
						{
							try {
								double frequencyValue = std::stod(inputMatrix->getDimensionLabel(1, frequency), nullptr);
								int stringSize = snprintf(nullptr, 0, "%.2f", frequencyValue) + 1;
								if (stringSize > 0)
								{
									std::unique_ptr<char[]> buffer(new char[stringSize]);
									snprintf(buffer.get(), static_cast<size_t>(stringSize), "%.2f", frequencyValue);
									m_pRendererContext->setDimensionLabel(1, frequencyCount - frequency - 1, buffer.get());
								}
							} catch (...) {
								m_pRendererContext->setDimensionLabel(1, frequencyCount - frequency - 1, "NaN");
							}
							m_pSubRendererContext->addChannel("", 0, 0, 0);
						}


						m_pRendererContext->setDataType(IRendererContext::DataType_TimeFrequency);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_TimeFrequency);

						m_pRendererContext->setElementCount(sampleCount);
						m_pSubRendererContext->setElementCount(sampleCount);
						::gtk_tree_view_set_model(m_pChannelTreeView, NULL);

						for (uint32_t channel = 0; channel < channelCount; channel++)
						{
							std::string channelName = trim(inputMatrix->getDimensionLabel(0, channel));
							std::string lowercaseChannelName = channelName;
							std::transform(channelName.begin(), channelName.end(), lowercaseChannelName.begin(), ::tolower);
							CVertex v = m_vChannelLocalisation[lowercaseChannelName];

							if (channelName == "")
							{
								char indexedChannelName[1024];
								::sprintf(indexedChannelName, "Channel %u", channel + 1);
								channelName = indexedChannelName;
							}

							m_vRenderer[channel] = m_oRendererFactory.create();

							// The channels in the sub-renderer are the frequencies in the spectrum
							m_vRenderer[channel]->setChannelCount(frequencyCount);
							m_vRenderer[channel]->setSampleCount(sampleCount);

							m_pRendererContext->addChannel(channelName, v.x, v.y, v.z);
							::gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
							::gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, channel+1, 1, channelName.c_str(), -1);
						}
						::gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
						::gtk_tree_selection_select_all(::gtk_tree_view_get_selection(m_pChannelTreeView));
					}
					else
					{
						this->getLogManager() << LogLevel_Error << "Input stream type is not supported\n";
						return false;
					}

					m_pRuler->setRenderer(channelCount ? m_vRenderer[0] : nullptr);

					m_bRebuildNeeded = true;
					m_bRefreshNeeded = true;
					m_bRedrawNeeded = true;
				}

				if(m_oMatrixDecoder.isBufferReceived())
				{
					if (m_oTypeIdentifier == OV_TypeId_TimeFrequency)
					{
						m_ui64Time1 = m_ui64Time2;
						m_ui64Time2 = dynamicBoxContext.getInputChunkEndTime(0, chunk);

						uint32_t frequencyCount = inputMatrix->getDimensionSize(1);
						uint32_t sampleCount = inputMatrix->getDimensionSize(2);

						uint64_t chunkDuration = dynamicBoxContext.getInputChunkEndTime(0, chunk) - dynamicBoxContext.getInputChunkStartTime(0, chunk);
						uint64_t sampleDuration = chunkDuration / sampleCount;

						m_pSubRendererContext->setSampleDuration(sampleDuration);
						m_pRendererContext->setSampleDuration(sampleDuration);

						for (uint32_t channel = 0; channel < channelCount; channel++)
						{
							// Feed renderer with actual samples
							for (uint32_t sample = 0; sample < sampleCount; sample++)
							{
								m_vSwap.resize(frequencyCount);
								for (uint32_t frequency = 0; frequency < frequencyCount; frequency++)
								{
									m_vSwap[frequencyCount - frequency - 1] = static_cast<float>(inputMatrix->getBuffer()[sample + frequency * sampleCount + channel * sampleCount * frequencyCount]);
								}
								m_vRenderer[channel]->feed(&m_vSwap[0]);
							}
						}

						m_bRefreshNeeded=true;
						m_bRedrawNeeded=true;
					}
				}
			}

			if(staticBoxContext.getInputCount() > 1)
			{
				for(i=0; i<dynamicBoxContext.getInputChunkCount(1); i++)
				{
					m_oStimulationDecoder.decode(i);
					if(m_oStimulationDecoder.isBufferReceived())
					{
						OpenViBE::IStimulationSet* l_pStimulationSet=m_oStimulationDecoder.getOutputStimulationSet();
						for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
						{
							m_vRenderer[0]->feed(l_pStimulationSet->getStimulationDate(j), l_pStimulationSet->getStimulationIdentifier(j));
							m_bRedrawNeeded=true;
						}
					}
				}
			}

			if(m_bRebuildNeeded)
			{
				for (auto& renderer : m_vRenderer)
				{
					renderer->rebuild(*m_pSubRendererContext);
				}
			}
			if(m_bRefreshNeeded)
			{
				for (auto& renderer : m_vRenderer)
				{
					renderer->refresh(*m_pSubRendererContext);
				}
			}
			if(m_bRedrawNeeded) this->redraw();

			m_bRebuildNeeded=false;
			m_bRefreshNeeded=false;
			m_bRedrawNeeded=false;

			return true;
		}

		template <bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		void TBoxAlgorithmStackedInstantViz<bDrawBorders, TRendererFactoryClass, TRulerClass>::draw(void)
		{
			CBoxAlgorithmViz::preDraw();

			uint32_t i;

			if(m_pRendererContext->getSelectedCount()!=0)
			{
				::glPushMatrix();
				::glScalef(1, 1.f/m_pRendererContext->getSelectedCount(), 1);
				for(i=0; i<m_pRendererContext->getSelectedCount(); i++)
				{
					::glPushAttrib(GL_ALL_ATTRIB_BITS);
					::glPushMatrix();
					::glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
					::glTranslatef(0, m_pRendererContext->getSelectedCount()-i-1.f, 0);

					m_pSubRendererContext->setAspect(m_pRendererContext->getAspect());
					m_pSubRendererContext->setStackCount(m_pRendererContext->getSelectedCount());
					m_pSubRendererContext->setStackIndex(i);
					m_vRenderer[m_pRendererContext->getSelected(i)]->render(*m_pSubRendererContext);
					if(bDrawBorders)
					{
						::glDisable(GL_TEXTURE_1D);
						::glDisable(GL_BLEND);
						::glColor3f(0, 0, 0);
						::glBegin(GL_LINE_LOOP);
						::glVertex2f(0, 0);
						::glVertex2f(1, 0);
						::glVertex2f(1, 1);
						::glVertex2f(0, 1);
						::glEnd();
					}
					::glPopMatrix();
					::glPopAttrib();
				}
				::glPopMatrix();
			}

			CBoxAlgorithmViz::postDraw();
		}

	}
}

