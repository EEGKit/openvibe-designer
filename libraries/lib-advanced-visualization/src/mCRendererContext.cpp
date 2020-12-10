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
#include <iostream>

using namespace Mensia;
using namespace AdvancedVisualization;


namespace {
void getLeftRightScore(std::map<std::string, float>& scores, const std::vector<std::string>& names, const std::vector<CVertex>& positions)
{
	for (size_t i = 0; i < names.size() && i < positions.size(); ++i)
	{
		std::string name = std::string(",") + names[i] + std::string(",");
		std::transform(name.begin(), name.end(), name.begin(), tolower);
		scores[name] = float(positions[i].x * 1E-0 + positions[i].y * 1E-10 + positions[i].z * 1E-5);
	}
}

void getFrontBackScore(std::map<std::string, float>& scores, const std::vector<std::string>& names, const std::vector<CVertex>& positions)
{
	for (size_t i = 0; i < names.size() && i < positions.size(); ++i)
	{
		std::string name = std::string(",") + names[i] + std::string(",");
		std::transform(name.begin(), name.end(), name.begin(), tolower);
		scores[name] = float(positions[i].x * 1E-5 + positions[i].y * 1E-10 + positions[i].z * 1E-0);
	}
}

struct SSortAlpha
{
	explicit SSortAlpha(const std::vector<std::string>& channels) : names(channels) { }

	bool operator()(const size_t i, const size_t j) const
	{
		std::string nameI = names[i];
		std::string nameJ = names[j];
		std::transform(nameI.begin(), nameI.end(), nameI.begin(), tolower);
		std::transform(nameJ.begin(), nameJ.end(), nameJ.begin(), tolower);

		return nameI < nameJ;
	}

	const std::vector<std::string>& names;
};

struct SSortSpecial
{
	SSortSpecial(const std::vector<std::string>& channels, const std::map<std::string, float>& scoreMap)
		: names(channels), scores(scoreMap) { }

	bool operator()(const size_t i, const size_t j) const
	{
		float scoreI = 0;
		float scoreJ = 0;

		std::string nameI = std::string(",") + names[i] + std::string(",");
		std::string nameJ = std::string(",") + names[j] + std::string(",");
		std::transform(nameI.begin(), nameI.end(), nameI.begin(), tolower);
		std::transform(nameJ.begin(), nameJ.end(), nameJ.begin(), tolower);

		for (auto it = scores.begin(); it != scores.end(); ++it)
		{
			if (it->first == nameI) { scoreI = it->second; }
			if (it->first == nameJ) { scoreJ = it->second; }
		}

		return scoreI < scoreJ;
	}

	const std::vector<std::string>& names;
	const std::map<std::string, float>& scores;
};
}  // namespace


CRendererContext::CRendererContext(CRendererContext* parentCtx)
{
	clear();
	setParentRendererContext(parentCtx);
}

void CRendererContext::clear()
{
	this->clearChannelInfo();
	this->clearTransformInfo();
	m_dimLabels.clear();
}

void CRendererContext::clearChannelInfo()
{
	m_channelLookup.clear();
	m_channelName.clear();
	m_channelPos.clear();

	m_leftRightScore.clear();
	m_frontBackScore.clear();
}

void CRendererContext::addChannel(const std::string& name, const float x, const float y, const float z)
{
	const auto norm     = float(sqrt(x * x + y * y + z * z));
	const float invNorm = (norm != 0 ? 1.F / norm : 0);
	CVertex pos;
	pos.x = x * invNorm;
	pos.y = y * invNorm;
	pos.z = z * invNorm;
	m_channelLookup.push_back(m_channelName.size());
	m_channelName.push_back(name);
	m_channelPos.push_back(pos);
}

void CRendererContext::selectChannel(const size_t index)
{
	for (auto& i : m_channelLookup) { if (i == index) { return; } }
	m_channelLookup.push_back(index);
}

void CRendererContext::unselectChannel(const size_t index)
{
	for (size_t i = 0; i < m_channelLookup.size(); ++i)
	{
		if (m_channelLookup[i] == index)
		{
			m_channelLookup.erase(m_channelLookup.begin() + i);
			return;
		}
	}
}

