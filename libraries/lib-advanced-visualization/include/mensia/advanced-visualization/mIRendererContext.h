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
#include <cstdlib>	// size_t for unix

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

			static IRendererContext* create(IRendererContext* parentCtx = nullptr);
			static void release(IRendererContext* ctx);

			virtual ~IRendererContext() = default;

			virtual void clear() = 0;

			virtual void setParentRendererContext(IRendererContext* ctx) = 0;

			virtual void clearChannelInfo() = 0;
			virtual void addChannel(const std::string& name, float x = 0, float y = 0, float z = 0) = 0;
			virtual void selectChannel(size_t index) = 0;
			virtual void unselectChannel(size_t index) = 0;
			virtual void sortSelectedChannel(size_t mode) = 0;

			virtual void setDimensionLabel(size_t idx1, size_t idx2, const char* label) = 0;
			virtual size_t getDimensionLabelCount(size_t index) const = 0;
			virtual const char* getDimensionLabel(size_t idx1, size_t idx2) const = 0;

			virtual void clearTransformInfo() = 0;
			virtual void scaleBy(const float scale) = 0;
			virtual void setScale(const float scale) = 0;
			virtual void zoomBy(const float zoom) = 0;
			virtual void rotateByX(const float rotation) = 0;
			virtual void rotateByY(const float rotation) = 0;

			virtual void setTranslucency(const float translucency) = 0;
			virtual void setAspect(const float aspect) = 0;
			virtual void setSampleDuration(const uint64_t duration) = 0;
			virtual void setTimeScale(const size_t scale) = 0;
			virtual void setElementCount(const size_t count) = 0;
			virtual void setFlowerRingCount(const size_t count) = 0;
			virtual void setXYZPlotDepth(const bool hasDepth) = 0;
			virtual void setAxisDisplay(const bool isAxisDisplayed) = 0;
			virtual void setPositiveOnly(const bool positiveOnly) = 0;
			virtual void setTimeLocked(const bool timeLocked) = 0;
			virtual void setScrollModeActive(const bool scrollModeActive) = 0;
			virtual void setScaleVisibility(const bool visibility) = 0;
			virtual void setCheckBoardVisibility(const bool visibility) = 0;
			virtual void setDataType(const EDataType type) = 0;
			virtual void setSpectrumFrequencyRange(const size_t range) = 0;
			virtual void setMinimumSpectrumFrequency(const size_t frequency) = 0;
			virtual void setMaximumSpectrumFrequency(const size_t frequency) = 0;
			virtual void setStackCount(const size_t count) = 0;
			virtual void setStackIndex(const size_t index) = 0;
			virtual void setFaceMeshVisible(const bool visible) = 0;
			virtual void setScalpMeshVisible(const bool visible) = 0;

			virtual void setERPPlayerActive(const bool active) = 0;
			virtual void stepERPFractionBy(const float erpFraction) = 0;

			virtual std::string getChannelName(const size_t index) const = 0;
			virtual bool getChannelLocalisation(const size_t index, float& x, float& y, float& z) const = 0;
			virtual size_t getChannelCount() const = 0;
			virtual size_t getSelectedCount() const = 0;
			virtual size_t getSelected(const size_t index) const = 0;
			virtual bool isSelected(const size_t index) const = 0;

			virtual float getScale() const = 0;
			virtual float getZoom() const = 0;
			virtual float getRotationX() const = 0;
			virtual float getRotationY() const = 0;

			virtual float getTranslucency() const = 0;
			virtual float getAspect() const = 0;
			virtual uint64_t getSampleDuration() const = 0;
			virtual size_t getTimeScale() const = 0;
			virtual size_t getElementCount() const = 0;
			virtual size_t getFlowerRingCount() const = 0;
			virtual bool hasXYZPlotDepth() const = 0;
			virtual bool isAxisDisplayed() const = 0;
			virtual bool isPositiveOnly() const = 0;
			virtual bool isTimeLocked() const = 0;
			virtual bool isScrollModeActive() const = 0;
			virtual bool getScaleVisibility() const = 0;
			virtual bool getCheckBoardVisibility() const = 0;
			virtual EDataType getDataType() const = 0;
			virtual size_t getSpectrumFrequencyRange() const = 0;
			virtual size_t getMinSpectrumFrequency() const = 0;
			virtual size_t getMaxSpectrumFrequency() const = 0;
			virtual size_t getStackCount() const = 0;
			virtual size_t getStackIndex() const = 0;
			virtual bool isFaceMeshVisible() const = 0;
			virtual bool isScalpMeshVisible() const = 0;

			virtual bool isERPPlayerActive() const = 0;
			virtual float getERPFraction() const = 0;

			virtual size_t getMaximumSampleCountPerDisplay() const = 0;
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
