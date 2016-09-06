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

#include "mTBoxAlgorithmInstantViz.hpp"

template <class TRendererFactoryClass, class TRulerClass>
TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::TBoxAlgorithmInstantViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
	:CBoxAlgorithmViz(rClassId, vParameter)
{
}

template <class TRendererFactoryClass, class TRulerClass>
OpenViBE::boolean TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::initialize(void)
{
	bool l_bResult=CBoxAlgorithmViz::initialize();

	m_dLastERPFraction=0;

	OpenViBE::Kernel::IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	m_ui32InputCount=l_rStaticBoxContext.getInputCount();
	m_vRenderer.resize(m_ui32InputCount);
	m_vMatrixDecoder.resize(m_ui32InputCount);
	for(OpenViBE::uint32 i=0; i<m_ui32InputCount; i++)
	{
		m_vRenderer[i]=m_oRendererFactory.create();
		m_vMatrixDecoder[i].initialize(*this);
		if(!m_vRenderer[i])
		{
			this->getLogManager() << OpenViBE::Kernel::LogLevel_ImportantWarning << "Could not create renderer, it might have disabled at compile time\n";
			l_bResult = false;
		}
	}

	m_pRuler=new TRulerClass;
	m_pRuler->setRendererContext(m_pRendererContext);
	m_pRuler->setRenderer(m_vRenderer[0]);

	::gtk_widget_set_sensitive(m_pTimeScale, false);

	return l_bResult;
}

template <class TRendererFactoryClass, class TRulerClass>
OpenViBE::boolean TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::uninitialize(void)
{
	for(OpenViBE::uint32 i=0; i<m_ui32InputCount; i++)
	{
		m_oRendererFactory.release(m_vRenderer[i]);
		m_vMatrixDecoder[i].uninitialize();
	}

	m_vMatrixDecoder.clear();
	m_vRenderer.clear();

	return CBoxAlgorithmViz::uninitialize();
}

