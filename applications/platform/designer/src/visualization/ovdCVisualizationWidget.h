#pragma once

#include <visualization-toolkit/ovvizIVisualizationWidget.h>

#include <vector>
#include <limits>

namespace OpenViBEDesigner
{
	class CVisualizationWidget final : public OpenViBEVisualizationToolkit::IVisualizationWidget
	{
	public:
		CVisualizationWidget(const OpenViBE::Kernel::IKernelContext& ctx);

		~CVisualizationWidget() = default;

		bool initialize(const OpenViBE::CIdentifier& identifier, const OpenViBE::CString& name, OpenViBEVisualizationToolkit::EVisualizationWidgetType type,
						const OpenViBE::CIdentifier& parentIdentifier, const OpenViBE::CIdentifier& boxID, uint32_t childCount) override;

		OpenViBE::CIdentifier getIdentifier() const override { return m_id; }

		const OpenViBE::CString& getName() const override { return m_Name; }

		void setName(const OpenViBE::CString& name) override { m_Name = name; }

		OpenViBEVisualizationToolkit::EVisualizationWidgetType getType() const override { return m_Type; }

		OpenViBE::CIdentifier getParentIdentifier() const override { return m_ParentID; }

		void setParentIdentifier(const OpenViBE::CIdentifier& parentIdentifier) override { m_ParentID = parentIdentifier; }

		OpenViBE::CIdentifier getBoxIdentifier() const override { return m_BoxID; }

		uint32_t getNbChildren() const override { return uint32_t(m_Children.size()); }

		bool getChildIndex(const OpenViBE::CIdentifier& identifier, uint32_t& index) const override;

		//for windows, the number of children is unknown a priori
		bool addChild(const OpenViBE::CIdentifier& childIdentifier) override;

		bool removeChild(const OpenViBE::CIdentifier& identifier) override;

		bool getChildIdentifier(const uint32_t childIndex, OpenViBE::CIdentifier& identifier) const override;

		bool setChildIdentifier(const uint32_t childIndex, const OpenViBE::CIdentifier& identifier) override;

		void setWidth(uint32_t width) override { m_Width = width; }
		void setHeight(uint32_t height) override { m_Height = height; }

		uint32_t getWidth() override { return m_Width; }
		uint32_t getHeight() override { return m_Height; }

		void setDividerPosition(int dividerPosition) override { m_DividerPosition = dividerPosition; }
		void setMaxDividerPosition(int maxDividerPosition) override { m_MaxDividerPosition = maxDividerPosition; }

		int getDividerPosition() override { return m_DividerPosition; }
		int getMaxDividerPosition() override { return m_MaxDividerPosition; }

	private:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::CIdentifier m_id = OV_UndefinedIdentifier;
		OpenViBE::CString m_Name;
		OpenViBEVisualizationToolkit::EVisualizationWidgetType m_Type;
		OpenViBE::CIdentifier m_ParentID = OV_UndefinedIdentifier;
		OpenViBE::CIdentifier m_BoxID = OV_UndefinedIdentifier;
		std::vector<OpenViBE::CIdentifier> m_Children;

		// @fixme should initialize meaningfully in constructor or initialize()?
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		int m_DividerPosition = std::numeric_limits<int>::min();
		int m_MaxDividerPosition = std::numeric_limits<int>::min();
	};
}
