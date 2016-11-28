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

#ifndef __Mensia_AdvancedVisualization_IRendererContext_H__
#define __Mensia_AdvancedVisualization_IRendererContext_H__

#include "m_defines.h"

#include <mensia/base.h>

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
			virtual void selectChannel(uint32 ui32Index)=0;
			virtual void unselectChannel(uint32 ui32Index)=0;
			virtual void sortSelectedChannel(uint32 ui32SortMode)=0;

			virtual void setDimensionLabel(size_t dimensionIndex, size_t labelIndex, const char* label) = 0;
			virtual size_t getDimensionLabelCount(size_t dimensionIndex) const = 0;
			virtual const char* getDimensionLabel(size_t dimensionIndex, size_t labelIndex) const = 0;

			virtual void clearTransformInfo(void)=0;
			virtual void scaleBy(float32 f32Scale)=0;
			virtual void setScale(float32 f32Scale) = 0;
			virtual void zoomBy(float32 f32Zoom)=0;
			virtual void rotateByX(float32 f32Rotation)=0;
			virtual void rotateByY(float32 f32Rotation)=0;

			virtual void setTranslucency(float32 f32Translucency)=0;
			virtual void setAspect(float32 f32Aspect)=0;
			virtual void setSampleDuration(uint64 ui64SampleDuration)=0;
			virtual void setTimeScale(uint64 ui64TimeScale)=0;
			virtual void setElementCount(uint64 ui64ElementCount)=0;
			virtual void setFlowerRingCount(uint64 ui64FlowerRingCount)=0;
			virtual void setXYZPlotDepth(boolean bHasDepth)=0;
			virtual void setAxisDisplay(boolean bIsAxisDisplayed)=0;
			virtual void setPositiveOnly(boolean bPositiveOnly)=0;
			virtual void setTimeLocked(boolean bTimeLocked)=0;
			virtual void setScrollModeActive(boolean bScrollModeActive)=0;
			virtual void setScaleVisibility(boolean bVisibility)=0;
			virtual void setCheckBoardVisibility(boolean bVisibility)=0;
			virtual void setDataType(EDataType eDataType)=0;
			virtual void setSpectrumFrequencyRange(uint32 ui32SpectrumFrequencyRange)=0;
			virtual void setMinimumSpectrumFrequency(uint32 ui32MinSpectrumFrequency)=0;
			virtual void setMaximumSpectrumFrequency(uint32 ui32MaxSpectrumFrequency)=0;
			virtual void setStackCount(uint32 ui32StackCount)=0;
			virtual void setStackIndex(uint32 ui32StackIndex)=0;
			virtual void setFaceMeshVisible(boolean bVisible)=0;
			virtual void setScalpMeshVisible(boolean bVisible)=0;

			virtual void setERPPlayerActive(boolean bActive)=0;
			virtual void stepERPFractionBy(float32 f32ERPFraction)=0;

			virtual std::string getChannelName(uint32 ui32Index) const=0;
			virtual boolean getChannelLocalisation(uint32 ui32Index, float& x, float& y, float& z) const=0;
			virtual uint32 getChannelCount(void) const=0;
			virtual uint32 getSelectedCount(void) const=0;
			virtual uint32 getSelected(uint32 ui32Index) const=0;
			virtual boolean isSelected(uint32 ui32Index) const=0;

			virtual float32 getScale(void) const=0;
			virtual float32 getZoom(void) const=0;
			virtual float32 getRotationX(void) const=0;
			virtual float32 getRotationY(void) const=0;

			virtual float32 getTranslucency(void) const=0;
			virtual float32 getAspect(void) const=0;
			virtual uint64 getSampleDuration(void) const=0;
			virtual uint64 getTimeScale(void) const=0;
			virtual uint64 getElementCount(void) const=0;
			virtual uint64 getFlowerRingCount(void) const=0;
			virtual boolean hasXYZPlotDepth(void) const=0;
			virtual boolean isAxisDisplayed(void) const=0;
			virtual boolean isPositiveOnly(void) const=0;
			virtual boolean isTimeLocked(void) const=0;
			virtual boolean isScrollModeActive(void) const=0;
			virtual boolean getScaleVisibility(void) const=0;
			virtual boolean getCheckBoardVisibility(void) const=0;
			virtual EDataType getDataType(void) const=0;
			virtual uint32 getSpectrumFrequencyRange(void) const=0;
			virtual uint32 getMinSpectrumFrequency(void) const=0;
			virtual uint32 getMaxSpectrumFrequency(void) const=0;
			virtual uint32 getStackCount(void) const=0;
			virtual uint32 getStackIndex(void) const=0;
			virtual boolean isFaceMeshVisible(void) const=0;
			virtual boolean isScalpMeshVisible(void) const=0;

			virtual boolean isERPPlayerActive(void) const=0;
			virtual float32 getERPFraction(void) const=0;

			virtual uint32 getMaximumSampleCountPerDisplay(void) const=0;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_IRendererContext_H__
