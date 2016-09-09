#include "ovdCFileFormats.h"
#include <ovp_global_defines.h>

namespace OpenViBEDesigner
{
	const std::map<std::string, std::pair<std::string, CFileFormats::EFileFormatType>> CFileFormats::filenameExtensionDescriptions = {
		{".xml", {"OpenViBE XML Format", CFileFormats::FileFormatType_Scenario}},
		{".mxs", {"Mensia XML Format", CFileFormats::FileFormatType_Scenario}},
		{".mxb", {"Mensia XML Component", CFileFormats::FileFormatType_Metabox}}
	};

	const std::map<std::string, OpenViBE::CIdentifier> CFileFormats::filenameExtensionImporters = {
		{".xml", OVP_GD_ClassId_Algorithm_XMLScenarioImporter},
		{".mxs", OVP_GD_ClassId_Algorithm_XMLScenarioImporter},
		{".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioImporter}
	};

	const std::map<std::string, OpenViBE::CIdentifier> CFileFormats::filenameExtensionExporters = {
		{".xml", OVP_GD_ClassId_Algorithm_XMLScenarioExporter},
		{".mxs", OVP_GD_ClassId_Algorithm_XMLScenarioExporter},
		{".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioExporter}
	};
}
