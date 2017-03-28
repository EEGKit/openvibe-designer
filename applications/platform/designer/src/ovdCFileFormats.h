#pragma once

#include <map>
#include <string>
#include <openvibe/ovCIdentifier.h>

namespace OpenViBEDesigner
{
	class CFileFormats
	{
	public:
		enum EFileFormatType
		{
			FileFormatType_Scenario,
			FileFormatType_Metabox
		};
		static const std::map<std::string, std::pair<std::string, CFileFormats::EFileFormatType>> filenameExtensionDescriptions;
		static const std::map<std::string, OpenViBE::CIdentifier> filenameExtensionExporters;
	};

}
