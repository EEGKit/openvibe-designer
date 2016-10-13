#pragma once

#include <visualization-toolkit/ovvtkIVisualizationWidget.h>

#include <vector>

namespace OpenViBE
{
	namespace Kernel
	{
		class CVisualisationWidget final: public OpenViBEVisualizationToolkit::IVisualisationWidget
		{
		public:
			CVisualisationWidget(const OpenViBE::Kernel::IKernelContext& rKernelContext);

			~CVisualisationWidget(void);

			OpenViBE::boolean initialize(
				const OpenViBE::CIdentifier& rIdentifier,
				const OpenViBE::CString& rName,
				OpenViBEVisualizationToolkit::EVisualisationWidgetType oType,
				const OpenViBE::CIdentifier& rParentIdentifier,
				const OpenViBE::CIdentifier& rBoxIdentifier,
				OpenViBE::uint32 ui32NbChildren);

			OpenViBE::CIdentifier getIdentifier(void) const;

			const OpenViBE::CString& getName(void) const;

			void setName(
				const OpenViBE::CString& rName);

			OpenViBEVisualizationToolkit::EVisualisationWidgetType getType(void) const;

			OpenViBE::CIdentifier getParentIdentifier(void) const;

			void setParentIdentifier(
				const OpenViBE::CIdentifier& rParentIdentifier);

			OpenViBE::CIdentifier getBoxIdentifier(void) const;

			OpenViBE::uint32 getNbChildren(void) const;

			OpenViBE::boolean getChildIndex(
				const OpenViBE::CIdentifier& rIdentifier,
				OpenViBE::uint32& ui32Index) const;

			//for windows, the number of children is unknown a priori
			OpenViBE::boolean addChild(
				const OpenViBE::CIdentifier& rChildIdentifier);

			OpenViBE::boolean removeChild(
				const OpenViBE::CIdentifier& rIdentifier);

			OpenViBE::boolean getChildIdentifier(
				OpenViBE::uint32 ui32ChildIndex,
				OpenViBE::CIdentifier& rIdentifier) const;

			OpenViBE::boolean setChildIdentifier(
				OpenViBE::uint32 ui32ChildIndex,
				const OpenViBE::CIdentifier& rIdentifier);

			void setWidth(unsigned int width)
			{
				m_width = width;
			}

			void setHeight(unsigned int height)
			{
				m_height = height;
			}

			unsigned int getWidth()
			{
				return m_width;
			}

			unsigned int getHeight()
			{
				return m_height;
			}

			void setDividerPosition(int dividerPosition)
			{
				m_dividerPosition = dividerPosition;
			}

			void setMaxDividerPosition(int maxDividerPosition)
			{
				m_maxDividerPosition = maxDividerPosition;
			}

			int getDividerPosition()
			{
				return m_dividerPosition;
			}

			int getMaxDividerPosition()
			{
				return m_maxDividerPosition;
			}

		private:

			OpenViBE::CIdentifier m_oIdentifier; //unique identifier
			OpenViBE::CString m_oName;
			OpenViBEVisualizationToolkit::EVisualisationWidgetType m_oType;
			OpenViBE::CIdentifier m_oParentIdentifier; //VisualisationWidget
			OpenViBE::CIdentifier m_oBoxIdentifier; //id of an existing CBox
			std::vector<OpenViBE::CIdentifier> m_vChildren;
			void* m_pParentWidget;
			unsigned int m_width;
			unsigned int m_height;
			int m_dividerPosition;
			int m_maxDividerPosition;
		};
	};
};
