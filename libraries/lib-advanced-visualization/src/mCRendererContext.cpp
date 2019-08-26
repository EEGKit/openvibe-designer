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
#include "mCRendererContext.hpp"

#include <algorithm>
#include <map>
#include <cstdio>
#include <cmath>

using namespace Mensia;
using namespace AdvancedVisualization;

namespace
{
	void getLeftRightScore(std::map<std::string, float>& vScore, const std::vector<std::string>& vChannelName, const std::vector<CVertex>& vChannelLocalisation)
	{
		for (uint32_t i = 0; i < vChannelName.size() && i < vChannelLocalisation.size(); ++i)
		{
			std::string l_sChannelName = std::string(",") + vChannelName[i] + std::string(",");
			std::transform(l_sChannelName.begin(), l_sChannelName.end(), l_sChannelName.begin(), tolower);
			vScore[l_sChannelName] = float(vChannelLocalisation[i].x * 1E-0 + vChannelLocalisation[i].y * 1E-10 + vChannelLocalisation[i].z * 1E-5);
		}
	}

	void getFrontBackScore(std::map<std::string, float>& vScore, const std::vector<std::string>& vChannelName, const std::vector<CVertex>& vChannelLocalisation)
	{
		for (uint32_t i = 0; i < vChannelName.size() && i < vChannelLocalisation.size(); ++i)
		{
			std::string l_sChannelName = std::string(",") + vChannelName[i] + std::string(",");
			std::transform(l_sChannelName.begin(), l_sChannelName.end(), l_sChannelName.begin(), tolower);
			vScore[l_sChannelName] = float(vChannelLocalisation[i].x * 1E-5 + vChannelLocalisation[i].y * 1E-10 + vChannelLocalisation[i].z * 1E-0);
		}
	}

	struct sort_alpha
	{
		explicit sort_alpha(const std::vector<std::string>& vChannelName) : m_vChannelName(vChannelName) { }

		bool operator()(const size_t i, const size_t j) const
		{
			std::string channelNameI = m_vChannelName[i];
			std::string channelNameJ = m_vChannelName[j];
			std::transform(channelNameI.begin(), channelNameI.end(), channelNameI.begin(), tolower);
			std::transform(channelNameJ.begin(), channelNameJ.end(), channelNameJ.begin(), tolower);

			return channelNameI < channelNameJ;
		}

		const std::vector<std::string>& m_vChannelName;
	};

	struct sort_special
	{
		sort_special(const std::vector<std::string>& vChannelName, const std::map<std::string, float>& vScore)
			: m_vChannelName(vChannelName)
			  , m_vScore(vScore) { }

		bool operator()(const size_t i, const size_t j) const
		{
			float scoreI = 0;
			float scoreJ = 0;

			std::string channelNameI = std::string(",") + m_vChannelName[i] + std::string(",");
			std::string channelNameJ = std::string(",") + m_vChannelName[j] + std::string(",");
			std::transform(channelNameI.begin(), channelNameI.end(), channelNameI.begin(), tolower);
			std::transform(channelNameJ.begin(), channelNameJ.end(), channelNameJ.begin(), tolower);

			for (std::map<std::string, float>::const_iterator it = m_vScore.begin(); it != m_vScore.end(); ++it)
			{
				if (it->first == channelNameI) { scoreI = it->second; }
				if (it->first == channelNameJ) { scoreJ = it->second; }
			}

			return scoreI < scoreJ;
		}

		const std::vector<std::string>& m_vChannelName;
		const std::map<std::string, float>& m_vScore;
	};
} // namespace

// ____________________________________________________________________________________________________________________________________________________________________________________
//

CRendererContext::CRendererContext(IRendererContext* pParentRendererContext)
{
	this->CRendererContext::clear();
	this->CRendererContext::setParentRendererContext(pParentRendererContext);
}

void CRendererContext::clear()

{
	this->clearChannelInfo();
	this->clearTransformInfo();
	m_DimensionLabels.clear();
}

