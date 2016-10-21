#pragma once

#include "ovviz_base.h"

namespace OpenViBEVisualizationToolkit
{
	namespace Tools
	{
		namespace ColorGradient
		{
			OVVIZ_API bool parse(OpenViBE::IMatrix& colorGradient, const OpenViBE::CString& string);
			OVVIZ_API bool format(OpenViBE::CString& string, const OpenViBE::IMatrix& colorGradient);
			OVVIZ_API bool interpolate(OpenViBE::IMatrix& interpolatedColorGradient, const OpenViBE::IMatrix& colorGradient, const OpenViBE::uint32 steps);
		}
	}
}

