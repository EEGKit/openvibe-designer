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

namespace OpenViBE {
namespace Designer {
namespace Setting {

CAbstractSettingView* CSettingViewFactory::getSettingView(Kernel::IBox& box, const size_t index)
{
	CIdentifier type;
	box.getSettingType(index, type);

	if (type == OV_TypeId_Boolean) { return new CBooleanSettingView(box, index, m_builderName); }
	if (type == OV_TypeId_Integer) { return new CIntegerSettingView(box, index, m_builderName, m_kernelCtx); }
	if (type == OV_TypeId_Float) { return new CFloatSettingView(box, index, m_builderName, m_kernelCtx); }
	if (type == OV_TypeId_String) { return new CStringSettingView(box, index, m_builderName); }
	if (type == OV_TypeId_Filename) { return new CFilenameSettingView(box, index, m_builderName, m_kernelCtx); }
	if (type == OV_TypeId_Script) { return new CScriptSettingView(box, index, m_builderName, m_kernelCtx); }
	if (type == OV_TypeId_Color) { return new CColorSettingView(box, index, m_builderName, m_kernelCtx); }
	if (type == OV_TypeId_ColorGradient) { return new CColorGradientSettingView(box, index, m_builderName, m_kernelCtx); }
	if (m_kernelCtx.getTypeManager().isEnumeration(type)) { return new CEnumerationSettingView(box, index, m_builderName, m_kernelCtx, type); }
	if (m_kernelCtx.getTypeManager().isBitMask(type)) { return new CBitMaskSettingView(box, index, m_builderName, m_kernelCtx, type); }

	//By default we consider every settings as a string
	return new CStringSettingView(box, index, m_builderName);
}

}  // namespace Setting
}  // namespace Designer
}  // namespace OpenViBE
