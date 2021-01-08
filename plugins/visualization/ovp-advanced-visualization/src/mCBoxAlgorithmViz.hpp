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

#include <mensia/advanced-visualization.hpp>

#include <system/ovCTime.h>

#include "mTGtkGLWidget.hpp"
#include "mCMouse.hpp"

#include "mIRuler.hpp"

#include <vector>
#include <string>
#include <cstdarg>
#include <algorithm>
#include <iostream>

namespace Mensia {
namespace AdvancedVisualization {
enum EParameter
{
	// Parameter
	P_None,

	// Input
	I_Matrix, I_Signal, I_Spectrum, I_TimeFrequency, I_Covariance, I_Stimulations,

	// Setting
	S_ChannelLocalisation, S_TemporalCoherence, S_TimeScale, S_ElementCount, S_DataScale, S_Caption, S_DataPositive,
	S_Translucency, S_FlowerRingCount, S_Color, S_ColorGradient, S_ShowAxis, S_XYZPlotHasDepth,

	// Flag
	F_CanAddInput, F_FixedChannelOrder, F_FixedChannelSelection, F_Unstable
};

class CParameterSet
{
public:

	explicit CParameterSet(int p, ...)
	{
		va_list args;
		va_start(args, p);
		while (p != P_None)
		{
			m_parameters.push_back(p);
			p = va_arg(args, int);
		}
		va_end(args);
	}

	explicit operator const std::vector<int>&() const { return m_parameters; }

protected:

	std::vector<int> m_parameters;
};

class CBoxAlgorithmViz : public OpenViBE::Toolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
{
public:

	typedef struct
	{
		float r, g, b;
	} color_t;

	CBoxAlgorithmViz(const OpenViBE::CIdentifier& classID, const std::vector<int>& parameters)
		: m_ClassID(classID), m_Parameters(parameters), m_MouseHandler(*this)
	{
		m_Color.r = 1;
		m_Color.g = 1;
		m_Color.b = 1;
	}

	void release() override { delete this; }

	uint64_t getClockFrequency() override { return (32LL << 32); }
	bool initialize() override;
	bool uninitialize() override;
	bool processInput(const size_t /*index*/) override { return true; }
	bool processClock(OpenViBE::Kernel::CMessageClock& msg) override;

	virtual void redrawTopLevelWindow(const bool immediate = false) { m_GtkGLWidget.redrawTopLevelWindow(immediate); }

	virtual void redraw(const bool immediate = false)
	{
		const uint64_t currentTime = System::Time::zgetTime();
		if (m_RedrawNeeded || currentTime - m_LastRenderTime > ((1LL << 32) / 16))
		{
			// immediate |= (currentTime - m_lastRenderTime > ((1LL<<32)/4));
			m_GtkGLWidget.redraw(immediate);
			m_GtkGLWidget.redrawLeft(immediate);
			m_GtkGLWidget.redrawRight(immediate);
			m_GtkGLWidget.redrawBottom(immediate);
			m_LastRenderTime = currentTime;
			m_RedrawNeeded   = false;
		}
	}

	virtual void updateRulerVisibility();
	virtual void reshape(int width, int height);
	virtual void preDraw();
	virtual void postDraw();
	virtual void draw() { }
	virtual void drawLeft() { if (m_Ruler != nullptr) { m_Ruler->doRenderLeft(m_Left); } }
	virtual void drawRight() { if (m_Ruler != nullptr) { m_Ruler->doRenderRight(m_Right); } }
	virtual void drawBottom() { if (m_Ruler != nullptr) { m_Ruler->doRenderBottom(m_Bottom); } }
	virtual void mouseButton(int x, int y, int button, int status);
	virtual void mouseMotion(int x, int y);
	virtual void keyboard(int x, int y, size_t key, bool status);

protected:

	static void parseColor(color_t& rColor, const std::string& sColor);

public:

	OpenViBE::CIdentifier m_ClassID = OpenViBE::CIdentifier::undefined();
	std::vector<int> m_Parameters;
	uint64_t m_lastProcessTime = 0;

