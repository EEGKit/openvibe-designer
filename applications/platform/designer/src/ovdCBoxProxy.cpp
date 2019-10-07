#include "ovdCBoxProxy.h"
#include "ovdTAttributeHandler.h"
#include <fs/Files.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace std;

CBoxProxy::CBoxProxy(const IKernelContext& ctx, IScenario& scenario, const CIdentifier& boxID)
	: m_kernelCtx(ctx), m_pConstBox(scenario.getBoxDetails(boxID)), m_pBox(scenario.getBoxDetails(boxID)),
	  m_IsDeprecated(m_kernelCtx.getPluginManager().isPluginObjectFlaggedAsDeprecated(m_pConstBox->getAlgorithmClassIdentifier()))
{
	m_IsBoxAlgorithmPresent = false;

	if (m_pConstBox)
	{
		if (m_pConstBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
		{
			CIdentifier metaboxId;
			metaboxId.fromString(m_pConstBox->getAttributeValue(OVP_AttributeId_Metabox_Identifier));
			const CString l_sMetaboxScenarioPath(m_kernelCtx.getMetaboxManager().getMetaboxFilePath(metaboxId));

			m_IsBoxAlgorithmPresent = FS::Files::fileExists(l_sMetaboxScenarioPath.toASCIIString());
		}
		else { m_IsBoxAlgorithmPresent = m_kernelCtx.getPluginManager().canCreatePluginObject(m_pConstBox->getAlgorithmClassIdentifier()); }

		const TAttributeHandler handler(*m_pConstBox);
		m_centerX = handler.getAttributeValue<int>(OV_AttributeId_Box_XCenterPosition);
		m_centerY = handler.getAttributeValue<int>(OV_AttributeId_Box_YCenterPosition);
	}
	m_bShowOriginalNameWhenModified = m_kernelCtx.getConfigurationManager().expandAsBoolean("${Designer_ShowOriginalBoxName}", true);
}

int CBoxProxy::getWidth(GtkWidget* widget) const
{
	int x, y;
	updateSize(widget, getLabel(), getStatusLabel(), &x, &y);
	return x;
}

int CBoxProxy::getHeight(GtkWidget* widget) const
{
	int x, y;
	updateSize(widget, getLabel(), getStatusLabel(), &x, &y);
	return y;
}

void CBoxProxy::setCenter(const int centerX, const int centerY)
{
	m_centerX  = centerX;
	m_centerY  = centerY;
	m_applied = false;
}

void CBoxProxy::setBoxAlgorithmDescriptorOverride(const IBoxAlgorithmDesc* pBoxAlgorithmDescriptor)
{
	m_pBoxAlgorithmDescriptorOverride = pBoxAlgorithmDescriptor;
}

void CBoxProxy::apply()
{
	if (m_pBox)
	{
		TAttributeHandler handler(*m_pBox);

		if (handler.hasAttribute(OV_AttributeId_Box_XCenterPosition))
		{
			handler.setAttributeValue<int>(OV_AttributeId_Box_XCenterPosition, m_centerX);
		}
		else { handler.addAttribute<int>(OV_AttributeId_Box_XCenterPosition, m_centerX); }

		if (handler.hasAttribute(OV_AttributeId_Box_YCenterPosition))
		{
			handler.setAttributeValue<int>(OV_AttributeId_Box_YCenterPosition, m_centerY);
		}
		else { handler.addAttribute<int>(OV_AttributeId_Box_YCenterPosition, m_centerY); }

		m_applied = true;
	}
}

const char* CBoxProxy::getLabel() const
{
	const bool l_bBoxCanChangeInput(
		m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput) || m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput));
	const bool l_bBoxCanChangeOutput(
		m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput) || m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput));
	const bool l_bBoxCanChangeSetting(
		m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting) || m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting));

	const IPluginObjectDesc* l_pDesc = nullptr;

	if (m_pBoxAlgorithmDescriptorOverride == nullptr)
	{
		l_pDesc = m_kernelCtx.getPluginManager().getPluginObjectDescCreating(m_pConstBox->getAlgorithmClassIdentifier());
	}
	else { l_pDesc = m_pBoxAlgorithmDescriptorOverride; }

	string l_sBoxName(m_pConstBox->getName());

	const string l_sRed("#602020");
	const string l_sGreen("#206020");
	const string l_sGrey("#404040");

	// pango is used in the box diplay to format the box name (e.g. bold to tell it's a configurable box)
	// incidently, this makes the box name pango-enabled. If an error appears in the markup, the box display would be wrong.
	if (!pango_parse_markup(l_sBoxName.c_str(), -1, 0, nullptr, nullptr, nullptr, nullptr))
	{
		// the name uses invalid markup characters
		// we sanitize the markup tag overture '<'
		// markup should not be used in the box name anyway (hidden feature),
		// but the character '<' may actually be useful in a valid name
		for (size_t c = 0; c < l_sBoxName.size(); ++c)
		{
			if (l_sBoxName[c] == '<')
			{
				l_sBoxName[c] = ';';
				l_sBoxName.insert(c, "&lt");
			}
			else if (l_sBoxName[c] == '&')
			{
				l_sBoxName[c] = ';';
				l_sBoxName.insert(c, "&amp");
			}
		}
	}

	m_sLabel = l_sBoxName;

	if (l_sBoxName.empty()) { m_sLabel = "Unnamed Box"; }

	const std::string l_sBoxNameColor = "#000000";

	if (m_pConstBox->getSettingCount() != 0) { m_sLabel = "<span color=\"" + l_sBoxNameColor + R"(" weight="bold">)" + m_sLabel + "</span>"; }

	if (m_bShowOriginalNameWhenModified)
	{
		const string l_sBoxOriginalName(l_pDesc ? string(l_pDesc->getName()) : l_sBoxName);
		if (l_sBoxOriginalName != l_sBoxName)
		{
			m_sLabel = "<small><i><span foreground=\"" + l_sGrey + "\">" + l_sBoxOriginalName + "</span></i></small>\n" + m_sLabel;
		}
	}

	if (l_bBoxCanChangeInput || l_bBoxCanChangeOutput || l_bBoxCanChangeSetting)
	{
		m_sLabel += "\n";
		m_sLabel += "<span size=\"smaller\">";
		m_sLabel += "<span foreground=\"" + (l_bBoxCanChangeInput ? l_sGreen : l_sRed) + "\">In</span>";
		m_sLabel += "|";
		m_sLabel += "<span foreground=\"" + (l_bBoxCanChangeOutput ? l_sGreen : l_sRed) + "\">Out</span>";
		m_sLabel += "|";
		m_sLabel += "<span foreground=\"" + (l_bBoxCanChangeSetting ? l_sGreen : l_sRed) + "\">Set</span>";
		m_sLabel += "</span>";
	}

	return m_sLabel.c_str();
}

