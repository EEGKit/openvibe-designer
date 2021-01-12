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

namespace OpenViBE {
namespace AdvancedVisualization {
template <class TRulerMatrix, class TRulerSignal, class TRulerSpectrum>
class TRulerAutoType : public IRuler
{
public:

	void setRendererContext(const CRendererContext* ctx) override
	{
		IRuler::setRendererContext(ctx);
		m_rulerSignal.setRendererContext(ctx);
		m_rulerSpectrum.setRendererContext(ctx);
		m_rulerMatrix.setRendererContext(ctx);
	}

	void setRenderer(const IRenderer* renderer) override
	{
		IRuler::setRenderer(renderer);
		m_rulerSignal.setRenderer(renderer);
		m_rulerSpectrum.setRenderer(renderer);
		m_rulerMatrix.setRenderer(renderer);
	}

	void render() override
	{
		const CRendererContext::EDataType dataType = m_rendererCtx->getDataType();
		if (dataType == CRendererContext::EDataType::Signal) { m_rulerSignal.doRender(); }
		else if (dataType == CRendererContext::EDataType::Spectrum) { m_rulerSpectrum.doRender(); }
		else { m_rulerMatrix.doRender(); }
	}

	void renderLeft(GtkWidget* widget) override
	{
		const CRendererContext::EDataType dataType = m_rendererCtx->getDataType();
		if (dataType == CRendererContext::EDataType::Signal) { m_rulerSignal.doRenderLeft(widget); }
		else if (dataType == CRendererContext::EDataType::Spectrum) { m_rulerSpectrum.doRenderLeft(widget); }
		else { m_rulerMatrix.doRenderLeft(widget); }
	}

	void renderRight(GtkWidget* widget) override
	{
		const CRendererContext::EDataType dataType = m_rendererCtx->getDataType();
		if (dataType == CRendererContext::EDataType::Signal) { m_rulerSignal.doRenderRight(widget); }
		else if (dataType == CRendererContext::EDataType::Spectrum) { m_rulerSpectrum.doRenderRight(widget); }
		else { m_rulerMatrix.doRenderRight(widget); }
	}

	void renderBottom(GtkWidget* widget) override
	{
		const CRendererContext::EDataType dataType = m_rendererCtx->getDataType();

		if (dataType == CRendererContext::EDataType::Signal) { m_rulerSignal.doRenderBottom(widget); }
		else if (dataType == CRendererContext::EDataType::Spectrum) { m_rulerSpectrum.doRenderBottom(widget); }
		else { m_rulerMatrix.doRenderBottom(widget); }
	}

protected:

	TRulerMatrix m_rulerMatrix;
	TRulerSignal m_rulerSignal;
	TRulerSpectrum m_rulerSpectrum;
};
}  // namespace AdvancedVisualization
}  // namespace OpenViBE
