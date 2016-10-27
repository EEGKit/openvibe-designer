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

#ifndef __OpenViBEPlugins_CRulerAutoType_H__
#define __OpenViBEPlugins_CRulerAutoType_H__

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class TRulerMatrix, class TRulerSignal, class TRulerSpectrum>
		class TRulerAutoType : public IRuler
		{
		public:

			virtual void setRendererContext(const IRendererContext* pRendererContext)
			{
				IRuler::setRendererContext(pRendererContext);
				m_oRulerSignal.setRendererContext(pRendererContext);
				m_oRulerSpectrum.setRendererContext(pRendererContext);
				m_oRulerMatrix.setRendererContext(pRendererContext);
			}

			virtual void setRenderer(const IRenderer* pRenderer)
			{
				IRuler::setRenderer(pRenderer);
				m_oRulerSignal.setRenderer(pRenderer);
				m_oRulerSpectrum.setRenderer(pRenderer);
				m_oRulerMatrix.setRenderer(pRenderer);
			}

			virtual void render(void)
			{
				IRendererContext::EDataType l_eDataType=m_pRendererContext->getDataType();
				if(l_eDataType==IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRender();
				}
				else if(l_eDataType==IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRender();
				}
				else
				{
					m_oRulerMatrix.doRender();
				}
			}

			virtual void renderLeft(::GtkWidget* pWidget)
			{
				IRendererContext::EDataType l_eDataType=m_pRendererContext->getDataType();
				if(l_eDataType==IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRenderLeft(pWidget);
				}
				else if(l_eDataType==IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRenderLeft(pWidget);
				}
				else
				{
					m_oRulerMatrix.doRenderLeft(pWidget);
				}
			}

			virtual void renderRight(::GtkWidget* pWidget)
			{
				IRendererContext::EDataType l_eDataType=m_pRendererContext->getDataType();
				if(l_eDataType==IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRenderRight(pWidget);
				}
				else if(l_eDataType==IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRenderRight(pWidget);
				}
				else
				{
					m_oRulerMatrix.doRenderRight(pWidget);
				}
			}

			virtual void renderBottom(::GtkWidget* pWidget)
			{
				IRendererContext::EDataType l_eDataType=m_pRendererContext->getDataType();

				if(l_eDataType==IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRenderBottom(pWidget);
				}
				else if(l_eDataType==IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRenderBottom(pWidget);
				}
				else
				{
					m_oRulerMatrix.doRenderBottom(pWidget);
				}
			}

		protected:

			TRulerMatrix m_oRulerMatrix;
			TRulerSignal m_oRulerSignal;
			TRulerSpectrum m_oRulerSpectrum;
		};
	};
};

#endif // __OpenViBEPlugins_CRulerAutoType_H__
