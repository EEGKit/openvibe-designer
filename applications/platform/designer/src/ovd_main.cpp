#include <stack>
#include <vector>
#include <map>
#include <tuple>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <json/json.h>

#include <system/ovCTime.h>
#include <system/ovCMath.h>
#include <openvibe/kernel/metabox/ovIMetaboxManager.h>
#include "ovd_base.h"

#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"

#include "ovdAssert.h"
#if defined TARGET_OS_Windows
#include "Windows.h"
#include "shellapi.h"
#endif

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;
using namespace OpenViBEDesigner;
using namespace std;

map<uint32_t, GdkColor> g_vColors;

class CPluginObjectDescEnum
{
public:

	explicit CPluginObjectDescEnum(const IKernelContext& rKernelContext) : m_kernelContext(rKernelContext) { }

	virtual ~CPluginObjectDescEnum() = default;

	virtual bool enumeratePluginObjectDesc()

	{
		CIdentifier identifier;
		while ((identifier = m_kernelContext.getPluginManager().getNextPluginObjectDescIdentifier(identifier)) != OV_UndefinedIdentifier)
		{
			this->callback(*m_kernelContext.getPluginManager().getPluginObjectDesc(identifier));
		}
		return true;
	}

	virtual bool enumeratePluginObjectDesc(const CIdentifier& rParentClassIdentifier)
	{
		CIdentifier identifier;
		while ((identifier = m_kernelContext.getPluginManager().getNextPluginObjectDescIdentifier(identifier, rParentClassIdentifier)) != OV_UndefinedIdentifier)
		{
			this->callback(*m_kernelContext.getPluginManager().getPluginObjectDesc(identifier));
		}
		return true;
	}

	virtual bool callback(const IPluginObjectDesc& rPluginObjectDesc) = 0;

protected:

