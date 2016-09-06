#ifndef __OpenViBEToolkit_Tools_ColorGradient_H__
#define __OpenViBEToolkit_Tools_ColorGradient_H__

#include "ovd_base.h"

namespace OpenViBEToolkit
{
	namespace Tools
	{
		namespace ColorGradient
		{
			OpenViBE::boolean parse(OpenViBE::IMatrix& rColorGradient, const OpenViBE::CString& rString);
			OpenViBE::boolean format(OpenViBE::CString& rString, const OpenViBE::IMatrix& rColorGradient);
			OpenViBE::boolean interpolate(OpenViBE::IMatrix& rInterpolatedColorGradient, const OpenViBE::IMatrix& rColorGradient, const OpenViBE::uint32 ui32Steps);
		};
	};
};

#endif // __OpenViBEToolkit_Tools_ColorGradient_H__
