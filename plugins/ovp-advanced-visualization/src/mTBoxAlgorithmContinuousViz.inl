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

#include "mTBoxAlgorithmContinuousViz.hpp"

template <class TRendererFactoryClass, class TRulerClass>
TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::TBoxAlgorithmContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
	:CBoxAlgorithmViz(rClassId, vParameter)
{
}

template <class TRendererFactoryClass, class TRulerClass>
OpenViBE::boolean TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::initialize(void)
{
	bool l_bResult=CBoxAlgorithmViz::initialize();

	m_oMatrixDecoder.initialize(*this);
	m_oStimulationDecoder.initialize(*this);

	m_pRenderer=m_oRendererFactory.create();

	m_pRuler=new TRulerClass;
	m_pRuler->setRendererContext(m_pRendererContext);
	m_pRuler->setRenderer(m_pRenderer);

	return l_bResult;
}

template <class TRendererFactoryClass, class TRulerClass>
OpenViBE::boolean TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::uninitialize(void)
{
	m_oRendererFactory.release(m_pRenderer);
	m_pRenderer=NULL;

	delete m_pRuler;
	m_pRuler=NULL;

	m_oStimulationDecoder.uninitialize();
	m_oMatrixDecoder.uninitialize();

	return CBoxAlgorithmViz::uninitialize();
}

template <class TRendererFactoryClass, class TRulerClass>
OpenViBE::boolean TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::process(void)
{
	OpenViBE::Kernel::IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	OpenViBE::Kernel::IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	OpenViBE::uint32 i, j, k;

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oMatrixDecoder.decode(0, i);

		OpenViBE::IMatrix* l_pMatrix=m_oMatrixDecoder.getOutputMatrix();
		OpenViBE::uint32 l_ui32ChannelCount=l_pMatrix->getDimensionSize(0);
		OpenViBE::uint32 l_ui32SampleCount=l_pMatrix->getDimensionSize(1);

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

			m_vSwap.resize(l_ui32ChannelCount);

			m_pRendererContext->clear();
			m_pRendererContext->setTranslucency(float32(m_f64Translucency));
			m_pRendererContext->setFlowerRingCount(m_ui64FlowerRingCount);
			m_pRendererContext->setTimeScale(m_ui64TimeScale);
			m_pRendererContext->setElementCount(m_ui64ElementCount);
			m_pRendererContext->scaleBy(float32(m_f64DataScale));
			m_pRendererContext->setAxisDisplay(m_bShowAxis);
			m_pRendererContext->setPositiveOnly(m_bIsPositive);
			m_pRendererContext->setParentRendererContext(&getContext());
			m_pRendererContext->setTimeLocked(m_bIsTimeLocked);
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
//				m_pRendererContext->addChannel(sanitize(l_pMatrix->getDimensionLabel(0, j)));
				::gtk_list_store_append(m_pChannelListStore, &l_oGtkTreeIterator);
				::gtk_list_store_set(m_pChannelListStore, &l_oGtkTreeIterator, 0, j+1, 1, l_sName.c_str(), -1);
			}
			::gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));
			::gtk_tree_selection_select_all(::gtk_tree_view_get_selection(m_pChannelTreeView));

			m_pRenderer->setChannelCount(l_ui32ChannelCount);