	const IKernelContext& m_kernelContext;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescCollector : public CPluginObjectDescEnum
{
public:

	CPluginObjectDescCollector(const IKernelContext& rKernelContext) : CPluginObjectDescEnum(rKernelContext) { }

	bool callback(const IPluginObjectDesc& rPluginObjectDesc) override
	{
		const string l_sFullName      = string(rPluginObjectDesc.getCategory()) + "/" + string(rPluginObjectDesc.getName());
		const auto itPluginObjectDesc = m_vPluginObjectDesc.find(l_sFullName);
		if (itPluginObjectDesc != m_vPluginObjectDesc.end())
		{
			m_kernelContext.getLogManager() << LogLevel_ImportantWarning << "Duplicate plugin object name " << CString(l_sFullName.c_str()) << " " << itPluginObjectDesc->second->getCreatedClass() << " and " << rPluginObjectDesc.getCreatedClass() << "\n";
		}
		m_vPluginObjectDesc[l_sFullName] = &rPluginObjectDesc;
		return true;
	}

	map<string, const IPluginObjectDesc*>& getPluginObjectDescMap() { return m_vPluginObjectDesc; }

private:

	map<string, const IPluginObjectDesc*> m_vPluginObjectDesc;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescLogger : public CPluginObjectDescEnum
{
public:

	explicit CPluginObjectDescLogger(const IKernelContext& rKernelContext)
		: CPluginObjectDescEnum(rKernelContext) { }

	bool callback(const IPluginObjectDesc& rPluginObjectDesc) override
	{
		// Outputs plugin info to console
		m_kernelContext.getLogManager() << LogLevel_Trace << "Plugin <" << rPluginObjectDesc.getName() << ">\n";
		m_kernelContext.getLogManager() << LogLevel_Debug << " | Plugin category        : " << rPluginObjectDesc.getCategory() << "\n";
		m_kernelContext.getLogManager() << LogLevel_Debug << " | Class identifier       : " << rPluginObjectDesc.getCreatedClass() << "\n";
		m_kernelContext.getLogManager() << LogLevel_Debug << " | Author name            : " << rPluginObjectDesc.getAuthorName() << "\n";
		m_kernelContext.getLogManager() << LogLevel_Debug << " | Author company name    : " << rPluginObjectDesc.getAuthorCompanyName() << "\n";
		m_kernelContext.getLogManager() << LogLevel_Debug << " | Short description      : " << rPluginObjectDesc.getShortDescription() << "\n";
		m_kernelContext.getLogManager() << LogLevel_Debug << " | Detailed description   : " << rPluginObjectDesc.getDetailedDescription() << "\n";

		return true;
	}
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

namespace
{
	typedef std::map<std::string, std::tuple<int, int, int>> componentsMap;
	// Parses a JSON encoded list of components with their versions
	// We use an output variable because we want to be able to "enhance" an already existing list if necessary
	void getVersionComponentsFromConfigurationToken(const IKernelContext& context, const char* configurationToken, componentsMap& componentVersions)
	{
		json::Object componentVersionsObject;
		// We use a lookup instead of expansion as JSON can contain { } characters

		const CString componentVersionsJSON = context.getConfigurationManager().expand(CString("${") + configurationToken + "}");
		if (componentVersionsJSON.length() != 0)
		{
			// This check is necessary because the asignemt operator would fail with an assert
			if (json::Deserialize(componentVersionsJSON.toASCIIString()).GetType() == json::ObjectVal)
			{
				componentVersionsObject = json::Deserialize(componentVersionsJSON.toASCIIString());
			}
			for (const auto& component : componentVersionsObject)
			{
				int versionMajor, versionMinor, versionPatch;
				sscanf(component.second, "%d.%d.%d", &versionMajor, &versionMinor, &versionPatch);
				componentVersions[component.first] = std::make_tuple(versionMajor, versionMinor, versionPatch);
			}
		}
	}
} // namespace

static void insertPluginObjectDesc_to_GtkTreeStore(const IKernelContext& rKernelContext, map<string, const IPluginObjectDesc*>& vPluginObjectDesc, GtkTreeStore* pTreeStore,
												   std::vector<const IPluginObjectDesc*>& vNewBoxes, std::vector<const IPluginObjectDesc*>& vUpdatedBoxes, bool bIsNewVersion = false)
{
	typedef std::map<std::string, std::tuple<int, int, int>> componentsMap;
	componentsMap currentVersions;
	getVersionComponentsFromConfigurationToken(rKernelContext, "ProjectVersion_Components", currentVersions);
	// By default, fix version to current version - to display the new/update boxes available since current version only
	componentsMap lastUsedVersions = currentVersions;
	getVersionComponentsFromConfigurationToken(rKernelContext, "Designer_LastComponentVersionsUsed", lastUsedVersions);

	for (const auto& pPluginObjectDesc : vPluginObjectDesc)
	{
		const IPluginObjectDesc* l_pPluginObjectDesc = pPluginObjectDesc.second;

		CString l_sStockItemName;

		const auto* l_pBoxAlgorithmDesc = dynamic_cast<const IBoxAlgorithmDesc*>(l_pPluginObjectDesc);
		if (l_pBoxAlgorithmDesc != nullptr)
		{
			l_sStockItemName = l_pBoxAlgorithmDesc->getStockItemName();
		}

		bool l_bShouldShow = true;

		if (rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(l_pPluginObjectDesc->getCreatedClass())
			&& !rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowDeprecated}", false))
		{
			l_bShouldShow = false;
		}

		/*
		if  (rKernelContext.getPluginManager().isPluginObjectFlaggedAsUnstable(l_pPluginObjectDesc->getCreatedClass())
		&& !rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowUnstable}", false))
		{
		l_bShouldShow=false;
		}
		*/

		if (l_bShouldShow)
		{
			GtkStockItem l_oStockItem;
			if (gtk_stock_lookup(l_sStockItemName, &l_oStockItem) == 0)
			{
				l_sStockItemName = GTK_STOCK_NEW;
			}

			// Splits the plugin category
			vector<string> l_vCategory;
			string l_sCategory = string(l_pPluginObjectDesc->getCategory());
			size_t j, i        = size_t(-1);
			while ((j = l_sCategory.find('/', i + 1)) != string::npos)
			{
				string l_sSubCategory = string(l_sCategory, i + 1, j - i - 1);
				if (l_sSubCategory != string(""))
				{
					l_vCategory.push_back(l_sSubCategory);
				}
				i = j;
			}
			if (i + 1 != l_sCategory.length())
			{
				l_vCategory.emplace_back(l_sCategory, i + 1, l_sCategory.length() - i - 1);
			}

			// Fills plugin in the tree
			GtkTreeIter l_oGtkIter1;
			GtkTreeIter l_oGtkIter2;
			GtkTreeIter* l_pGtkIterParent = nullptr;
			GtkTreeIter* l_pGtkIterChild  = &l_oGtkIter1;
			for (const string& category : l_vCategory)
			{
				bool l_bFound = false;
				bool l_bValid = gtk_tree_model_iter_children(GTK_TREE_MODEL(pTreeStore), l_pGtkIterChild, l_pGtkIterParent) != 0;
				while (l_bValid && !l_bFound)
				{
					gchar* l_sName = nullptr;
					gboolean l_bIsPlugin;
					gtk_tree_model_get(GTK_TREE_MODEL(pTreeStore), l_pGtkIterChild, Resource_StringName, &l_sName, Resource_BooleanIsPlugin, &l_bIsPlugin, -1);
					if ((l_bIsPlugin == 0) && l_sName == category)
					{
						l_bFound = true;
					}
					else
					{
						l_bValid = gtk_tree_model_iter_next(GTK_TREE_MODEL(pTreeStore), l_pGtkIterChild) != 0;
					}
				}
				if (!l_bFound)
				{
					gtk_tree_store_append(GTK_TREE_STORE(pTreeStore), l_pGtkIterChild, l_pGtkIterParent);
					gtk_tree_store_set(GTK_TREE_STORE(pTreeStore), l_pGtkIterChild, Resource_StringName, category.c_str(),
									   Resource_StringShortDescription, "", Resource_StringStockIcon, "gtk-directory", Resource_StringColor, "#000000",
									   Resource_StringFont, "", Resource_BooleanIsPlugin, gboolean(FALSE), -1);
				}
				if (l_pGtkIterParent == nullptr)
				{
					l_pGtkIterParent = &l_oGtkIter2;
				}
				GtkTreeIter* l_pGtkIterSwap = l_pGtkIterChild;
				l_pGtkIterChild             = l_pGtkIterParent;
				l_pGtkIterParent            = l_pGtkIterSwap;
			}
			gtk_tree_store_append(GTK_TREE_STORE(pTreeStore), l_pGtkIterChild, l_pGtkIterParent);

			// define color of the text of the box
			std::string l_sTextColor       = "black";
			std::string l_sBackGroundColor = "white";
			std::string l_sTextFont;
			std::string l_sName(l_pPluginObjectDesc->getName().toASCIIString());

			if (rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(l_pPluginObjectDesc->getCreatedClass()))
			{
				l_sTextColor = "#3f7f7f";
			}

			// If the software is launched for the first time after update, highlight new/updated boxes in tree-view


			std::string boxSoftwareComponent = l_pPluginObjectDesc->getSoftwareComponent().toASCIIString();

			if (boxSoftwareComponent != "unknown")
			{
				int currentVersionMajor      = std::get<0>(currentVersions[boxSoftwareComponent]);
				int currentVersionMinor      = std::get<1>(currentVersions[boxSoftwareComponent]);
				int currentVersionPatch      = std::get<2>(currentVersions[boxSoftwareComponent]);
				int lastUsedVersionMajor     = std::get<0>(lastUsedVersions[boxSoftwareComponent]);
				int lastUsedVersionMinor     = std::get<1>(lastUsedVersions[boxSoftwareComponent]);
				int lastUsedVersionPatch     = std::get<2>(lastUsedVersions[boxSoftwareComponent]);
				int boxComponentVersionMajor = 0;
				int boxComponentVersionMinor = 0;
				int boxComponentVersionPatch = 0;
				sscanf(l_pPluginObjectDesc->getAddedSoftwareVersion().toASCIIString(), "%d.%d.%d", &boxComponentVersionMajor, &boxComponentVersionMinor, &boxComponentVersionPatch);
				// If this is a new version, then add in list all the updated/new boxes since last version opened
				if (bIsNewVersion && (
						(lastUsedVersionMajor < boxComponentVersionMajor && boxComponentVersionMajor <= currentVersionMajor)
						|| (boxComponentVersionMajor == currentVersionMajor && lastUsedVersionMinor < boxComponentVersionMinor && boxComponentVersionMinor <= currentVersionMinor)
						|| (boxComponentVersionMinor == currentVersionMinor && lastUsedVersionPatch < boxComponentVersionPatch && boxComponentVersionPatch <= currentVersionPatch)
						// As default value for l_uiMinorLastVersionOpened and l_uiMajorLastVersionOpened are the current software version
						|| (boxComponentVersionMajor == currentVersionMajor && boxComponentVersionMinor == currentVersionMinor && boxComponentVersionPatch == currentVersionPatch)))
				{
					l_sName += " (New)";
					l_sBackGroundColor = "#FFFFC4";
					vNewBoxes.push_back(l_pPluginObjectDesc);
				}
					// Otherwise
				else if (boxComponentVersionMajor == currentVersionMajor && boxComponentVersionMinor == currentVersionMinor && boxComponentVersionPatch == currentVersionPatch)
				{
					vNewBoxes.push_back(l_pPluginObjectDesc);
				}
				else
				{
					int boxComponentUpdatedVersionMajor = 0;
					int boxComponentUpdatedVersionMinor = 0;
					int boxComponentUpdatedVersionPatch = 0;
					sscanf(l_pPluginObjectDesc->getUpdatedSoftwareVersion().toASCIIString(), "%d.%d.%d", &boxComponentUpdatedVersionMajor, &boxComponentUpdatedVersionMinor, &boxComponentUpdatedVersionPatch);
					// If this is a new version, then add in list all the updated/new boxes since last version opened
					if (bIsNewVersion && (
							(lastUsedVersionMajor < boxComponentUpdatedVersionMajor && boxComponentUpdatedVersionMajor <= currentVersionMajor)
							|| (boxComponentUpdatedVersionMajor == currentVersionMajor && lastUsedVersionMinor < boxComponentUpdatedVersionMinor && boxComponentUpdatedVersionMinor <= currentVersionMinor)
							|| (boxComponentUpdatedVersionMinor == currentVersionMinor && lastUsedVersionPatch < boxComponentUpdatedVersionPatch && boxComponentUpdatedVersionPatch <= currentVersionPatch)
							// If this is a new version Designer, and last version opened was set to default value i.e. version of current software
							|| (boxComponentUpdatedVersionMajor == currentVersionMajor && boxComponentUpdatedVersionMinor == currentVersionMinor && boxComponentUpdatedVersionPatch == currentVersionPatch)))
					{
						l_sName += " (New)";
						l_sBackGroundColor = "#FFFFC4";
						vUpdatedBoxes.push_back(l_pPluginObjectDesc);
					}
						// Otherwise
					else if (!bIsNewVersion && (boxComponentUpdatedVersionMajor == currentVersionMajor && boxComponentUpdatedVersionMinor == currentVersionMinor && boxComponentUpdatedVersionPatch == currentVersionPatch))
					{
						vUpdatedBoxes.push_back(l_pPluginObjectDesc);
					}
				}
			}

			// Construct a string containing the BoxAlgorithmIdentifier concatenated with a metabox identifier if necessary
			std::string l_sBoxAlgorithmDescriptor(l_pPluginObjectDesc->getCreatedClass().toString().toASCIIString());

			if (l_pPluginObjectDesc->getCreatedClass() == OVP_ClassId_BoxAlgorithm_Metabox)
			{
				l_sBoxAlgorithmDescriptor += dynamic_cast<const Metabox::IMetaboxObjectDesc*>(l_pPluginObjectDesc)->getMetaboxDescriptor();
				l_sTextColor = "#007020";
			}
			else
			{
				if (l_pPluginObjectDesc->hasFunctionality(M_Functionality_IsMensia)) { l_sTextColor = "#00b090"; }
			}


			gtk_tree_store_set(GTK_TREE_STORE(pTreeStore), l_pGtkIterChild, Resource_StringName, l_sName.c_str(),
							   Resource_StringShortDescription, static_cast<const char*>(l_pPluginObjectDesc->getShortDescription()),
							   Resource_StringIdentifier, static_cast<const char*>(l_sBoxAlgorithmDescriptor.c_str()),
							   Resource_StringStockIcon, static_cast<const char*>(l_sStockItemName), Resource_StringColor, l_sTextColor.c_str(),
							   Resource_StringFont, l_sTextFont.c_str(), Resource_BooleanIsPlugin, gboolean(TRUE),
							   Resource_BackGroundColor, static_cast<const char*>(l_sBackGroundColor.c_str()), -1);
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

typedef struct _SConfiguration
{
	_SConfiguration() = default;

	ECommandLineFlag getFlags() const
	{
		return ECommandLineFlag(m_eNoGui | m_eNoCheckColorDepth | m_eNoManageSession | m_eNoVisualization | m_eDefine | m_eRandomSeed | m_eConfig);
	}

	std::vector<std::pair<ECommandLineFlag, std::string>> m_vFlag;
	ECommandLineFlag m_eNoGui             = CommandLineFlag_None;
	ECommandLineFlag m_eNoCheckColorDepth = CommandLineFlag_None;
	ECommandLineFlag m_eNoManageSession   = CommandLineFlag_None;
	ECommandLineFlag m_eNoVisualization   = CommandLineFlag_None;
	ECommandLineFlag m_eDefine            = CommandLineFlag_None;
	ECommandLineFlag m_eRandomSeed        = CommandLineFlag_None;
	ECommandLineFlag m_eConfig            = CommandLineFlag_None;
	bool m_help                           = false;
	// to resolve warning: padding struct '_SConfiguration' with 4 bytes to align 'm_oTokenMap
	int m_i32StructPadding = 0;
	std::map<std::string, std::string> m_oTokenMap;
} SConfiguration;

static char backslash_to_slash(const char c) { return c == '\\' ? '/' : c; }

/** ------------------------------------------------------------------------------------------------------------------------------------
* Use Mutex to ensure that only one instance with GUI of Designer runs at the same time
* if another instance exists, sends a message to it so that it opens a scenario or get the focus back
* \param configuration: play, play-fast or open
* \param l_rLogManager: name of the scenario to open
------------------------------------------------------------------------------------------------------------------------------------**/
static bool ensureOneInstanceOfDesigner(SConfiguration& configuration, ILogManager& l_rLogManager)
{
#if defined NDEBUG
	try
	{
		// If the mutex cannot be opened, it's the first instance of Designer, go to catch
		boost::interprocess::named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);

		// If the mutex was opened, then an instance of designer is already running, we send it a message before dying
		// The message contains the command to send: sMode: open, play, play-fast a scenario, sScenarioPath: path of the scenario
		boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex);
		std::string l_sMessage;
		if (configuration.m_vFlag.empty())
		{
			l_sMessage = std::to_string(int(CommandLineFlag_None)) + ": ;";
		}

		for (auto& flag : configuration.m_vFlag)
		{
			std::string fileName = flag.second;
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), backslash_to_slash);

			l_sMessage += std::to_string(int(flag.first)) + ": <" + fileName + "> ; ";
		}

		const size_t l_sizeMessage = strlen(l_sMessage.c_str()) * sizeof(char);

		boost::interprocess::message_queue messageToFirstInstance(boost::interprocess::open_or_create, MESSAGE_NAME, l_sizeMessage, l_sizeMessage);
		messageToFirstInstance.send(l_sMessage.c_str(), l_sizeMessage, 0);

		return false;
	}
	catch (boost::interprocess::interprocess_exception&)
	{
		//Create the named mutex to catch the potential next instance of Designer that could open
		boost::interprocess::named_mutex mutex(boost::interprocess::create_only, MUTEX_NAME);
		return true;
	}
#else
	return true;
#endif
}

bool parse_arguments(int argc, char** argv, SConfiguration& rConfiguration)
{
	SConfiguration l_oConfiguration;

	std::vector<std::string> l_vArgValue;
#if defined TARGET_OS_Windows
	int argCount;
	LPWSTR* argListUtf16 = CommandLineToArgvW(GetCommandLineW(), &argCount);
	for (int i = 1; i < argCount; ++i)
	{
		GError* error = nullptr;
		glong itemsRead, itemsWritten;
		char* argUtf8 = g_utf16_to_utf8(reinterpret_cast<gunichar2*>(argListUtf16[i]), glong(wcslen(argListUtf16[i])), &itemsRead, &itemsWritten, &error);
		l_vArgValue.emplace_back(argUtf8);
		if (error != nullptr)
		{
			g_error_free(error);
			return false;
		}
	}
#else
	l_vArgValue = std::vector<std::string>(argv + 1, argv + argc);
#endif
	l_vArgValue.emplace_back("");

	for (auto it = l_vArgValue.cbegin(); it != l_vArgValue.cend(); ++it)
	{
		if (*it == "") {}
		else if (*it == "-h" || *it == "--help")
		{
			l_oConfiguration.m_help = true;
			rConfiguration          = l_oConfiguration;
			return false;
		}
		else if (*it == "-o" || *it == "--open")
		{
			l_oConfiguration.m_vFlag.emplace_back(CommandLineFlag_Open, *++it);
		}
		else if (*it == "-p" || *it == "--play")
		{
			l_oConfiguration.m_vFlag.emplace_back(CommandLineFlag_Play, *++it);
		}
		else if (*it == "-pf" || *it == "--play-fast")
		{
			l_oConfiguration.m_vFlag.emplace_back(CommandLineFlag_PlayFast, *++it);
		}
		else if (*it == "--no-gui")
		{
			l_oConfiguration.m_eNoGui             = CommandLineFlag_NoGui;
			l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
			l_oConfiguration.m_eNoManageSession   = CommandLineFlag_NoManageSession;
		}
		else if (*it == "--no-visualization")
		{
			l_oConfiguration.m_eNoVisualization = CommandLineFlag_NoVisualization;
		}
		else if (*it == "--invisible")
		{
			// no-gui + no-visualization
			l_oConfiguration.m_eNoVisualization   = CommandLineFlag_NoVisualization;
			l_oConfiguration.m_eNoGui             = CommandLineFlag_NoGui;
			l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
			l_oConfiguration.m_eNoManageSession   = CommandLineFlag_NoManageSession;
		}
		else if (*it == "--no-check-color-depth")
		{
			l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
		}
		else if (*it == "--no-session-management")
		{
			l_oConfiguration.m_eNoManageSession = CommandLineFlag_NoManageSession;
		}
		else if (*it == "-c" || *it == "--config")
		{
			if (*++it == "")
			{
				std::cout << "Error: Switch --config needs an argument\n";
				return false;
			}
			l_oConfiguration.m_vFlag.emplace_back(CommandLineFlag_Config, *it);
		}
		else if (*it == "-d" || *it == "--define")
		{
			if (*++it == "")
			{
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			// Were not using = as a separator for token/value, as on Windows its a problem passing = to the cmd interpreter
			// which is used to launch the actual designer exe.
			const std::string& l_rToken = *it;
			if (*++it == "")
			{
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			const std::string& l_rValue = *it;	// iterator will increment later

			l_oConfiguration.m_oTokenMap[l_rToken] = l_rValue;
		}
		else if (*it == "--random-seed")
		{
			if (*++it == "")
			{
				std::cout << "Error: Switch --random-seed needs an argument\n";
				return false;
			}
			l_oConfiguration.m_vFlag.emplace_back(CommandLineFlag_RandomSeed, *it);
		}
		else if (*it == "--g-fatal-warnings")
		{
			// Do nothing here but accept this gtk flag
		}
		else
		{
#if 0
			// Assumes we just open a scenario - this is for retro compatibility and should not be supported in the future
			l_oConfiguration.m_vFlag.push_back(std::make_pair(CommandLineFlag_Open, *++it));
#endif
			return false;
		}
	}

#if 0
	rConfiguration.m_vFlag = l_oConfiguration.m_vFlag;
	rConfiguration.m_bCheckColorDepth = l_oConfiguration.m_bCheckColorDepth;
	rConfiguration.m_bShowGui = l_oConfiguration.m_bShowGui;
#else
	rConfiguration = l_oConfiguration;
#endif
	return true;
}


#if defined OPENVIBE_SPLASHSCREEN

gboolean cb_remove_splashscreen(gpointer data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	return false;
}
#endif


// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

void message(const char* sTitle, const char* sMessage, const GtkMessageType eType)
{
	GtkWidget* l_pDialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
												  eType, GTK_BUTTONS_OK, "%s", sTitle);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pDialog), "%s", sMessage);
	::gtk_window_set_icon_from_file(GTK_WINDOW(l_pDialog), (Directories::getDataDir() + CString("/applications/designer/designer.ico")).toASCIIString(), nullptr);
	gtk_window_set_title(GTK_WINDOW(l_pDialog), sTitle);
	gtk_dialog_run(GTK_DIALOG(l_pDialog));
	gtk_widget_destroy(l_pDialog);
}

void user_info(char** argv, ILogManager* l_rLogManager)
{
	const std::vector<std::string> messages =
	{
		"Syntax : " + std::string(argv[0]) + " [ switches ]\n",
		"Possible switches :\n",
		"  --help                  : displays this help message and exits\n",
		"  --config filename       : path to config file\n",
		"  --define token value    : specify configuration token with a given value\n",
		"  --open filename         : opens a scenario (see also --no-session-management)\n",
		"  --play filename         : plays the opened scenario (see also --no-session-management)\n",
		"  --play-fast filename    : plays fast forward the opened scenario (see also --no-session-management)\n",
		"  --no-gui                : hides the " DESIGNER_NAME " graphical user interface (assumes --no-color-depth-test)\n",
		"  --no-visualization      : hides the visualisation widgets\n",
		"  --invisible             : hides the designer and the visualisation widgets (assumes --no-check-color-depth and --no-session-management)\n",
		"  --no-check-color-depth  : does not check 24/32 bits color depth\n",
		"  --no-session-management : neither restore last used scenarios nor saves them at exit\n",
		"  --random-seed uint      : initialize random number generator with value, default=time(nullptr)\n"
	};

	if (l_rLogManager != nullptr)
	{
		for (const auto& m : messages) { (*l_rLogManager) << LogLevel_Info << m.c_str(); }
	}
	else
	{
		for (const auto& m : messages) { cout << m; }
	}
}

int go(int argc, char** argv)
{
	bool errorWhileLoadingScenario = false, playRequested = false;
	/*
	{ 0,     0,     0,     0 },
	{ 0, 16383, 16383, 16383 },
	{ 0, 32767, 32767, 32767 },
	{ 0, 49151, 49151, 49151 },
	{ 0, 65535, 65535, 65535 },
	*/
#define gdk_color_set(c, r, g, b) { (c).pixel=0; (c).red=r; (c).green=g; (c).blue=b; }
	gdk_color_set(g_vColors[Color_BackgroundPlayerStarted], 32767, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundSelected], 65535, 65535, 49151);
	gdk_color_set(g_vColors[Color_BoxBackgroundMissing], 49151, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundDisabled], 46767, 46767, 59151);
	gdk_color_set(g_vColors[Color_BoxBackgroundDeprecated], 65535, 50000, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundOutdated], 57343, 57343, 57343);
	gdk_color_set(g_vColors[Color_BoxBackgroundMetabox], 58343, 65535, 62343);
	gdk_color_set(g_vColors[Color_BoxBackgroundUnstable], 49151, 49151, 49151);
	gdk_color_set(g_vColors[Color_BoxBackgroundMensia], 65535, 65535, 65535);
	gdk_color_set(g_vColors[Color_BoxBackground], 65535, 65535, 65535);
	gdk_color_set(g_vColors[Color_BoxBorderSelected], 0, 0, 0);
	gdk_color_set(g_vColors[Color_BoxBorder], 0, 0, 0);
	gdk_color_set(g_vColors[Color_BoxBorderMensia], 10000, 45535, 35535);
	gdk_color_set(g_vColors[Color_BoxInputBackground], 65535, 49151, 32767);
	gdk_color_set(g_vColors[Color_BoxInputBorder], 16383, 16383, 16383);
	gdk_color_set(g_vColors[Color_BoxOutputBackground], 32767, 65535, 49151);
	gdk_color_set(g_vColors[Color_BoxOutputBorder], 16383, 16383, 16383);
	gdk_color_set(g_vColors[Color_BoxSettingBackground], 49151, 32767, 65535);
	gdk_color_set(g_vColors[Color_BoxSettingBorder], 16383, 16383, 16383);

	gdk_color_set(g_vColors[Color_CommentBackground], 65535, 65535, 57343);
	gdk_color_set(g_vColors[Color_CommentBackgroundSelected], 65535, 65535, 49151);
	gdk_color_set(g_vColors[Color_CommentBorder], 32767, 32767, 32767);
	gdk_color_set(g_vColors[Color_CommentBorderSelected], 32767, 32767, 32767);

	gdk_color_set(g_vColors[Color_Link], 0, 0, 0);
	gdk_color_set(g_vColors[Color_LinkSelected], 49151, 49151, 16383);
	gdk_color_set(g_vColors[Color_LinkUpCast], 32767, 16383, 16383);
	gdk_color_set(g_vColors[Color_LinkDownCast], 16383, 32767, 16383);
	gdk_color_set(g_vColors[Color_LinkInvalid], 49151, 16383, 16383);
	gdk_color_set(g_vColors[Color_SelectionArea], 0x3f00, 0x3f00, 0x3f00);
	gdk_color_set(g_vColors[Color_SelectionAreaBorder], 0, 0, 0);
