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

#include "mIRendererContext.h"
#include "mCVertex.hpp"

#include <vector>
#include <string>
#include <map>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererContext final : public IRendererContext
		{
		public:

			explicit CRendererContext(IRendererContext* parentCtx);

			void clear() override;

			void setParentRendererContext(IRendererContext* parentCtx) override { m_parentCtx = parentCtx; }

			void clearChannelInfo() override;
			void addChannel(const std::string& name, float x, float y, float z) override;
			void selectChannel(size_t index) override;
			void unselectChannel(size_t index) override;
			void sortSelectedChannel(size_t mode) override;

			void setDimensionLabel(size_t idx1, size_t idx2, const char* label) override;
			size_t getDimensionLabelCount(size_t index) const override;
			const char* getDimensionLabel(size_t idx1, size_t idx2) const override;

			void clearTransformInfo() override;
			void scaleBy(const float scale) override { m_scale *= scale; }
			void setScale(const float scale) override { m_scale = scale; }
			void zoomBy(const float zoom) override { m_zoom *= zoom; }
			void rotateByX(const float rotation) override { m_rotationX += rotation; }
			void rotateByY(const float rotation) override { m_rotationY += rotation; }

			void setTranslucency(const float translucency) override { m_translucency = translucency; }
			void setAspect(const float aspect) override { m_aspect = aspect; }
			void setSampleDuration(const uint64_t sampleDuration) override { m_sampleDuration = sampleDuration; }
			void setTimeScale(const size_t timeScale) override { m_timeScale = timeScale; }
			void setElementCount(const size_t elementCount) override { m_nElement = elementCount; }
			void setFlowerRingCount(const size_t flowerRingCount) override { m_nFlowerRing = flowerRingCount; }
			void setXYZPlotDepth(const bool hasDepth) override { m_hasXYZPlotDepth = hasDepth; }
			void setAxisDisplay(const bool isAxisDisplayed) override { m_isAxisDisplayed = isAxisDisplayed; }
			void setPositiveOnly(const bool bPositiveOnly) override { m_isPositiveOnly = bPositiveOnly; }
			void setTimeLocked(const bool timeLocked) override { m_isTimeLocked = timeLocked; }
			void setScrollModeActive(const bool scrollModeActive) override { m_isScrollModeActive = scrollModeActive; }
			void setScaleVisibility(const bool visibility) override { m_scaleVisiblity = visibility; }
			void setCheckBoardVisibility(const bool visibility) override { m_checkBoardVisiblity = visibility; }
			void setDataType(const EDataType type) override { m_dataType = type; }
			void setSpectrumFrequencyRange(const size_t range) override { m_spectrumFreqRange = range; }
			void setMinimumSpectrumFrequency(const size_t frequency) override { m_minSpectrumFreq = frequency; }
			void setMaximumSpectrumFrequency(const size_t frequency) override { m_maxSpectrumFreq = frequency; }
			void setStackCount(const size_t count) override { m_nStack = count; }
			void setStackIndex(const size_t index) override { m_stackIdx = index; }
			void setFaceMeshVisible(const bool visible) override { m_faceMeshVisible = visible; }
			void setScalpMeshVisible(const bool visible) override { m_scalpMeshVisible = visible; }

			void setERPPlayerActive(const bool active) override { m_erpPlayerActive = active; }
			void stepERPFractionBy(const float erpFraction) override { m_erpFraction += erpFraction; }

			std::string getChannelName(const size_t index) const override;
			bool getChannelLocalisation(const size_t index, float& x, float& y, float& z) const override;
			size_t getChannelCount() const override { return m_channelName.size(); }
			size_t getSelectedCount() const override { return m_channelLookup.size(); }
			size_t getSelected(const size_t index) const override { return m_channelLookup[index]; }
			bool isSelected(const size_t index) const override;

			float getScale() const override { return m_scale * (m_parentCtx ? m_parentCtx->getScale() : 1); }
			float getZoom() const override { return m_zoom * (m_parentCtx ? m_parentCtx->getZoom() : 1); }
			float getRotationX() const override { return m_rotationX + (m_parentCtx ? m_parentCtx->getRotationX() : 0); }
			float getRotationY() const override { return m_rotationY + (m_parentCtx ? m_parentCtx->getRotationY() : 0); }

			float getTranslucency() const override { return m_translucency * (m_parentCtx ? m_parentCtx->getTranslucency() : 1); }
			float getAspect() const override { return m_aspect * (m_parentCtx ? m_parentCtx->getAspect() : 1); }
			uint64_t getSampleDuration() const override { return m_sampleDuration; }
			size_t getTimeScale() const override { return m_timeScale; }
			size_t getElementCount() const override { return m_nElement; }
			size_t getFlowerRingCount() const override { return m_nFlowerRing; }
			bool hasXYZPlotDepth() const override { return m_hasXYZPlotDepth; }
			bool isAxisDisplayed() const override { return m_isAxisDisplayed; }
			bool isPositiveOnly() const override { return m_isPositiveOnly; }
			bool isTimeLocked() const override { return m_isTimeLocked; }
			bool isScrollModeActive() const override { return m_isScrollModeActive; }
			bool getScaleVisibility() const override { return (m_parentCtx ? m_parentCtx->getScaleVisibility() : m_scaleVisiblity); }

			bool getCheckBoardVisibility() const override { return (m_parentCtx ? m_parentCtx->getCheckBoardVisibility() : m_checkBoardVisiblity); }

			EDataType getDataType() const override { return m_dataType; }
			size_t getSpectrumFrequencyRange() const override { return m_spectrumFreqRange; }
			size_t getMinSpectrumFrequency() const override { return m_minSpectrumFreq > m_spectrumFreqRange ? m_spectrumFreqRange : m_minSpectrumFreq; }
			size_t getMaxSpectrumFrequency() const override { return m_maxSpectrumFreq > m_spectrumFreqRange ? m_spectrumFreqRange : m_maxSpectrumFreq; }

			size_t getStackCount() const override { return m_nStack; }
			size_t getStackIndex() const override { return m_stackIdx; }
			bool isFaceMeshVisible() const override { return m_faceMeshVisible; }
			bool isScalpMeshVisible() const override { return m_scalpMeshVisible; }

			bool isERPPlayerActive() const override { return m_erpPlayerActive || (m_parentCtx ? m_parentCtx->isERPPlayerActive() : false); }

			float getERPFraction() const override;

			size_t getMaximumSampleCountPerDisplay() const override { return 1000; } /*500;*/ /*128*/

		protected:

			IRendererContext* m_parentCtx = nullptr;

			std::vector<size_t> m_channelLookup;
			std::vector<std::string> m_channelName;
			std::vector<CVertex> m_channelLocalisation;
			std::map<size_t, std::vector<std::string>> m_dimLabels;

			std::map<std::string, float> m_leftRightScore;
			std::map<std::string, float> m_frontBackScore;

			float m_scale     = 1;
			float m_zoom      = 1;
			float m_rotationX = 2;
			float m_rotationY = 1;

			float m_translucency       = 1;
			float m_aspect             = 1;
			uint64_t m_sampleDuration  = 0;
			size_t m_timeScale         = 1;
			size_t m_nElement          = 1;
			size_t m_nFlowerRing       = 1;
			bool m_hasXYZPlotDepth     = false;
			bool m_isAxisDisplayed     = false;
			bool m_isPositiveOnly      = false;
			bool m_isTimeLocked        = true;
			bool m_isScrollModeActive  = false;
			bool m_scaleVisiblity      = true;
			bool m_checkBoardVisiblity = false;
			EDataType m_dataType       = DataType_Matrix;
			size_t m_spectrumFreqRange = 0;
			size_t m_minSpectrumFreq   = 0;
			size_t m_maxSpectrumFreq   = 0;
			size_t m_nStack            = 1;
			size_t m_stackIdx          = 1;
			bool m_faceMeshVisible     = true;
			bool m_scalpMeshVisible    = true;

			bool m_erpPlayerActive = false;
			float m_erpFraction    = 0;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
