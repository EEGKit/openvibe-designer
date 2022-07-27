///-------------------------------------------------------------------------------------------------
/// 
/// \file mTRulerAutoType.hpp
/// \copyright Copyright (C) 2022 Inria
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <https://www.gnu.org/licenses/>.
/// 
///-------------------------------------------------------------------------------------------------

#pragma once

#include "../mIRuler.hpp"
#include "../m_VisualizationTools.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {
template <class TRulerMatrix, class TRulerSignal, class TRulerSpectrum>
class TRulerAutoType final : public IRuler
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