	TGtkGLWidget<CBoxAlgorithmViz> m_GtkGLWidget;
	std::map<std::string, CVertex> m_ChannelPositions;

	CRendererContext* m_RendererCtx    = nullptr;
	CRendererContext* m_SubRendererCtx = nullptr;
	IRuler* m_Ruler                    = nullptr;
	CMouse m_MouseHandler;

	OpenViBE::CString m_Localisation;
	ETemporalCoherence m_TemporalCoherence = ETemporalCoherence::TimeLocked;
	uint64_t m_TimeScale                   = 0;
	size_t m_NElement                      = 0;
	double m_DataScale                     = 0.0;
	OpenViBE::CString m_Caption;
	uint32_t m_TextureID  = 0;
	size_t m_NFlowerRing  = 0;
	double m_Translucency = 0.0;
	OpenViBE::CString m_ColorGradient;
	bool m_ShowAxis        = false;
	bool m_XYZPlotHasDepth = false;
	bool m_IsPositive      = false;
	bool m_IsTimeLocked    = false;
	bool m_IsScaleVisible  = false;
	std::vector<color_t> m_Colors;
	color_t m_Color;

	OpenViBE::CIdentifier m_TypeID = OpenViBE::CIdentifier::undefined();
	uint64_t m_Time1               = 0;
	uint64_t m_Time2               = 0;

	float m_fastForwardMaxFactorHD = 0.0;
	float m_fastForwardMaxFactorLD = 0.0;

	std::vector<float> m_Swaps;

	GtkBuilder* m_Builder = nullptr;

	GtkWidget* m_Viewport    = nullptr;
	GtkWidget* m_Top         = nullptr;
	GtkWidget* m_Left        = nullptr;
	GtkWidget* m_Right       = nullptr;
	GtkWidget* m_Bottom      = nullptr;
	GtkWidget* m_CornerLeft  = nullptr;
	GtkWidget* m_CornerRight = nullptr;

	GtkWidget* m_TimeScaleW       = nullptr;
	GtkWidget* m_nElementW        = nullptr;
	GtkWidget* m_ERPRange         = nullptr;
	GtkWidget* m_ERPPlayerButton  = nullptr;
	GtkWidget* m_ERPPlayer        = nullptr;
	GtkWidget* m_ScaleVisible     = nullptr;
	GtkWidget* m_FrequencyBandMin = nullptr;
	GtkWidget* m_FrequencyBandMax = nullptr;

	GtkTreeView* m_ChannelTreeView   = nullptr;
	GtkListStore* m_ChannelListStore = nullptr;

	size_t m_Width  = 0;
	size_t m_Height = 0;

	bool m_RebuildNeeded      = false;
	bool m_RefreshNeeded      = false;
	bool m_RedrawNeeded       = false;
	uint64_t m_LastRenderTime = 0;

	bool m_IsVideoOutputEnabled = false; // for video output
	bool m_IsVideoOutputWorking = false;
	size_t m_FrameId            = 0;
	OpenViBE::CString m_FrameFilenameFormat;

private:
	OpenViBE::VisualizationToolkit::IVisualizationContext* m_visualizationCtx = nullptr;
};

class CBoxAlgorithmVizListener : public OpenViBE::Toolkit::TBoxListener<OpenViBE::Plugins::IBoxListener>
{
public:

	explicit CBoxAlgorithmVizListener(const std::vector<int>& parameters) : m_Parameters(parameters) { }

	size_t getBaseSettingCount()
	{
		size_t result = 0;
		for (const auto& p : m_Parameters)
		{
			// if(p == I_Matrix) { result++; }
			// if(p == I_Signal) { result++; }
			// if(p == I_Spectrum) { result++; }
			// if(p == I_Covariance) { result++; }
			// if(p == I_Stimulations) { result++; }
			if (p == S_ChannelLocalisation) { result++; }
			if (p == S_TemporalCoherence) { result++; }
			if (p == S_TimeScale) { result++; }
			if (p == S_ElementCount) { result++; }
			if (p == S_DataScale) { result++; }
			if (p == S_Caption) { result++; }
			if (p == S_DataPositive) { result++; }
			if (p == S_Translucency) { result++; }
			if (p == S_FlowerRingCount) { result++; }
			if (p == S_Color) { result++; }
			if (p == S_ColorGradient) { result++; }
			if (p == S_ShowAxis) { result++; }
			if (p == S_XYZPlotHasDepth) { result++; }
			// if(p == F_CanAddInput) { result++; }
			// if(p == F_FixedChannelOrder) { result++; }
			// if(p == F_FixedChannelSelection) { result++; }
		}
		return result;
	}

