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

			virtual void render()

			{
				IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();
				if (l_eDataType == IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRender();
				}
				else if (l_eDataType == IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRender();
				}
				else
				{
					m_oRulerMatrix.doRender();
				}
			}

			virtual void renderLeft(GtkWidget* pWidget)
			{
				IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();
				if (l_eDataType == IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRenderLeft(pWidget);
				}
				else if (l_eDataType == IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRenderLeft(pWidget);
				}
				else
				{
					m_oRulerMatrix.doRenderLeft(pWidget);
				}
			}

			virtual void renderRight(GtkWidget* pWidget)
			{
				IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();
				if (l_eDataType == IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRenderRight(pWidget);
				}
				else if (l_eDataType == IRendererContext::DataType_Spectrum)
				{
					m_oRulerSpectrum.doRenderRight(pWidget);
				}
				else
				{
					m_oRulerMatrix.doRenderRight(pWidget);
				}
			}

			virtual void renderBottom(GtkWidget* pWidget)
			{
				IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();

				if (l_eDataType == IRendererContext::DataType_Signal)
				{
					m_oRulerSignal.doRenderBottom(pWidget);
				}
				else if (l_eDataType == IRendererContext::DataType_Spectrum)
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
