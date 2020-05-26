#pragma once

#include "ovviz_base.h"

namespace OpenViBE {
namespace VisualizationToolkit {
namespace ColorGradient {

OVVIZ_API bool parse(IMatrix& colorGradient, const CString& string);
OVVIZ_API bool format(CString& string, const IMatrix& colorGradient);
OVVIZ_API bool interpolate(IMatrix& interpolatedColorGradient, const IMatrix& colorGradient, size_t steps);

}  // namespace ColorGradient
}  // namespace VisualizationToolkit
}  // namespace OpenViBE
