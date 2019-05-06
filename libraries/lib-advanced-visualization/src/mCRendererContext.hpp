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

			explicit CRendererContext(IRendererContext* pParentRendererContext);

			void clear() override;

			void setParentRendererContext(IRendererContext* pParentRendererContext) override;

			void clearChannelInfo() override;
			void addChannel(const std::string& sChannelName, float x, float y, float z) override;
			void selectChannel(uint32_t ui32Index) override;
			void unselectChannel(uint32_t ui32Index) override;
			void sortSelectedChannel(uint32_t ui32SortMode) override;

			void setDimensionLabel(size_t dimensionIndex, size_t labelIndex, const char* label) override;
			size_t getDimensionLabelCount(size_t dimensionIndex) const override;
			const char* getDimensionLabel(size_t dimensionIndex, size_t labelIndex) const override;

			void clearTransformInfo() override;
			void scaleBy(float f32Scale) override;
			void setScale(float f32Scale) override;
			void zoomBy(float f32Zoom) override;
			void rotateByX(float f32Rotation) override;
			void rotateByY(float f32Rotation) override;

			void setTranslucency(float f32Translucency) override;
			void setAspect(float f32Aspect) override;
			void setSampleDuration(uint64_t ui64SampleDuration) override;
			void setTimeScale(uint64_t ui64TimeScale) override;
			void setElementCount(uint64_t ui64ElementCount) override;
			void setFlowerRingCount(uint64_t ui64FlowerRingCount) override;
			void setXYZPlotDepth(bool bHasDepth) override;
			void setAxisDisplay(bool bIsAxisDisplayed) override;
			void setPositiveOnly(bool bPositiveOnly) override;
			void setTimeLocked(bool bTimeLocked) override;
			void setScrollModeActive(bool bScrollModeActive) override;
			void setScaleVisibility(bool bVisibility) override;
			void setCheckBoardVisibility(bool bVisibility) override;
			void setDataType(EDataType eDataType) override;
			void setSpectrumFrequencyRange(uint32_t ui32SpectrumFrequencyRange) override;
			void setMinimumSpectrumFrequency(uint32_t ui32MinSpectrumFrequency) override;
			void setMaximumSpectrumFrequency(uint32_t ui32MaxSpectrumFrequency) override;
			void setStackCount(uint32_t ui32StackCount) override;
			void setStackIndex(uint32_t ui32StackIndex) override;
			void setFaceMeshVisible(bool bVisible) override;
			void setScalpMeshVisible(bool bVisible) override;

			void setERPPlayerActive(bool bActive) override;
			void stepERPFractionBy(float f32ERPFraction) override;

			std::string getChannelName(uint32_t ui32Index) const override;
			bool getChannelLocalisation(uint32_t ui32Index, float& x, float& y, float& z) const override;
			uint32_t getChannelCount() const override;
			uint32_t getSelectedCount() const override;
			uint32_t getSelected(uint32_t ui32Index) const override;
			bool isSelected(uint32_t ui32Index) const override;

			float getScale() const override;
			float getZoom() const override;
			float getRotationX() const override;
			float getRotationY() const override;

			float getTranslucency() const override;
			float getAspect() const override;
			uint64_t getSampleDuration() const override;
			uint64_t getTimeScale() const override;
			uint64_t getElementCount() const override;
			uint64_t getFlowerRingCount() const override;
			bool hasXYZPlotDepth() const override;
			bool isAxisDisplayed() const override;
			bool isPositiveOnly() const override;
			bool isTimeLocked() const override;
			bool isScrollModeActive() const override;
			bool getScaleVisibility() const override;
			bool getCheckBoardVisibility() const override;
			EDataType getDataType() const override;
			uint32_t getSpectrumFrequencyRange() const override;
			uint32_t getMinSpectrumFrequency() const override;
			uint32_t getMaxSpectrumFrequency() const override;
			uint32_t getStackCount() const override;
			uint32_t getStackIndex() const override;
			bool isFaceMeshVisible() const override;
			bool isScalpMeshVisible() const override;

			bool isERPPlayerActive() const override;
			float getERPFraction() const override;

			uint32_t getMaximumSampleCountPerDisplay() const override
			{
				return 1000; /*500;*/ /*128*/
			}

		protected:

			IRendererContext* m_pParentRendererContext;

			std::vector<uint32_t> m_vChannelLookup;
			std::vector<std::string> m_vChannelName;
			std::vector<CVertex> m_vChannelLocalisation;
			std::map<size_t, std::vector<std::string>> m_DimensionLabels;

			std::map<std::string, float> m_vLeftRightScore;
			std::map<std::string, float> m_vFrontBackScore;

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
	} // namespace AdvancedVisualization
}  // namespace Mensia

#endif // __Mensia_AdvancedVisualization_CRendererContext_H__
