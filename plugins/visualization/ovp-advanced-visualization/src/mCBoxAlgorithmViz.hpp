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

			explicit CParameterSet(int p, ...)
			{
				va_list l_oArguments;
				va_start(l_oArguments, p);
				while (p != P_None)
				{
					m_vParameter.push_back(p);
					p = va_arg(l_oArguments, int);
				}
				va_end(l_oArguments);
			}

			explicit operator const std::vector<int>&() const { return m_vParameter; }

		protected:

			std::vector<int> m_vParameter;
		};

		class CBoxAlgorithmVizListener : public OpenViBEToolkit::TBoxListener<OpenViBE::Plugins::IBoxListener>
		{
		public:

			explicit CBoxAlgorithmVizListener(const std::vector<int>& vParameter) : m_vParameter(vParameter) { }

			uint32_t getBaseSettingCount()
			{
				uint32_t result = 0;
				for (std::vector<int>::const_iterator it = m_vParameter.begin(); it != m_vParameter.end(); ++it)
				{
					// if(*it==I_Matrix)              result++;
					// if(*it==I_Signal)              result++;
					// if(*it==I_Spectrum)            result++;
					// if(*it==I_Covariance)          result++;
					// if(*it==I_Stimulations)        result++;
					if (*it == S_ChannelLocalisation) { result++; }
					if (*it == S_TemporalCoherence) { result++; }
					if (*it == S_TimeScale) { result++; }
					if (*it == S_ElementCount) { result++; }
					if (*it == S_DataScale) { result++; }
					if (*it == S_Caption) { result++; }
					if (*it == S_DataPositive) { result++; }
					if (*it == S_Translucency) { result++; }
					if (*it == S_FlowerRingCount) { result++; }
					if (*it == S_Color) { result++; }
					if (*it == S_ColorGradient) { result++; }
					if (*it == S_ShowAxis) { result++; }
					if (*it == S_XYZPlotHasDepth) { result++; }
					// if(*it==F_CanAddInput)         result++;
					// if(*it==F_FixedChannelOrder)   result++;
					// if(*it==F_FixedChannelSelection)result++;
				}
				return result;
			}

			bool onInitialized(OpenViBE::Kernel::IBox& /*rBox*/) override
			{
#ifdef TARGET_OS_Windows
				//rBox.addAttribute(OV_AttributeId_Box_DocumentationURLBase, OpenViBE::CString("${Path_Root}/doc/Mensia Advanced Visualization Toolkit/Mensia Advanced Visualization Toolkit.chm::"));
#endif
				return true;
			}

			bool onDefaultInitialized(OpenViBE::Kernel::IBox& rBox) override
			{
				const bool l_bIsSignal     = (std::find(m_vParameter.begin(), m_vParameter.end(), I_Signal) != m_vParameter.end());
				const bool l_bIsSpectrum   = (std::find(m_vParameter.begin(), m_vParameter.end(), I_Spectrum) != m_vParameter.end());
				const bool l_bIsCovariance = (std::find(m_vParameter.begin(), m_vParameter.end(), I_Covariance) != m_vParameter.end());
				OpenViBE::CIdentifier l_oTypeIdentifier = OV_UndefinedIdentifier;

				for (uint32_t i = 0; i < rBox.getInputCount(); ++i)
				{
					rBox.getInputType(i, l_oTypeIdentifier);
					if (l_oTypeIdentifier == OV_TypeId_StreamedMatrix)
					{
						if (l_bIsSignal) { rBox.setInputType(i, OV_TypeId_Signal); }
						if (l_bIsSpectrum) { rBox.setInputType(i, OV_TypeId_Spectrum); }
						if (l_bIsCovariance) { rBox.setInputType(i, OV_TypeId_CovarianceMatrix); }
					}
				}
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier)

			std::vector<int> m_vParameter;
		};

		class CBoxAlgorithmViz : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			typedef struct
			{
				float r, g, b;
			} TColor;

			CBoxAlgorithmViz(const OpenViBE::CIdentifier& rClassId, const std::vector<int>& vParameter)
				: m_oClassId(rClassId), m_vParameter(vParameter), m_oMouseHandler(*this)
			{
				m_oColor.r = 1;
				m_oColor.g = 1;
				m_oColor.b = 1;
			}

			void release() override { delete this; }

			uint64_t getClockFrequency() override { return (32LL << 32); }
			bool initialize() override;
			bool uninitialize() override;
			bool processInput(const uint32_t /*index*/) override { return true; }
			bool processClock(OpenViBE::Kernel::IMessageClock& rClock) override;

			virtual void redrawTopLevelWindow(const bool immediate = false) { m_oGtkGLWidget.redrawTopLevelWindow(immediate); }

			virtual void redraw(const bool immediate = false)
			{
				const uint64_t currentTime = System::Time::zgetTime();
				if (m_bRedrawNeeded || currentTime - m_ui64LastRenderTime > ((1LL << 32) / 16))
				{
					// immediate |= (l_ui64CurrentTime - m_ui64LastRenderTime > ((1LL<<32)/4));
					m_oGtkGLWidget.redraw(immediate);
					m_oGtkGLWidget.redrawLeft(immediate);
					m_oGtkGLWidget.redrawRight(immediate);
					m_oGtkGLWidget.redrawBottom(immediate);
					m_ui64LastRenderTime = currentTime;
					m_bRedrawNeeded      = false;
				}
			}

			virtual void updateRulerVisibility();
			virtual void reshape(int width, int height);
			virtual void preDraw();
			virtual void postDraw();
			virtual void draw();
			virtual void drawLeft();
			virtual void drawRight();
			virtual void drawBottom();
			virtual void mouseButton(int x, int y, int button, int status);
			virtual void mouseMotion(int x, int y);
			virtual void keyboard(int x, int y, uint32_t key, bool status);

		protected:

			static void parseColor(TColor& rColor, const std::string& sColor);

		public:

			OpenViBE::CIdentifier m_oClassId = OV_UndefinedIdentifier;
			std::vector<int> m_vParameter;
			uint64_t m_ui64LastProcessTime = 0;

			TGtkGLWidget<CBoxAlgorithmViz> m_oGtkGLWidget;
			std::map<std::string, CVertex> m_vChannelLocalisation;

			IRendererContext* m_pRendererContext    = nullptr;
			IRendererContext* m_pSubRendererContext = nullptr;
			IRuler* m_pRuler                        = nullptr;
			CMouse m_oMouseHandler;

			OpenViBE::CString m_sLocalisation;
			uint64_t m_temporalCoherence = 0;
			uint64_t m_timeScale         = 0;
			uint64_t m_elementCount      = 0;
			double m_f64DataScale        = 0.0;
			OpenViBE::CString m_sCaption;
			uint32_t m_textureId       = 0;
			uint64_t m_flowerRingCount = 0;
			double m_translucency      = 0.0;
			OpenViBE::CString m_sColor;
			OpenViBE::CString m_sColorGradient;
			bool m_bShowAxis        = false;
			bool m_bXYZPlotHasDepth = false;
			bool m_bIsPositive      = false;
			bool m_bIsTimeLocked    = false;
			bool m_bIsScaleVisible  = false;
			std::vector<TColor> m_vColor;
			TColor m_oColor;

			OpenViBE::CIdentifier m_oTypeIdentifier = OV_UndefinedIdentifier;
			uint64_t m_time1 = 0;
			uint64_t m_time2 = 0;

			float m_fastForwardMaximumFactorHighDefinition = 0.0;
			float m_fastForwardMaximumFactorLowDefinition  = 0.0;

			std::vector<float> m_vSwap;

			GtkBuilder* m_pBuilder = nullptr;

			GtkWidget* m_pViewport    = nullptr;
			GtkWidget* m_pTop         = nullptr;
			GtkWidget* m_pLeft        = nullptr;
			GtkWidget* m_pRight       = nullptr;
			GtkWidget* m_pBottom      = nullptr;
			GtkWidget* m_pCornerLeft  = nullptr;
			GtkWidget* m_pCornerRight = nullptr;

			GtkWidget* m_pTimeScale        = nullptr;
			GtkWidget* m_pElementCount     = nullptr;
			GtkWidget* m_pERPRange         = nullptr;
			GtkWidget* m_pERPPlayerButton  = nullptr;
			GtkWidget* m_pERPPlayer        = nullptr;
			GtkWidget* m_pScaleVisible     = nullptr;
			GtkWidget* m_pFrequencyBandMin = nullptr;
			GtkWidget* m_pFrequencyBandMax = nullptr;

			GtkTreeView* m_pChannelTreeView   = nullptr;
			GtkListStore* m_pChannelListStore = nullptr;

			uint32_t m_width  = 0;
			uint32_t m_height = 0;

			bool m_bRebuildNeeded         = false;
			bool m_bRefreshNeeded         = false;
			bool m_bRedrawNeeded          = false;
			uint64_t m_ui64LastRenderTime = 0;

			bool m_bIsVideoOutputEnabled = false; // for video output
			bool m_bIsVideoOutputWorking = false;
			uint32_t m_ui32FrameId       = 0;
			OpenViBE::CString m_sFrameFilenameFormat;
		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext{};
		};

		class CBoxAlgorithmVizDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			OpenViBE::CString m_sName;
			OpenViBE::CString m_sCategoryName;
			OpenViBE::CString m_sShortDescription;
			OpenViBE::CString m_sDetailedDescription;
			OpenViBE::CIdentifier m_oDescClassId = OV_UndefinedIdentifier;
			OpenViBE::CIdentifier m_oClassId = OV_UndefinedIdentifier;
			OpenViBE::CString m_sAddedSoftwareVersion;
			OpenViBE::CString m_sUpdatedSoftwareVersion;
			std::vector<int> m_vParameter;

			CBoxAlgorithmVizDesc(const OpenViBE::CString& sFullName, const OpenViBE::CIdentifier& rDescClassId, const OpenViBE::CIdentifier& rClassId, const OpenViBE::CString& sAddedSoftwareVersion, const OpenViBE::CString& sUpdatedSoftwareVersion, const CParameterSet& rParameterSet, const OpenViBE::CString& sShortDescription, const OpenViBE::CString& sDetailedDescription)
				: m_sShortDescription(sShortDescription), m_sDetailedDescription(sDetailedDescription), m_oDescClassId(rDescClassId), m_oClassId(rClassId),
				  m_sAddedSoftwareVersion(sAddedSoftwareVersion), m_sUpdatedSoftwareVersion(sUpdatedSoftwareVersion), m_vParameter(rParameterSet)
			{
				const std::string l_sFullname(sFullName.toASCIIString());
				const size_t i = l_sFullname.rfind('/');
				if (i != std::string::npos)
				{
					m_sName         = OpenViBE::CString(l_sFullname.substr(i + 1).c_str());
					m_sCategoryName = OpenViBE::CString(l_sFullname.substr(0, i).c_str());
				}
				else
				{
					m_sName         = OpenViBE::CString(sFullName);
					m_sCategoryName = OpenViBE::CString("");
				}
			}

			void release() override { }

			OpenViBE::CString getName() const override { return m_sName; }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Yann Renard"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("Mensia Technologies SA"); }
			OpenViBE::CString getShortDescription() const override { return m_sShortDescription; }
			OpenViBE::CString getDetailedDescription() const override { return m_sDetailedDescription; }
			//			virtual OpenViBE::CString getCategory() const            { return OpenViBE::CString(""); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getSoftwareComponent() const override { return "openvibe-designer"; }
			OpenViBE::CString getAddedSoftwareVersion() const override { return m_sAddedSoftwareVersion; }
			OpenViBE::CString getUpdatedSoftwareVersion() const override { return m_sUpdatedSoftwareVersion; }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-find"); }
			OpenViBE::CIdentifier getCreatedClass() const override { return m_oClassId; }

			void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const override
			{
				delete pBoxListener;
			}

			bool hasFunctionality(const OpenViBE::CIdentifier functionalityIdentifier) const override { return functionalityIdentifier == OVD_Functionality_Visualization; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const override
			{
				for (auto p : m_vParameter)
				{
					if (p == I_Matrix) { rBoxAlgorithmPrototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }
					if (p == I_Signal) { rBoxAlgorithmPrototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }		// This is later changed in the listener 
					if (p == I_Spectrum) { rBoxAlgorithmPrototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }	// This is later changed in the listener 
					if (p == I_TimeFrequency) rBoxAlgorithmPrototype.addInput("Matrix", OV_TypeId_TimeFrequency);	// This is later changed in the listener
					if (p == I_Covariance) { rBoxAlgorithmPrototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }	// This is later changed in the listener 
					if (p == I_Stimulations) { rBoxAlgorithmPrototype.addInput("Markers", OV_TypeId_Stimulations); }
					if (p == S_ChannelLocalisation) { rBoxAlgorithmPrototype.addSetting("Channel Localisation", OV_TypeId_Filename, "${AdvancedViz_ChannelLocalisation}"); } // "../share/electrode_sets/electrode_set_standard_cartesian.txt" 
					if (p == S_DataPositive) { rBoxAlgorithmPrototype.addSetting("Positive Data Only ?", OV_TypeId_Boolean, "false"); }
					if (p == S_TemporalCoherence) { rBoxAlgorithmPrototype.addSetting("Temporal Coherence", OVP_TypeId_TemporalCoherence, OVP_TypeId_TemporalCoherence_TimeLocked.toString()); }
					if (p == S_TimeScale) { rBoxAlgorithmPrototype.addSetting("Time Scale", OV_TypeId_Float, "20"); }
					if (p == S_ElementCount) { rBoxAlgorithmPrototype.addSetting("Matrix Count", OV_TypeId_Integer, "50"); }
					if (p == S_DataScale) { rBoxAlgorithmPrototype.addSetting("Gain", OV_TypeId_Float, "1"); }
					if (p == S_Caption) { rBoxAlgorithmPrototype.addSetting("Caption", OV_TypeId_String, ""); }
					if (p == S_FlowerRingCount) { rBoxAlgorithmPrototype.addSetting("Flower Ring Count", OV_TypeId_Integer, "1"); }
					if (p == S_Translucency) { rBoxAlgorithmPrototype.addSetting("Translucency", OV_TypeId_Float, "1"); }
					if (p == S_ShowAxis) { rBoxAlgorithmPrototype.addSetting("Show Axis", OV_TypeId_Boolean, "true"); }
					if (p == S_XYZPlotHasDepth) rBoxAlgorithmPrototype.addSetting("Use third channel as depth", OV_TypeId_Boolean, "false"); // XYZ Plot
					if (p == S_Color) { rBoxAlgorithmPrototype.addSetting("Color", OV_TypeId_Color, "${AdvancedViz_DefaultColor}"); }
					if (p == S_ColorGradient) { rBoxAlgorithmPrototype.addSetting("Color", OV_TypeId_ColorGradient, "${AdvancedViz_DefaultColorGradient}"); }
					if (p == F_CanAddInput) { rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput); }
					if (p == F_FixedChannelOrder) {} // rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_);
					if (p == F_FixedChannelSelection) {} // rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_);
				}
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
				return true;
			}
		};
	} // namespace AdvancedVisualization
}  // namespace Mensia
