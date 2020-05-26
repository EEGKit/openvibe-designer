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

#include "m_defines.hpp"

#include <cstdint>
#include <string>
#include <cstdlib>	// size_t for unix
#include <vector>
#include "mCVertex.hpp"
#include <map>

namespace Mensia {
namespace AdvancedVisualization {

class LMAV_API CRendererContext
{
public:

	enum class EDataType
	{
		Matrix,
		Signal,
		Spectrum,
		TimeFrequency
	};

	CRendererContext() { clear(); }
	explicit CRendererContext(CRendererContext* parentCtx);

	~CRendererContext() = default;

	void clear();

	void setParentRendererContext(CRendererContext* ctx) { m_parentCtx = ctx; }

	void clearChannelInfo();
	void addChannel(const std::string& name, float x = 0, float y = 0, float z = 0);
	void selectChannel(const size_t index);
	void unselectChannel(const size_t index);
	void sortSelectedChannel(const size_t mode);

	void setDimensionLabel(const size_t idx1, const size_t idx2, const char* label);
	size_t getDimensionLabelCount(const size_t index) const;
	const char* getDimensionLabel(const size_t idx1, const size_t idx2) const;

	void clearTransformInfo();
	void scaleBy(const float scale) { m_scale *= scale; }
	void setScale(const float scale) { m_scale = scale; }
	void zoomBy(const float zoom) { m_zoom *= zoom; }
	void rotateByX(const float rotation) { m_rotationX += rotation; }
	void rotateByY(const float rotation) { m_rotationY += rotation; }

	void setTranslucency(const float translucency) { m_translucency = translucency; }
	void setAspect(const float aspect) { m_aspect = aspect; }
	void setSampleDuration(const uint64_t sampleDuration) { m_sampleDuration = sampleDuration; }
	void setTimeScale(const size_t timeScale) { m_timeScale = timeScale; }
	void setElementCount(const size_t elementCount) { m_nElement = elementCount; }
	void setFlowerRingCount(const size_t flowerRingCount) { m_nFlowerRing = flowerRingCount; }
	void setXYZPlotDepth(const bool hasDepth) { m_hasXYZPlotDepth = hasDepth; }
	void setAxisDisplay(const bool isAxisDisplayed) { m_isAxisDisplayed = isAxisDisplayed; }
	void setPositiveOnly(const bool bPositiveOnly) { m_isPositiveOnly = bPositiveOnly; }
	void setTimeLocked(const bool timeLocked) { m_isTimeLocked = timeLocked; }
	void setScrollModeActive(const bool scrollModeActive) { m_isScrollModeActive = scrollModeActive; }
	void setScaleVisibility(const bool visibility) { m_scaleVisiblity = visibility; }
	void setCheckBoardVisibility(const bool visibility) { m_checkBoardVisiblity = visibility; }
	void setDataType(const EDataType type) { m_dataType = type; }
	void setSpectrumFrequencyRange(const size_t range) { m_spectrumFreqRange = range; }
	void setMinimumSpectrumFrequency(const size_t frequency) { m_minSpectrumFreq = frequency; }
	void setMaximumSpectrumFrequency(const size_t frequency) { m_maxSpectrumFreq = frequency; }
	void setStackCount(const size_t count) { m_nStack = count; }
	void setStackIndex(const size_t index) { m_stackIdx = index; }
	void setFaceMeshVisible(const bool visible) { m_faceMeshVisible = visible; }
	void setScalpMeshVisible(const bool visible) { m_scalpMeshVisible = visible; }

	void setERPPlayerActive(const bool active) { m_erpPlayerActive = active; }
	void stepERPFractionBy(const float erpFraction) { m_erpFraction += erpFraction; }

	std::string getChannelName(const size_t index) const;
	bool getChannelLocalisation(const size_t index, float& x, float& y, float& z) const;
	size_t getChannelCount() const { return m_channelName.size(); }
	size_t getSelectedCount() const { return m_channelLookup.size(); }
	size_t getSelected(const size_t index) const { return m_channelLookup[index]; }
	bool isSelected(const size_t index) const;

	float getScale() const { return m_scale * (m_parentCtx ? m_parentCtx->getScale() : 1); }
	float getZoom() const { return m_zoom * (m_parentCtx ? m_parentCtx->getZoom() : 1); }
	float getRotationX() const { return m_rotationX + (m_parentCtx ? m_parentCtx->getRotationX() : 0); }
	float getRotationY() const { return m_rotationY + (m_parentCtx ? m_parentCtx->getRotationY() : 0); }

	float getTranslucency() const { return m_translucency * (m_parentCtx ? m_parentCtx->getTranslucency() : 1); }
	float getAspect() const { return m_aspect * (m_parentCtx ? m_parentCtx->getAspect() : 1); }
	uint64_t getSampleDuration() const { return m_sampleDuration; }
	size_t getTimeScale() const { return m_timeScale; }
	size_t getElementCount() const { return m_nElement; }
	size_t getFlowerRingCount() const { return m_nFlowerRing; }
	bool hasXYZPlotDepth() const { return m_hasXYZPlotDepth; }
	bool isAxisDisplayed() const { return m_isAxisDisplayed; }
	bool isPositiveOnly() const { return m_isPositiveOnly; }
	bool isTimeLocked() const { return m_isTimeLocked; }
	bool isScrollModeActive() const { return m_isScrollModeActive; }
	bool getScaleVisibility() const { return (m_parentCtx ? m_parentCtx->getScaleVisibility() : m_scaleVisiblity); }
	bool getCheckBoardVisibility() const { return (m_parentCtx ? m_parentCtx->getCheckBoardVisibility() : m_checkBoardVisiblity); }
	EDataType getDataType() const { return m_dataType; }
	size_t getSpectrumFrequencyRange() const { return m_spectrumFreqRange; }
	size_t getMinSpectrumFrequency() const { return m_minSpectrumFreq > m_spectrumFreqRange ? m_spectrumFreqRange : m_minSpectrumFreq; }
	size_t getMaxSpectrumFrequency() const { return m_maxSpectrumFreq > m_spectrumFreqRange ? m_spectrumFreqRange : m_maxSpectrumFreq; }
	size_t getStackCount() const { return m_nStack; }
	size_t getStackIndex() const { return m_stackIdx; }
	bool isFaceMeshVisible() const { return m_faceMeshVisible; }
	bool isScalpMeshVisible() const { return m_scalpMeshVisible; }

	bool isERPPlayerActive() const { return m_erpPlayerActive || (m_parentCtx ? m_parentCtx->isERPPlayerActive() : false); }
	float getERPFraction() const;

	static size_t getMaximumSampleCountPerDisplay() { return 1000; } /*500;*/ /*128*/

protected:

	CRendererContext* m_parentCtx = nullptr;

	std::vector<size_t> m_channelLookup;
	std::vector<std::string> m_channelName;
	std::vector<CVertex> m_channelPos;
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
	EDataType m_dataType       = EDataType::Matrix;
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

}  // namespace AdvancedVisualization
}  // namespace Mensia
