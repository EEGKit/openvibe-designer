#include <stack>
#include <vector>
#include <map>
#include <tuple>
#include <string>
#include <iostream>
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
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/Plugins;
using namespace /*OpenViBE::*/Designer;
using namespace std;

map<size_t, GdkColor> gColors;

class CPluginObjectDescEnum
{
public:

	explicit CPluginObjectDescEnum(const IKernelContext& ctx) : m_kernelCtx(ctx) { }
	virtual ~CPluginObjectDescEnum() = default;

	virtual bool enumeratePluginObjectDesc()
	{
		CIdentifier id;
		while ((id = m_kernelCtx.getPluginManager().getNextPluginObjectDescIdentifier(id)) != OV_UndefinedIdentifier)
		{
			this->callback(*m_kernelCtx.getPluginManager().getPluginObjectDesc(id));
		}
		return true;
	}

	virtual bool enumeratePluginObjectDesc(const CIdentifier& parentClassID)
	{
		CIdentifier id;
		while ((id = m_kernelCtx.getPluginManager().getNextPluginObjectDescIdentifier(id, parentClassID)) != OV_UndefinedIdentifier)
		{
			this->callback(*m_kernelCtx.getPluginManager().getPluginObjectDesc(id));
		}
		return true;
	}

	virtual bool callback(const IPluginObjectDesc& pod) = 0;

protected:

