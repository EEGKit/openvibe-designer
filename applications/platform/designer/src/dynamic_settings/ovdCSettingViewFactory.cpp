#include "ovdCSettingViewFactory.h"

#include "ovdCBooleanSettingView.h"
#include "ovdCIntegerSettingView.h"
#include "ovdCFloatSettingView.h"
#include "ovdCStringSettingView.h"
#include "ovdCFilenameSettingView.h"
#include "ovdCScriptSettingView.h"
#include "ovdCColorSettingView.h"
#include "ovdCColorGradientSettingView.h"
#include "ovdCEnumerationSettingView.h"
#include "ovdCBitMaskSettingView.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace Setting;

CSettingViewFactory::CSettingViewFactory(const CString& rBuilderName, const Kernel::IKernelContext& ctx)
	: m_builderName(rBuilderName), m_kernelContext(ctx) {}

CAbstractSettingView* CSettingViewFactory::getSettingView(Kernel::IBox& box, const uint32_t index)
{
	CIdentifier l_oSettingType;
	box.getSettingType(index, l_oSettingType);

	if (l_oSettingType == OV_TypeId_Boolean) { return new CBooleanSettingView(box, index, m_builderName); }
	if (l_oSettingType == OV_TypeId_Integer) { return new CIntegerSettingView(box, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_Float) { return new CFloatSettingView(box, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_String) { return new CStringSettingView(box, index, m_builderName); }
	if (l_oSettingType == OV_TypeId_Filename) { return new CFilenameSettingView(box, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_Script) { return new CScriptSettingView(box, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_Color) { return new CColorSettingView(box, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_ColorGradient) { return new CColorGradientSettingView(box, index, m_builderName, m_kernelContext); }
	if (m_kernelContext.getTypeManager().isEnumeration(l_oSettingType))
	{
		return new CEnumerationSettingView(box, index, m_builderName, m_kernelContext, l_oSettingType);
	}
	if (m_kernelContext.getTypeManager().isBitMask(l_oSettingType))
	{
		return new CBitMaskSettingView(box, index, m_builderName, m_kernelContext, l_oSettingType);
	}

	//By default we consider every settings as a string
	return new CStringSettingView(box, index, m_builderName);
}
