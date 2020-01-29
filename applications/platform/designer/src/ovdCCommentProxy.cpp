#include "ovdCCommentProxy.h"
#include "ovdTAttributeHandler.h"

using namespace OpenViBE;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Designer;
using namespace std;

CCommentProxy::CCommentProxy(const IKernelContext& ctx, const IComment& comment)
	: m_kernelCtx(ctx), m_constComment(&comment)
{
	if (m_constComment)
	{
		const TAttributeHandler handler(*m_constComment);
		m_centerX = handler.getAttributeValue<int>(OV_AttributeId_Comment_XCenterPosition);
		m_centerY = handler.getAttributeValue<int>(OV_AttributeId_Comment_YCenterPosition);
	}
}

CCommentProxy::CCommentProxy(const IKernelContext& ctx, IScenario& scenario, const CIdentifier& commentID)
	: m_kernelCtx(ctx), m_constComment(scenario.getCommentDetails(commentID)), m_comment(scenario.getCommentDetails(commentID))
{
	if (m_constComment)
	{
		const TAttributeHandler handler(*m_constComment);
		m_centerX = handler.getAttributeValue<int>(OV_AttributeId_Comment_XCenterPosition);
		m_centerY = handler.getAttributeValue<int>(OV_AttributeId_Comment_YCenterPosition);
	}
}

int CCommentProxy::getWidth(GtkWidget* widget) const
{
	int x, y;
	updateSize(widget, getLabel(), &x, &y);
	return x;
}

int CCommentProxy::getHeight(GtkWidget* widget) const
{
	int x, y;
	updateSize(widget, getLabel(), &x, &y);
	return y;
}

void CCommentProxy::setCenter(const int centerX, const int centerY)
{
	m_centerX = centerX;
	m_centerY = centerY;
	m_applied = false;
}

void CCommentProxy::apply()
{
	if (m_comment)
	{
		TAttributeHandler handler(*m_comment);

		if (handler.hasAttribute(OV_AttributeId_Comment_XCenterPosition)) { handler.setAttributeValue<int>(OV_AttributeId_Comment_XCenterPosition, m_centerX); }
		else { handler.addAttribute<int>(OV_AttributeId_Comment_XCenterPosition, m_centerX); }

		if (handler.hasAttribute(OV_AttributeId_Comment_YCenterPosition)) { handler.setAttributeValue<int>(OV_AttributeId_Comment_YCenterPosition, m_centerY); }
		else { handler.addAttribute<int>(OV_AttributeId_Comment_YCenterPosition, m_centerY); }
		m_applied = true;
	}
}

const char* CCommentProxy::getLabel() const
{
	m_label = m_constComment->getText().toASCIIString();
	return m_label.c_str();
}

void CCommentProxy::updateSize(GtkWidget* widget, const char* text, int* xSize, int* ySize)
{
	PangoRectangle rectangle;
	PangoContext* context = gtk_widget_create_pango_context(widget);
	PangoLayout* layout   = pango_layout_new(context);

	pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
	if (pango_parse_markup(text, -1, 0, nullptr, nullptr, nullptr, nullptr)) { pango_layout_set_markup(layout, text, -1); }
	else { pango_layout_set_text(layout, text, -1); }
	pango_layout_get_pixel_extents(layout, nullptr, &rectangle);
	*xSize = rectangle.width;
	*ySize = rectangle.height;
	g_object_unref(layout);
	g_object_unref(context);
}
