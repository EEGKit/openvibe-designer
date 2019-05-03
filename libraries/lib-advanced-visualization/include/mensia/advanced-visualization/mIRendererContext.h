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

#include "m_defines.h"

#include <cstdint>
#include <string>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class LMAV_API IRendererContext
		{
		public:

			typedef enum
			{
				DataType_Matrix,
				DataType_Signal,
				DataType_Spectrum,
				DataType_TimeFrequency
			} EDataType;

			static IRendererContext* create(IRendererContext* pParentRendererContext = nullptr);
			static void release(IRendererContext* pRendererContext);

			virtual ~IRendererContext() = default;

			virtual void clear() = 0;

			virtual void setParentRendererContext(IRendererContext* pParentRendererContext) = 0;

			virtual void clearChannelInfo() = 0;
			virtual void addChannel(const std::string& sChannelName, float x = 0, float y = 0, float z = 0) = 0;
			virtual void selectChannel(uint32_t ui32Index) = 0;
			virtual void unselectChannel(uint32_t ui32Index) = 0;
			virtual void sortSelectedChannel(uint32_t ui32SortMode) = 0;

			virtual void setDimensionLabel(size_t dimensionIndex, size_t labelIndex, const char* label) = 0;
			virtual size_t getDimensionLabelCount(size_t dimensionIndex) const = 0;
			virtual const char* getDimensionLabel(size_t dimensionIndex, size_t labelIndex) const = 0;

			virtual void clearTransformInfo() = 0;
			virtual void scaleBy(float f32Scale) = 0;
			virtual void setScale(float f32Scale) = 0;
			virtual void zoomBy(float f32Zoom) = 0;
			virtual void rotateByX(float f32Rotation) = 0;
			virtual void rotateByY(float f32Rotation) = 0;

			virtual void setTranslucency(float f32Translucency) = 0;
			virtual void setAspect(float f32Aspect) = 0;
			virtual void setSampleDuration(uint64_t ui64SampleDuration) = 0;
			virtual void setTimeScale(uint64_t ui64TimeScale) = 0;
			virtual void setElementCount(uint64_t ui64ElementCount) = 0;
			virtual void setFlowerRingCount(uint64_t ui64FlowerRingCount) = 0;
			virtual void setXYZPlotDepth(bool bHasDepth) = 0;
			virtual void setAxisDisplay(bool bIsAxisDisplayed) = 0;
			virtual void setPositiveOnly(bool bPositiveOnly) = 0;
			virtual void setTimeLocked(bool bTimeLocked) = 0;
			virtual void setScrollModeActive(bool bScrollModeActive) = 0;
			virtual void setScaleVisibility(bool bVisibility) = 0;
			virtual void setCheckBoardVisibility(bool bVisibility) = 0;
			virtual void setDataType(EDataType eDataType) = 0;
			virtual void setSpectrumFrequencyRange(uint32_t ui32SpectrumFrequencyRange) = 0;
			virtual void setMinimumSpectrumFrequency(uint32_t ui32MinSpectrumFrequency) = 0;
			virtual void setMaximumSpectrumFrequency(uint32_t ui32MaxSpectrumFrequency) = 0;
			virtual void setStackCount(uint32_t ui32StackCount) = 0;
			virtual void setStackIndex(uint32_t ui32StackIndex) = 0;
			virtual void setFaceMeshVisible(bool bVisible) = 0;
			virtual void setScalpMeshVisible(bool bVisible) = 0;

			virtual void setERPPlayerActive(bool bActive) = 0;
			virtual void stepERPFractionBy(float f32ERPFraction) = 0;

			virtual std::string getChannelName(uint32_t ui32Index) const = 0;
			virtual bool getChannelLocalisation(uint32_t ui32Index, float& x, float& y, float& z) const = 0;
			virtual uint32_t getChannelCount() const = 0;
			virtual uint32_t getSelectedCount() const = 0;
			virtual uint32_t getSelected(uint32_t ui32Index) const = 0;
			virtual bool isSelected(uint32_t ui32Index) const = 0;

			virtual float getScale() const = 0;
			virtual float getZoom() const = 0;
			virtual float getRotationX() const = 0;
			virtual float getRotationY() const = 0;

			virtual float getTranslucency() const = 0;
			virtual float getAspect() const = 0;
			virtual uint64_t getSampleDuration() const = 0;
			virtual uint64_t getTimeScale() const = 0;
			virtual uint64_t getElementCount() const = 0;
			virtual uint64_t getFlowerRingCount() const = 0;
			virtual bool hasXYZPlotDepth() const = 0;
			virtual bool isAxisDisplayed() const = 0;
			virtual bool isPositiveOnly() const = 0;
			virtual bool isTimeLocked() const = 0;
			virtual bool isScrollModeActive() const = 0;
			virtual bool getScaleVisibility() const = 0;
			virtual bool getCheckBoardVisibility() const = 0;
			virtual EDataType getDataType() const = 0;
			virtual uint32_t getSpectrumFrequencyRange() const = 0;
			virtual uint32_t getMinSpectrumFrequency() const = 0;
			virtual uint32_t getMaxSpectrumFrequency() const = 0;
			virtual uint32_t getStackCount() const = 0;
			virtual uint32_t getStackIndex() const = 0;
			virtual bool isFaceMeshVisible() const = 0;
			virtual bool isScalpMeshVisible() const = 0;

			virtual bool isERPPlayerActive() const = 0;
			virtual float getERPFraction() const = 0;

			virtual uint32_t getMaximumSampleCountPerDisplay() const = 0;
		};
	}  // namespace AdvancedVisualization
}  // namespace Mensia