//			m_pRenderer->setSampleCount(uint32(m_f64TimeScale)); // $$$

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

			if(l_ui32SampleCount!=1)
			{
				OpenViBE::Kernel::ELogLevel l_eLogLevel = this->getConfigurationManager().expandAsBoolean("${AdvancedViz_WarnIfSuspiciousSettings}", true) ? OpenViBE::Kernel::LogLevel_Warning : OpenViBE::Kernel::LogLevel_Trace;
				boolean l_bWarned=false;
				if(m_oTypeIdentifier == OV_TypeId_Spectrum)
				{
					l_bWarned=true;
					this->getLogManager() << l_eLogLevel << "Input matrix has 'spectrum' type\n";
					this->getLogManager() << l_eLogLevel << "Such configuration is uncommon for a 'continous' kind of visualization !\n";
					this->getLogManager() << l_eLogLevel << "You might want to consider the 'stacked' kind of visualization for time/frequency analysis for instance\n";
					this->getLogManager() << l_eLogLevel << "Please double check your scenario\n";
				}
				else
				{
					if(!m_pRendererContext->isTimeLocked())
					{
						l_bWarned=true;
						this->getLogManager() << l_eLogLevel << "Input matrix has " << l_ui32SampleCount << " elements and the box settings say the elements are independant with " << m_ui64ElementCount << " elements to render\n";
						this->getLogManager() << l_eLogLevel << "Such configuration is uncommon for a 'continous' kind of visualization !\n";
						this->getLogManager() << l_eLogLevel << "You might want either of the following alternative :\n";
						this->getLogManager() << l_eLogLevel << " - an 'instant' kind of visualization to highlight the " << m_ui64ElementCount << " elements of the matrix\n";
						this->getLogManager() << l_eLogLevel << " - a 'time locked' kind of elements (thus the scenario must refresh the matrix on a regular basis)\n";
						this->getLogManager() << l_eLogLevel << "Please double check your scenario and box settings\n";
					}
				}

				if(l_bWarned && l_eLogLevel == OpenViBE::Kernel::LogLevel_Warning)
				{
					this->getLogManager() << l_eLogLevel << "This message can be logged at down from Warning to Trace level setting the environment variable " << OpenViBE::CString("AdvancedViz_WarnIfSuspiciousSettings") << " to " << boolean(false) << "\n";
				}
			}

			m_bRebuildNeeded=true;
			m_bRefreshNeeded=true;
			m_bRedrawNeeded=true;
		}
		if(m_oMatrixDecoder.isBufferReceived())
		{
			m_ui64Time1=m_ui64Time2;
			m_ui64Time2=l_rDynamicBoxContext.getInputChunkEndTime(0, i);
			uint64 l_ui64SampleDuration=(m_ui64Time2-m_ui64Time1)/l_ui32SampleCount;
			if((l_ui64SampleDuration&~0xf)!=(m_pRendererContext->getSampleDuration()&~0xf) && l_ui64SampleDuration!=0) // 0xf mask avoids rounding errors
			{
				m_pRendererContext->setSampleDuration(l_ui64SampleDuration);
			}
			m_pRendererContext->setSpectrumFrequencyRange(uint32((uint64(l_ui32SampleCount)<<32)/(l_rDynamicBoxContext.getInputChunkEndTime(0, i)-l_rDynamicBoxContext.getInputChunkStartTime(0, i))));
			m_pRendererContext->setMinimumSpectrumFrequency(uint32(::gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMin))));
			m_pRendererContext->setMaximumSpectrumFrequency(uint32(::gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pFrequencyBandMax))));

			// Feed renderer with actual samples
			for(j=0; j<l_ui32SampleCount; j++)
			{
				for(k=0; k<l_ui32ChannelCount; k++)
				{
					m_vSwap[k]=float32(l_pMatrix->getBuffer()[k*l_ui32SampleCount+j]);
				}
				m_pRenderer->feed(&m_vSwap[0]);
			}

			// Adjust feeding depending on theoretical dates
			if(m_pRendererContext->isTimeLocked() && m_pRendererContext->getSampleDuration())
			{
				uint32 l_ui32TheoreticalSampleCount = uint32(m_ui64Time2 / m_pRendererContext->getSampleDuration());
				if(l_ui32TheoreticalSampleCount > m_pRenderer->getHistoryCount())
				{
					m_pRenderer->prefeed(l_ui32TheoreticalSampleCount - m_pRenderer->getHistoryCount());
				}
			}

			m_bRefreshNeeded=true;
			m_bRedrawNeeded=true;
		}
	}

	if(l_rStaticBoxContext.getInputCount()>1)
	{
		for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
		{
			m_oStimulationDecoder.decode(1, i);
			if(m_oStimulationDecoder.isBufferReceived())
			{
				OpenViBE::IStimulationSet* l_pStimulationSet=m_oStimulationDecoder.getOutputStimulationSet();
				for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
				{
					m_pRenderer->feed(l_pStimulationSet->getStimulationDate(j), l_pStimulationSet->getStimulationIdentifier(j));
					m_bRedrawNeeded=true;
				}
			}
		}
	}

#if 1
	if(true)
	{
		uint32 l_ui32RendererSampleCount=0;
		if(m_pRendererContext->isTimeLocked())
		{
			if(0 != m_pRendererContext->getSampleDuration())
			{
				l_ui32RendererSampleCount=uint32(m_pRendererContext->getTimeScale()/m_pRendererContext->getSampleDuration());
			}
		}
		else
		{
			l_ui32RendererSampleCount=static_cast<uint32>(m_pRendererContext->getElementCount()); // *l_ui32SampleCount;
		}

		if(l_ui32RendererSampleCount!=0 && l_ui32RendererSampleCount!=m_pRenderer->getSampleCount())
		{
			m_pRenderer->setSampleCount(l_ui32RendererSampleCount);
			m_bRebuildNeeded=true;
			m_bRefreshNeeded=true;
			m_bRedrawNeeded=true;
		}
	}
#endif

	if(m_bRebuildNeeded) m_pRenderer->rebuild(*m_pRendererContext);
	if(m_bRefreshNeeded) m_pRenderer->refresh(*m_pRendererContext);
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

template <class TRendererFactoryClass, class TRulerClass>
void TBoxAlgorithmContinuousViz<TRendererFactoryClass, TRulerClass>::draw(void)
{
	CBoxAlgorithmViz::preDraw();

	::glPushAttrib(GL_ALL_ATTRIB_BITS);
	::glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
	m_pRenderer->render(*m_pRendererContext);
	::glPopAttrib();

	CBoxAlgorithmViz::postDraw();
}