	bool onInitialized(OpenViBE::Kernel::IBox& /*box*/) override
	{
#ifdef TARGET_OS_Windows
		//box.addAttribute(OV_AttributeId_Box_DocumentationURLBase, OpenViBE::CString("${Path_Root}/doc/Mensia Advanced Visualization Toolkit/Mensia Advanced Visualization Toolkit.chm::"));
#endif
		return true;
	}

	bool onDefaultInitialized(OpenViBE::Kernel::IBox& box) override
	{
		const bool isSignal          = (std::find(m_Parameters.begin(), m_Parameters.end(), I_Signal) != m_Parameters.end());
		const bool isSpectrum        = (std::find(m_Parameters.begin(), m_Parameters.end(), I_Spectrum) != m_Parameters.end());
		const bool isCovariance      = (std::find(m_Parameters.begin(), m_Parameters.end(), I_Covariance) != m_Parameters.end());
		OpenViBE::CIdentifier typeID = OpenViBE::CIdentifier::undefined();

		for (size_t i = 0; i < box.getInputCount(); ++i)
		{
			box.getInputType(i, typeID);
			if (typeID == OV_TypeId_StreamedMatrix)
			{
				if (isSignal) { box.setInputType(i, OV_TypeId_Signal); }
				if (isSpectrum) { box.setInputType(i, OV_TypeId_Spectrum); }
				if (isCovariance) { box.setInputType(i, OV_TypeId_CovarianceMatrix); }
			}
		}
		return true;
	}

	_IsDerivedFromClass_Final_(OpenViBE::Toolkit::TBoxListener<OpenViBE::Plugins::IBoxListener>, OpenViBE::CIdentifier::undefined())

	std::vector<int> m_Parameters;
};

class CBoxAlgorithmVizDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
{
public:

	OpenViBE::CString m_Name;
	OpenViBE::CString m_CategoryName;
	OpenViBE::CString m_ShortDesc;
	OpenViBE::CString m_DetailedDesc;
	OpenViBE::CIdentifier m_DescClassID = OpenViBE::CIdentifier::undefined();
	OpenViBE::CIdentifier m_ClassID     = OpenViBE::CIdentifier::undefined();
	OpenViBE::CString m_AddedSoftwareVersion;
	OpenViBE::CString m_UpdatedSoftwareVersion;
	std::vector<int> m_Parameters;

	CBoxAlgorithmVizDesc(const OpenViBE::CString& name, const OpenViBE::CIdentifier& descClassID, const OpenViBE::CIdentifier& classID,
						 const OpenViBE::CString& addedSoftwareVersion, const OpenViBE::CString& updatedSoftwareVersion,
						 const CParameterSet& parameterSet, const OpenViBE::CString& shortDesc, const OpenViBE::CString& detailedDesc)
		: m_ShortDesc(shortDesc), m_DetailedDesc(detailedDesc), m_DescClassID(descClassID), m_ClassID(classID),
		  m_AddedSoftwareVersion(addedSoftwareVersion), m_UpdatedSoftwareVersion(updatedSoftwareVersion), m_Parameters(parameterSet)
	{
		const std::string fullname(name.toASCIIString());
		const size_t i = fullname.rfind('/');
		if (i != std::string::npos)
		{
			m_Name         = OpenViBE::CString(fullname.substr(i + 1).c_str());
			m_CategoryName = OpenViBE::CString(fullname.substr(0, i).c_str());
		}
		else
		{
			m_Name         = OpenViBE::CString(name);
			m_CategoryName = OpenViBE::CString("");
		}
	}

	void release() override { }

