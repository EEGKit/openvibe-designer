#pragma once

#include "ovd_base.h"

#include <string>

namespace OpenViBE {
namespace Designer {

class CCommentProxy final
{
public:

	CCommentProxy(const Kernel::IKernelContext& ctx, const Kernel::IComment& comment);
	CCommentProxy(const Kernel::IKernelContext& ctx, Kernel::IScenario& scenario, const CIdentifier& commentID);
	~CCommentProxy() { if (!m_applied) { this->apply(); } }

	operator Kernel::IComment*() const { return m_comment; }
	operator const Kernel::IComment*() const { return m_constComment; }

	int getWidth(GtkWidget* widget) const;
	int getHeight(GtkWidget* widget) const;

	int getXCenter() const { return m_centerX; }
	int getYCenter() const { return m_centerY; }

	void setCenter(int centerX, int centerY);

	void apply();

	const char* getLabel() const;

protected:

	static void updateSize(GtkWidget* widget, const char* text, int* xSize, int* ySize);

	const Kernel::IKernelContext& m_kernelCtx;
	const Kernel::IComment* m_constComment = nullptr;
	Kernel::IComment* m_comment            = nullptr;
	bool m_applied                         = false;
	int m_centerX                          = 0;
	int m_centerY                          = 0;
	mutable std::string m_label;
};

}  // namespace Designer
}  // namespace OpenViBE