void CRendererContext::sortSelectedChannel(const size_t mode)
{
	if (m_leftRightScore.empty()) { getLeftRightScore(m_leftRightScore, m_channelName, m_channelPos); }
	if (m_frontBackScore.empty()) { getFrontBackScore(m_frontBackScore, m_channelName, m_channelPos); }

	switch (mode)
	{
		case 0: break;

		case 1: std::stable_sort(m_channelLookup.begin(), m_channelLookup.end());
			break;

		case 2: std::stable_sort(m_channelLookup.begin(), m_channelLookup.end(), SSortAlpha(m_channelName));
			break;

		case 3: std::reverse(m_channelLookup.begin(), m_channelLookup.end());
			break;

		case 4: std::stable_sort(m_channelLookup.begin(), m_channelLookup.end(), SSortSpecial(m_channelName, m_leftRightScore));
			break;

		case 5: std::stable_sort(m_channelLookup.begin(), m_channelLookup.end(), SSortSpecial(m_channelName, m_frontBackScore));
			break;

		default: break;
	}
}

///-------------------------------------------------------------------------------------------------
void CRendererContext::setDimensionLabel(const size_t idx1, const size_t idx2, const char* label)
{
	if (m_dimLabels[idx1].size() <= idx2) { m_dimLabels[idx1].resize(idx2 + 1); }
	m_dimLabels[idx1][idx2] = label;
}

size_t CRendererContext::getDimensionLabelCount(const size_t index) const
{
	if (m_dimLabels.count(index) == 0) { return 0; }
	return m_dimLabels.at(index).size();
}

const char* CRendererContext::getDimensionLabel(const size_t idx1, const size_t idx2) const
{
	if (m_dimLabels.count(idx1) == 0 || m_dimLabels.at(idx1).size() <= idx2) { return nullptr; }
	return m_dimLabels.at(idx1)[idx2].c_str();
}

///-------------------------------------------------------------------------------------------------
void CRendererContext::clearTransformInfo()
{
	m_parentCtx           = nullptr;
	m_scale               = 1;
	m_zoom                = 1;
	m_rotationX           = 2;
	m_rotationY           = 1;
	m_translucency        = 1;
	m_aspect              = 1;
	m_sampleDuration      = 0;
	m_timeScale           = 1;
	m_nElement            = 1;
	m_nFlowerRing         = 1;
	m_hasXYZPlotDepth     = false;
	m_isAxisDisplayed     = false;
	m_isPositiveOnly      = false;
	m_isTimeLocked        = true;
	m_isScrollModeActive  = false;
	m_checkBoardVisiblity = false;
	m_scaleVisiblity      = true;
	m_dataType            = EDataType::Matrix;
	m_spectrumFreqRange   = 0;
	m_minSpectrumFreq     = 0;
	m_maxSpectrumFreq     = 0;
	m_erpPlayerActive     = false;
	m_erpFraction         = 0;
	m_nStack              = 1;
	m_stackIdx            = 1;
	m_faceMeshVisible     = true;
	m_scalpMeshVisible    = true;
}

std::string CRendererContext::getChannelName(const size_t index) const
{
	if (index < m_channelName.size()) { return m_channelName[index]; }
	std::cout << "No name for channel " << index << std::endl;
	return std::string();
}

bool CRendererContext::getChannelLocalisation(const size_t index, float& x, float& y, float& z) const
{
	const CVertex& tmp = m_channelPos[index];
	x                  = tmp.x;
	y                  = tmp.y;
	z                  = tmp.z;
	return true;
}

bool CRendererContext::isSelected(const size_t index) const
{
	for (auto& i : m_channelLookup) { if (i == index) { return true; } }
	return false;
}

float CRendererContext::getERPFraction() const
{
	const float erpFraction = m_erpFraction + (m_parentCtx ? m_parentCtx->getERPFraction() : 0);
	return erpFraction - floorf(erpFraction);
}
