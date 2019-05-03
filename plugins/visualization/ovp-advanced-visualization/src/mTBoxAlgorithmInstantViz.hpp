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
		class TBoxAlgorithmInstantViz : public CBoxAlgorithmViz
		{
		public:

			TBoxAlgorithmInstantViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter);
			virtual bool initialize();
			virtual bool uninitialize();
			virtual bool process();

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId);

			TRendererFactoryClass m_oRendererFactory;

			uint32_t m_ui32InputCount;
			std::vector < IRenderer* > m_vRenderer;
			std::vector < OpenViBEToolkit::TStreamedMatrixDecoder < TBoxAlgorithmInstantViz <TRendererFactoryClass, TRulerClass> > > m_vMatrixDecoder;

			double m_dLastERPFraction;

		protected:

			virtual void draw();
		};

		class CBoxAlgorithmInstantVizListener : public CBoxAlgorithmVizListener
		{
		public:

			CBoxAlgorithmInstantVizListener(const std::vector < int >& vParameter)
				:CBoxAlgorithmVizListener(vParameter) { }

			virtual bool onInputTypeChanged(IBox& rBox, const uint32_t ui32Index)
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(ui32Index, l_oTypeIdentifier);
				if (!this->getTypeManager().isDerivedFromStream(l_oTypeIdentifier, OV_TypeId_StreamedMatrix))
				{
					rBox.setInputType(ui32Index, OV_TypeId_StreamedMatrix);
				}
				else
				{
					for (uint32_t i = 0; i < rBox.getInputCount(); i++)
					{
						rBox.setInputType(i, l_oTypeIdentifier);
					}
				}
				return true;
			}

			virtual bool onInputAdded(IBox& rBox, const uint32_t ui32Index)
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(0, l_oTypeIdentifier);
				rBox.setInputType(ui32Index, l_oTypeIdentifier);
				rBox.setInputName(ui32Index, "Matrix");
				rBox.addSetting("Color", OV_TypeId_Color, "${AdvancedViz_DefaultColor}");
				return true;
			}

			virtual bool onInputRemoved(IBox& rBox, const uint32_t ui32Index)
			{
				rBox.removeSetting(this->getBaseSettingCount() + ui32Index - 1);
				return true;
			}
		};

		template <class TRendererFactoryClass, class TRulerClass = IRuler, template < typename, typename > class TBoxAlgorithm = TBoxAlgorithmInstantViz>
		class TBoxAlgorithmInstantVizDesc : public CBoxAlgorithmVizDesc
		{
		public:

			TBoxAlgorithmInstantVizDesc(const OpenViBE::CString& sName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, const CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription, const OpenViBE::CString& sDetailedDescription)
				:CBoxAlgorithmVizDesc(sName, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription, sDetailedDescription) { }

			virtual OpenViBE::Plugins::IPluginObject* create()
 { return new TBoxAlgorithm<TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter); }

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener() const { return new CBoxAlgorithmInstantVizListener(m_vParameter); }

			virtual OpenViBE::CString getCategory() const { return OpenViBE::CString("Advanced Visualization/") + m_sCategoryName; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId);
		};


		template <class TRendererFactoryClass, class TRulerClass>
		TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::TBoxAlgorithmInstantViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
			:CBoxAlgorithmViz(rClassId, vParameter) { }

		template <class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::initialize()

		{
			bool l_bResult = CBoxAlgorithmViz::initialize();

			m_dLastERPFraction = 0;

			const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
			m_ui32InputCount = l_rStaticBoxContext.getInputCount();
			m_vRenderer.resize(m_ui32InputCount);
			m_vMatrixDecoder.resize(m_ui32InputCount);
			for (uint32_t i = 0; i < m_ui32InputCount; i++)
			{
				m_vRenderer[i] = m_oRendererFactory.create();
				m_vMatrixDecoder[i].initialize(*this, i);
				if (!m_vRenderer[i])
				{
					this->getLogManager() << LogLevel_Error << "Could not create renderer, it might have been disabled at compile time\n";
					l_bResult = false;
				}
			}

			m_pRuler = new TRulerClass;
			m_pRuler->setRendererContext(m_pRendererContext);
			m_pRuler->setRenderer(m_vRenderer[0]);

			gtk_widget_set_sensitive(m_pTimeScale, false);

			return l_bResult;
		}

		template <class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::uninitialize()

		{
			for (uint32_t i = 0; i < m_ui32InputCount; i++)
			{
				m_oRendererFactory.release(m_vRenderer[i]);
				m_vMatrixDecoder[i].uninitialize();
			}

			m_vMatrixDecoder.clear();
			m_vRenderer.clear();

			return CBoxAlgorithmViz::uninitialize();
		}

		template <class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::process()

		{
			const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
			IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();
			uint32_t i, j, k, l;

			for (i = 0; i < l_rStaticBoxContext.getInputCount(); i++)
			{
				for (j = 0; j < l_rDynamicBoxContext.getInputChunkCount(i); j++)
				{
					m_vMatrixDecoder[i].decode(j);

					OpenViBE::IMatrix* l_pMatrix = m_vMatrixDecoder[i].getOutputMatrix();
					uint32_t l_ui32ChannelCount = l_pMatrix->getDimensionSize(0);
					uint32_t l_ui32SampleCount = l_pMatrix->getDimensionSize(1);

					if (l_ui32ChannelCount == 0)
					{
						this->getLogManager() << LogLevel_Error << "Input stream " << static_cast<uint32_t>(i) << " has 0 channels\n";
						return false;
					}

					if (l_pMatrix->getDimensionCount() == 1)
					{
						l_ui32ChannelCount = 1;
						l_ui32SampleCount = l_pMatrix->getDimensionSize(0);
					}

					if (m_vMatrixDecoder[i].isHeaderReceived())
					{
						// TODO
						// Check dimension coherence
						// Only apply renderer context when first header is received

						GtkTreeIter l_oGtkTreeIterator;
						gtk_list_store_clear(m_pChannelListStore);

						m_vSwap.resize(l_ui32ChannelCount);

						m_pRendererContext->clear();
						m_pRendererContext->setTranslucency(float(m_f64Translucency));
						m_pRendererContext->setFlowerRingCount(m_ui64FlowerRingCount);
						//				m_pRendererContext->setTimeScale(1); // Won't be used
						m_pRendererContext->scaleBy(float(m_f64DataScale));
						m_pRendererContext->setPositiveOnly(m_bIsPositive);
						m_pRendererContext->setAxisDisplay(m_bShowAxis);
						m_pRendererContext->setParentRendererContext(&getContext());
						m_pRendererContext->setXYZPlotDepth(m_bXYZPlotHasDepth);

						gtk_tree_view_set_model(m_pChannelTreeView, nullptr);
						for (j = 0; j < l_ui32ChannelCount; j++)
						{
							std::string l_sName = trim(l_pMatrix->getDimensionLabel(0, j));
							std::string l_sSubname = l_sName;
							std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), tolower);
							CVertex v = m_vChannelLocalisation[l_sSubname];

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

						m_vRenderer[i]->setChannelCount(l_ui32ChannelCount);
						m_vRenderer[i]->setSampleCount(l_ui32SampleCount);

						if (l_ui32SampleCount > 1 && m_oTypeIdentifier != OV_TypeId_Spectrum)
						{
							gtk_widget_show(m_pERPPlayer);
						}

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

						m_bRebuildNeeded = true;
						m_bRefreshNeeded = true;
						m_bRedrawNeeded = true;
					}
					if (m_vMatrixDecoder[i].isBufferReceived())
					{
						uint64_t l_ui64ChunkDuration = (l_rDynamicBoxContext.getInputChunkEndTime(i, j) - l_rDynamicBoxContext.getInputChunkStartTime(i, j));

						m_pRendererContext->setSampleDuration(l_ui64ChunkDuration / l_ui32SampleCount);
						m_pRendererContext->setSpectrumFrequencyRange(uint32_t((uint64_t(l_ui32SampleCount) << 32) / l_ui64ChunkDuration));
						m_pRendererContext->setMinimumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin))));
						m_pRendererContext->setMaximumSpectrumFrequency(uint32_t(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax))));

						// Sets time scale
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilder, "spinbutton_time_scale")), (l_ui64ChunkDuration >> 22) / 1024.);

						m_vRenderer[i]->clear(0); // Drop last samples as they will be fed again
						for (k = 0; k < l_ui32SampleCount; k++)
						{
							for (l = 0; l < l_ui32ChannelCount; l++)
							{
								m_vSwap[l] = float(l_pMatrix->getBuffer()[l * l_ui32SampleCount + k]);
							}
							m_vRenderer[i]->feed(&m_vSwap[0]);
						}

						m_bRefreshNeeded = true;
						m_bRedrawNeeded = true;
					}
				}
			}

			double l_dERPFraction = getContext().getERPFraction();
			if (m_pRendererContext->isERPPlayerActive())
			{
				l_dERPFraction += .0025;
				if (l_dERPFraction > 1) l_dERPFraction = 0;
			}
			if (m_dLastERPFraction != l_dERPFraction)
			{
				gtk_range_set_value(GTK_RANGE(m_pERPRange), l_dERPFraction);
				m_dLastERPFraction = l_dERPFraction;
				m_bRefreshNeeded = true;
				m_bRedrawNeeded = true;
			}

			if (m_bRebuildNeeded)
			{
				for (auto& renderer : m_vRenderer)
				{
					renderer->rebuild(*m_pRendererContext);
				}
			}
			if (m_bRefreshNeeded)
			{
				for (auto& renderer : m_vRenderer)
				{
					renderer->refresh(*m_pRendererContext);
				}
			}
			if (m_bRedrawNeeded) this->redraw();

			m_bRebuildNeeded = false;
			m_bRefreshNeeded = false;
			m_bRedrawNeeded = false;

			if (m_pRendererContext->isERPPlayerActive())
			{
				gtk_button_set_label(GTK_BUTTON(m_pERPPlayerButton), GTK_STOCK_MEDIA_PAUSE);
			}
			else
			{
				gtk_button_set_label(GTK_BUTTON(m_pERPPlayerButton), GTK_STOCK_MEDIA_PLAY);
			}

			return true;
		}

		template <class TRendererFactoryClass, class TRulerClass>
		void TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::draw()

		{
			CBoxAlgorithmViz::preDraw();

			for (uint32_t i = 0; i < m_ui32InputCount; i++)
			{
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				if (i < m_vColor.size()) glColor4f(m_vColor[i].r, m_vColor[i].g, m_vColor[i].b, m_pRendererContext->getTranslucency());
				else                  glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
				if (m_vRenderer[i]) m_vRenderer[i]->render(*m_pRendererContext);
				glPopAttrib();
			}

			CBoxAlgorithmViz::postDraw();
		}
	};
};