template <class TRendererFactoryClass, class TRulerClass>
OpenViBE::boolean TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::process(void)
{
	OpenViBE::Kernel::IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	OpenViBE::Kernel::IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	OpenViBE::uint32 i, j, k, l;

	for(i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
			m_vMatrixDecoder[i].decode(i, j);

			OpenViBE::IMatrix* l_pMatrix=m_vMatrixDecoder[i].getOutputMatrix();
			OpenViBE::uint32 l_ui32ChannelCount=l_pMatrix->getDimensionSize(0);
			OpenViBE::uint32 l_ui32SampleCount=l_pMatrix->getDimensionSize(1);

			if(l_ui32ChannelCount == 0)
			{
				this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Input stream " << i << " has 0 channels\n";
				return false;
			}

			if(l_pMatrix->getDimensionCount()==1)
			{
#if 0
				l_ui32ChannelCount=l_pMatrix->getDimensionSize(0);
				l_ui32SampleCount=1;
#else
				l_ui32ChannelCount=1;
				l_ui32SampleCount=l_pMatrix->getDimensionSize(0);
#endif
			}
//			if(l_ui32SampleCount==0) l_ui32SampleCount=1;

			if(m_vMatrixDecoder[i].isHeaderReceived())
			{
				// TODO
				// Check dimension coherence
				// Only apply renderer context when first header is received

				::GtkTreeIter l_oGtkTreeIterator;
				::gtk_list_store_clear(m_pChannelListStore);

				m_vSwap.resize(l_ui32ChannelCount);

				m_pRendererContext->clear();
				m_pRendererContext->setTranslucency(float32(m_f64Translucency));
				m_pRendererContext->setFlowerRingCount(m_ui64FlowerRingCount);
//				m_pRendererContext->setTimeScale(1); // Won't be used
				m_pRendererContext->scaleBy(float32(m_f64DataScale));
				m_pRendererContext->setPositiveOnly(m_bIsPositive);
				m_pRendererContext->setAxisDisplay(m_bShowAxis);
				m_pRendererContext->setParentRendererContext(&Mensia::AdvancedVisualization::getContext());
				m_pRendererContext->setXYZPlotDepth(m_bXYZPlotHasDepth);

				::gtk_tree_view_set_model(m_pChannelTreeView, NULL);
				for(j=0; j<l_ui32ChannelCount; j++)
				{
					std::string l_sName=sanitize(l_pMatrix->getDimensionLabel(0, j));
					std::string l_sSubname=l_sName;
					std::transform(l_sName.begin(), l_sName.end(), l_sSubname.begin(), ::tolower);
					CVertex v=m_vChannelLocalisation[l_sSubname];

					if(l_sName == "")
					{
						char l_sIndexedChannelName[1024];
						::sprintf(l_sIndexedChannelName, "Channel %i", j+1);
						l_sName=l_sIndexedChannelName;
					}

					m_pRendererContext->addChannel(l_sName, v.x, v.y, v.z);
//					m_pRendererContext->addChannel(sanitize(l_pMatrix->getDimensionLabel(0, j)));
					::gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
					::gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, j+1, 1, l_sName.c_str(), -1);
				}
				::gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
				::gtk_tree_selection_select_all(::gtk_tree_view_get_selection(m_pChannelTreeView));

				m_vRenderer[i]->setChannelCount(l_ui32ChannelCount);
				m_vRenderer[i]->setSampleCount(l_ui32SampleCount);

				if(l_ui32SampleCount>1 && m_oTypeIdentifier!=OV_TypeId_Spectrum)
				{
					::gtk_widget_show(m_pERPPlayer);
				}

				if(m_oTypeIdentifier==OV_TypeId_Signal)
				{
					m_pRendererContext->setDataType(IRendererContext::DataType_Signal);
				}
				else if(m_oTypeIdentifier==OV_TypeId_Spectrum)
				{
					m_pRendererContext->setDataType(IRendererContext::DataType_Spectrum);
				}
				else
				{
					m_pRendererContext->setDataType(IRendererContext::DataType_Matrix);
				}

				m_bRebuildNeeded=true;
				m_bRefreshNeeded=true;
				m_bRedrawNeeded=true;
			}
			if(m_vMatrixDecoder[i].isBufferReceived())
			{
				uint64 l_ui64ChunkDuration = (l_rDynamicBoxContext.getInputChunkEndTime(i, j)-l_rDynamicBoxContext.getInputChunkStartTime(i, j));

				m_pRendererContext->setSampleDuration(l_ui64ChunkDuration/l_ui32SampleCount);
				m_pRendererContext->setSpectrumFrequencyRange(uint32((uint64(l_ui32SampleCount)<<32)/l_ui64ChunkDuration));
				m_pRendererContext->setMinimumSpectrumFrequency(uint32(::gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin))));
				m_pRendererContext->setMaximumSpectrumFrequency(uint32(::gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax))));

				// Sets time scale
				::gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilder, "spinbutton_time_scale")), (l_ui64ChunkDuration>>22)/1024.);

				m_vRenderer[i]->clear(0); // Drop last samples as they will be fed again
				for(k=0; k<l_ui32SampleCount; k++)
				{
					for(l=0; l<l_ui32ChannelCount; l++)
					{
						m_vSwap[l]=float32(l_pMatrix->getBuffer()[l*l_ui32SampleCount+k]);
					}
					m_vRenderer[i]->feed(&m_vSwap[0]);
				}

				m_bRefreshNeeded=true;
				m_bRedrawNeeded=true;
			}
		}
	}

	double l_dERPFraction = Mensia::AdvancedVisualization::getContext().getERPFraction();
	if(m_pRendererContext->isERPPlayerActive())
	{
		l_dERPFraction+=.0025;
		if(l_dERPFraction>1) l_dERPFraction=0;
	}
	if(m_dLastERPFraction!=l_dERPFraction)
	{
		::gtk_range_set_value(GTK_RANGE(m_pERPRange), l_dERPFraction);
		m_dLastERPFraction=l_dERPFraction;
		m_bRefreshNeeded=true;
		m_bRedrawNeeded=true;
	}

	if(m_bRebuildNeeded) for(j=0; j<m_vRenderer.size(); j++) m_vRenderer[j]->rebuild(*m_pRendererContext);
	if(m_bRefreshNeeded) for(j=0; j<m_vRenderer.size(); j++) m_vRenderer[j]->refresh(*m_pRendererContext);
	if(m_bRedrawNeeded) this->redraw();
//	if(m_bRedrawNeeded) m_oGtkGLWidget.redraw();
//	if(m_bRedrawNeeded) m_oGtkGLWidget.redrawLeft();
//	if(m_bRedrawNeeded) m_oGtkGLWidget.redrawRight();
//	if(m_bRedrawNeeded) m_oGtkGLWidget.redrawBottom();

	m_bRebuildNeeded=false;
	m_bRefreshNeeded=false;
	m_bRedrawNeeded=false;

	if(m_pRendererContext->isERPPlayerActive())
	{
		::gtk_button_set_label(GTK_BUTTON(m_pERPPlayerButton), GTK_STOCK_MEDIA_PAUSE);
	}
	else
	{
		::gtk_button_set_label(GTK_BUTTON(m_pERPPlayerButton), GTK_STOCK_MEDIA_PLAY);
	}

	return true;
}

template <class TRendererFactoryClass, class TRulerClass>
void TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::draw(void)
{
	CBoxAlgorithmViz::preDraw();

	for(OpenViBE::uint32 i=0; i<m_ui32InputCount; i++)
	{
		::glPushAttrib(GL_ALL_ATTRIB_BITS);
		if(i<m_vColor.size()) ::glColor4f(m_vColor[i].r, m_vColor[i].g, m_vColor[i].b, m_pRendererContext->getTranslucency());
		else                  ::glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
		if(m_vRenderer[i]) m_vRenderer[i]->render(*m_pRendererContext);
		::glPopAttrib();
	}

	CBoxAlgorithmViz::postDraw();
}
