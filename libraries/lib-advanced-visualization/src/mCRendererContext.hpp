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

			virtual void clear(void);

			virtual void setParentRendererContext(IRendererContext* pParentRendererContext);

			virtual void clearChannelInfo(void);
			virtual void addChannel(const std::string& sChannelName, float x, float y, float z);
			virtual void selectChannel(uint32 ui32Index);
			virtual void unselectChannel(uint32 ui32Index);
			virtual void sortSelectedChannel(uint32 ui32SortMode);

			virtual void clearTransformInfo(void);
			virtual void scaleBy(float32 f32Scale);
			virtual void setScale(float32 f32Scale);
			virtual void zoomBy(float32 f32Zoom);
			virtual void rotateByX(float32 f32Rotation);
			virtual void rotateByY(float32 f32Rotation);

			virtual void setTranslucency(float32 f32Translucency);
			virtual void setAspect(float32 f32Aspect);
			virtual void setSampleDuration(uint64 ui64SampleDuration);
			virtual void setTimeScale(uint64 ui64TimeScale);
			virtual void setElementCount(uint64 ui64ElementCount);
			virtual void setFlowerRingCount(uint64 ui64FlowerRingCount);
			virtual void setXYZPlotDepth(boolean bHasDepth);
			virtual void setAxisDisplay(boolean bIsAxisDisplayed);
			virtual void setPositiveOnly(boolean bPositiveOnly);
			virtual void setTimeLocked(boolean bTimeLocked);
			virtual void setScrollModeActive(boolean bScrollModeActive);
			virtual void setScaleVisibility(boolean bVisibility);
			virtual void setCheckBoardVisibility(boolean bVisibility);
			virtual void setDataType(EDataType eDataType);
			virtual void setSpectrumFrequencyRange(uint32 ui32SpectrumFrequencyRange);
			virtual void setMinimumSpectrumFrequency(uint32 ui32MinSpectrumFrequency);
			virtual void setMaximumSpectrumFrequency(uint32 ui32MaxSpectrumFrequency);
			virtual void setStackCount(uint32 ui32StackCount);
			virtual void setStackIndex(uint32 ui32StackIndex);
			virtual void setFaceMeshVisible(boolean bVisible);
			virtual void setScalpMeshVisible(boolean bVisible);

			virtual void setERPPlayerActive(boolean bActive);
			virtual void stepERPFractionBy(float32 f32ERPFraction);

			virtual std::string getChannelName(uint32 ui32Index) const;
			virtual boolean getChannelLocalisation(uint32 ui32Index, float& x, float& y, float& z) const;
			virtual uint32 getChannelCount(void) const;
			virtual uint32 getSelectedCount(void) const;
			virtual uint32 getSelected(uint32 ui32Index) const;
			virtual boolean isSelected(uint32 ui32Index) const;

			virtual float32 getScale(void) const;
			virtual float32 getZoom(void) const;
			virtual float32 getRotationX(void) const;
			virtual float32 getRotationY(void) const;

			virtual float32 getTranslucency(void) const;
			virtual float32 getAspect(void) const;
			virtual uint64 getSampleDuration(void) const;
			virtual uint64 getTimeScale(void) const;
			virtual uint64 getElementCount(void) const;
			virtual uint64 getFlowerRingCount(void) const;
			virtual boolean hasXYZPlotDepth(void) const;
			virtual boolean isAxisDisplayed(void) const;
			virtual boolean isPositiveOnly(void) const;
			virtual boolean isTimeLocked(void) const;
			virtual boolean isScrollModeActive(void) const;
			virtual boolean getScaleVisibility(void) const;
			virtual boolean getCheckBoardVisibility(void) const;
			virtual EDataType getDataType(void) const;
			virtual uint32 getSpectrumFrequencyRange(void) const;
			virtual uint32 getMinSpectrumFrequency(void) const;
			virtual uint32 getMaxSpectrumFrequency(void) const;
			virtual uint32 getStackCount(void) const;
			virtual uint32 getStackIndex(void) const;
			virtual boolean isFaceMeshVisible(void) const;
			virtual boolean isScalpMeshVisible(void) const;

			virtual boolean isERPPlayerActive(void) const;
			virtual float32 getERPFraction(void) const;

			uint32 getMaximumSampleCountPerDisplay(void) const { return 1000; /*500;*/ /*128*/ }

		protected:

			IRendererContext* m_pParentRendererContext;

			std::vector < uint32 > m_vChannelLookup;
			std::vector < std::string > m_vChannelName;
			std::vector < CVertex > m_vChannelLocalisation;

			std::map < std::string, float32 > m_vLeftRightScore;
			std::map < std::string, float32 > m_vFrontBackScore;

			float32 m_f32Scale;
			float32 m_f32Zoom;
			float32 m_f32RotationX;
			float32 m_f32RotationY;

			float32 m_f32Translucency;
			float32 m_f32Aspect;
			uint64 m_ui64SampleDuration;
			uint64 m_ui64TimeScale;
			uint64 m_ui64ElementCount;
			uint64 m_ui64FlowerRingCount;
			boolean m_bHasXYZPlotDepth;
			boolean m_bIsAxisDisplayed;
			boolean m_bIsPositiveOnly;
			boolean m_bIsTimeLocked;
			boolean m_bIsScrollModeActive;
			boolean m_bScaleVisiblity;
			boolean m_bCheckBoardVisiblity;
			EDataType m_eDataType;
			uint32 m_ui32SpectrumFrequencyRange;
			uint32 m_ui32MinSpectrumFrequency;
			uint32 m_ui32MaxSpectrumFrequency;
			uint32 m_ui32StackCount;
			uint32 m_ui32StackIndex;
			boolean m_bFaceMeshVisible;
			boolean m_bScalpMeshVisible;

			boolean m_bERPPlayerActive;
			float32 m_f32ERPFraction;
		};
	};
};

#endif // __Mensia_AdvancedVisualization_CRendererContext_H__
