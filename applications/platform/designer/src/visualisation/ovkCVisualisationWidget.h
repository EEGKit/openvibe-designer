#pragma once

#include <visualization-toolkit/ovvtkIVisualizationWidget.h>

#include <vector>

namespace OpenViBEDesigner
{
	class CVisualisationWidget final: public OpenViBEVisualizationToolkit::IVisualisationWidget
	{
	public:
		CVisualisationWidget(const OpenViBE::Kernel::IKernelContext& kernelContext);

		~CVisualisationWidget(void);

		bool initialize(
		        const OpenViBE::CIdentifier& identifier,
		        const OpenViBE::CString& name,
		        OpenViBEVisualizationToolkit::EVisualisationWidgetType type,
		        const OpenViBE::CIdentifier& parentIdentifier,
		        const OpenViBE::CIdentifier& boxIdentifier,
		        OpenViBE::uint32 childCount);

		OpenViBE::CIdentifier getIdentifier(void) const;

		const OpenViBE::CString& getName(void) const;

		void setName(const OpenViBE::CString& name);

		OpenViBEVisualizationToolkit::EVisualisationWidgetType getType(void) const;

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
		OpenViBEVisualizationToolkit::EVisualisationWidgetType m_Type;
		OpenViBE::CIdentifier m_ParentIdentifier;
		OpenViBE::CIdentifier m_BoxIdentifier;
		std::vector<OpenViBE::CIdentifier> m_Children;
		unsigned int m_Width;
		unsigned int m_Height;
		int m_DividerPosition;
		int m_MaxDividerPosition;
	};
}
