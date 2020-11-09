#pragma once

#include <visualization-toolkit/ovvizIVisualizationWidget.h>

#include <vector>
#include <limits>

namespace OpenViBE {
namespace Designer {
class CVisualizationWidget final : public VisualizationToolkit::IVisualizationWidget
{
public:
	explicit CVisualizationWidget(const Kernel::IKernelContext& ctx) : m_kernelCtx(ctx) { }
	~CVisualizationWidget() override = default;

	bool initialize(const CIdentifier& id, const CString& name, const VisualizationToolkit::EVisualizationWidget type,
					const CIdentifier& parentID, const CIdentifier& boxID, const size_t nChild) override;

	CIdentifier getIdentifier() const override { return m_id; }
	const CString& getName() const override { return m_name; }
	void setName(const CString& name) override { m_name = name; }
	VisualizationToolkit::EVisualizationWidget getType() const override { return m_type; }
	CIdentifier getParentIdentifier() const override { return m_parentID; }
	void setParentIdentifier(const CIdentifier& parentID) override { m_parentID = parentID; }
	CIdentifier getBoxIdentifier() const override { return m_boxID; }
	size_t getNbChildren() const override { return m_childrens.size(); }
	bool getChildIndex(const CIdentifier& id, size_t& index) const override;

	//for windows, the number of children is unknown a priori
	bool addChild(const CIdentifier& childID) override;
	bool removeChild(const CIdentifier& id) override;
	bool getChildIdentifier(const size_t index, CIdentifier& id) const override;
	bool setChildIdentifier(const size_t index, const CIdentifier& id) override;

	void setWidth(const size_t width) override { m_width = width; }
	void setHeight(const size_t height) override { m_height = height; }

	size_t getWidth() override { return m_width; }
	size_t getHeight() override { return m_height; }

	void setDividerPosition(const int pos) override { m_dividerPosition = pos; }
	void setMaxDividerPosition(const int pos) override { m_maxDividerPosition = pos; }

	int getDividerPosition() override { return m_dividerPosition; }
	int getMaxDividerPosition() override { return m_maxDividerPosition; }

private:

	const Kernel::IKernelContext& m_kernelCtx;
	CIdentifier m_id = OV_UndefinedIdentifier;
	CString m_name;
	VisualizationToolkit::EVisualizationWidget m_type = VisualizationToolkit::EVisualizationWidget::Undefined;
	CIdentifier m_parentID                            = OV_UndefinedIdentifier;
	CIdentifier m_boxID                               = OV_UndefinedIdentifier;
	std::vector<CIdentifier> m_childrens;

	// @fixme should initialize meaningfully in constructor or initialize()?
	size_t m_width           = 0;
	size_t m_height          = 0;
	int m_dividerPosition    = std::numeric_limits<int>::min();
	int m_maxDividerPosition = std::numeric_limits<int>::min();
};
}  // namespace Designer
}  // namespace OpenViBE
