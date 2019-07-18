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

CSettingViewFactory::CSettingViewFactory(const CString& rBuilderName, const Kernel::IKernelContext& rKernelContext)
	: m_builderName(rBuilderName), m_kernelContext(rKernelContext) {}

CAbstractSettingView* CSettingViewFactory::getSettingView(Kernel::IBox& rBox, const uint32_t index)
{
	CIdentifier l_oSettingType;
	rBox.getSettingType(index, l_oSettingType);

	if (l_oSettingType == OV_TypeId_Boolean) { return new CBooleanSettingView(rBox, index, m_builderName); }
	if (l_oSettingType == OV_TypeId_Integer) { return new CIntegerSettingView(rBox, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_Float) { return new CFloatSettingView(rBox, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_String) { return new CStringSettingView(rBox, index, m_builderName); }
	if (l_oSettingType == OV_TypeId_Filename) { return new CFilenameSettingView(rBox, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_Script) { return new CScriptSettingView(rBox, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_Color) { return new CColorSettingView(rBox, index, m_builderName, m_kernelContext); }
	if (l_oSettingType == OV_TypeId_ColorGradient) { return new CColorGradientSettingView(rBox, index, m_builderName, m_kernelContext); }
	if (m_kernelContext.getTypeManager().isEnumeration(l_oSettingType))
	{
		return new CEnumerationSettingView(rBox, index, m_builderName, m_kernelContext, l_oSettingType);
	}
	if (m_kernelContext.getTypeManager().isBitMask(l_oSettingType))
	{
		return new CBitMaskSettingView(rBox, index, m_builderName, m_kernelContext, l_oSettingType);
	}

	//By default we consider every settings as a string
	return new CStringSettingView(rBox, index, m_builderName);
}
