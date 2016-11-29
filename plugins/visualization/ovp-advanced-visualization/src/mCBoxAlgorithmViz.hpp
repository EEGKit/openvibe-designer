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

#ifndef TARGET_IS_Application

#include "m_defines.hpp"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include <mensia/advanced-visualization.h>

#include <system/ovCMemory.h>
#include <system/ovCTime.h>

#include "mCVertex.hpp"

#include "mTGtkGLWidget.hpp"
#include "mCMouse.hpp"

#include "mIRuler.hpp"

#include "m_VisualizationTools.hpp"

#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <cstdarg>
#include <algorithm>
#include <iostream>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		enum EParameter
		{
			// Parameter
			P_None,

			// Input
			I_Matrix,
			I_Signal,
			I_Spectrum,
			I_TimeFrequency,
			I_Covariance,
			I_Stimulations,

			// Setting
			S_ChannelLocalisation,
			S_TemporalCoherence,
			S_TimeScale,
			S_ElementCount,
			S_DataScale,
			S_Caption,
			S_DataPositive,
			S_Translucency,
			S_FlowerRingCount,
			S_Color,
			S_ColorGradient,
			S_ShowAxis,
			S_XYZPlotHasDepth,

			// Flag
			F_CanAddInput,
			F_FixedChannelOrder,
			F_FixedChannelSelection,
			F_Unstable,
		};

		class CParameterSet
		{
		public:

			CParameterSet(int p, ...)
			{
				va_list l_oArguments;
				va_start(l_oArguments, p);
				while(p!=P_None)
				{
					m_vParameter.push_back(p);
					p=va_arg(l_oArguments, int);
				}
				va_end(l_oArguments);
			}

			operator const std::vector < int >& () const
			{
				return m_vParameter;
			}

		protected:

			std::vector < int > m_vParameter;
		};

		class CBoxAlgorithmVizListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			CBoxAlgorithmVizListener(const std::vector < int >& vParameter)
				:m_vParameter(vParameter)
			{
			}

			uint32_t getBaseSettingCount(void)
			{
				uint32_t l_ui32Result=0;
				std::vector < int >::const_iterator it;
				for(it=m_vParameter.begin(); it!=m_vParameter.end(); it++)
				{
					// if(*it==I_Matrix)              l_ui32Result++;
					// if(*it==I_Signal)              l_ui32Result++;
					// if(*it==I_Spectrum)            l_ui32Result++;
					// if(*it==I_Covariance)          l_ui32Result++;
					// if(*it==I_Stimulations)        l_ui32Result++;
					if(*it==S_ChannelLocalisation) l_ui32Result++;
					if(*it==S_TemporalCoherence)   l_ui32Result++;
					if(*it==S_TimeScale)           l_ui32Result++;
					if(*it==S_ElementCount)        l_ui32Result++;
					if(*it==S_DataScale)           l_ui32Result++;
					if(*it==S_Caption)             l_ui32Result++;
					if(*it==S_DataPositive)        l_ui32Result++;
					if(*it==S_Translucency)        l_ui32Result++;
					if(*it==S_FlowerRingCount)     l_ui32Result++;
					if(*it==S_Color)               l_ui32Result++;
					if(*it==S_ColorGradient)       l_ui32Result++;
					if(*it==S_ShowAxis)            l_ui32Result++;
					if(*it==S_XYZPlotHasDepth)     l_ui32Result++;
					// if(*it==F_CanAddInput)         l_ui32Result++;
					// if(*it==F_FixedChannelOrder)   l_ui32Result++;
					// if(*it==F_FixedChannelSelection)l_ui32Result++;
				}
				return l_ui32Result;
			}

			virtual bool onInitialized(OpenViBE::Kernel::IBox& rBox)
			{
#ifdef TARGET_OS_Windows
				//rBox.addAttribute(OV_AttributeId_Box_DocumentationURLBase, OpenViBE::CString("${Path_Root}/doc/Mensia Advanced Visualization Toolkit/Mensia Advanced Visualization Toolkit.chm::"));
#endif
				return true;
			}

			virtual bool onDefaultInitialized(OpenViBE::Kernel::IBox& rBox)
			{
				bool l_bIsSignal = (std::find(m_vParameter.begin(), m_vParameter.end(), I_Signal) != m_vParameter.end());
				bool l_bIsSpectrum = (std::find(m_vParameter.begin(), m_vParameter.end(), I_Spectrum) != m_vParameter.end());
				bool l_bIsCovariance = (std::find(m_vParameter.begin(), m_vParameter.end(), I_Covariance) != m_vParameter.end());
				OpenViBE::CIdentifier l_oTypeIdentifier;

				for(uint32_t i=0; i<rBox.getInputCount(); i++)
				{
					rBox.getInputType(i, l_oTypeIdentifier);
					if(l_oTypeIdentifier == OV_TypeId_StreamedMatrix)
					{
						if(l_bIsSignal) rBox.setInputType(i, OV_TypeId_Signal);
						if(l_bIsSpectrum) rBox.setInputType(i, OV_TypeId_Spectrum);
						if(l_bIsCovariance) rBox.setInputType(i, OV_TypeId_CovarianceMatrix);
					}
				}
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);

			std::vector < int > m_vParameter;
		};

		class CBoxAlgorithmViz : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			typedef struct
			{
				float r;
				float g;
				float b;
			} TColor;

			CBoxAlgorithmViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
				:m_oClassId(rClassId)
				,m_vParameter(vParameter)
				,m_pRuler(NULL)
				,m_oMouseHandler(*this)
			{
				m_oColor.r=1;
				m_oColor.g=1;
				m_oColor.b=1;
			}

			virtual void release(void) { delete this; }

			virtual uint64_t getClockFrequency(void) { return (32LL<<32); }
			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool processInput(OpenViBE::uint32 ui32InputIndex) { return true; };
			virtual bool processClock(OpenViBE::Kernel::IMessageClock& rClock);

		public:

			virtual void redrawTopLevelWindow(bool bImmediate=false)
			{
				m_oGtkGLWidget.redrawTopLevelWindow(bImmediate);
			}

			virtual void redraw(bool bImmediate=false)
			{
				bool l_bImmediate = bImmediate;
				uint64_t l_ui64CurrentTime = System::Time::zgetTime();
				if(m_bRedrawNeeded || l_ui64CurrentTime - m_ui64LastRenderTime > ((1LL<<32)/16))
				{
//					l_bImmediate |= (l_ui64CurrentTime - m_ui64LastRenderTime > ((1LL<<32)/4));
					m_oGtkGLWidget.redraw(l_bImmediate);
					m_oGtkGLWidget.redrawLeft(l_bImmediate);
					m_oGtkGLWidget.redrawRight(l_bImmediate);
					m_oGtkGLWidget.redrawBottom(l_bImmediate);
					m_ui64LastRenderTime = l_ui64CurrentTime;
					m_bRedrawNeeded = false;
				}
			}

			virtual void updateRulerVisibility(void);
			virtual void reshape(int32_t width, int32_t height);
			virtual void preDraw(void);
			virtual void postDraw(void);
			virtual void draw(void);
			virtual void drawLeft(void);
			virtual void drawRight(void);
			virtual void drawBottom(void);
			virtual void mouseButton(int32_t x, int32_t y, int32_t button, int32_t status);
			virtual void mouseMotion(int32_t x, int32_t y);
			virtual void keyboard(int32_t x, int32_t y, uint32_t key, bool status);

		protected:

			void parseColor(TColor& rColor, const std::string& sColor);

		public:

			OpenViBE::CIdentifier m_oClassId;
			std::vector < int > m_vParameter;
			uint64_t m_ui64LastProcessTime;

			TGtkGLWidget < CBoxAlgorithmViz > m_oGtkGLWidget;
			std::map < std::string, CVertex > m_vChannelLocalisation;

			IRendererContext* m_pRendererContext;
			IRendererContext* m_pSubRendererContext;
			IRuler* m_pRuler;
			CMouse m_oMouseHandler;

			OpenViBE::CString m_sLocalisation;
			uint64_t m_ui64TemporalCoherence;
			uint64_t m_ui64TimeScale;
			uint64_t m_ui64ElementCount;
			double m_f64DataScale;
			OpenViBE::CString m_sCaption;
			uint32_t m_ui32TextureId;
			uint64_t m_ui64FlowerRingCount;
			double m_f64Translucency;
			OpenViBE::CString m_sColor;
			OpenViBE::CString m_sColorGradient;
			bool m_bShowAxis;
			bool m_bXYZPlotHasDepth;
			bool m_bIsPositive;
			bool m_bIsTimeLocked;
			bool m_bIsScaleVisible;
			std::vector < TColor > m_vColor;
			TColor m_oColor;

			OpenViBE::CIdentifier m_oTypeIdentifier;
			uint64_t m_ui64Time1;
			uint64_t m_ui64Time2;

			float m_f32FastForwardMaximumFactorHighDefinition;
			float m_f32FastForwardMaximumFactorLowDefinition;

			std::vector < float > m_vSwap;

			::GtkBuilder* m_pBuilder;

			::GtkWidget* m_pViewport;
			::GtkWidget* m_pTop;
			::GtkWidget* m_pLeft;
			::GtkWidget* m_pRight;
			::GtkWidget* m_pBottom;
			::GtkWidget* m_pCornerLeft;
			::GtkWidget* m_pCornerRight;

			::GtkWidget* m_pTimeScale;
			::GtkWidget* m_pElementCount;
			::GtkWidget* m_pERPRange;
			::GtkWidget* m_pERPPlayerButton;
			::GtkWidget* m_pERPPlayer;
			::GtkWidget* m_pScaleVisible;
			::GtkWidget* m_pFrequencyBandMin;
			::GtkWidget* m_pFrequencyBandMax;

			::GtkTreeView* m_pChannelTreeView;
			::GtkListStore* m_pChannelListStore;

			uint32_t m_ui32Width;
			uint32_t m_ui32Height;

			bool m_bRebuildNeeded;
			bool m_bRefreshNeeded;
			bool m_bRedrawNeeded;
			uint64_t m_ui64LastRenderTime;

			bool m_bIsVideoOutputEnabled; // for video output
			bool m_bIsVideoOutputWorking;
			uint32_t m_ui32FrameId;
			OpenViBE::CString m_sFrameFilenameFormat;
		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext;
		};

		class CBoxAlgorithmVizDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			OpenViBE::CString m_sName;
			OpenViBE::CString m_sCategoryName;
			OpenViBE::CString m_sShortDescription;
			OpenViBE::CString m_sDetailedDescription;
			OpenViBE::CIdentifier m_oDescClassId;
			OpenViBE::CIdentifier m_oClassId;
			OpenViBE::CString m_sAddedSoftwareVersion;
			OpenViBE::CString m_sUpdatedSoftwareVersion;
			std::vector < int > m_vParameter;

			CBoxAlgorithmVizDesc(const OpenViBE::CString& sFullName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, const CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription,const OpenViBE::CString& sDetailedDescription)
				:m_sShortDescription(sShortDescription)
				,m_sDetailedDescription(sDetailedDescription)
				,m_oDescClassId(rDescClassId)
				,m_oClassId(rClassId)
				,m_sAddedSoftwareVersion(sAddedSoftwareVersion)
				,m_sUpdatedSoftwareVersion(sUpdatedSoftwareVersion)
				,m_vParameter(rParameterSet)
			{
				std::string l_sFullname(sFullName.toASCIIString());
				size_t i=l_sFullname.rfind('/');
				if(i!=std::string::npos)
				{
					m_sName=OpenViBE::CString(l_sFullname.substr(i+1).c_str());
					m_sCategoryName=OpenViBE::CString(l_sFullname.substr(0, i).c_str());
				}
				else
				{
					m_sName=OpenViBE::CString(sFullName);
					m_sCategoryName=OpenViBE::CString("");
				}
			}

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return m_sName; }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Yann Renard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Mensia Technologies SA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return m_sShortDescription; }
			virtual OpenViBE::CString getDetailedDescription(void) const { return m_sDetailedDescription; }