void CRendererContext::setParentRendererContext(IRendererContext* pParentRendererContext)
{
	m_parentRendererContext = pParentRendererContext;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

void CRendererContext::clearChannelInfo()

{
	m_channelLookup.clear();
	m_channelName.clear();
	m_channelLocalisation.clear();

	m_leftRightScore.clear();
	m_frontBackScore.clear();
}

void CRendererContext::addChannel(const std::string& sChannelName, const float x, const float y, const float z)
{
	const auto l_fNorm     = float(sqrt(x * x + y * y + z * z));
	const float l_fInvNorm = (l_fNorm != 0 ? 1.f / l_fNorm : 0);
	CVertex l_oChannelLocalisation;
	l_oChannelLocalisation.x = x * l_fInvNorm;
	l_oChannelLocalisation.y = y * l_fInvNorm;
	l_oChannelLocalisation.z = z * l_fInvNorm;
	m_channelLookup.push_back(uint32_t(m_channelName.size()));
	m_channelName.push_back(sChannelName);
	m_channelLocalisation.push_back(l_oChannelLocalisation);
}

void CRendererContext::selectChannel(const uint32_t index)
{
	for (auto& i : m_channelLookup)
	{
		if (i == index) { return; }
	}
	m_channelLookup.push_back(index);
}

void CRendererContext::unselectChannel(const uint32_t index)
{
	for (uint32_t i = 0; i < m_channelLookup.size(); ++i)
	{
		if (m_channelLookup[i] == index)
		{
			m_channelLookup.erase(m_channelLookup.begin() + i);
			return;
		}
	}
}

void CRendererContext::sortSelectedChannel(const uint32_t ui32SortMode)
{
	if (m_leftRightScore.empty()) { getLeftRightScore(m_leftRightScore, m_channelName, m_channelLocalisation); }
	if (m_frontBackScore.empty()) { getFrontBackScore(m_frontBackScore, m_channelName, m_channelLocalisation); }

	switch (ui32SortMode)
	{
		case 0:
			break;

		case 1:
			std::stable_sort(m_channelLookup.begin(), m_channelLookup.end());
			break;

		case 2:
			std::stable_sort(m_channelLookup.begin(), m_channelLookup.end(), sort_alpha(m_channelName));
			break;

		case 3:
			std::reverse(m_channelLookup.begin(), m_channelLookup.end());
			break;

		case 4:
			std::stable_sort(m_channelLookup.begin(), m_channelLookup.end(), sort_special(m_channelName, m_leftRightScore));
			break;

		case 5:
			std::stable_sort(m_channelLookup.begin(), m_channelLookup.end(), sort_special(m_channelName, m_frontBackScore));
			break;

		default:
			break;
	}
}

// ____________________________________________________________________________________________________________________________________________________________________________________

void CRendererContext::setDimensionLabel(const size_t dimensionIndex, const size_t labelIndex, const char* label)
{
	if (m_DimensionLabels[dimensionIndex].size() <= labelIndex)
	{
		m_DimensionLabels[dimensionIndex].resize(labelIndex + 1);
	}
	m_DimensionLabels[dimensionIndex][labelIndex] = label;
}

size_t CRendererContext::getDimensionLabelCount(const size_t dimensionIndex) const
{
	if (m_DimensionLabels.count(dimensionIndex) == 0) { return 0; }
	return m_DimensionLabels.at(dimensionIndex).size();
}

const char* CRendererContext::getDimensionLabel(const size_t dimensionIndex, const size_t labelIndex) const
{
	if (m_DimensionLabels.count(dimensionIndex) == 0 || m_DimensionLabels.at(dimensionIndex).size() <= labelIndex) { return nullptr; }
	return m_DimensionLabels.at(dimensionIndex)[labelIndex].c_str();
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

void CRendererContext::clearTransformInfo()

{
	m_parentRendererContext  = nullptr;
	m_scale                  = 1;
	m_zoom                   = 1;
	m_rotationX              = 2;
	m_rotationY              = 1;
	m_translucency           = 1;
	m_aspect                 = 1;
	m_sampleDuration         = 0;
	m_timeScale              = 1;
	m_elementCount           = 1;
	m_flowerRingCount        = 1;
	m_hasXYZPlotDepth        = false;
	m_isAxisDisplayed        = false;
	m_isPositiveOnly         = false;
	m_isTimeLocked           = true;
	m_isScrollModeActive     = false;
	m_checkBoardVisiblity    = false;
	m_scaleVisiblity         = true;
	m_dataType               = DataType_Matrix;
	m_spectrumFrequencyRange = 0;
	m_minSpectrumFrequency   = 0;
	m_maxSpectrumFrequency   = 0;
	m_ERPPlayerActive        = false;
	m_ERPFraction            = 0;
	m_stackCount             = 1;
	m_stackIndex             = 1;
	m_faceMeshVisible        = true;
	m_scalpMeshVisible       = true;
}

void CRendererContext::scaleBy(const float f32Scale) { m_scale *= f32Scale; }

void CRendererContext::setScale(const float f32Scale) { m_scale = f32Scale; }

void CRendererContext::zoomBy(const float f32Zoom) { m_zoom *= f32Zoom; }

void CRendererContext::rotateByX(const float f32Rotation)
{
	//	#define M_SMALL_PI2 (45./2)
	m_rotationX += f32Rotation;
	//	if(m_rotationX > M_SMALL_PI2) m_rotationX= M_SMALL_PI2;
	//	if(m_rotationX <-M_SMALL_PI2) m_rotationX=-M_SMALL_PI2;
}

void CRendererContext::rotateByY(const float f32Rotation) { m_rotationY += f32Rotation; }

void CRendererContext::setTranslucency(const float f32Translucency) { m_translucency = f32Translucency; }

void CRendererContext::setAspect(const float f32Aspect) { m_aspect = f32Aspect; }

void CRendererContext::setSampleDuration(const uint64_t ui64SampleDuration) { m_sampleDuration = ui64SampleDuration; }

void CRendererContext::setTimeScale(const uint64_t ui64TimeScale) { m_timeScale = ui64TimeScale; }

void CRendererContext::setElementCount(const uint64_t ui64ElementCount) { m_elementCount = ui64ElementCount; }

void CRendererContext::setFlowerRingCount(const uint64_t ui64FlowerRingCount) { m_flowerRingCount = ui64FlowerRingCount; }

void CRendererContext::setXYZPlotDepth(const bool bHasDepth) { m_hasXYZPlotDepth = bHasDepth; }

void CRendererContext::setAxisDisplay(const bool bIsAxisDisplayed) { m_isAxisDisplayed = bIsAxisDisplayed; }

void CRendererContext::setPositiveOnly(const bool bPositiveOnly) { m_isPositiveOnly = bPositiveOnly; }

void CRendererContext::setTimeLocked(const bool timeLocked) { m_isTimeLocked = timeLocked; }

void CRendererContext::setScrollModeActive(const bool bScrollModeActive) { m_isScrollModeActive = bScrollModeActive; }

void CRendererContext::setScaleVisibility(const bool bVisibility) { m_scaleVisiblity = bVisibility; }

void CRendererContext::setCheckBoardVisibility(const bool bVisibility) { m_checkBoardVisiblity = bVisibility; }

void CRendererContext::setDataType(const EDataType eDataType) { m_dataType = eDataType; }

void CRendererContext::setSpectrumFrequencyRange(const uint32_t ui32SpectrumFrequencyRange) { m_spectrumFrequencyRange = ui32SpectrumFrequencyRange; }

void CRendererContext::setMinimumSpectrumFrequency(const uint32_t ui32MinSpectrumFrequency) { m_minSpectrumFrequency = ui32MinSpectrumFrequency; }

void CRendererContext::setMaximumSpectrumFrequency(const uint32_t ui32MaxSpectrumFrequency) { m_maxSpectrumFrequency = ui32MaxSpectrumFrequency; }

void CRendererContext::setStackCount(const uint32_t ui32StackCount) { m_stackCount = ui32StackCount; }

void CRendererContext::setStackIndex(const uint32_t ui32StackIndex) { m_stackIndex = ui32StackIndex; }

void CRendererContext::setFaceMeshVisible(const bool bVisible) { m_faceMeshVisible = bVisible; }

void CRendererContext::setScalpMeshVisible(const bool bVisible) { m_scalpMeshVisible = bVisible; }

// ____________________________________________________________________________________________________________________________________________________________________________________
//

void CRendererContext::setERPPlayerActive(const bool bActive)
{
	m_ERPPlayerActive = bActive;
}

void CRendererContext::stepERPFractionBy(const float f32ERPFraction)
{
	m_ERPFraction += f32ERPFraction;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

std::string CRendererContext::getChannelName(const uint32_t index) const
{
	if (index < m_channelName.size()) { return m_channelName[index]; }
	printf("No name for channel %u\n", index);
	return "";
}

bool CRendererContext::getChannelLocalisation(const uint32_t index, float& x, float& y, float& z) const
{
	const CVertex& l_rChannelLocalisation = m_channelLocalisation[index];
	x                                     = l_rChannelLocalisation.x;
	y                                     = l_rChannelLocalisation.y;
	z                                     = l_rChannelLocalisation.z;
	return true;
}

uint32_t CRendererContext::getChannelCount() const { return uint32_t(m_channelName.size()); }

uint32_t CRendererContext::getSelectedCount() const { return uint32_t(m_channelLookup.size()); }

uint32_t CRendererContext::getSelected(const uint32_t index) const { return m_channelLookup[index]; }

bool CRendererContext::isSelected(const uint32_t index) const
{
	for (auto& i : m_channelLookup)
	{
		if (i == index) { return true; }
	}
	return false;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

float CRendererContext::getScale() const { return m_scale * (m_parentRendererContext ? m_parentRendererContext->getScale() : 1); }

float CRendererContext::getZoom() const { return m_zoom * (m_parentRendererContext ? m_parentRendererContext->getZoom() : 1); }

float CRendererContext::getRotationX() const { return m_rotationX + (m_parentRendererContext ? m_parentRendererContext->getRotationX() : 0); }

float CRendererContext::getRotationY() const { return m_rotationY + (m_parentRendererContext ? m_parentRendererContext->getRotationY() : 0); }

// ____________________________________________________________________________________________________________________________________________________________________________________
//

float CRendererContext::getTranslucency() const { return m_translucency * (m_parentRendererContext ? m_parentRendererContext->getTranslucency() : 1); }

float CRendererContext::getAspect() const { return m_aspect * (m_parentRendererContext ? m_parentRendererContext->getAspect() : 1); }

uint64_t CRendererContext::getSampleDuration() const { return m_sampleDuration; }

uint64_t CRendererContext::getTimeScale() const { return m_timeScale; }

uint64_t CRendererContext::getElementCount() const { return m_elementCount; }

uint64_t CRendererContext::getFlowerRingCount() const { return m_flowerRingCount; }

bool CRendererContext::hasXYZPlotDepth() const { return m_hasXYZPlotDepth; }

bool CRendererContext::isAxisDisplayed() const { return m_isAxisDisplayed; }

bool CRendererContext::isPositiveOnly() const { return m_isPositiveOnly; }

bool CRendererContext::isFaceMeshVisible() const { return m_faceMeshVisible; }

bool CRendererContext::isScalpMeshVisible() const { return m_scalpMeshVisible; }

bool CRendererContext::isTimeLocked() const { return m_isTimeLocked; }

bool CRendererContext::isScrollModeActive() const { return m_isScrollModeActive; }

bool CRendererContext::getCheckBoardVisibility() const { return (m_parentRendererContext ? m_parentRendererContext->getCheckBoardVisibility() : m_checkBoardVisiblity); }

bool CRendererContext::getScaleVisibility() const { return (m_parentRendererContext ? m_parentRendererContext->getScaleVisibility() : m_scaleVisiblity); }

IRendererContext::EDataType CRendererContext::getDataType() const { return m_dataType; }

uint32_t CRendererContext::getSpectrumFrequencyRange() const { return m_spectrumFrequencyRange; }

uint32_t CRendererContext::getMinSpectrumFrequency() const { return m_minSpectrumFrequency > m_spectrumFrequencyRange ? m_spectrumFrequencyRange : m_minSpectrumFrequency; }

uint32_t CRendererContext::getMaxSpectrumFrequency() const { return m_maxSpectrumFrequency > m_spectrumFrequencyRange ? m_spectrumFrequencyRange : m_maxSpectrumFrequency; }

uint32_t CRendererContext::getStackCount() const { return m_stackCount; }

uint32_t CRendererContext::getStackIndex() const { return m_stackIndex; }

// ____________________________________________________________________________________________________________________________________________________________________________________
//

bool CRendererContext::isERPPlayerActive() const { return m_ERPPlayerActive || (m_parentRendererContext ? m_parentRendererContext->isERPPlayerActive() : false); }

float CRendererContext::getERPFraction() const
{
	const float l_fERPFraction = m_ERPFraction + (m_parentRendererContext ? m_parentRendererContext->getERPFraction() : 0);
	return l_fERPFraction - floorf(l_fERPFraction);
}
