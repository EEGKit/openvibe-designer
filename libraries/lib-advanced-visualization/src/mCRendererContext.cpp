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

#include "mCRendererContext.hpp"

#include <algorithm>
#include <map>
#include <cstdio>
#include <cmath>

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

namespace
{
	static void getLeftRightScore(std::map < std::string, float >& vScore, const std::vector < std::string >& vChannelName, const std::vector < CVertex >& vChannelLocalisation)
	{
		for(uint32_t i=0; i<vChannelName.size() && i<vChannelLocalisation.size(); i++)
		{
			std::string l_sChannelName=std::string(",")+vChannelName[i]+std::string(",");
			std::transform(l_sChannelName.begin(), l_sChannelName.end(), l_sChannelName.begin(), ::tolower);
			vScore[l_sChannelName] = float(vChannelLocalisation[i].x*1E-0 + vChannelLocalisation[i].y*1E-10 + vChannelLocalisation[i].z*1E-5);
		}
	}

	static void getFrontBackScore(std::map < std::string, float >& vScore, const std::vector < std::string >& vChannelName, const std::vector < CVertex >& vChannelLocalisation)
	{
		for(uint32_t i=0; i<vChannelName.size() && i<vChannelLocalisation.size(); i++)
		{
			std::string l_sChannelName=std::string(",")+vChannelName[i]+std::string(",");
			std::transform(l_sChannelName.begin(), l_sChannelName.end(), l_sChannelName.begin(), ::tolower);
			vScore[l_sChannelName] = float(vChannelLocalisation[i].x*1E-5 + vChannelLocalisation[i].y*1E-10 + vChannelLocalisation[i].z*1E-0);
		}
	}

	struct sort_alpha
	{
		sort_alpha(const std::vector < std::string >& vChannelName)
			:m_vChannelName(vChannelName)
		{
		}

		bool operator()(uint32_t i, uint32_t j)
		{
			std::string l_sChannelName_i=m_vChannelName[i];
			std::string l_sChannelName_j=m_vChannelName[j];
			std::transform(l_sChannelName_i.begin(), l_sChannelName_i.end(), l_sChannelName_i.begin(), ::tolower);
			std::transform(l_sChannelName_j.begin(), l_sChannelName_j.end(), l_sChannelName_j.begin(), ::tolower);

			return l_sChannelName_i < l_sChannelName_j;
		}

		const std::vector < std::string >& m_vChannelName;
	};

	struct sort_special
	{
		sort_special(const std::vector < std::string >& vChannelName, const std::map < std::string, float >& vScore)
			:m_vChannelName(vChannelName)
			,m_vScore(vScore)
		{
		}

		bool operator()(uint32_t i, uint32_t j)
		{
			std::map < std::string, float >::const_iterator it;

			float l_f32Score_i=0;
			float l_f32Score_j=0;

			std::string l_sChannelName_i=std::string(",")+m_vChannelName[i]+std::string(",");
			std::string l_sChannelName_j=std::string(",")+m_vChannelName[j]+std::string(",");
			std::transform(l_sChannelName_i.begin(), l_sChannelName_i.end(), l_sChannelName_i.begin(), ::tolower);
			std::transform(l_sChannelName_j.begin(), l_sChannelName_j.end(), l_sChannelName_j.begin(), ::tolower);

			for(it=m_vScore.begin(); it!=m_vScore.end(); it++)
			{
				if(it->first == l_sChannelName_i) l_f32Score_i = it->second;
				if(it->first == l_sChannelName_j) l_f32Score_j = it->second;
			}

			return l_f32Score_i < l_f32Score_j;
		}

