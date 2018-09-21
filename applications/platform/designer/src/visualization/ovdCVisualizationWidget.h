#pragma once

#include <visualization-toolkit/ovvizIVisualizationWidget.h>

#include <vector>

namespace OpenViBEDesigner
{
	class CVisualizationWidget final: public OpenViBEVisualizationToolkit::IVisualizationWidget
	{
	public:
		CVisualizationWidget(const OpenViBE::Kernel::IKernelContext& kernelContext);

		~CVisualizationWidget(void);

		bool initialize(
		        const OpenViBE::CIdentifier& identifier,
		        const OpenViBE::CString& name,
		        OpenViBEVisualizationToolkit::EVisualizationWidgetType type,
		        const OpenViBE::CIdentifier& parentIdentifier,
		        const OpenViBE::CIdentifier& boxIdentifier,
		        OpenViBE::uint32 childCount);

		OpenViBE::CIdentifier getIdentifier(void) const;

		const OpenViBE::CString& getName(void) const;

		void setName(const OpenViBE::CString& name);

		OpenViBEVisualizationToolkit::EVisualizationWidgetType getType(void) const;

		OpenViBE::CIdentifier getParentIdentifier(void) const;

		void setParentIdentifier(const OpenViBE::CIdentifier& parentIdentifier);

		OpenViBE::CIdentifier getBoxIdentifier(void) const;

		OpenViBE::uint32 getNbChildren(void) const;

		bool getChildIndex(const OpenViBE::CIdentifier& identifier, OpenViBE::uint32& index) const;

		//for windows, the number of children is unknown a priori
		bool addChild(const OpenViBE::CIdentifier& childIdentifier);

		bool removeChild(const OpenViBE::CIdentifier& identifier);

		bool getChildIdentifier(OpenViBE::uint32 childIndex, OpenViBE::CIdentifier& identifier) const;

		bool setChildIdentifier(OpenViBE::uint32 childIndex, const OpenViBE::CIdentifier& identifier);

		void setWidth(unsigned int width)
		{
			m_Width = width;
		}

		void setHeight(unsigned int height)
		{
			m_Height = height;
		}

		unsigned int getWidth()
		{
			return m_Width;
		}

		unsigned int getHeight()
		{
			return m_Height;
		}

		void setDividerPosition(int dividerPosition)
		{
			m_DividerPosition = dividerPosition;
		}

		void setMaxDividerPosition(int maxDividerPosition)
		{
			m_MaxDividerPosition = maxDividerPosition;
		}

		int getDividerPosition()
		{
			return m_DividerPosition;
		}

		int getMaxDividerPosition()
		{
			return m_MaxDividerPosition;
		}

	private:

		const OpenViBE::Kernel::IKernelContext& m_KernelContext;
		OpenViBE::CIdentifier m_Identifier;
		OpenViBE::CString m_Name;
		OpenViBEVisualizationToolkit::EVisualizationWidgetType m_Type;
		OpenViBE::CIdentifier m_ParentIdentifier;
		OpenViBE::CIdentifier m_BoxIdentifier;
		std::vector<OpenViBE::CIdentifier> m_Children;

		// @fixme should initialize meaningfully in constructor or initialize()?
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
		int m_DividerPosition = std::numeric_limits<int>::min();
		int m_MaxDividerPosition = std::numeric_limits<int>::min();
	};
}
