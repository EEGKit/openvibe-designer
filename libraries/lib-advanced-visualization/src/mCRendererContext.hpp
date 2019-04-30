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

#ifndef __Mensia_AdvancedVisualization_CRendererContext_H__
#define __Mensia_AdvancedVisualization_CRendererContext_H__

#include "mIRendererContext.h"
#include "mCVertex.hpp"

#include <vector>
#include <string>
#include <map>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRendererContext : public IRendererContext
		{
		public:

			CRendererContext(IRendererContext* pParentRendererContext);

			virtual void clear();

			virtual void setParentRendererContext(IRendererContext* pParentRendererContext);

			virtual void clearChannelInfo();
			virtual void addChannel(const std::string& sChannelName, float x, float y, float z);
			virtual void selectChannel(uint32_t ui32Index);
			virtual void unselectChannel(uint32_t ui32Index);
			virtual void sortSelectedChannel(uint32_t ui32SortMode);

			virtual void setDimensionLabel(size_t dimensionIndex, size_t labelIndex, const char* label);
			virtual size_t getDimensionLabelCount(size_t dimensionIndex) const;
			virtual const char* getDimensionLabel(size_t dimensionIndex, size_t labelIndex) const;

			virtual void clearTransformInfo();
			virtual void scaleBy(float f32Scale);
			virtual void setScale(float f32Scale);
			virtual void zoomBy(float f32Zoom);
			virtual void rotateByX(float f32Rotation);
			virtual void rotateByY(float f32Rotation);

			virtual void setTranslucency(float f32Translucency);
			virtual void setAspect(float f32Aspect);
			virtual void setSampleDuration(uint64_t ui64SampleDuration);
			virtual void setTimeScale(uint64_t ui64TimeScale);
			virtual void setElementCount(uint64_t ui64ElementCount);
			virtual void setFlowerRingCount(uint64_t ui64FlowerRingCount);
			virtual void setXYZPlotDepth(bool bHasDepth);
			virtual void setAxisDisplay(bool bIsAxisDisplayed);
			virtual void setPositiveOnly(bool bPositiveOnly);
			virtual void setTimeLocked(bool bTimeLocked);
			virtual void setScrollModeActive(bool bScrollModeActive);
			virtual void setScaleVisibility(bool bVisibility);
			virtual void setCheckBoardVisibility(bool bVisibility);
			virtual void setDataType(EDataType eDataType);
			virtual void setSpectrumFrequencyRange(uint32_t ui32SpectrumFrequencyRange);
			virtual void setMinimumSpectrumFrequency(uint32_t ui32MinSpectrumFrequency);
			virtual void setMaximumSpectrumFrequency(uint32_t ui32MaxSpectrumFrequency);
			virtual void setStackCount(uint32_t ui32StackCount);
			virtual void setStackIndex(uint32_t ui32StackIndex);
			virtual void setFaceMeshVisible(bool bVisible);
			virtual void setScalpMeshVisible(bool bVisible);

			virtual void setERPPlayerActive(bool bActive);
			virtual void stepERPFractionBy(float f32ERPFraction);

			virtual std::string getChannelName(uint32_t ui32Index) const;
			virtual bool getChannelLocalisation(uint32_t ui32Index, float& x, float& y, float& z) const;
			virtual uint32_t getChannelCount() const;
			virtual uint32_t getSelectedCount() const;
			virtual uint32_t getSelected(uint32_t ui32Index) const;
			virtual bool isSelected(uint32_t ui32Index) const;

			virtual float getScale() const;
			virtual float getZoom() const;
			virtual float getRotationX() const;
			virtual float getRotationY() const;

			virtual float getTranslucency() const;
			virtual float getAspect() const;
			virtual uint64_t getSampleDuration() const;
			virtual uint64_t getTimeScale() const;
			virtual uint64_t getElementCount() const;
			virtual uint64_t getFlowerRingCount() const;
			virtual bool hasXYZPlotDepth() const;
			virtual bool isAxisDisplayed() const;
			virtual bool isPositiveOnly() const;
			virtual bool isTimeLocked() const;
			virtual bool isScrollModeActive() const;
			virtual bool getScaleVisibility() const;
			virtual bool getCheckBoardVisibility() const;
			virtual EDataType getDataType() const;
			virtual uint32_t getSpectrumFrequencyRange() const;
			virtual uint32_t getMinSpectrumFrequency() const;
			virtual uint32_t getMaxSpectrumFrequency() const;
			virtual uint32_t getStackCount() const;
			virtual uint32_t getStackIndex() const;
			virtual bool isFaceMeshVisible() const;
			virtual bool isScalpMeshVisible() const;

			virtual bool isERPPlayerActive() const;
			virtual float getERPFraction() const;

			uint32_t getMaximumSampleCountPerDisplay() const { return 1000; /*500;*/ /*128*/ }

		protected:

			IRendererContext* m_pParentRendererContext;

			std::vector < uint32_t > m_vChannelLookup;
			std::vector < std::string > m_vChannelName;
			std::vector < CVertex > m_vChannelLocalisation;
			std::map<size_t, std::vector<std::string>> m_DimensionLabels;

			std::map < std::string, float > m_vLeftRightScore;
			std::map < std::string, float > m_vFrontBackScore;

			float m_f32Scale;
			float m_f32Zoom;
			float m_f32RotationX;
			float m_f32RotationY;

			float m_f32Translucency;
			float m_f32Aspect;
			uint64_t m_ui64SampleDuration;
			uint64_t m_ui64TimeScale;
			uint64_t m_ui64ElementCount;
			uint64_t m_ui64FlowerRingCount;
			bool m_bHasXYZPlotDepth;
			bool m_bIsAxisDisplayed;
			bool m_bIsPositiveOnly;
			bool m_bIsTimeLocked;
			bool m_bIsScrollModeActive;
			bool m_bScaleVisiblity;
			bool m_bCheckBoardVisiblity;
			EDataType m_eDataType;
			uint32_t m_ui32SpectrumFrequencyRange;
			uint32_t m_ui32MinSpectrumFrequency;
			uint32_t m_ui32MaxSpectrumFrequency;
			uint32_t m_ui32StackCount;
			uint32_t m_ui32StackIndex;
			bool m_bFaceMeshVisible;
			bool m_bScalpMeshVisible;

			bool m_bERPPlayerActive;
			float m_f32ERPFraction;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererContext_H__
