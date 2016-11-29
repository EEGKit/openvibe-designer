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

			static IRendererContext* create(IRendererContext* pParentRendererContext=NULL);
			static void release(IRendererContext* pRendererContext);

			virtual ~IRendererContext(void) { }

			virtual void clear(void)=0;

			virtual void setParentRendererContext(IRendererContext* pParentRendererContext)=0;

			virtual void clearChannelInfo(void)=0;
			virtual void addChannel(const std::string& sChannelName, float x=0, float y=0, float z=0)=0;
			virtual void selectChannel(uint32_t ui32Index)=0;
			virtual void unselectChannel(uint32_t ui32Index)=0;
			virtual void sortSelectedChannel(uint32_t ui32SortMode)=0;

			virtual void setDimensionLabel(size_t dimensionIndex, size_t labelIndex, const char* label) = 0;
			virtual size_t getDimensionLabelCount(size_t dimensionIndex) const = 0;
			virtual const char* getDimensionLabel(size_t dimensionIndex, size_t labelIndex) const = 0;

			virtual void clearTransformInfo(void)=0;
			virtual void scaleBy(float f32Scale)=0;
			virtual void setScale(float f32Scale) = 0;
			virtual void zoomBy(float f32Zoom)=0;
			virtual void rotateByX(float f32Rotation)=0;
			virtual void rotateByY(float f32Rotation)=0;

			virtual void setTranslucency(float f32Translucency)=0;
			virtual void setAspect(float f32Aspect)=0;
			virtual void setSampleDuration(uint64_t ui64SampleDuration)=0;
			virtual void setTimeScale(uint64_t ui64TimeScale)=0;
			virtual void setElementCount(uint64_t ui64ElementCount)=0;
			virtual void setFlowerRingCount(uint64_t ui64FlowerRingCount)=0;
			virtual void setXYZPlotDepth(bool bHasDepth)=0;
			virtual void setAxisDisplay(bool bIsAxisDisplayed)=0;
			virtual void setPositiveOnly(bool bPositiveOnly)=0;
			virtual void setTimeLocked(bool bTimeLocked)=0;
			virtual void setScrollModeActive(bool bScrollModeActive)=0;
			virtual void setScaleVisibility(bool bVisibility)=0;
			virtual void setCheckBoardVisibility(bool bVisibility)=0;
			virtual void setDataType(EDataType eDataType)=0;
			virtual void setSpectrumFrequencyRange(uint32_t ui32SpectrumFrequencyRange)=0;
			virtual void setMinimumSpectrumFrequency(uint32_t ui32MinSpectrumFrequency)=0;
			virtual void setMaximumSpectrumFrequency(uint32_t ui32MaxSpectrumFrequency)=0;
			virtual void setStackCount(uint32_t ui32StackCount)=0;
			virtual void setStackIndex(uint32_t ui32StackIndex)=0;
			virtual void setFaceMeshVisible(bool bVisible)=0;
			virtual void setScalpMeshVisible(bool bVisible)=0;

			virtual void setERPPlayerActive(bool bActive)=0;
			virtual void stepERPFractionBy(float f32ERPFraction)=0;

			virtual std::string getChannelName(uint32_t ui32Index) const=0;
			virtual bool getChannelLocalisation(uint32_t ui32Index, float& x, float& y, float& z) const=0;
			virtual uint32_t getChannelCount(void) const=0;
			virtual uint32_t getSelectedCount(void) const=0;
			virtual uint32_t getSelected(uint32_t ui32Index) const=0;
			virtual bool isSelected(uint32_t ui32Index) const=0;

			virtual float getScale(void) const=0;
			virtual float getZoom(void) const=0;
			virtual float getRotationX(void) const=0;
			virtual float getRotationY(void) const=0;

			virtual float getTranslucency(void) const=0;
			virtual float getAspect(void) const=0;
			virtual uint64_t getSampleDuration(void) const=0;
			virtual uint64_t getTimeScale(void) const=0;
			virtual uint64_t getElementCount(void) const=0;
			virtual uint64_t getFlowerRingCount(void) const=0;
			virtual bool hasXYZPlotDepth(void) const=0;
			virtual bool isAxisDisplayed(void) const=0;
			virtual bool isPositiveOnly(void) const=0;
			virtual bool isTimeLocked(void) const=0;
			virtual bool isScrollModeActive(void) const=0;
			virtual bool getScaleVisibility(void) const=0;
			virtual bool getCheckBoardVisibility(void) const=0;
			virtual EDataType getDataType(void) const=0;
			virtual uint32_t getSpectrumFrequencyRange(void) const=0;
			virtual uint32_t getMinSpectrumFrequency(void) const=0;
			virtual uint32_t getMaxSpectrumFrequency(void) const=0;
			virtual uint32_t getStackCount(void) const=0;
			virtual uint32_t getStackIndex(void) const=0;
			virtual bool isFaceMeshVisible(void) const=0;
			virtual bool isScalpMeshVisible(void) const=0;

			virtual bool isERPPlayerActive(void) const=0;
			virtual float getERPFraction(void) const=0;

			virtual uint32_t getMaximumSampleCountPerDisplay(void) const=0;
		};
	};
};