	const IKernelContext& m_kernelCtx;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescCollector final : public CPluginObjectDescEnum
{
public:

	explicit CPluginObjectDescCollector(const IKernelContext& ctx) : CPluginObjectDescEnum(ctx) { }

	bool callback(const IPluginObjectDesc& pod) override
	{
		const string name = string(pod.getCategory()) + "/" + string(pod.getName());
		const auto it     = m_pods.find(name);
		if (it != m_pods.end())
		{
			m_kernelCtx.getLogManager() << LogLevel_ImportantWarning << "Duplicate plugin object name " << name << " "
					<< it->second->getCreatedClass() << " and " << pod.getCreatedClass() << "\n";
		}
		m_pods[name] = &pod;
		return true;
	}

	map<string, const IPluginObjectDesc*>& getPluginObjectDescMap() { return m_pods; }

private:

	map<string, const IPluginObjectDesc*> m_pods;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescLogger final : public CPluginObjectDescEnum
{
public:

	explicit CPluginObjectDescLogger(const IKernelContext& ctx) : CPluginObjectDescEnum(ctx) { }

	bool callback(const IPluginObjectDesc& pod) override
	{
		// Outputs plugin info to console
		m_kernelCtx.getLogManager() << LogLevel_Trace << "Plugin <" << pod.getName() << ">\n";
		m_kernelCtx.getLogManager() << LogLevel_Debug << " | Plugin category        : " << pod.getCategory() << "\n";
		m_kernelCtx.getLogManager() << LogLevel_Debug << " | Class identifier       : " << pod.getCreatedClass() << "\n";
		m_kernelCtx.getLogManager() << LogLevel_Debug << " | Author name            : " << pod.getAuthorName() << "\n";
		m_kernelCtx.getLogManager() << LogLevel_Debug << " | Author company name    : " << pod.getAuthorCompanyName() << "\n";
		m_kernelCtx.getLogManager() << LogLevel_Debug << " | Short description      : " << pod.getShortDescription() << "\n";
		m_kernelCtx.getLogManager() << LogLevel_Debug << " | Detailed description   : " << pod.getDetailedDescription() << "\n";

		return true;
	}
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

namespace
{
	typedef std::map<std::string, std::tuple<int, int, int>> components_map_t;
	// Parses a JSON encoded list of components with their versions
	// We use an output variable because we want to be able to "enhance" an already existing list if necessary
	void getVersionComponentsFromConfigToken(const IKernelContext& ctx, const char* configToken, components_map_t& componentVersions)
	{
		json::Object versionsObject;
		// We use a lookup instead of expansion as JSON can contain { } characters

		const CString versionsJSON = ctx.getConfigurationManager().expand(CString("${") + configToken + "}");
		if (versionsJSON.length() != 0)
		{
			// This check is necessary because the asignemt operator would fail with an assert
			if (json::Deserialize(versionsJSON.toASCIIString()).GetType() == json::ObjectVal)
			{
				versionsObject = json::Deserialize(versionsJSON.toASCIIString());
			}
			for (const auto& component : versionsObject)
			{
				int versionMajor, versionMinor, versionPatch;
				sscanf(component.second, "%d.%d.%d", &versionMajor, &versionMinor, &versionPatch);
				componentVersions[component.first] = std::make_tuple(versionMajor, versionMinor, versionPatch);
			}
		}
	}
} // namespace

static void InsertPluginObjectDescToGtkTreeStore(const IKernelContext& ctx, map<string, const IPluginObjectDesc*>& pods, GtkTreeStore* treeStore,
												 std::vector<const IPluginObjectDesc*>& newBoxes, std::vector<const IPluginObjectDesc*>& updatedBoxes,
												 bool isNewVersion = false)
{
	typedef std::map<std::string, std::tuple<int, int, int>> components_map_t;
	components_map_t currentVersions;
	getVersionComponentsFromConfigToken(ctx, "ProjectVersion_Components", currentVersions);
	// By default, fix version to current version - to display the new/update boxes available since current version only
	components_map_t lastUsedVersions = currentVersions;
	getVersionComponentsFromConfigToken(ctx, "Designer_LastComponentVersionsUsed", lastUsedVersions);

	for (const auto& pod : pods)
	{
		const IPluginObjectDesc* p = pod.second;

		CString stockItemName;

		const auto* desc = dynamic_cast<const IBoxAlgorithmDesc*>(p);
		if (desc != nullptr) { stockItemName = desc->getStockItemName(); }

		bool shouldShow = true;

		if (ctx.getPluginManager().isPluginObjectFlaggedAsDeprecated(p->getCreatedClass())
			&& !ctx.getConfigurationManager().expandAsBoolean("${Designer_ShowDeprecated}", false)) { shouldShow = false; }

		/*
		if  (ctx.getPluginManager().isPluginObjectFlaggedAsUnstable(desc->getCreatedClass()) 
		&& !ctx.getConfigurationManager().expandAsBoolean("${Designer_ShowUnstable}", false)) { shouldShow = false; }
		*/

		if (shouldShow)
		{
			GtkStockItem stockItem;
			if (gtk_stock_lookup(stockItemName, &stockItem) == 0) { stockItemName = GTK_STOCK_NEW; }

			// Splits the plugin category
			vector<string> categories;
			string str  = string(p->getCategory());
			size_t j, i = size_t(-1);
			while ((j = str.find('/', i + 1)) != string::npos)
			{
				string subCategory = string(str, i + 1, j - i - 1);
				if (subCategory != string("")) { categories.push_back(subCategory); }
				i = j;
			}
			if (i + 1 != str.length()) { categories.emplace_back(str, i + 1, str.length() - i - 1); }

			// Fills plugin in the tree
			GtkTreeIter iter1;
			GtkTreeIter iter2;
			GtkTreeIter* iterParent = nullptr;
			GtkTreeIter* iterChild  = &iter1;
			for (const string& category : categories)
			{
				bool found = false;
				bool valid = gtk_tree_model_iter_children(GTK_TREE_MODEL(treeStore), iterChild, iterParent) != 0;
				while (valid && !found)
				{
					gchar* name = nullptr;
					gboolean isPlugin;
					gtk_tree_model_get(GTK_TREE_MODEL(treeStore), iterChild, Resource_StringName, &name, Resource_BooleanIsPlugin, &isPlugin, -1);
					if ((isPlugin == 0) && name == category) { found = true; }
					else { valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(treeStore), iterChild) != 0; }
				}
				if (!found)
				{
					gtk_tree_store_append(GTK_TREE_STORE(treeStore), iterChild, iterParent);
					gtk_tree_store_set(GTK_TREE_STORE(treeStore), iterChild, Resource_StringName, category.c_str(),
									   Resource_StringShortDescription, "", Resource_StringStockIcon, "gtk-directory", Resource_StringColor, "#000000",
									   Resource_StringFont, "", Resource_BooleanIsPlugin, gboolean(FALSE), -1);
				}
				if (iterParent == nullptr) { iterParent = &iter2; }
				GtkTreeIter* iterSwap = iterChild;
				iterChild             = iterParent;
				iterParent            = iterSwap;
			}
			gtk_tree_store_append(GTK_TREE_STORE(treeStore), iterChild, iterParent);

			// define color of the text of the box
			std::string textColor = "black";
			std::string bgColor   = "white";
			std::string textFont;
			str = p->getName().toASCIIString();

			if (ctx.getPluginManager().isPluginObjectFlaggedAsDeprecated(p->getCreatedClass())) { textColor = "#3f7f7f"; }

			// If the software is launched for the first time after update, highlight new/updated boxes in tree-view
			std::string boxSoftwareComponent = p->getSoftwareComponent().toASCIIString();

			if (boxSoftwareComponent != "unknown")
			{
				int currentVMajor  = std::get<0>(currentVersions[boxSoftwareComponent]);
				int currentVMinor  = std::get<1>(currentVersions[boxSoftwareComponent]);
				int currentVPatch  = std::get<2>(currentVersions[boxSoftwareComponent]);
				int lastUsedVMajor = std::get<0>(lastUsedVersions[boxSoftwareComponent]);
				int lastUsedVMinor = std::get<1>(lastUsedVersions[boxSoftwareComponent]);
				int lastUsedVPatch = std::get<2>(lastUsedVersions[boxSoftwareComponent]);
				int boxCompoVMajor = 0;
				int boxCompoVMinor = 0;
				int boxCompoVPatch = 0;

				sscanf(p->getAddedSoftwareVersion().toASCIIString(), "%d.%d.%d", &boxCompoVMajor, &boxCompoVMinor, &boxCompoVPatch);
				// If this is a new version, then add in list all the updated/new boxes since last version opened
				if (isNewVersion
					&& ((lastUsedVMajor < boxCompoVMajor && boxCompoVMajor <= currentVMajor)
						|| (boxCompoVMajor == currentVMajor && lastUsedVMinor < boxCompoVMinor && boxCompoVMinor <= currentVMinor)
						|| (boxCompoVMinor == currentVMinor && lastUsedVPatch < boxCompoVPatch && boxCompoVPatch <= currentVPatch)
						// As default value for lastUsedVMinor and lastUsedVMajor are the current software version
						|| (boxCompoVMajor == currentVMajor && boxCompoVMinor == currentVMinor && boxCompoVPatch == currentVPatch)))
				{
					str += " (New)";
					bgColor = "#FFFFC4";
					newBoxes.push_back(p);
				}
					// Otherwise
				else if (boxCompoVMajor == currentVMajor && boxCompoVMinor == currentVMinor && boxCompoVPatch ==
						 currentVPatch) { newBoxes.push_back(p); }
				else
				{
					int boxCompoUpdatedVMajor = 0;
					int boxCompoUpdatedVMinor = 0;
					int boxCompoUpdatedVPatch = 0;
					sscanf(p->getUpdatedSoftwareVersion().toASCIIString(), "%d.%d.%d", &boxCompoUpdatedVMajor,
						   &boxCompoUpdatedVMinor, &boxCompoUpdatedVPatch);
					// If this is a new version, then add in list all the updated/new boxes since last version opened
					if (isNewVersion
						&& ((lastUsedVMajor < boxCompoUpdatedVMajor && boxCompoUpdatedVMajor <= currentVMajor)
							|| (boxCompoUpdatedVMajor == currentVMajor && lastUsedVMinor < boxCompoUpdatedVMinor && boxCompoUpdatedVMinor <= currentVMinor)
							|| (boxCompoUpdatedVMinor == currentVMinor && lastUsedVPatch < boxCompoUpdatedVPatch && boxCompoUpdatedVPatch <= currentVPatch)
							// If this is a new version Designer, and last version opened was set to default value i.e. version of current software
							|| (boxCompoUpdatedVMajor == currentVMajor && boxCompoUpdatedVMinor == currentVMinor && boxCompoUpdatedVPatch == currentVPatch)))
					{
						str += " (New)";
						bgColor = "#FFFFC4";
						updatedBoxes.push_back(p);
					}
						// Otherwise
					else if (!isNewVersion && (boxCompoUpdatedVMajor == currentVMajor && boxCompoUpdatedVMinor == currentVMinor && boxCompoUpdatedVPatch ==
											   currentVPatch)) { updatedBoxes.push_back(p); }
				}
			}

			// Construct a string containing the BoxAlgorithmIdentifier concatenated with a metabox identifier if necessary
			std::string boxAlgorithmDesc = p->getCreatedClass().str();

			if (p->getCreatedClass() == OVP_ClassId_BoxAlgorithm_Metabox)
			{
				boxAlgorithmDesc += dynamic_cast<const Metabox::IMetaboxObjectDesc*>(p)->getMetaboxDescriptor();
				textColor = "#007020";
			}

			gtk_tree_store_set(GTK_TREE_STORE(treeStore), iterChild, Resource_StringName, str.c_str(),
							   Resource_StringShortDescription, static_cast<const char*>(p->getShortDescription()),
							   Resource_StringIdentifier, static_cast<const char*>(boxAlgorithmDesc.c_str()),
							   Resource_StringStockIcon, static_cast<const char*>(stockItemName), Resource_StringColor, textColor.c_str(),
							   Resource_StringFont, textFont.c_str(), Resource_BooleanIsPlugin, gboolean(TRUE),
							   Resource_BackGroundColor, static_cast<const char*>(bgColor.c_str()), -1);
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

typedef struct SConfig
{
	SConfig() = default;

	ECommandLineFlag getFlags() const { return ECommandLineFlag(noGui | noCheckColorDepth | noManageSession | noVisualization | define | randomSeed | config); }

	std::vector<std::pair<ECommandLineFlag, std::string>> flags;
	ECommandLineFlag noGui             = CommandLineFlag_None;
	ECommandLineFlag noCheckColorDepth = CommandLineFlag_None;
	ECommandLineFlag noManageSession   = CommandLineFlag_None;
	ECommandLineFlag noVisualization   = CommandLineFlag_None;
	ECommandLineFlag define            = CommandLineFlag_None;
	ECommandLineFlag randomSeed        = CommandLineFlag_None;
	ECommandLineFlag config            = CommandLineFlag_None;
	bool help                          = false;
	// to resolve warning: padding struct 'SConfig' with 4 bytes to align 'tokens
	int structPadding = 0;
	std::map<std::string, std::string> tokens;
} config_t;

static char backslash_to_slash(const char c) { return c == '\\' ? '/' : c; }

/** ------------------------------------------------------------------------------------------------------------------------------------
* Use Mutex to ensure that only one instance with GUI of Designer runs at the same time
* if another instance exists, sends a message to it so that it opens a scenario or get the focus back
* \param config: play, play-fast or open
* \param logMgr: name of the scenario to open
------------------------------------------------------------------------------------------------------------------------------------**/
#if defined NDEBUG
static bool ensureOneInstanceOfDesigner(config_t& config, ILogManager& logMgr)
{
	try
	{
		// If the mutex cannot be opened, it's the first instance of Designer, go to catch
		boost::interprocess::named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);

		// If the mutex was opened, then an instance of designer is already running, we send it a message before dying
		// The message contains the command to send: sMode: open, play, play-fast a scenario, sScenarioPath: path of the scenario
		boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex);
		std::string msg;
		if (config.flags.empty()) { msg = std::to_string(int(CommandLineFlag_None)) + ": ;"; }

		for (auto& flag : config.flags)
		{
			std::string fileName = flag.second;
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), backslash_to_slash);

			msg += std::to_string(int(flag.first)) + ": <" + fileName + "> ; ";
		}

		const size_t msgSize = strlen(msg.c_str()) * sizeof(char);

		boost::interprocess::message_queue messageToFirstInstance(boost::interprocess::open_or_create, MESSAGE_NAME, msgSize, msgSize);
		messageToFirstInstance.send(msg.c_str(), msgSize, 0);

		return false;
	}
	catch (boost::interprocess::interprocess_exception&)
	{
		//Create the named mutex to catch the potential next instance of Designer that could open
		boost::interprocess::named_mutex mutex(boost::interprocess::create_only, MUTEX_NAME);
		return true;
	}
}
#else
static bool ensureOneInstanceOfDesigner(config_t& /*config*/, ILogManager& /*logMgr*/) { return true; }
#endif

bool parse_arguments(int argc, char** argv, config_t& config)
{
	config_t tmp;

	std::vector<std::string> args;
#if defined TARGET_OS_Windows
	int nArg;
	LPWSTR* argListUtf16 = CommandLineToArgvW(GetCommandLineW(), &nArg);
	for (int i = 1; i < nArg; ++i)
	{
		GError* error = nullptr;
		glong itemsRead, itemsWritten;
		char* argUtf8 = g_utf16_to_utf8(reinterpret_cast<gunichar2*>(argListUtf16[i]), glong(wcslen(argListUtf16[i])), &itemsRead, &itemsWritten, &error);
		args.emplace_back(argUtf8);
		if (error != nullptr)
		{
			g_error_free(error);
			return false;
		}
	}
#else
	args = std::vector<std::string>(argv + 1, argv + argc);
#endif
	args.emplace_back("");

	for (auto it = args.cbegin(); it != args.cend(); ++it)
	{
		if (*it == "") {}
		else if (*it == "-h" || *it == "--help")
		{
			tmp.help = true;
			config   = tmp;
			return false;
		}
		else if (*it == "-o" || *it == "--open") { tmp.flags.emplace_back(CommandLineFlag_Open, *++it); }
		else if (*it == "-p" || *it == "--play") { tmp.flags.emplace_back(CommandLineFlag_Play, *++it); }
		else if (*it == "-pf" || *it == "--play-fast") { tmp.flags.emplace_back(CommandLineFlag_PlayFast, *++it); }
		else if (*it == "--no-gui")
		{
			tmp.noGui             = CommandLineFlag_NoGui;
			tmp.noCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
			tmp.noManageSession   = CommandLineFlag_NoManageSession;
		}
		else if (*it == "--no-visualization") { tmp.noVisualization = CommandLineFlag_NoVisualization; }
		else if (*it == "--invisible")
		{
			// no-gui + no-visualization
			tmp.noVisualization   = CommandLineFlag_NoVisualization;
			tmp.noGui             = CommandLineFlag_NoGui;
			tmp.noCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
			tmp.noManageSession   = CommandLineFlag_NoManageSession;
		}
		else if (*it == "--no-check-color-depth") { tmp.noCheckColorDepth = CommandLineFlag_NoCheckColorDepth; }
		else if (*it == "--no-session-management") { tmp.noManageSession = CommandLineFlag_NoManageSession; }
		else if (*it == "-c" || *it == "--config")
		{
			if (*++it == "")
			{
				std::cout << "Error: Switch --config needs an argument\n";
				return false;
			}
			tmp.flags.emplace_back(CommandLineFlag_Config, *it);
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
			const std::string& token = *it;
			if (*++it == "")
			{
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			const std::string& value = *it;	// iterator will increment later

			tmp.tokens[token] = value;
		}
		else if (*it == "--random-seed")
		{
			if (*++it == "")
			{
				std::cout << "Error: Switch --random-seed needs an argument\n";
				return false;
			}
			tmp.flags.emplace_back(CommandLineFlag_RandomSeed, *it);
		}
		else if (*it == "--g-fatal-warnings")
		{
			// Do nothing here but accept this gtk flag
		}
		else
		{
#if 0
			// Assumes we just open a scenario - this is for retro compatibility and should not be supported in the future
			config.flags.push_back(std::make_pair(CommandLineFlag_Open, *++it));
#endif
			return false;
		}
	}

#if 0
	config.flags = config.flags;
	config.checkColorDepth = config.m_bCheckColorDepth;
	config.showGui = config.m_bShowGui;
#else
	config = tmp;
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

void message(const char* title, const char* msg, const GtkMessageType type)
{
	GtkWidget* dialog = gtk_message_dialog_new(nullptr, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), type, GTK_BUTTONS_OK, "%s", title);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg);
	::gtk_window_set_icon_from_file(GTK_WINDOW(dialog), (Directories::getDataDir() + CString("/applications/designer/designer.ico")).toASCIIString(),
									nullptr);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void user_info(char** argv, ILogManager* logManager)
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

	if (logManager != nullptr) { for (const auto& m : messages) { (*logManager) << LogLevel_Info << m; } }
	else { for (const auto& m : messages) { cout << m; } }
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
#define GDK_COLOR_SET(c, r, g, b) { (c).pixel=0; (c).red=r; (c).green=g; (c).blue=b; }
	GDK_COLOR_SET(gColors[Color_BackgroundPlayerStarted], 32767, 32767, 32767);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundSelected], 65535, 65535, 49151);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundMissing], 49151, 32767, 32767);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundDisabled], 46767, 46767, 59151);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundDeprecated], 65535, 50000, 32767);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundOutdated], 57343, 57343, 57343);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundMetabox], 58343, 65535, 62343);
	GDK_COLOR_SET(gColors[Color_BoxBackgroundUnstable], 49151, 49151, 49151);
	GDK_COLOR_SET(gColors[Color_BoxBackground], 65535, 65535, 65535);
	GDK_COLOR_SET(gColors[Color_BoxBorderSelected], 0, 0, 0);
	GDK_COLOR_SET(gColors[Color_BoxBorder], 0, 0, 0);
	GDK_COLOR_SET(gColors[Color_BoxInputBackground], 65535, 49151, 32767);
	GDK_COLOR_SET(gColors[Color_BoxInputBorder], 16383, 16383, 16383);
	GDK_COLOR_SET(gColors[Color_BoxOutputBackground], 32767, 65535, 49151);
	GDK_COLOR_SET(gColors[Color_BoxOutputBorder], 16383, 16383, 16383);
	GDK_COLOR_SET(gColors[Color_BoxSettingBackground], 49151, 32767, 65535);
	GDK_COLOR_SET(gColors[Color_BoxSettingBorder], 16383, 16383, 16383);

	GDK_COLOR_SET(gColors[Color_CommentBackground], 65535, 65535, 57343);
	GDK_COLOR_SET(gColors[Color_CommentBackgroundSelected], 65535, 65535, 49151);
	GDK_COLOR_SET(gColors[Color_CommentBorder], 32767, 32767, 32767);
	GDK_COLOR_SET(gColors[Color_CommentBorderSelected], 32767, 32767, 32767);

	GDK_COLOR_SET(gColors[Color_Link], 0, 0, 0);
	GDK_COLOR_SET(gColors[Color_LinkSelected], 49151, 49151, 16383);
	GDK_COLOR_SET(gColors[Color_LinkUpCast], 32767, 16383, 16383);
	GDK_COLOR_SET(gColors[Color_LinkDownCast], 16383, 32767, 16383);
	GDK_COLOR_SET(gColors[Color_LinkInvalid], 49151, 16383, 16383);
	GDK_COLOR_SET(gColors[Color_SelectionArea], 0x3f00, 0x3f00, 0x3f00);
	GDK_COLOR_SET(gColors[Color_SelectionAreaBorder], 0, 0, 0);
