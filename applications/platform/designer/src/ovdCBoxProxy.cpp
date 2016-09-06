#include "ovdCBoxProxy.h"
#include "ovdTAttributeHandler.h"
#include <fs/Files.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

CBoxProxy::CBoxProxy(const IKernelContext& rKernelContext, const IBox& rBox)
	:m_rKernelContext(rKernelContext)
	,m_pBoxAlgorithmDescriptorOverride(NULL)
	,m_pConstBox(&rBox)
	,m_pBox(NULL)
	,m_bApplied(false)
	,m_bShowOriginalNameWhenModified(false)
	,m_iXCenter(0)
	,m_iYCenter(0)
{
	if(m_pConstBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstBox);
		m_iXCenter=l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Box_XCenterPosition);
		m_iYCenter=l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Box_YCenterPosition);
	}
	m_bShowOriginalNameWhenModified=m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowOriginalBoxName}", true);
}

CBoxProxy::CBoxProxy(const IKernelContext& rKernelContext, IScenario& rScenario, const CIdentifier& rBoxIdentifier)
	:m_rKernelContext(rKernelContext)
	,m_pBoxAlgorithmDescriptorOverride(NULL)
	,m_pConstBox(rScenario.getBoxDetails(rBoxIdentifier))
	,m_pBox(rScenario.getBoxDetails(rBoxIdentifier))
	,m_bApplied(false)
	,m_bShowOriginalNameWhenModified(false)
	,m_iXCenter(0)
	,m_iYCenter(0)
{
	if(m_pConstBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstBox);
		m_iXCenter=l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Box_XCenterPosition);
		m_iYCenter=l_oAttributeHandler.getAttributeValue<int>(OV_AttributeId_Box_YCenterPosition);
	}
	m_bShowOriginalNameWhenModified=m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowOriginalBoxName}", true);
}

CBoxProxy::~CBoxProxy(void)
{
	if(!m_bApplied)
	{
		this->apply();
	}
}

CBoxProxy::operator IBox* (void)
{
	return m_pBox;
}

CBoxProxy::operator const IBox* (void)
{
	return m_pConstBox;
}

int32 CBoxProxy::getWidth(::GtkWidget* pWidget) const
{
	int x, y;
	updateSize(pWidget, getLabel(), &x, &y);
	return x;
}

int32 CBoxProxy::getHeight(::GtkWidget* pWidget) const
{
	int x, y;
	updateSize(pWidget, getLabel(), &x, &y);
	return y;
}

int32 CBoxProxy::getXCenter(void) const
{
	return m_iXCenter;
}

int32 CBoxProxy::getYCenter(void) const
{
	return m_iYCenter;
}

void CBoxProxy::setCenter(int32 i32XCenter, int32 i32YCenter)
{
	m_iXCenter=i32XCenter;
	m_iYCenter=i32YCenter;
	m_bApplied=false;
}

void CBoxProxy::setBoxAlgorithmDescriptorOverride(const IBoxAlgorithmDesc* pBoxAlgorithmDescriptor)
{
	m_pBoxAlgorithmDescriptorOverride = pBoxAlgorithmDescriptor;
}

void CBoxProxy::apply(void)
{
	if(m_pBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pBox);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_XCenterPosition))
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Box_XCenterPosition, m_iXCenter);
		else
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Box_XCenterPosition, m_iXCenter);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_YCenterPosition))
			l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Box_YCenterPosition, m_iYCenter);
		else
			l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Box_YCenterPosition, m_iYCenter);

		m_bApplied=true;
	}
}

const char* CBoxProxy::getLabel(void) const
{
	boolean l_bBoxCanChangeInput  (m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput)  ||m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput));
	boolean l_bBoxCanChangeOutput (m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput) ||m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput));
	boolean l_bBoxCanChangeSetting(m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting)||m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting));
	boolean l_bBoxIsUpToDate      (this->isUpToDate());