#undef gdk_color_set
	//___________________________________________________________________//
	//                                                                   //

	SConfiguration l_oConfiguration;
	bool bArgParseResult = parse_arguments(argc, argv, l_oConfiguration);
	if (!bArgParseResult)
	{
		if (l_oConfiguration.m_help)
		{
			user_info(argv, nullptr);
			return 0;
		}
	}

	CKernelLoader l_oKernelLoader;

	cout << "[  INF  ] Created kernel loader, trying to load kernel module" << "\n";
	CString l_sError;
#if defined TARGET_OS_Windows
	CString l_sKernelFile = Directories::getLibDir() + "/openvibe-kernel.dll";
#elif defined TARGET_OS_Linux
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so";
#elif defined TARGET_OS_MacOS
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.dylib";
#endif
	if (!l_oKernelLoader.load(l_sKernelFile, &l_sError))
	{
		cout << "[ FAILED ] Error loading kernel (" << l_sError << ")" << " from [" << l_sKernelFile << "]\n";
	}
	else
	{
		cout << "[  INF  ] Kernel module loaded, trying to get kernel descriptor" << "\n";
		IKernelDesc* l_pKernelDesc       = nullptr;
		IKernelContext* l_pKernelContext = nullptr;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if (l_pKernelDesc == nullptr)
		{
			cout << "[ FAILED ] No kernel descriptor" << "\n";
		}
		else
		{
			cout << "[  INF  ] Got kernel descriptor, trying to create kernel" << "\n";

			l_pKernelContext = l_pKernelDesc->createKernel("designer", Directories::getDataDir() + "/kernel/openvibe.conf");
			l_pKernelContext->initialize();
			l_pKernelContext->getConfigurationManager().addConfigurationFromFile(Directories::getDataDir() + "/applications/designer/designer.conf");
			CString l_sAppConfigFile = l_pKernelContext->getConfigurationManager().expand("${Designer_CustomConfigurationFile}");
			l_pKernelContext->getConfigurationManager().addConfigurationFromFile(l_sAppConfigFile);
			// add other configuration file if --config option
			std::vector<std::pair<ECommandLineFlag, std::string>>::iterator it = l_oConfiguration.m_vFlag.begin();

			// initialize random number generator with nullptr by default
			System::Math::initializeRandomMachine(time(nullptr));

			while (it != l_oConfiguration.m_vFlag.end())
			{
				if (it->first == CommandLineFlag_Config)
				{
					l_sAppConfigFile = CString(it->second.c_str());
					l_pKernelContext->getConfigurationManager().addConfigurationFromFile(l_sAppConfigFile);
				}
				else if (it->first == CommandLineFlag_RandomSeed)
				{
					const int64_t l_i32Seed = strtol(it->second.c_str(), nullptr, 10);
					System::Math::initializeRandomMachine(static_cast<const uint32_t>(l_i32Seed));
				}
				++it;
			}


			if (l_pKernelContext == nullptr)
			{
				cout << "[ FAILED ] No kernel created by kernel descriptor" << "\n";
			}
			else
			{
				OpenViBEToolkit::initialize(*l_pKernelContext);
				OpenViBEVisualizationToolkit::initialize(*l_pKernelContext);

				//initialise Gtk before 3D context
				gtk_init(&argc, &argv);
				// gtk_rc_parse(OpenViBE::Directories::getDataDir() + "/applications/designer/interface.gtkrc");


#if defined OPENVIBE_SPLASHSCREEN
				GtkWidget* l_pSplashScreenWindow = gtk_window_new(GTK_WINDOW_POPUP);
				gtk_window_set_position(GTK_WINDOW(l_pSplashScreenWindow), GTK_WIN_POS_CENTER);
				gtk_window_set_type_hint(GTK_WINDOW(l_pSplashScreenWindow), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
				gtk_window_set_default_size(GTK_WINDOW(l_pSplashScreenWindow), 600, 400);
				GtkWidget* l_pSplashScreenImage = gtk_image_new_from_file(OpenViBE::Directories::getDataDir() + "/applications/designer/splashscreen.png");
				gtk_container_add(GTK_CONTAINER(l_pSplashScreenWindow), (l_pSplashScreenImage));
				gtk_widget_show(l_pSplashScreenImage);
				gtk_widget_show(l_pSplashScreenWindow);
				g_timeout_add(500, cb_remove_splashscreen, l_pSplashScreenWindow);

				while (gtk_events_pending())
				{
					gtk_main_iteration();
				}
#endif

				IConfigurationManager& l_rConfigurationManager = l_pKernelContext->getConfigurationManager();
				ILogManager& l_rLogManager                     = l_pKernelContext->getLogManager();

				bArgParseResult = parse_arguments(argc, argv, l_oConfiguration);

				l_pKernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));

				//FIXME : set locale only when needed
				CString l_sLocale = l_rConfigurationManager.expand("${Designer_Locale}");
				if (l_sLocale == CString(""))
				{
					l_sLocale = "C";
				}
				setlocale(LC_ALL, l_sLocale.toASCIIString());

				if (!(bArgParseResult || l_oConfiguration.m_help))
				{
					user_info(argv, &l_rLogManager);
				}
				else
				{
					if ((!l_rConfigurationManager.expandAsBoolean("${Kernel_WithGUI}", true)) && ((l_oConfiguration.getFlags() & CommandLineFlag_NoGui) == 0))
					{
						l_rLogManager << LogLevel_ImportantWarning << "${Kernel_WithGUI} is set to false and --no-gui flag not set. Forcing the --no-gui flag\n";
						l_oConfiguration.m_eNoGui             = CommandLineFlag_NoGui;
						l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
						l_oConfiguration.m_eNoManageSession   = CommandLineFlag_NoManageSession;
					}

					if (l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui && !ensureOneInstanceOfDesigner(l_oConfiguration, l_rLogManager))
					{
						l_rLogManager << LogLevel_Trace << "An instance of Designer is already running.\n";
						return 0;
					}

					{
						CApplication app(*l_pKernelContext);
						app.initialize(l_oConfiguration.getFlags());

						// FIXME is it necessary to keep next line uncomment ?
						//bool l_bIsScreenValid=true;
						if (l_oConfiguration.m_eNoCheckColorDepth == 0)
						{
							if (GDK_IS_DRAWABLE(GTK_WIDGET(app.m_pMainWindow)->window))
							{
								// FIXME is it necessary to keep next line uncomment ?
								//l_bIsScreenValid=false;
								switch (gdk_drawable_get_depth(GTK_WIDGET(app.m_pMainWindow)->window))
								{
									case 24:
									case 32:
										// FIXME is it necessary to keep next line uncomment ?
										//l_bIsScreenValid=true;
										break;
									default:
										l_rLogManager << LogLevel_Error << "Please change the color depth of your screen to either 24 or 32 bits\n";
										// TODO find a way to break
										break;
								}
							}
						}

						// Add or replace a configuration token if required in command line
						for (const auto& token : l_oConfiguration.m_oTokenMap)
						{
							l_rLogManager << LogLevel_Trace << "Adding command line configuration token [" << token.first.c_str() << " = " << token.second.c_str() << "]\n";
							l_rConfigurationManager.addOrReplaceConfigurationToken(token.first.c_str(), token.second.c_str());
						}

						for (size_t i = 0; i < l_oConfiguration.m_vFlag.size(); ++i)
						{
							std::string fileName = l_oConfiguration.m_vFlag[i].second;
							std::transform(fileName.begin(), fileName.end(), fileName.begin(), backslash_to_slash);
							bool error;
							switch (l_oConfiguration.m_vFlag[i].first)
							{
								case CommandLineFlag_Open:
									l_rLogManager << LogLevel_Info << "Opening scenario [" << CString(fileName.c_str()) << "]\n";
									if (!app.openScenario(fileName.c_str()))
									{
										l_rLogManager << LogLevel_Error << "Could not open scenario " << fileName.c_str() << "\n";
										errorWhileLoadingScenario = l_oConfiguration.m_eNoGui == CommandLineFlag_NoGui;
									}
									break;
								case CommandLineFlag_Play:
									l_rLogManager << LogLevel_Info << "Opening and playing scenario [" << CString(fileName.c_str()) << "]\n";
									error = !app.openScenario(fileName.c_str());
									if (!error)
									{
										app.playScenarioCB();
										error = app.getCurrentInterfacedScenario()->m_ePlayerStatus != PlayerStatus_Play;
									}
									if (error)
									{
										l_rLogManager << LogLevel_Error << "Scenario open or load error with --play.\n";
										errorWhileLoadingScenario = l_oConfiguration.m_eNoGui == CommandLineFlag_NoGui;
									}
									break;
								case CommandLineFlag_PlayFast:
									l_rLogManager << LogLevel_Info << "Opening and fast playing scenario [" << CString(fileName.c_str()) << "]\n";
									error = !app.openScenario(fileName.c_str());
									if (!error)
									{
										app.forwardScenarioCB();
										error = app.getCurrentInterfacedScenario()->m_ePlayerStatus != PlayerStatus_Forward;
									}
									if (error)
									{
										l_rLogManager << LogLevel_Error << "Scenario open or load error with --play-fast.\n";
										errorWhileLoadingScenario = l_oConfiguration.m_eNoGui == CommandLineFlag_NoGui;
									}
									playRequested = true;
									break;
									//								case CommandLineFlag_Define:
									//									break;
								default:
									break;
							}
						}

						if (!playRequested && l_oConfiguration.m_eNoGui == CommandLineFlag_NoGui)
						{
							l_rLogManager << LogLevel_Info << "Switch --no-gui is enabled but no play operation was requested. Designer will exit automatically.\n";
						}

						if (app.m_vInterfacedScenario.empty() && l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui)
						{
							app.newScenarioCB();
						}

						if (!app.m_vInterfacedScenario.empty())
						{
							CPluginObjectDescCollector cb_collector1(*l_pKernelContext);
							CPluginObjectDescCollector cb_collector2(*l_pKernelContext);
							CPluginObjectDescLogger cb_logger(*l_pKernelContext);
							cb_logger.enumeratePluginObjectDesc();
							cb_collector1.enumeratePluginObjectDesc(OV_ClassId_Plugins_BoxAlgorithmDesc);
							cb_collector2.enumeratePluginObjectDesc(OV_ClassId_Plugins_AlgorithmDesc);
							insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, cb_collector1.getPluginObjectDescMap(), app.m_pBoxAlgorithmTreeModel, app.m_vNewBoxes, app.m_vUpdatedBoxes, app.m_bIsNewVersion);
							insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, cb_collector2.getPluginObjectDescMap(), app.m_pAlgorithmTreeModel, app.m_vNewBoxes, app.m_vUpdatedBoxes);

							std::map<std::string, const IPluginObjectDesc*> metaboxDescMap;
							CIdentifier identifier;
							while ((identifier = l_pKernelContext->getMetaboxManager().getNextMetaboxObjectDescIdentifier(identifier)) != OV_UndefinedIdentifier)
							{
								metaboxDescMap[std::string(identifier.toString())] = l_pKernelContext->getMetaboxManager().getMetaboxObjectDesc(identifier);
							}
							insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, metaboxDescMap, app.m_pBoxAlgorithmTreeModel, app.m_vNewBoxes, app.m_vUpdatedBoxes, app.m_bIsNewVersion);

							l_pKernelContext->getLogManager() << LogLevel_Info << "Initialization took " << l_pKernelContext->getConfigurationManager().expand("$Core{real-time}") << " ms\n";
							// If the application is a newly launched version, and not launched without GUI -> display changelog
							if (app.m_bIsNewVersion && l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui)
							{
								app.displayChangelogWhenAvailable();
							}
							try
							{
								gtk_main();
							}
							catch (DesignerException& ex)
							{
								std::cerr << "Caught designer exception" << std::endl;
								GtkWidget* errorDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", ex.getErrorString().c_str());
								gtk_window_set_title(GTK_WINDOW(errorDialog), (std::string(BRAND_NAME) + " has stopped functioning").c_str());
								gtk_dialog_run(GTK_DIALOG(errorDialog));
							}
							catch (...)
							{
								std::cerr << "Caught top level exception" << std::endl;
							}
						}
					}
				}

				l_rLogManager << LogLevel_Info << "Application terminated, releasing allocated objects\n";

				OpenViBEVisualizationToolkit::uninitialize(*l_pKernelContext);
				OpenViBEToolkit::uninitialize(*l_pKernelContext);

				l_pKernelDesc->releaseKernel(l_pKernelContext);

				// Remove the mutex only if the application was run with a gui
				if (l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui)
				{
					boost::interprocess::named_mutex::remove(MUTEX_NAME);
				}
			}
		}
		l_oKernelLoader.uninitialize();
		l_oKernelLoader.unload();
	}
	return errorWhileLoadingScenario ? -1 : 0;
}

int main(int argc, char** argv)
{
	// Remove mutex at startup, as the main loop regenerates frequently this mutex,
	// if another instance is running, it should have the time to regenerate it
	// Avoids that after crashing, a mutex stays blocking
	boost::interprocess::named_mutex::remove(MUTEX_NAME);
	int ret = -1;
	try{ ret = go(argc, argv); }
	catch (...) { std::cout << "Caught an exception at the very top...\nLeaving application!\n"; }
	//return go(argc, argv);
}

#if defined TARGET_OS_Windows
// Should be used once we get rid of the .cmd launchers
int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*windowStyle*/) { return main(__argc, __argv); }
#endif //defined TARGET_OS_Windows