#undef GDK_COLOR_SET
	//___________________________________________________________________//
	//                                                                   //

	config_t config;
	bool bArgParseResult = parse_arguments(argc, argv, config);
	if (!bArgParseResult)
	{
		if (config.help)
		{
			user_info(argv, nullptr);
			return 0;
		}
	}

	CKernelLoader loader;

	cout << "[  INF  ] Created kernel loader, trying to load kernel module" << "\n";
	CString errorMsg;
#if defined TARGET_OS_Windows
	CString file = Directories::getLibDir() + "/openvibe-kernel.dll";
#elif defined TARGET_OS_Linux
	CString file = Directories::getLibDir() + "/libopenvibe-kernel.so";
#elif defined TARGET_OS_MacOS
	CString file = Directories::getLibDir() + "/libopenvibe-kernel.dylib";
#endif
	if (!loader.load(file, &errorMsg)) { cout << "[ FAILED ] Error loading kernel (" << errorMsg << ")" << " from [" << file << "]\n"; }
	else
	{
		cout << "[  INF  ] Kernel module loaded, trying to get kernel descriptor" << "\n";
		IKernelDesc* desc       = nullptr;
		IKernelContext* context = nullptr;
		loader.initialize();
		loader.getKernelDesc(desc);
		if (desc == nullptr) { cout << "[ FAILED ] No kernel descriptor" << "\n"; }
		else
		{
			cout << "[  INF  ] Got kernel descriptor, trying to create kernel" << "\n";

			context = desc->createKernel("designer", Directories::getDataDir() + "/kernel/openvibe.conf");
			context->initialize();
			context->getConfigurationManager().addConfigurationFromFile(Directories::getDataDir() + "/applications/designer/designer.conf");
			CString appConfigFile = context->getConfigurationManager().expand("${Designer_CustomConfigurationFile}");
			context->getConfigurationManager().addConfigurationFromFile(appConfigFile);
			// add other configuration file if --config option
			auto it = config.flags.begin();

			// initialize random number generator with nullptr by default
			System::Math::initializeRandomMachine(time(nullptr));

			while (it != config.flags.end())
			{
				if (it->first == CommandLineFlag_Config)
				{
					appConfigFile = CString(it->second.c_str());
					context->getConfigurationManager().addConfigurationFromFile(appConfigFile);
				}
				else if (it->first == CommandLineFlag_RandomSeed)
				{
					const size_t seed = size_t(strtol(it->second.c_str(), nullptr, 10));
					System::Math::initializeRandomMachine(seed);
				}
				++it;
			}


			if (context == nullptr) { cout << "[ FAILED ] No kernel created by kernel descriptor" << "\n"; }
			else
			{
				Toolkit::initialize(*context);
				VisualizationToolkit::initialize(*context);

				//initialise Gtk before 3D context
				gtk_init(&argc, &argv);
				// gtk_rc_parse(Directories::getDataDir() + "/applications/designer/interface.gtkrc");


#if defined OPENVIBE_SPLASHSCREEN
				GtkWidget* splashScreenWindow = gtk_window_new(GTK_WINDOW_POPUP);
				gtk_window_set_position(GTK_WINDOW(splashScreenWindow), GTK_WIN_POS_CENTER);
				gtk_window_set_type_hint(GTK_WINDOW(splashScreenWindow), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
				gtk_window_set_default_size(GTK_WINDOW(splashScreenWindow), 600, 400);
				GtkWidget* splashScreenImage = gtk_image_new_from_file(Directories::getDataDir() + "/applications/designer/splashscreen.png");
				gtk_container_add(GTK_CONTAINER(splashScreenWindow), (splashScreenImage));
				gtk_widget_show(splashScreenImage);
				gtk_widget_show(splashScreenWindow);
				g_timeout_add(500, cb_remove_splashscreen, splashScreenWindow);

				while (gtk_events_pending()) { gtk_main_iteration(); }
#endif

				IConfigurationManager& configMgr = context->getConfigurationManager();
				ILogManager& logMgr              = context->getLogManager();

				bArgParseResult = parse_arguments(argc, argv, config);

				context->getPluginManager().addPluginsFromFiles(configMgr.expand("${Kernel_Plugins}"));

				//FIXME : set locale only when needed
				CString locale = configMgr.expand("${Designer_Locale}");
				if (locale == CString("")) { locale = "C"; }
				setlocale(LC_ALL, locale.toASCIIString());

				if (!(bArgParseResult || config.help)) { user_info(argv, &logMgr); }
				else
				{
					if ((!configMgr.expandAsBoolean("${Kernel_WithGUI}", true)) && ((config.getFlags() & CommandLineFlag_NoGui) == 0))
					{
						logMgr << LogLevel_ImportantWarning << "${Kernel_WithGUI} is set to false and --no-gui flag not set. Forcing the --no-gui flag\n";
						config.noGui             = CommandLineFlag_NoGui;
						config.noCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
						config.noManageSession   = CommandLineFlag_NoManageSession;
					}

					if (config.noGui != CommandLineFlag_NoGui && !ensureOneInstanceOfDesigner(config, logMgr))
					{
						logMgr << LogLevel_Trace << "An instance of Designer is already running.\n";
						return 0;
					}

					{
						CApplication app(*context);
						app.initialize(config.getFlags());

						// FIXME is it necessary to keep next line uncomment ?
						//bool isScreenValid=true;
						if (config.noCheckColorDepth == 0)
						{
							if (GDK_IS_DRAWABLE(GTK_WIDGET(app.m_MainWindow)->window))
							{
								// FIXME is it necessary to keep next line uncomment ?
								//isScreenValid=false;
								switch (gdk_drawable_get_depth(GTK_WIDGET(app.m_MainWindow)->window))
								{
									case 24:
									case 32:
										// FIXME is it necessary to keep next line uncomment ?
										//isScreenValid=true;
										break;
									default:
										logMgr << LogLevel_Error << "Please change the color depth of your screen to either 24 or 32 bits\n";
										// TODO find a way to break
										break;
								}
							}
						}

						// Add or replace a configuration token if required in command line
						for (const auto& t : config.tokens)
						{
							logMgr << LogLevel_Trace << "Adding command line configuration token [" << t.first << " = " << t.second << "]\n";
							configMgr.addOrReplaceConfigurationToken(t.first.c_str(), t.second.c_str());
						}

						for (const auto& f : config.flags)
						{
							std::string fileName = f.second;
							std::transform(fileName.begin(), fileName.end(), fileName.begin(), backslash_to_slash);
							bool error;
							switch (f.first)
							{
								case CommandLineFlag_Open:
									logMgr << LogLevel_Info << "Opening scenario [" << fileName << "]\n";
									if (!app.openScenario(fileName.c_str()))
									{
										logMgr << LogLevel_Error << "Could not open scenario " << fileName << "\n";
										errorWhileLoadingScenario = config.noGui == CommandLineFlag_NoGui;
									}
									break;
								case CommandLineFlag_Play:
									logMgr << LogLevel_Info << "Opening and playing scenario [" << fileName << "]\n";
									error = !app.openScenario(fileName.c_str());
									if (!error)
									{
										app.playScenarioCB();
										error = app.getCurrentInterfacedScenario()->m_PlayerStatus != EPlayerStatus::Play;
									}
									if (error)
									{
										logMgr << LogLevel_Error << "Scenario open or load error with --play.\n";
										errorWhileLoadingScenario = config.noGui == CommandLineFlag_NoGui;
									}
									break;
								case CommandLineFlag_PlayFast:
									logMgr << LogLevel_Info << "Opening and fast playing scenario [" << fileName << "]\n";
									error = !app.openScenario(fileName.c_str());
									if (!error)
									{
										app.forwardScenarioCB();
										error = app.getCurrentInterfacedScenario()->m_PlayerStatus != EPlayerStatus::Forward;
									}
									if (error)
									{
										logMgr << LogLevel_Error << "Scenario open or load error with --play-fast.\n";
										errorWhileLoadingScenario = config.noGui == CommandLineFlag_NoGui;
									}
									playRequested = true;
									break;
									//case CommandLineFlag_Define:
									//break;
								default:
									break;
							}
						}

						if (!playRequested && config.noGui == CommandLineFlag_NoGui)
						{
							logMgr << LogLevel_Info << "Switch --no-gui is enabled but no play operation was requested. Designer will exit automatically.\n";
						}

						if (app.m_Scenarios.empty() && config.noGui != CommandLineFlag_NoGui) { app.newScenarioCB(); }

						if (!app.m_Scenarios.empty())
						{
							CPluginObjectDescCollector cbCollector1(*context);
							CPluginObjectDescCollector cbCollector2(*context);
							CPluginObjectDescLogger cbLogger(*context);
							cbLogger.enumeratePluginObjectDesc();
							cbCollector1.enumeratePluginObjectDesc(OV_ClassId_Plugins_BoxAlgorithmDesc);
							cbCollector2.enumeratePluginObjectDesc(OV_ClassId_Plugins_AlgorithmDesc);
							InsertPluginObjectDescToGtkTreeStore(*context, cbCollector1.getPluginObjectDescMap(), app.m_BoxAlgorithmTreeModel, app.m_NewBoxes,
																 app.m_UpdatedBoxes, app.m_IsNewVersion);
							InsertPluginObjectDescToGtkTreeStore(*context, cbCollector2.getPluginObjectDescMap(), app.m_AlgorithmTreeModel, app.m_NewBoxes,
																 app.m_UpdatedBoxes);

							std::map<std::string, const IPluginObjectDesc*> metaboxDescMap;
							CIdentifier identifier;
							while ((identifier = context->getMetaboxManager().getNextMetaboxObjectDescIdentifier(identifier)) != OV_UndefinedIdentifier
							) { metaboxDescMap[std::string(identifier.toString())] = context->getMetaboxManager().getMetaboxObjectDesc(identifier); }
							InsertPluginObjectDescToGtkTreeStore(*context, metaboxDescMap, app.m_BoxAlgorithmTreeModel, app.m_NewBoxes, app.m_UpdatedBoxes,
																 app.m_IsNewVersion);

							context->getLogManager() << LogLevel_Info << "Initialization took " << context->getConfigurationManager().expand("$Core{real-time}")
									<< " ms\n";
							// If the application is a newly launched version, and not launched without GUI -> display changelog
							if (app.m_IsNewVersion && config.noGui != CommandLineFlag_NoGui) { app.displayChangelogWhenAvailable(); }
							try { gtk_main(); }
							catch (DesignerException& ex)
							{
								std::cerr << "Caught designer exception" << std::endl;
								GtkWidget* errorDialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
																				ex.getErrorString().c_str());
								gtk_window_set_title(GTK_WINDOW(errorDialog), (std::string(BRAND_NAME) + " has stopped functioning").c_str());
								gtk_dialog_run(GTK_DIALOG(errorDialog));
							}
							catch (...) { std::cerr << "Caught top level exception" << std::endl; }
						}
					}
				}

				logMgr << LogLevel_Info << "Application terminated, releasing allocated objects\n";

				VisualizationToolkit::uninitialize(*context);
				Toolkit::uninitialize(*context);

				desc->releaseKernel(context);

				// Remove the mutex only if the application was run with a gui
				if (config.noGui != CommandLineFlag_NoGui) { boost::interprocess::named_mutex::remove(MUTEX_NAME); }
			}
		}
		loader.uninitialize();
		loader.unload();
	}
	return errorWhileLoadingScenario ? -1 : 0;
}

int main(const int argc, char** argv)
{
	// Remove mutex at startup, as the main loop regenerates frequently this mutex,
	// if another instance is running, it should have the time to regenerate it
	// Avoids that after crashing, a mutex stays blocking
	boost::interprocess::named_mutex::remove(MUTEX_NAME);
	try { go(argc, argv); }
	catch (...) { std::cout << "Caught an exception at the very top...\nLeaving application!\n"; }
	//return go(argc, argv);
}

#if defined TARGET_OS_Windows
// Should be used once we get rid of the .cmd launchers
int WINAPI WinMain(HINSTANCE /*instance*/, HINSTANCE /*prevInstance*/, LPSTR /*cmdLine*/, int /*windowStyle*/) { return main(__argc, __argv); }
#endif //defined TARGET_OS_Windows
