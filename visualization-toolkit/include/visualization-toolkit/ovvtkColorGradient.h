#pragma once

#include "ovvtk_base.h"

namespace OpenViBEToolkit
{
	namespace Tools
	{
		namespace ColorGradient
		{
			OVVTK_API OpenViBE::boolean parse(OpenViBE::IMatrix& rColorGradient, const OpenViBE::CString& rString);
			OVVTK_API OpenViBE::boolean format(OpenViBE::CString& rString, const OpenViBE::IMatrix& rColorGradient);
			OVVTK_API OpenViBE::boolean interpolate(OpenViBE::IMatrix& rInterpolatedColorGradient, const OpenViBE::IMatrix& rColorGradient, const OpenViBE::uint32 ui32Steps);
		};
	};
};

