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

#ifndef __OpenViBEPlugins_BoxAlgorithm_ContinuousViz_H__
#define __OpenViBEPlugins_BoxAlgorithm_ContinuousViz_H__

#include "mCBoxAlgorithmViz.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class TRendererFactoryClass, class TRulerClass >
		class TBoxAlgorithmContinuousViz : public CBoxAlgorithmViz
		{
		public:

			TBoxAlgorithmContinuousViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter);
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId);

			OpenViBEToolkit::TStreamedMatrixDecoder < TBoxAlgorithmContinuousViz < TRendererFactoryClass, TRulerClass > > m_oMatrixDecoder;
			OpenViBEToolkit::TStimulationDecoder < TBoxAlgorithmContinuousViz < TRendererFactoryClass, TRulerClass > > m_oStimulationDecoder;
			TRendererFactoryClass m_oRendererFactory;
			IRenderer* m_pRenderer;

		protected:

			virtual void draw(void);
		};

		class CBoxAlgorithmContinuousVizListener : public CBoxAlgorithmVizListener
		{
		public:

			CBoxAlgorithmContinuousVizListener(const std::vector < int >& vParameter)
				:CBoxAlgorithmVizListener(vParameter)
			{
			}

			virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
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

		template <class TRendererFactoryClass, class TRulerClass=IRuler>
		class TBoxAlgorithmContinuousVizDesc : public CBoxAlgorithmVizDesc
		{
		public:

			TBoxAlgorithmContinuousVizDesc(const OpenViBE::CString& sName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, Mensia::AdvancedVisualization::CParameterSet rParameterSet, const OpenViBE::CString& sShortDescription,const OpenViBE::CString& sDetailedDescription)
				:CBoxAlgorithmVizDesc(sName, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription, sDetailedDescription)
			{
			}

			virtual OpenViBE::Plugins::IPluginObject* create(void)
			{
				return new Mensia::AdvancedVisualization::TBoxAlgorithmContinuousViz < TRendererFactoryClass, TRulerClass > (m_oClassId, m_vParameter);
			}

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const
			{
				return new CBoxAlgorithmContinuousVizListener(m_vParameter);
			}

			virtual OpenViBE::CString getCategory(void) const
			{
				return OpenViBE::CString("Advanced Visualization/")+m_sCategoryName;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId);
		};

#include "mTBoxAlgorithmContinuousViz.inl"

	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_ContinuousViz_H__