const char* CBoxProxy::getStatusLabel() const
{
	const bool l_bBoxToBeUpdated(m_pBox->hasAttribute(OV_AttributeId_Box_ToBeUpdated));
	const bool l_bBoxPendingDeprecatedInterfacors(m_pBox->hasAttribute(OV_AttributeId_Box_PendingDeprecatedInterfacors));
	const bool l_bBoxIsDeprecated(this->isBoxAlgorithmPluginPresent() && this->isDeprecated());
	const bool l_bBoxIsDisabled(this->isDisabled());

	const string l_sBlue("#202060");

	m_sStatus = "";
	if (l_bBoxIsDeprecated || l_bBoxToBeUpdated || l_bBoxIsDisabled || l_bBoxPendingDeprecatedInterfacors)
	{
		m_sStatus += R"(<span size="smaller" foreground=")" + l_sBlue + "\">";
		if (l_bBoxIsDeprecated) { m_sStatus += " <span style=\"italic\">deprecated</span>"; }
		if (l_bBoxToBeUpdated) { m_sStatus += " <span style=\"italic\">update</span>"; }
		if (l_bBoxIsDisabled) { m_sStatus += " <span style=\"italic\">disabled</span>"; }
		if (l_bBoxPendingDeprecatedInterfacors) { m_sStatus += " <span style=\"italic\">deprecated I/O/S</span>"; }
		m_sStatus += " </span>";
	}
	return m_sStatus.c_str();
}

bool CBoxProxy::isDisabled() const
{
	const TAttributeHandler handler(*m_pConstBox);
	return handler.hasAttribute(OV_AttributeId_Box_Disabled);
}

void CBoxProxy::updateSize(GtkWidget* widget, const char* sLabel, const char* sStatus, int* pXSize, int* pYSize) const
{
	PangoRectangle l_oPangoLabelRect;
	PangoRectangle l_oPangoStatusRect;
	PangoContext* l_pPangoContext = gtk_widget_create_pango_context(widget);
	PangoLayout* l_pPangoLayout   = pango_layout_new(l_pPangoContext);
	pango_layout_set_markup(l_pPangoLayout, sLabel, -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoLabelRect);
	pango_layout_set_markup(l_pPangoLayout, sStatus, -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, nullptr, &l_oPangoStatusRect);

	if (!strlen(sStatus))
	{
		l_oPangoStatusRect.width  = 0;
		l_oPangoStatusRect.height = 0;
	}

	*pXSize = max(l_oPangoLabelRect.width, l_oPangoStatusRect.width);
	*pYSize = l_oPangoLabelRect.height + l_oPangoStatusRect.height;

	g_object_unref(l_pPangoLayout);
	g_object_unref(l_pPangoContext);
}
