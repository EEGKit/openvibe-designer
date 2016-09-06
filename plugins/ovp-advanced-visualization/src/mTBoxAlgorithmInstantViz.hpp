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

#ifndef __OpenViBEPlugins_BoxAlgorithm_InstantViz_H__
#define __OpenViBEPlugins_BoxAlgorithm_InstantViz_H__

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
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(CBoxAlgorithmViz, m_oClassId);

			TRendererFactoryClass m_oRendererFactory;

			OpenViBE::uint32 m_ui32InputCount;
			std::vector < IRenderer* > m_vRenderer;
			std::vector < OpenViBEToolkit::TStreamedMatrixDecoder < TBoxAlgorithmInstantViz <TRendererFactoryClass, TRulerClass> > > m_vMatrixDecoder;

			double m_dLastERPFraction;

		protected:

			virtual void draw(void);
		};

		class CBoxAlgorithmInstantVizListener : public CBoxAlgorithmVizListener
		{
		public:

			CBoxAlgorithmInstantVizListener(const std::vector < int >& vParameter)
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
				else
				{
					for(OpenViBE::uint32 i=0; i<rBox.getInputCount(); i++)
					{
						rBox.setInputType(i, l_oTypeIdentifier);
					}
				}
				return true;
			}

			virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(0, l_oTypeIdentifier);
				rBox.setInputType(ui32Index, l_oTypeIdentifier);
				rBox.setInputName(ui32Index, "Matrix");
				rBox.addSetting("Color", OV_TypeId_Color, "${AdvancedViz_DefaultColor}");
				return true;
			}

			virtual OpenViBE::boolean onInputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				rBox.removeSetting(this->getBaseSettingCount()+ui32Index-1);
				return true;
			}
		};

		template <class TRendererFactoryClass, class TRulerClass=IRuler, template < typename, typename > class TBoxAlgorithm=TBoxAlgorithmInstantViz>
		class TBoxAlgorithmInstantVizDesc : public CBoxAlgorithmVizDesc
		{
		public:

			TBoxAlgorithmInstantVizDesc(const OpenViBE::CString& sName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, const Mensia::AdvancedVisualization::CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription,const OpenViBE::CString& sDetailedDescription)
				:CBoxAlgorithmVizDesc(sName, rDescClassId, rClassId, sAddedSoftwareVersion, sUpdatedSoftwareVersion, rParameterSet, sShortDescription, sDetailedDescription)
			{
			}

			virtual OpenViBE::Plugins::IPluginObject* create(void)
			{
				return new TBoxAlgorithm<TRendererFactoryClass, TRulerClass>(m_oClassId, m_vParameter);
			}

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const
			{
				return new CBoxAlgorithmInstantVizListener(m_vParameter);
			}

			virtual OpenViBE::CString getCategory(void) const
			{
				return OpenViBE::CString("Advanced Visualization/")+m_sCategoryName;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, m_oDescClassId);
		};

#include "mTBoxAlgorithmInstantViz.inl"

	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_InstantViz_H__
