#pragma once

#include <visualization-toolkit/ovvizIVisualizationWidget.h>

#include <vector>
#include <limits>

namespace OpenViBE
{
	namespace Designer
	{
	class CVisualizationWidget final : public OpenViBEVisualizationToolkit::IVisualizationWidget
	{
	public:
		explicit CVisualizationWidget(const OpenViBE::Kernel::IKernelContext& ctx) : m_kernelCtx(ctx) { }
		~CVisualizationWidget() override = default;

		bool initialize(const OpenViBE::CIdentifier& id, const OpenViBE::CString& name, const OpenViBEVisualizationToolkit::EVisualizationWidgetType type,
						const OpenViBE::CIdentifier& parentID, const OpenViBE::CIdentifier& boxID, const size_t nChild) override;

		OpenViBE::CIdentifier getIdentifier() const override { return m_id; }
		const OpenViBE::CString& getName() const override { return m_name; }
		void setName(const OpenViBE::CString& name) override { m_name = name; }
		OpenViBEVisualizationToolkit::EVisualizationWidgetType getType() const override { return m_type; }
		OpenViBE::CIdentifier getParentIdentifier() const override { return m_parentID; }
		void setParentIdentifier(const OpenViBE::CIdentifier& parentID) override { m_parentID = parentID; }
		OpenViBE::CIdentifier getBoxIdentifier() const override { return m_boxID; }
		size_t getNbChildren() const override { return m_childrens.size(); }
		bool getChildIndex(const OpenViBE::CIdentifier& id, size_t& index) const override;

		//for windows, the number of children is unknown a priori
		bool addChild(const OpenViBE::CIdentifier& childID) override;
		bool removeChild(const OpenViBE::CIdentifier& id) override;
		bool getChildIdentifier(const size_t index, OpenViBE::CIdentifier& id) const override;
		bool setChildIdentifier(const size_t index, const OpenViBE::CIdentifier& id) override;

		void setWidth(const size_t width) override { m_width = width; }
		void setHeight(const size_t height) override { m_height = height; }

		size_t getWidth() override { return m_width; }
		size_t getHeight() override { return m_height; }

		void setDividerPosition(const int pos) override { m_dividerPosition = pos; }
		void setMaxDividerPosition(const int pos) override { m_maxDividerPosition = pos; }

		int getDividerPosition() override { return m_dividerPosition; }
		int getMaxDividerPosition() override { return m_maxDividerPosition; }

	private:

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::CIdentifier m_id = OV_UndefinedIdentifier;
		OpenViBE::CString m_name;
		OpenViBEVisualizationToolkit::EVisualizationWidgetType m_type = OpenViBEVisualizationToolkit::VisualizationWidget_Undefined;
		OpenViBE::CIdentifier m_parentID                              = OV_UndefinedIdentifier;
		OpenViBE::CIdentifier m_boxID                                 = OV_UndefinedIdentifier;
		std::vector<OpenViBE::CIdentifier> m_childrens;

		// @fixme should initialize meaningfully in constructor or initialize()?
		size_t m_width           = 0;
		size_t m_height          = 0;
		int m_dividerPosition    = std::numeric_limits<int>::min();
		int m_maxDividerPosition = std::numeric_limits<int>::min();
	};
	}  // namespace Designer
}  // namespace OpenViBE
