#include "ovdCCommentProxy.h"
#include "ovdTAttributeHandler.h"

using namespace OpenViBE;
using namespace Kernel;
using namespace OpenViBEDesigner;
using namespace std;

CCommentProxy::CCommentProxy(const IKernelContext& rKernelContext, const IComment& rComment)
	: m_rKernelContext(rKernelContext), m_pConstComment(&rComment)
{
	if (m_pConstComment)
	{
		const TAttributeHandler l_oAttributeHandler(*m_pConstComment);
		m_centerX = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Comment_XCenterPosition);
		m_centerY = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Comment_YCenterPosition);
	}
}

CCommentProxy::CCommentProxy(const IKernelContext& rKernelContext, IScenario& rScenario, const CIdentifier& rCommentIdentifier)
	: m_rKernelContext(rKernelContext), m_pConstComment(rScenario.getCommentDetails(rCommentIdentifier)), m_pComment(rScenario.getCommentDetails(rCommentIdentifier))
{
	if (m_pConstComment)
	{
		const TAttributeHandler l_oAttributeHandler(*m_pConstComment);
		m_centerX = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Comment_XCenterPosition);
		m_centerY = l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Comment_YCenterPosition);
	}
}

CCommentProxy::~CCommentProxy()
{
	if (!m_bApplied) { this->apply(); }
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
	m_bApplied = false;
}

void CCommentProxy::apply()

{
	if (m_pComment)
	{
		TAttributeHandler l_oAttributeHandler(*m_pComment);

		if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Comment_XCenterPosition))
		{
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Comment_XCenterPosition, m_centerX);
		}
		else
		{
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Comment_XCenterPosition, m_centerX);
		}

		if (l_oAttributeHandler.hasAttribute(OV_AttributeId_Comment_YCenterPosition))
		{
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Comment_YCenterPosition, m_centerY);
		}
		else
		{
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Comment_YCenterPosition, m_centerY);
		}
		m_bApplied = true;
	}
}

const char* CCommentProxy::getLabel() const
{
	m_sLabel = m_pConstComment->getText().toASCIIString();
	return m_sLabel.c_str();
}

void CCommentProxy::updateSize(GtkWidget* widget, const char* sText, int* pXSize, int* pYSize) const
{
	PangoContext* l_pPangoContext = nullptr;
	PangoLayout* l_pPangoLayout = nullptr;
	PangoRectangle l_oPangoRectangle;
	l_pPangoContext = gtk_widget_create_pango_context(widget);
	l_pPangoLayout = pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	if (pango_parse_markup(sText, -1, 0, nullptr, nullptr, nullptr, nullptr))
	{
		pango_layout_set_markup(l_pPangoLayout, sText, -1);
	}
	else
	{
		pango_layout_set_text(l_pPangoLayout, sText, -1);
	}
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoRectangle);
	*pXSize = l_oPangoRectangle.width;
	*pYSize = l_oPangoRectangle.height;
	g_object_unref(l_pPangoLayout);
	g_object_unref(l_pPangoContext);
}