//			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getAddedSoftwareVersion(void) const   { return m_sAddedSoftwareVersion; }
			virtual OpenViBE::CString getUpdatedSoftwareVersion(void) const { return m_sUpdatedSoftwareVersion; }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-find"); }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return m_oClassId; }

			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const
			{
				delete pBoxListener;
			}

			virtual bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}

			virtual bool getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				std::vector < int >::const_iterator it;
				for(it=m_vParameter.begin(); it!=m_vParameter.end(); it++)
				{
					if(*it==I_Matrix)                  rBoxAlgorithmPrototype.addInput  ("Matrix", OV_TypeId_StreamedMatrix);
					if(*it==I_Signal)                  rBoxAlgorithmPrototype.addInput  ("Matrix", OV_TypeId_StreamedMatrix); // This is later changed in the listener
					if(*it==I_Spectrum)                rBoxAlgorithmPrototype.addInput  ("Matrix", OV_TypeId_StreamedMatrix); // This is later changed in the listener
					if(*it==I_TimeFrequency)           rBoxAlgorithmPrototype.addInput  ("Matrix", OV_TypeId_TimeFrequency); // This is later changed in the listener
					if(*it==I_Covariance)              rBoxAlgorithmPrototype.addInput  ("Matrix", OV_TypeId_StreamedMatrix); // This is later changed in the listener
					if(*it==I_Stimulations)            rBoxAlgorithmPrototype.addInput  ("Markers", OV_TypeId_Stimulations);
					if(*it==S_ChannelLocalisation)     rBoxAlgorithmPrototype.addSetting("Channel Localisation", OV_TypeId_Filename, "${AdvancedViz_ChannelLocalisation}"); // "../share/electrode_sets/electrode_set_standard_cartesian.txt"
					if(*it==S_DataPositive)            rBoxAlgorithmPrototype.addSetting("Positive Data Only ?", OV_TypeId_Boolean, "false");
					if(*it==S_TemporalCoherence)       rBoxAlgorithmPrototype.addSetting("Temporal Coherence", OVP_TypeId_TemporalCoherence, OVP_TypeId_TemporalCoherence_TimeLocked.toString());
					if(*it==S_TimeScale)               rBoxAlgorithmPrototype.addSetting("Time Scale", OV_TypeId_Float, "20");
					if(*it==S_ElementCount)            rBoxAlgorithmPrototype.addSetting("Matrix Count", OV_TypeId_Integer, "50");
					if(*it==S_DataScale)               rBoxAlgorithmPrototype.addSetting("Gain", OV_TypeId_Float, "1");
					if(*it==S_Caption)                 rBoxAlgorithmPrototype.addSetting("Caption", OV_TypeId_String, "");
					if(*it==S_FlowerRingCount)         rBoxAlgorithmPrototype.addSetting("Flower Ring Count", OV_TypeId_Integer, "1");
					if(*it==S_Translucency)            rBoxAlgorithmPrototype.addSetting("Translucency", OV_TypeId_Float, "1");
					if(*it==S_ShowAxis)                rBoxAlgorithmPrototype.addSetting("Show Axis", OV_TypeId_Boolean, "true");
					if(*it==S_XYZPlotHasDepth)         rBoxAlgorithmPrototype.addSetting("Use third channel as depth", OV_TypeId_Boolean, "false"); // XYZ Plot
					if(*it==S_Color)                   rBoxAlgorithmPrototype.addSetting("Color", OV_TypeId_Color, "${AdvancedViz_DefaultColor}");
					if(*it==S_ColorGradient)           rBoxAlgorithmPrototype.addSetting("Color", OV_TypeId_ColorGradient, "${AdvancedViz_DefaultColorGradient}");
					if(*it==F_CanAddInput)             rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput);
					if(*it==F_FixedChannelOrder)       { } // rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_);
					if(*it==F_FixedChannelSelection)   { } // rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_);
				}
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
				return true;
			}
		};
	};
};

#endif // TARGET_IS_Application