//	boolean l_bBoxIsUpToDate      (this->isBoxAlgorithmPluginPresent()  ? this->isUpToDate() : true);
	boolean l_bBoxIsDeprecated    (this->isBoxAlgorithmPluginPresent() && this->isDeprecated());
	boolean l_bBoxIsMetabox       (this->isMetabox());
	boolean l_bBoxIsDisabled      (this->isDisabled());

	const IPluginObjectDesc* l_pDesc = NULL;

	if (m_pBoxAlgorithmDescriptorOverride == NULL)
	{
		l_pDesc = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(m_pConstBox->getAlgorithmClassIdentifier());
	}
	else
	{
		l_pDesc = m_pBoxAlgorithmDescriptorOverride;
	}

	string l_sBoxName(m_pConstBox->getName());

	const string l_sRed("#602020");
	const string l_sGreen("#206020");
	const string l_sBlue("#202060");
	const string l_sGrey("#404040");

	// pango is used in the box diplay to format the box name (e.g. bold to tell it's a configurable box)
	// incidently, this makes the box name pango-enabled. If an error appears in the markup, the box display would be wrong.
	if (!pango_parse_markup(l_sBoxName.c_str(), -1, 0, NULL, NULL, NULL, NULL))
	{	
		// the name uses invalid markup characters
		// we sanitize the markup tag overture '<'
		// markup should not be used in the box name anyway (hidden feature),
		// but the character '<' may actually be useful in a valid name
		for (uint32 c = 0; c<l_sBoxName.size(); c++)
		{
			if (l_sBoxName[c] == '<')
			{
				l_sBoxName[c] = ';';
				l_sBoxName.insert(c,"&lt");
			}
			else if (l_sBoxName[c] == '&')
			{
				l_sBoxName[c] = ';';
				l_sBoxName.insert(c, "&amp");
			}
		}
	}


	m_sLabel = l_sBoxName;

	if (l_sBoxName == "")
	{
		m_sLabel = "Unnamed Box";
	}

	std::string l_sBoxNameColor = "#000000";

	if (m_pConstBox->getSettingCount()!=0)
	{
		m_sLabel = "<span color=\"" + l_sBoxNameColor + "\" weight=\"bold\">" + m_sLabel + "</span>";
	}

	/*
	if (l_bBoxIsMetabox)
	{
		m_sLabel = "<span color=\"#005020\" weight=\"bold\">[ </span>" + m_sLabel + "<span color=\"#005020\" weight=\"bold\"> ]</span>";
	}
	*/

	if(m_bShowOriginalNameWhenModified)
	{
		string l_sBoxOriginalName(l_pDesc ? string(l_pDesc->getName()) : l_sBoxName);
		if (l_sBoxOriginalName != l_sBoxName)
		{
			m_sLabel = "<small><i><span foreground=\"" + l_sGrey + "\">" + l_sBoxOriginalName + "</span></i></small>\n" + m_sLabel;
		}
	}

	if(l_bBoxCanChangeInput || l_bBoxCanChangeOutput || l_bBoxCanChangeSetting)
	{
		m_sLabel+="\n";
		m_sLabel+="<span size=\"smaller\">";
		m_sLabel+="<span foreground=\""+(l_bBoxCanChangeInput?l_sGreen:l_sRed)+"\">In</span>";
		m_sLabel+="|";
		m_sLabel+="<span foreground=\""+(l_bBoxCanChangeOutput?l_sGreen:l_sRed)+"\">Out</span>";
		m_sLabel+="|";
		m_sLabel+="<span foreground=\""+(l_bBoxCanChangeSetting?l_sGreen:l_sRed)+"\">Set</span>";
		m_sLabel+="</span>";
	}

	if(l_bBoxIsDeprecated || !l_bBoxIsUpToDate || l_bBoxIsDisabled )
	{
		m_sLabel+="\n";
		m_sLabel+="<span size=\"smaller\" foreground=\""+l_sBlue+"\">";
		if(l_bBoxIsDeprecated) m_sLabel+=" <span style=\"italic\">deprecated</span>";
		if(!l_bBoxIsUpToDate)  m_sLabel+=" <span style=\"italic\">update</span>";
		if(l_bBoxIsDisabled)   m_sLabel+=" <span style=\"italic\">disabled</span>";
		m_sLabel+=" </span>";
	}
	return m_sLabel.c_str();
}

boolean CBoxProxy::isBoxAlgorithmPluginPresent(void) const
{
	if (m_pConstBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CString l_sMetaboxIdentifier = m_pConstBox->getAttributeValue(OVP_AttributeId_Metabox_Scenario);
		CString l_sMetaboxScenarioPath = m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenValue(CString("Metabox_Scenario_Path_For_") + l_sMetaboxIdentifier);

		return FS::Files::fileExists(l_sMetaboxScenarioPath.toASCIIString());
	}
	return m_rKernelContext.getPluginManager().canCreatePluginObject(m_pConstBox->getAlgorithmClassIdentifier());
}

boolean CBoxProxy::isUpToDate(void) const
{
	// TODO_JL Manage the updating stuff here
	// one way would be to calculate the hashes inside the metaboxmanager and use that here

	CIdentifier l_oBoxHashCode1;
	CIdentifier l_oBoxHashCode2;
	if (m_pConstBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox)
	{
		CString l_sMetaboxIdentifier = m_pConstBox->getAttributeValue(OVP_AttributeId_Metabox_Scenario);
		l_oBoxHashCode1.fromString(m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenValue(CString("Metabox_Scenario_Hash_For_") + l_sMetaboxIdentifier));
	}
	else
	{
		l_oBoxHashCode1 = m_rKernelContext.getPluginManager().getPluginObjectHashValue(m_pConstBox->getAlgorithmClassIdentifier());
	}

	l_oBoxHashCode2.fromString(m_pConstBox->getAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue));
	return l_oBoxHashCode1==OV_UndefinedIdentifier || (l_oBoxHashCode1!=OV_UndefinedIdentifier && l_oBoxHashCode1==l_oBoxHashCode2);
}

boolean CBoxProxy::isDeprecated(void) const
{
	return m_rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(m_pConstBox->getAlgorithmClassIdentifier());
}

boolean CBoxProxy::isMetabox(void) const
{
	return m_pConstBox->getAlgorithmClassIdentifier() == OVP_ClassId_BoxAlgorithm_Metabox;
}

boolean CBoxProxy::isDisabled(void) const
{
	TAttributeHandler l_oAttributeHandler(*m_pConstBox);
	return l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Disabled);
}

void CBoxProxy::updateSize(::GtkWidget* pWidget, const char* sText, int* pXSize, int* pYSize) const
{
	::PangoContext* l_pPangoContext=NULL;
	::PangoLayout* l_pPangoLayout=NULL;
	::PangoRectangle l_oPangoRectangle;
	l_pPangoContext=gtk_widget_create_pango_context(pWidget);
	l_pPangoLayout=pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(l_pPangoLayout, sText, -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, NULL, &l_oPangoRectangle);
	*pXSize=l_oPangoRectangle.width;
	*pYSize=l_oPangoRectangle.height;
	g_object_unref(l_pPangoLayout);
	g_object_unref(l_pPangoContext);
}