		const std::vector < std::string >& m_vChannelName;
		const std::map < std::string, float >& m_vScore;
	};
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

CRendererContext::CRendererContext(IRendererContext* pParentRendererContext)
	:m_pParentRendererContext(NULL)
{
	this->clear();
	this->setParentRendererContext(pParentRendererContext);
}

void CRendererContext::clear(void)
{
	this->clearChannelInfo();
	this->clearTransformInfo();
	m_DimensionLabels.clear();
}

void CRendererContext::setParentRendererContext(IRendererContext* pParentRendererContext)
{
	m_pParentRendererContext=pParentRendererContext;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

void CRendererContext::clearChannelInfo(void)
{
	m_vChannelLookup.clear();
	m_vChannelName.clear();
	m_vChannelLocalisation.clear();

	m_vLeftRightScore.clear();
	m_vFrontBackScore.clear();
}

void CRendererContext::addChannel(const std::string& sChannelName, float x, float y, float z)
{
	float l_fNorm=float(::sqrt(x*x+ y*y + z*z));
	float l_fInvNorm=(l_fNorm!=0?1.f/l_fNorm:0);
	CVertex l_oChannelLocalisation;
	l_oChannelLocalisation.x = x*l_fInvNorm;
	l_oChannelLocalisation.y = y*l_fInvNorm;
	l_oChannelLocalisation.z = z*l_fInvNorm;
	m_vChannelLookup.push_back(m_vChannelName.size());
	m_vChannelName.push_back(sChannelName);
	m_vChannelLocalisation.push_back(l_oChannelLocalisation);
}

void CRendererContext::selectChannel(uint32_t ui32Index)
{
	for(uint32_t i=0; i<m_vChannelLookup.size(); i++)
	{
		if(m_vChannelLookup[i]==ui32Index) return;
	}
	m_vChannelLookup.push_back(ui32Index);
}

void CRendererContext::unselectChannel(uint32_t ui32Index)
{
	for(uint32_t i=0; i<m_vChannelLookup.size(); i++)
	{
		if(m_vChannelLookup[i]==ui32Index)
		{
			m_vChannelLookup.erase(m_vChannelLookup.begin()+i);
			return;
		}
	}
}

void CRendererContext::sortSelectedChannel(uint32_t ui32SortMode)
{
	if(m_vLeftRightScore.size() == 0) getLeftRightScore(m_vLeftRightScore, m_vChannelName, m_vChannelLocalisation);
	if(m_vFrontBackScore.size() == 0) getFrontBackScore(m_vFrontBackScore, m_vChannelName, m_vChannelLocalisation);

	switch(ui32SortMode)
	{
		case 0:
			break;

		case 1:
			std::stable_sort(m_vChannelLookup.begin(), m_vChannelLookup.end());
			break;

		case 2:
			std::stable_sort(m_vChannelLookup.begin(), m_vChannelLookup.end(), sort_alpha(m_vChannelName));
			break;

		case 3:
			std::reverse(m_vChannelLookup.begin(), m_vChannelLookup.end());
			break;

		case 4:
			std::stable_sort(m_vChannelLookup.begin(), m_vChannelLookup.end(), sort_special(m_vChannelName, m_vLeftRightScore));
			break;

		case 5:
			std::stable_sort(m_vChannelLookup.begin(), m_vChannelLookup.end(), sort_special(m_vChannelName, m_vFrontBackScore));
			break;

		default:
			break;
	}
}

// ____________________________________________________________________________________________________________________________________________________________________________________

void CRendererContext::setDimensionLabel(size_t dimensionIndex, size_t labelIndex, const char* label)
{
	if (m_DimensionLabels[dimensionIndex].size() <= labelIndex)
	{
		m_DimensionLabels[dimensionIndex].resize(labelIndex + 1);
	}
	m_DimensionLabels[dimensionIndex][labelIndex] = label;
}

size_t CRendererContext::getDimensionLabelCount(size_t dimensionIndex) const
{
	if (m_DimensionLabels.count(dimensionIndex) == 0)
	{
		return 0;
	}
	return m_DimensionLabels.at(dimensionIndex).size();
}

const char* CRendererContext::getDimensionLabel(size_t dimensionIndex, size_t labelIndex) const
{
	if (m_DimensionLabels.count(dimensionIndex) == 0 || m_DimensionLabels.at(dimensionIndex).size() <= labelIndex)
	{
		return nullptr;
	}
	return m_DimensionLabels.at(dimensionIndex)[labelIndex].c_str();
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

void CRendererContext::clearTransformInfo(void)
{
	m_pParentRendererContext=NULL;
	m_f32Scale=1;
	m_f32Zoom=1;
	m_f32RotationX=2;
	m_f32RotationY=1;
	m_f32Translucency=1;
	m_f32Aspect=1;
	m_ui64SampleDuration=0;
	m_ui64TimeScale=1;
	m_ui64ElementCount=1;
	m_ui64FlowerRingCount=1;
	m_bHasXYZPlotDepth=false;
	m_bIsAxisDisplayed=false;
	m_bIsPositiveOnly=false;
	m_bIsTimeLocked=true;
	m_bIsScrollModeActive=false;
	m_bCheckBoardVisiblity=false;
	m_bScaleVisiblity=true;
	m_eDataType=DataType_Matrix;
	m_ui32SpectrumFrequencyRange=0;
	m_ui32MinSpectrumFrequency=0;
	m_ui32MaxSpectrumFrequency=0;
	m_bERPPlayerActive=false;
	m_f32ERPFraction=0;
	m_ui32StackCount=1;
	m_ui32StackIndex=1;
	m_bFaceMeshVisible = true;
	m_bScalpMeshVisible = true;
}

void CRendererContext::scaleBy(float f32Scale)
{
	m_f32Scale*=f32Scale;
}

void CRendererContext::setScale(float f32Scale)
{
	m_f32Scale = f32Scale;
}

void CRendererContext::zoomBy(float f32Zoom)
{
	m_f32Zoom*=f32Zoom;
}

void CRendererContext::rotateByX(float f32Rotation)
{
//	#define M_SMALL_PI2 (45./2)
	m_f32RotationX+=f32Rotation;
//	if(m_f32RotationX > M_SMALL_PI2) m_f32RotationX= M_SMALL_PI2;
//	if(m_f32RotationX <-M_SMALL_PI2) m_f32RotationX=-M_SMALL_PI2;
}

void CRendererContext::rotateByY(float f32Rotation)
{
	m_f32RotationY+=f32Rotation;
}

void CRendererContext::setTranslucency(float f32Translucency)
{
	m_f32Translucency=f32Translucency;
}

void CRendererContext::setAspect(float f32Aspect)
{
	m_f32Aspect=f32Aspect;
}

void CRendererContext::setSampleDuration(uint64_t ui64SampleDuration)
{
	m_ui64SampleDuration=ui64SampleDuration;
}

void CRendererContext::setTimeScale(uint64_t ui64TimeScale)
{
	m_ui64TimeScale=ui64TimeScale;
}

void CRendererContext::setElementCount(uint64_t ui64ElementCount)
{
	m_ui64ElementCount=ui64ElementCount;
}

void CRendererContext::setFlowerRingCount(uint64_t ui64FlowerRingCount)
{
	m_ui64FlowerRingCount=ui64FlowerRingCount;
}

void CRendererContext::setXYZPlotDepth(bool bHasDepth)
{
	m_bHasXYZPlotDepth=bHasDepth;
}

void CRendererContext::setAxisDisplay(bool bIsAxisDisplayed)
{
	m_bIsAxisDisplayed=bIsAxisDisplayed;
}

void CRendererContext::setPositiveOnly(bool bPositiveOnly)
{
	m_bIsPositiveOnly=bPositiveOnly;
}

void CRendererContext::setTimeLocked(bool bIsTimeLocked)
{
	m_bIsTimeLocked=bIsTimeLocked;
}

void CRendererContext::setScrollModeActive(bool bScrollModeActive)
{
	m_bIsScrollModeActive=bScrollModeActive;
}

void CRendererContext::setScaleVisibility(bool bVisibility)
{
	m_bScaleVisiblity=bVisibility;
}

void CRendererContext::setCheckBoardVisibility(bool bVisibility)
{
	m_bCheckBoardVisiblity=bVisibility;
}

void CRendererContext::setDataType(IRendererContext::EDataType eDataType)
{
	m_eDataType=eDataType;
}

void CRendererContext::setSpectrumFrequencyRange(uint32_t ui32SpectrumFrequencyRange)
{
	m_ui32SpectrumFrequencyRange=ui32SpectrumFrequencyRange;
}

void CRendererContext::setMinimumSpectrumFrequency(uint32_t ui32MinSpectrumFrequency)
{
	m_ui32MinSpectrumFrequency=ui32MinSpectrumFrequency;
}

void CRendererContext::setMaximumSpectrumFrequency(uint32_t ui32MaxSpectrumFrequency)
{
	m_ui32MaxSpectrumFrequency=ui32MaxSpectrumFrequency;
}

void CRendererContext::setStackCount(uint32_t ui32StackCount)
{
	m_ui32StackCount=ui32StackCount;
}

void CRendererContext::setStackIndex(uint32_t ui32StackIndex)
{
	m_ui32StackIndex=ui32StackIndex;
}

void CRendererContext::setFaceMeshVisible(bool bVisible)
{
	m_bFaceMeshVisible = bVisible;
}

void CRendererContext::setScalpMeshVisible(bool bVisible)
{
	m_bScalpMeshVisible = bVisible;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

void CRendererContext::setERPPlayerActive(bool bActive)
{
	m_bERPPlayerActive=bActive;
}

void CRendererContext::stepERPFractionBy(float f32ERPFraction)
{
	m_f32ERPFraction+=f32ERPFraction;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

std::string CRendererContext::getChannelName(uint32_t ui32Index) const
{
	if(ui32Index < m_vChannelName.size())
	{
		return m_vChannelName[ui32Index];
	}
	::printf("No name for channel %u\n", ui32Index);
	return "";
}

bool CRendererContext::getChannelLocalisation(uint32_t ui32Index, float& x, float& y, float& z) const
{
	const CVertex& l_rChannelLocalisation=m_vChannelLocalisation[ui32Index];
	x = l_rChannelLocalisation.x;
	y = l_rChannelLocalisation.y;
	z = l_rChannelLocalisation.z;
	return true;
}

uint32_t CRendererContext::getChannelCount(void) const
{
	return m_vChannelName.size();
}

uint32_t CRendererContext::getSelectedCount(void) const
{
	return m_vChannelLookup.size();
}

uint32_t CRendererContext::getSelected(uint32_t ui32Index) const
{
	return m_vChannelLookup[ui32Index];
}

bool CRendererContext::isSelected(uint32_t ui32Index) const
{
	for(uint32_t i=0; i<m_vChannelLookup.size(); i++)
	{
		if(m_vChannelLookup[i]==ui32Index)
		{
			return true;
		}
	}
	return false;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

float CRendererContext::getScale(void) const
{
	return m_f32Scale*(m_pParentRendererContext?m_pParentRendererContext->getScale():1);
}

float CRendererContext::getZoom(void) const
{
	return m_f32Zoom*(m_pParentRendererContext?m_pParentRendererContext->getZoom():1);
}

float CRendererContext::getRotationX(void) const
{
	return m_f32RotationX+(m_pParentRendererContext?m_pParentRendererContext->getRotationX():0);
}

float CRendererContext::getRotationY(void) const
{
	return m_f32RotationY+(m_pParentRendererContext?m_pParentRendererContext->getRotationY():0);
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

float CRendererContext::getTranslucency(void) const
{
	return m_f32Translucency*(m_pParentRendererContext?m_pParentRendererContext->getTranslucency():1);
}

float CRendererContext::getAspect(void) const
{
	return m_f32Aspect*(m_pParentRendererContext?m_pParentRendererContext->getAspect():1);
}

uint64_t CRendererContext::getSampleDuration(void) const
{
	return m_ui64SampleDuration;
}

uint64_t CRendererContext::getTimeScale(void) const
{
	return m_ui64TimeScale;
}

uint64_t CRendererContext::getElementCount(void) const
{
	return m_ui64ElementCount;
}

uint64_t CRendererContext::getFlowerRingCount(void) const
{
	return m_ui64FlowerRingCount;
}

bool CRendererContext::hasXYZPlotDepth(void) const
{
	return m_bHasXYZPlotDepth;
}

bool CRendererContext::isAxisDisplayed(void) const
{
	return m_bIsAxisDisplayed;
}

bool CRendererContext::isPositiveOnly(void) const
{
	return m_bIsPositiveOnly;
}

bool CRendererContext::isFaceMeshVisible(void) const
{
	return m_bFaceMeshVisible;
}

bool CRendererContext::isScalpMeshVisible(void) const
{
	return m_bScalpMeshVisible;
}

bool CRendererContext::isTimeLocked(void) const
{
	return m_bIsTimeLocked;
}

bool CRendererContext::isScrollModeActive(void) const
{
	return m_bIsScrollModeActive;
}

bool CRendererContext::getCheckBoardVisibility(void) const
{
	return (m_pParentRendererContext?m_pParentRendererContext->getCheckBoardVisibility():m_bCheckBoardVisiblity);
}

bool CRendererContext::getScaleVisibility(void) const
{
	return (m_pParentRendererContext?m_pParentRendererContext->getScaleVisibility():m_bScaleVisiblity);
}

IRendererContext::EDataType CRendererContext::getDataType(void) const
{
	return m_eDataType;
}

uint32_t CRendererContext::getSpectrumFrequencyRange(void) const
{
	return m_ui32SpectrumFrequencyRange;
}

uint32_t CRendererContext::getMinSpectrumFrequency(void) const
{
	return m_ui32MinSpectrumFrequency>m_ui32SpectrumFrequencyRange?m_ui32SpectrumFrequencyRange:m_ui32MinSpectrumFrequency;
}

uint32_t CRendererContext::getMaxSpectrumFrequency(void) const
{
	return m_ui32MaxSpectrumFrequency>m_ui32SpectrumFrequencyRange?m_ui32SpectrumFrequencyRange:m_ui32MaxSpectrumFrequency;
}

uint32_t CRendererContext::getStackCount(void) const
{
	return m_ui32StackCount;
}

uint32_t CRendererContext::getStackIndex(void) const
{
	return m_ui32StackIndex;
}

// ____________________________________________________________________________________________________________________________________________________________________________________
//

bool CRendererContext::isERPPlayerActive(void) const
{
	return m_bERPPlayerActive || (m_pParentRendererContext?m_pParentRendererContext->isERPPlayerActive():false);
}

float CRendererContext::getERPFraction(void) const
{
	float l_fERPFraction = m_f32ERPFraction + (m_pParentRendererContext?m_pParentRendererContext->getERPFraction():0);
	return l_fERPFraction - ::floorf(l_fERPFraction);
}
