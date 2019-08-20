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

			void setRendererContext(const IRendererContext* pRendererContext) override
			{
				IRuler::setRendererContext(pRendererContext);
				m_oRulerSignal.setRendererContext(pRendererContext);
				m_oRulerSpectrum.setRendererContext(pRendererContext);
				m_oRulerMatrix.setRendererContext(pRendererContext);
			}

			void setRenderer(const IRenderer* pRenderer) override
			{
				IRuler::setRenderer(pRenderer);
				m_oRulerSignal.setRenderer(pRenderer);
				m_oRulerSpectrum.setRenderer(pRenderer);
				m_oRulerMatrix.setRenderer(pRenderer);
			}

			void render() override

			{
				const IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();
				if (l_eDataType == IRendererContext::DataType_Signal) { m_oRulerSignal.doRender(); }
				else if (l_eDataType == IRendererContext::DataType_Spectrum) { m_oRulerSpectrum.doRender(); }
				else { m_oRulerMatrix.doRender(); }
			}

			void renderLeft(GtkWidget* pWidget) override
			{
				const IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();
				if (l_eDataType == IRendererContext::DataType_Signal) { m_oRulerSignal.doRenderLeft(pWidget); }
				else if (l_eDataType == IRendererContext::DataType_Spectrum) { m_oRulerSpectrum.doRenderLeft(pWidget); }
				else { m_oRulerMatrix.doRenderLeft(pWidget); }
			}

			void renderRight(GtkWidget* pWidget) override
			{
				const IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();
				if (l_eDataType == IRendererContext::DataType_Signal) { m_oRulerSignal.doRenderRight(pWidget); }
				else if (l_eDataType == IRendererContext::DataType_Spectrum) { m_oRulerSpectrum.doRenderRight(pWidget); }
				else { m_oRulerMatrix.doRenderRight(pWidget); }
			}

			void renderBottom(GtkWidget* pWidget) override
			{
				const IRendererContext::EDataType l_eDataType = m_pRendererContext->getDataType();

				if (l_eDataType == IRendererContext::DataType_Signal) { m_oRulerSignal.doRenderBottom(pWidget); }
				else if (l_eDataType == IRendererContext::DataType_Spectrum) { m_oRulerSpectrum.doRenderBottom(pWidget); }
				else { m_oRulerMatrix.doRenderBottom(pWidget); }
			}

		protected:

			TRulerMatrix m_oRulerMatrix;
			TRulerSignal m_oRulerSignal;
			TRulerSpectrum m_oRulerSpectrum;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