	OpenViBE::CString getName() const override { return m_Name; }
	OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Yann Renard"); }
	OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("Mensia Technologies SA"); }
	OpenViBE::CString getShortDescription() const override { return m_ShortDesc; }
	OpenViBE::CString getDetailedDescription() const override { return m_DetailedDesc; }
	// virtual OpenViBE::CString getCategory() const            { return OpenViBE::CString(""); }
	OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
	OpenViBE::CString getSoftwareComponent() const override { return "openvibe-designer"; }
	OpenViBE::CString getAddedSoftwareVersion() const override { return m_AddedSoftwareVersion; }
	OpenViBE::CString getUpdatedSoftwareVersion() const override { return m_UpdatedSoftwareVersion; }
	OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-find"); }
	OpenViBE::CIdentifier getCreatedClass() const override { return m_ClassID; }

	void releaseBoxListener(OpenViBE::Plugins::IBoxListener* listener) const override { delete listener; }

	bool hasFunctionality(const OpenViBE::Plugins::EPluginFunctionality functionality) const override
	{
		return functionality == OpenViBE::Plugins::EPluginFunctionality::Visualization;
	}

	bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& prototype) const override
	{
		for (auto p : m_Parameters)
		{
			if (p == I_Matrix) { prototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }
			if (p == I_Signal) { prototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }			// This is later changed in the listener 
			if (p == I_Spectrum) { prototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }		// This is later changed in the listener 
			if (p == I_TimeFrequency) { prototype.addInput("Matrix", OV_TypeId_TimeFrequency); }	// This is later changed in the listener
			if (p == I_Covariance) { prototype.addInput("Matrix", OV_TypeId_StreamedMatrix); }		// This is later changed in the listener 
			if (p == I_Stimulations) { prototype.addInput("Markers", OV_TypeId_Stimulations); }
			if (p == S_ChannelLocalisation
			)
			{
				prototype.addSetting("Channel Localisation", OV_TypeId_Filename, "${AdvancedViz_ChannelLocalisation}");
			} // "../share/electrode_sets/electrode_set_standard_cartesian.txt" 
			if (p == S_DataPositive) { prototype.addSetting("Positive Data Only ?", OV_TypeId_Boolean, "false"); }
			if (p == S_TemporalCoherence) { prototype.addSetting("Temporal Coherence", OVP_TypeId_TemporalCoherence, "Time Locked"); }
			if (p == S_TimeScale) { prototype.addSetting("Time Scale", OV_TypeId_Float, "20"); }
			if (p == S_ElementCount) { prototype.addSetting("Matrix Count", OV_TypeId_Integer, "50"); }
			if (p == S_DataScale) { prototype.addSetting("Gain", OV_TypeId_Float, "1"); }
			if (p == S_Caption) { prototype.addSetting("Caption", OV_TypeId_String, ""); }
			if (p == S_FlowerRingCount) { prototype.addSetting("Flower Ring Count", OV_TypeId_Integer, "1"); }
			if (p == S_Translucency) { prototype.addSetting("Translucency", OV_TypeId_Float, "1"); }
			if (p == S_ShowAxis) { prototype.addSetting("Show Axis", OV_TypeId_Boolean, "true"); }
			if (p == S_XYZPlotHasDepth) { prototype.addSetting("Use third channel as depth", OV_TypeId_Boolean, "false"); } // XYZ Plot
			if (p == S_Color) { prototype.addSetting("Color", OV_TypeId_Color, "${AdvancedViz_DefaultColor}"); }
			if (p == S_ColorGradient) { prototype.addSetting("Color", OV_TypeId_ColorGradient, "${AdvancedViz_DefaultColorGradient}"); }
			if (p == F_CanAddInput) { prototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput); }
			if (p == F_FixedChannelOrder) {} // prototype.addFlag(OpenViBE::Kernel::BoxFlag_);
			if (p == F_FixedChannelSelection) {} // prototype.addFlag(OpenViBE::Kernel::BoxFlag_);
		}
		prototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
		return true;
	}
};
}  // namespace AdvancedVisualization
}  // namespace Mensia
