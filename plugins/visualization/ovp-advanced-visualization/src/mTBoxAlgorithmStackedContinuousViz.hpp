/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#ifndef __OpenViBEPlugins_BoxAlgorithm_StackedContinuousViz_H__
#define __OpenViBEPlugins_BoxAlgorithm_StackedContinuousViz_H__

#include "mCBoxAlgorithmViz.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		class TBoxAlgorithmStackedContinuousViz : public CBoxAlgorithmViz
		{
		public:

			TBoxAlgorithmStackedContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter);
			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool process(void);

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId);

			OpenViBEToolkit::TStimulationDecoder < TBoxAlgorithmStackedContinuousViz < bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass > > m_oStimulationDecoder;
			OpenViBEToolkit::TStreamedMatrixDecoder < TBoxAlgorithmStackedContinuousViz < bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass > > m_oMatrixDecoder;

			TRendererFactoryClass m_oRendererFactory;
			std::vector < IRenderer* > m_vRenderer;

		protected:

			virtual void draw(void);
		};

		class CBoxAlgorithmStackedContinuousVizListener : public CBoxAlgorithmVizListener
		{
		public:

			CBoxAlgorithmStackedContinuousVizListener(const std::vector < int >& vParameter)
			    :CBoxAlgorithmVizListener(vParameter)
			{
			}

			virtual bool onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index)
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(ui32Index, l_oTypeIdentifier);
				if(!this->getTypeManager().isDerivedFromStream(l_oTypeIdentifier, OV_TypeId_StreamedMatrix))
				{
					rBox.setInputType(ui32Index, OV_TypeId_StreamedMatrix);
				}
				rBox.setInputType(1, OV_TypeId_Stimulations);
				return true;
			}
		};

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass=IRuler>
		class TBoxAlgorithmStackedContinuousVizDesc : public CBoxAlgorithmVizDesc
		{
		public:

			TBoxAlgorithmStackedContinuousVizDesc(const OpenViBE::CString& sName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, const Mensia::AdvancedVisualization::CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription, const OpenViBE::CString& sDetailedDescription)
			    :CBoxAlgorithmVizDesc(sName, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription, sDetailedDescription)
			{
			}

			virtual OpenViBE::Plugins::IPluginObject* create(void)
			{
				return new Mensia::AdvancedVisualization::TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter);
			}

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const
			{
				return new CBoxAlgorithmStackedContinuousVizListener(m_vParameter);
			}

			virtual OpenViBE::CString getCategory(void) const
			{
				return OpenViBE::CString("Advanced Visualization/")+m_sCategoryName;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId);
		};


		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::TBoxAlgorithmStackedContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
		    :CBoxAlgorithmViz(rClassId, vParameter)
		{
		}

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::initialize(void)
		{
			bool l_bResult=CBoxAlgorithmViz::initialize();

			m_oMatrixDecoder.initialize(*this, 0);
			m_oStimulationDecoder.initialize(*this, 1);

			m_pRendererContext->clear();
			m_pRendererContext->setTranslucency(float(m_f64Translucency));
			// m_pRendererContext->setTranslucency(m_ui64FlowerRingCount);
			m_pRendererContext->scaleBy(float(m_f64DataScale));
			m_pRendererContext->setPositiveOnly(m_bIsPositive);
			m_pRendererContext->setAxisDisplay(m_bShowAxis);
			m_pRendererContext->setParentRendererContext(&getContext());

			m_pSubRendererContext->clear();
			m_pSubRendererContext->setParentRendererContext(m_pRendererContext);

			m_pRuler=new TRulerClass;
			m_pRuler->setRendererContext(m_pRendererContext);

			return l_bResult;
		}

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::uninitialize(void)
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

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		bool TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::process(void)
		{
			const OpenViBE::Kernel::IBox& l_rStaticBoxContext=this->getStaticBoxContext();
			OpenViBE::Kernel::IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
			uint32_t i, j, k;

			for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
			{
				m_oMatrixDecoder.decode(0, i);

				OpenViBE::IMatrix* l_pMatrix=m_oMatrixDecoder.getOutputMatrix();
				uint32_t l_ui32ChannelCount=l_pMatrix->getDimensionSize(0);
				uint32_t l_ui32SampleCount=l_pMatrix->getDimensionSize(1);

				if(l_ui32ChannelCount == 0)
				{
					this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Input stream " << i << " has 0 channels\n";
					return false;
				}

				if(l_pMatrix->getDimensionCount()==1)
				{
#if 1
					l_ui32ChannelCount=l_pMatrix->getDimensionSize(0);
					l_ui32SampleCount=1;
#else
					l_ui32ChannelCount=1;
					l_ui32SampleCount=l_pMatrix->getDimensionSize(0);
#endif
				}
				//		if(l_ui32SampleCount==0) l_ui32SampleCount=1;

				if(m_oMatrixDecoder.isHeaderReceived())
				{
					::GtkTreeIter l_oGtkTreeIterator;
					::gtk_list_store_clear(m_pChannelListStore);

					m_vSwap.resize(l_ui32SampleCount);

					for(j=0; j<m_vRenderer.size(); j++)
					{
						m_oRendererFactory.release(m_vRenderer[j]);
					}
					m_vRenderer.clear();
					m_vRenderer.resize(l_ui32ChannelCount);

					m_pSubRendererContext->clear();
					m_pSubRendererContext->setParentRendererContext(m_pRendererContext);
					m_pSubRendererContext->setTimeLocked(m_bIsTimeLocked);
					m_pSubRendererContext->setStackCount(l_ui32ChannelCount);
					for(j=0; j<l_ui32SampleCount; j++)
					{
						std::string l_sName=trim(l_pMatrix->getDimensionLabel(1, j));
						std::string l_sSubname=l_sName;
						std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), ::tolower);
						CVertex v=m_vChannelLocalisation[l_sSubname];
						m_pSubRendererContext->addChannel(l_sName, v.x, v.y, v.z);
					}

					m_pRendererContext->clear();
					m_pRendererContext->setTranslucency(float(m_f64Translucency));
					m_pRendererContext->setTimeScale(m_ui64TimeScale);
					m_pRendererContext->setElementCount(m_ui64ElementCount);
					m_pRendererContext->scaleBy(float(m_f64DataScale));
					m_pRendererContext->setParentRendererContext(&getContext());
					m_pRendererContext->setTimeLocked(m_bIsTimeLocked);
					m_pRendererContext->setXYZPlotDepth(m_bXYZPlotHasDepth);

					::gtk_tree_view_set_model(m_pChannelTreeView, NULL);
					for(j=0; j<l_ui32ChannelCount; j++)
					{
						std::string l_sName=trim(l_pMatrix->getDimensionLabel(0, j));
						std::string l_sSubname=l_sName;
						std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), ::tolower);
						CVertex v=m_vChannelLocalisation[l_sSubname];

						if(l_sName == "")
						{
							char l_sIndexedChannelName[1024];
							::sprintf(l_sIndexedChannelName, "Channel %i", j+1);
							l_sName=l_sIndexedChannelName;
						}

						m_vRenderer[j]=m_oRendererFactory.create();
						m_vRenderer[j]->setChannelCount(l_ui32SampleCount);
						m_vRenderer[j]->setSampleCount(static_cast < uint32_t >(m_ui64ElementCount)); // $$$
						//				m_vRenderer[j]->setSampleCount(uint32_t(m_f64TimeScale)); // $$$

						m_pRendererContext->addChannel(l_sName, v.x, v.y, v.z);
						//				m_pRendererContext->addChannel(sanitize(l_pMatrix->getDimensionLabel(0, j)));
						::gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
						::gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, j+1, 1, l_sName.c_str(), -1);
					}
					::gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
					::gtk_tree_selection_select_all(::gtk_tree_view_get_selection(m_pChannelTreeView));

					if(m_oTypeIdentifier==OV_TypeId_Signal)
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Signal);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_Signal);
					}
					else if(m_oTypeIdentifier==OV_TypeId_Spectrum)
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Spectrum);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_Spectrum);
					}
					else
					{
						m_pRendererContext->setDataType(IRendererContext::DataType_Matrix);
						m_pSubRendererContext->setDataType(IRendererContext::DataType_Matrix);
					}

					m_pRuler->setRenderer(l_ui32ChannelCount?m_vRenderer[0]:NULL);

					m_bRebuildNeeded=true;
					m_bRefreshNeeded=true;
					m_bRedrawNeeded=true;
				}
				if(m_oMatrixDecoder.isBufferReceived())
				{
#if 0 // TODO FIXME ***
					m_ui64Time1=m_ui64Time2;
					m_ui64Time2=l_rDynamicBoxContext.getInputChunkEndTime(0, i);
					uint64_t l_ui64SampleDuration=m_ui64Time2-m_ui64Time1;
					if((l_ui64SampleDuration&~0xf)!=(m_pRendererContext->getSampleDuration()&~0xf) && l_ui64SampleDuration!=0) // 0xf mask avoids rounding errors
					{
						m_pSubRendererContext->setSampleDuration(l_ui64SampleDuration);
						m_pRendererContext->setSampleDuration(l_ui64SampleDuration);
					}
					m_pRendererContext->setSpectrumFrequencyRange(uint32_t((uint64_t(l_ui32SampleCount)<<32)/(l_rDynamicBoxContext.getInputChunkEndTime(0, i)-l_rDynamicBoxContext.getInputChunkStartTime(0, i))));
#else
					m_ui64Time1=m_ui64Time2;
					m_ui64Time2=l_rDynamicBoxContext.getInputChunkEndTime(0, i);
					uint64_t l_ui64InterChunkDuration=m_ui64Time2-m_ui64Time1;
					uint64_t l_ui64ChunkDuration = (l_rDynamicBoxContext.getInputChunkEndTime(0, i)-l_rDynamicBoxContext.getInputChunkStartTime(0, i));
					uint64_t l_ui64SampleDuration = l_ui64ChunkDuration / m_ui64ElementCount;
					if(m_pRendererContext->isTimeLocked())
					{
						if((l_ui64InterChunkDuration&~0xf)!=(m_pRendererContext->getSampleDuration()&~0xf) && l_ui64InterChunkDuration!=0) // 0xf mask avoids rounding errors
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

					m_pRendererContext->setSpectrumFrequencyRange(uint32_t((uint64_t(l_ui32SampleCount)<<32)/l_ui64ChunkDuration));
#endif
					m_pRendererContext->setMinimumSpectrumFrequency(uint32_t(::gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin))));
					m_pRendererContext->setMaximumSpectrumFrequency(uint32_t(::gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax))));

					for(j=0; j<l_ui32ChannelCount; j++)
					{
						// Feed renderer with actual samples
						for(k=0; k<l_ui32SampleCount; k++)
						{
							m_vSwap[l_ui32SampleCount-k-1]=float(l_pMatrix->getBuffer()[j*l_ui32SampleCount+k]);
						}
						m_vRenderer[j]->feed(&m_vSwap[0]);

						// Adjust feeding depending on theoretical dates
						if(m_pRendererContext->isTimeLocked() && m_pRendererContext->getSampleDuration())
						{
							uint32_t l_ui32TheoreticalSampleCount = uint32_t(m_ui64Time2 / m_pRendererContext->getSampleDuration());
							if(l_ui32TheoreticalSampleCount > m_vRenderer[j]->getHistoryCount())
							{
								m_vRenderer[j]->prefeed(l_ui32TheoreticalSampleCount - m_vRenderer[j]->getHistoryCount());
							}
						}
					}

					m_bRefreshNeeded=true;
					m_bRedrawNeeded=true;
				}
			}

			if(l_rStaticBoxContext.getInputCount() > 1)
			{
				for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
				{
					m_oStimulationDecoder.decode(1, i);
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

#if 1
			if(true)
			{
				uint32_t l_ui32RendererSampleCount=0;
				if(m_pRendererContext->isTimeLocked())
				{
					if(0 != m_pRendererContext->getSampleDuration())
					{
						l_ui32RendererSampleCount=uint32_t(m_pRendererContext->getTimeScale()/m_pRendererContext->getSampleDuration());
					}
				}
				else
				{
					l_ui32RendererSampleCount=static_cast<uint32_t>(m_pRendererContext->getElementCount());
				}

				if(l_ui32RendererSampleCount!=0)
				{
					for(j=0; j<m_vRenderer.size(); j++)
					{
						if(l_ui32RendererSampleCount!=m_vRenderer[j]->getSampleCount())
						{
							m_vRenderer[j]->setSampleCount(l_ui32RendererSampleCount);
							m_bRebuildNeeded=true;
							m_bRefreshNeeded=true;
							m_bRedrawNeeded=true;
						}
					}
				}
			}
#endif

			if(m_bRebuildNeeded) for(j=0; j<m_vRenderer.size(); j++) m_vRenderer[j]->rebuild(*m_pSubRendererContext);
			if(m_bRefreshNeeded) for(j=0; j<m_vRenderer.size(); j++) m_vRenderer[j]->refresh(*m_pSubRendererContext);
			if(m_bRedrawNeeded) this->redraw();
			//	if(m_bRedrawNeeded) m_oGtkGLWidget.redraw();
			//	if(m_bRedrawNeeded) m_oGtkGLWidget.redrawLeft();
			//	if(m_bRedrawNeeded) m_oGtkGLWidget.redrawRight();
			//	if(m_bRedrawNeeded) m_oGtkGLWidget.redrawBottom();

			m_bRebuildNeeded=false;
			m_bRefreshNeeded=false;
			m_bRedrawNeeded=false;

			return true;
		}

		template <bool bHorizontalStack, bool bDrawBorders, class TRendererFactoryClass, class TRulerClass>
		void TBoxAlgorithmStackedContinuousViz<bHorizontalStack, bDrawBorders, TRendererFactoryClass, TRulerClass>::draw(void)
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
					if(!bHorizontalStack)
					{
						::glScalef(1, -1, 1);
						::glRotatef(-90, 0, 0, 1);
					}
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

	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_StackedContinuousViz_H__
