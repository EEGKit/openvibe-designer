#include <stack>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <system/ovCTime.h>
#include <openvibe/kernel/metabox/ovIMetaboxManager.h>
#include "ovd_base.h"

#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"

#include "ovdAssert.h"
#if defined TARGET_OS_Windows
#include "windows.h"
#endif

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

map<uint32, ::GdkColor> g_vColors;

class CPluginObjectDescEnum
{
public:

	CPluginObjectDescEnum(const IKernelContext& rKernelContext)
		:m_rKernelContext(rKernelContext)
	{
	}

	virtual ~CPluginObjectDescEnum(void)
	{
	}

	virtual OpenViBE::boolean enumeratePluginObjectDesc(void)
	{
		CIdentifier l_oIdentifier;
		while ((l_oIdentifier = m_rKernelContext.getPluginManager().getNextPluginObjectDescIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
		{
			this->callback(*m_rKernelContext.getPluginManager().getPluginObjectDesc(l_oIdentifier));
		}
		return true;
	}

	virtual OpenViBE::boolean enumeratePluginObjectDesc(
		const CIdentifier& rParentClassIdentifier)
	{
		CIdentifier l_oIdentifier;
		while ((l_oIdentifier = m_rKernelContext.getPluginManager().getNextPluginObjectDescIdentifier(l_oIdentifier, rParentClassIdentifier)) != OV_UndefinedIdentifier)
		{
			this->callback(*m_rKernelContext.getPluginManager().getPluginObjectDesc(l_oIdentifier));
		}
		return true;
	}

	virtual OpenViBE::boolean callback(
		const IPluginObjectDesc& rPluginObjectDesc) = 0;

protected:

	const IKernelContext& m_rKernelContext;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescCollector : public CPluginObjectDescEnum
{
public:

	CPluginObjectDescCollector(const IKernelContext& rKernelContext)
		:CPluginObjectDescEnum(rKernelContext)
	{
	}

	virtual OpenViBE::boolean callback(
		const IPluginObjectDesc& rPluginObjectDesc)
	{
		string l_sFullName = string(rPluginObjectDesc.getCategory()) + "/" + string(rPluginObjectDesc.getName());
		map<string, const IPluginObjectDesc* >::iterator itPluginObjectDesc = m_vPluginObjectDesc.find(l_sFullName);
		if (itPluginObjectDesc != m_vPluginObjectDesc.end())
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Duplicate plugin object name " << CString(l_sFullName.c_str()) << " " << itPluginObjectDesc->second->getCreatedClass() << " and " << rPluginObjectDesc.getCreatedClass() << "\n";
		}
		m_vPluginObjectDesc[l_sFullName] = &rPluginObjectDesc;
		return true;
	}

	map<string, const IPluginObjectDesc*>& getPluginObjectDescMap(void)
	{
		return m_vPluginObjectDesc;
	}

private:

	map<string, const IPluginObjectDesc*> m_vPluginObjectDesc;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescLogger : public CPluginObjectDescEnum
{
public:

	CPluginObjectDescLogger(const IKernelContext& rKernelContext)
		:CPluginObjectDescEnum(rKernelContext)
	{
	}

	virtual OpenViBE::boolean callback(
		const IPluginObjectDesc& rPluginObjectDesc)
	{
		// Outputs plugin info to console
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Plugin <" << rPluginObjectDesc.getName() << ">\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Plugin category        : " << rPluginObjectDesc.getCategory() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Class identifier       : " << rPluginObjectDesc.getCreatedClass() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Author name            : " << rPluginObjectDesc.getAuthorName() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Author company name    : " << rPluginObjectDesc.getAuthorCompanyName() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Short description      : " << rPluginObjectDesc.getShortDescription() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Detailed description   : " << rPluginObjectDesc.getDetailedDescription() << "\n";

		return true;
	}
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

static void insertPluginObjectDesc_to_GtkTreeStore(const IKernelContext& rKernelContext, map<string, const IPluginObjectDesc*>& vPluginObjectDesc, ::GtkTreeStore* pTreeStore, 
	std::vector<const IPluginObjectDesc*>& vNewBoxes, std::vector<const IPluginObjectDesc*>& vUpdatedBoxes, bool bIsNewVersion = false)
{
	// By default, fix version to current version - to display the new/update boxes available since current version only
	uint64 l_uiMajorLastVersionOpened = rKernelContext.getConfigurationManager().expandAsUInteger("${ProjectVersion_Major}");
	uint64 l_uiMinorLastVersionOpened = rKernelContext.getConfigurationManager().expandAsUInteger("${ProjectVersion_Minor}");
	CString l_sLastUsedVersion = rKernelContext.getConfigurationManager().expand("${Designer_LastVersionUsed}");
	if(l_sLastUsedVersion.length() != 0)
	{
		sscanf(l_sLastUsedVersion.toASCIIString(), "%3u.%3u.%*u.%*u", &l_uiMajorLastVersionOpened, &l_uiMinorLastVersionOpened);
	}
	for(auto pPluginObjectDesc : vPluginObjectDesc)
	{
		const IPluginObjectDesc* l_pPluginObjectDesc = pPluginObjectDesc.second;

		CString l_sStockItemName;

		const IBoxAlgorithmDesc* l_pBoxAlgorithmDesc = dynamic_cast<const IBoxAlgorithmDesc*>(l_pPluginObjectDesc);
		if (l_pBoxAlgorithmDesc)
		{
			l_sStockItemName = l_pBoxAlgorithmDesc->getStockItemName();
		}

		OpenViBE::boolean l_bShouldShow = true;

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
			::GtkStockItem l_oStockItem;
			if (!gtk_stock_lookup(l_sStockItemName, &l_oStockItem))
			{
				l_sStockItemName = GTK_STOCK_NEW;
			}

			// Splits the plugin category
			vector<string> l_vCategory;
			string l_sCategory = string(l_pPluginObjectDesc->getCategory());
			size_t j, i = (size_t)-1;
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
				l_vCategory.push_back(string(l_sCategory, i + 1, l_sCategory.length() - i - 1));
			}

			// Fills plugin in the tree
			::GtkTreeIter l_oGtkIter1;
			::GtkTreeIter l_oGtkIter2;
			::GtkTreeIter* l_pGtkIterParent = NULL;
			::GtkTreeIter* l_pGtkIterChild = &l_oGtkIter1;
			for(string l_sCategory : l_vCategory)
			{
				OpenViBE::boolean l_bFound = false;
				OpenViBE::boolean l_bValid = gtk_tree_model_iter_children(
					GTK_TREE_MODEL(pTreeStore),
					l_pGtkIterChild,
					l_pGtkIterParent) ? true : false;
				while (l_bValid && !l_bFound)
				{
					gchar* l_sName = NULL;
					gboolean l_bIsPlugin;
					gtk_tree_model_get(
						GTK_TREE_MODEL(pTreeStore),
						l_pGtkIterChild,
						Resource_StringName, &l_sName,
						Resource_BooleanIsPlugin, &l_bIsPlugin,
						-1);
					if (!l_bIsPlugin && l_sName == l_sCategory)
					{
						l_bFound = true;
					}
					else
					{
						l_bValid = gtk_tree_model_iter_next(
							GTK_TREE_MODEL(pTreeStore),
							l_pGtkIterChild) ? true : false;
					}
				}
				if (!l_bFound)
				{
					gtk_tree_store_append(
						GTK_TREE_STORE(pTreeStore),
						l_pGtkIterChild,
						l_pGtkIterParent);
					gtk_tree_store_set(
						GTK_TREE_STORE(pTreeStore),
						l_pGtkIterChild,
						Resource_StringName, l_sCategory.c_str(),
						Resource_StringShortDescription, "",
						Resource_StringStockIcon, "gtk-directory",
						Resource_StringColor, "#000000",
						Resource_StringFont, "",
						Resource_BooleanIsPlugin, (gboolean)FALSE,
						-1);
				}
				if (!l_pGtkIterParent)
				{
					l_pGtkIterParent = &l_oGtkIter2;
				}
				::GtkTreeIter* l_pGtkIterSwap = l_pGtkIterChild;
				l_pGtkIterChild = l_pGtkIterParent;
				l_pGtkIterParent = l_pGtkIterSwap;
			}
			gtk_tree_store_append(
				GTK_TREE_STORE(pTreeStore),
				l_pGtkIterChild,
				l_pGtkIterParent);

			// define color of the text of the box
			std::string l_sTextColor = "black";
			std::string l_sBackGroundColor = "white";
			std::string l_sTextFont = "";
			std::string l_sName(l_pPluginObjectDesc->getName().toASCIIString());

			if (rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(l_pPluginObjectDesc->getCreatedClass()))
			{
				l_sTextColor = "#3f7f7f";
			}

			// If the software is launched for the first time after update, highlight new/updated boxes in tree-view
			uint64 currentVersionMajor = rKernelContext.getConfigurationManager().expandAsUInteger("${ProjectVersion_Major}");
			uint64 currentVersionMinor = rKernelContext.getConfigurationManager().expandAsUInteger("${ProjectVersion_Minor}");
			uint32 l_uiMajor = 0;
			uint32 l_uiMinor = 0;
			sscanf(l_pPluginObjectDesc->getAddedSoftwareVersion().toASCIIString(), "%3u.%3u.%*u.%*u", &l_uiMajor, &l_uiMinor);
			// If this is a new version, then add in list all the updated/new boxes since last version opened
			if(bIsNewVersion && (
				(l_uiMajorLastVersionOpened < l_uiMajor && l_uiMajor <= currentVersionMajor)
				|| (l_uiMajor == currentVersionMajor && l_uiMinorLastVersionOpened < l_uiMinor && l_uiMinor <= currentVersionMajor)
				// As default value for l_uiMinorLastVersionOpened and l_uiMajorLastVersionOpened are the current software version
				|| (l_uiMajor == currentVersionMajor && l_uiMinor == currentVersionMinor)) )
			{
				l_sName = l_sName + " (New)";
				l_sBackGroundColor = "#FFFFC4";
				vNewBoxes.push_back(l_pPluginObjectDesc);
			}
			// Otherwise 
			else if(l_uiMajor == currentVersionMajor && l_uiMinor == currentVersionMinor)
			{
				vNewBoxes.push_back(l_pPluginObjectDesc);
			}
			else
			{
				uint32 l_uiMajor = 0;
				uint32 l_uiMinor = 0;
				sscanf(l_pPluginObjectDesc->getUpdatedSoftwareVersion().toASCIIString(), "%3u.%3u.%*u.%*u", &l_uiMajor, &l_uiMinor);
				// If this is a new version, then add in list all the updated/new boxes since last version opened
				if( bIsNewVersion && (
					(l_uiMajorLastVersionOpened < l_uiMajor && l_uiMajor <= currentVersionMajor)
					|| (l_uiMajor == currentVersionMajor && l_uiMinorLastVersionOpened < l_uiMinor && l_uiMinor <= currentVersionMinor)
					// If this is a new version Studio, and last version opened was set to default value i.e. version of current software
					|| (l_uiMajor == currentVersionMajor && l_uiMinor == currentVersionMinor)) )
				{
					l_sName = l_sName + " (New)";
					l_sBackGroundColor = "#FFFFC4";
					vUpdatedBoxes.push_back(l_pPluginObjectDesc);
				}
				// Otherwise 
				else if(!bIsNewVersion && (l_uiMajor == currentVersionMajor && l_uiMinor == currentVersionMinor))
				{
					vUpdatedBoxes.push_back(l_pPluginObjectDesc);
				}
			}

			// Construct a string containing the BoxAlgorithmIdentifier concatenated with a metabox identifier if necessary
			std::string l_sBoxAlgorithmDescriptor(l_pPluginObjectDesc->getCreatedClass().toString().toASCIIString());

			if (l_pPluginObjectDesc->getCreatedClass() == OVP_ClassId_BoxAlgorithm_Metabox)
			{
				l_sBoxAlgorithmDescriptor += dynamic_cast<const OpenViBE::Metabox::IMetaboxObjectDesc*>(l_pPluginObjectDesc)->getMetaboxDescriptor();
				l_sTextColor = "#007020";
			}
			else
			{
				if (l_pPluginObjectDesc->hasFunctionality(M_Functionality_IsMensia))
				{
					l_sTextColor = "#00b090";
				}
			}


			gtk_tree_store_set(
				GTK_TREE_STORE(pTreeStore),
				l_pGtkIterChild,
				Resource_StringName, l_sName.c_str(),
				Resource_StringShortDescription, (const char*)l_pPluginObjectDesc->getShortDescription(),
				Resource_StringIdentifier, (const char*)l_sBoxAlgorithmDescriptor.c_str(),
				Resource_StringStockIcon, (const char*)l_sStockItemName,
				Resource_StringColor, l_sTextColor.c_str(),
				Resource_StringFont, l_sTextFont.c_str(),
				Resource_BooleanIsPlugin, (gboolean)TRUE,
				Resource_BackGroundColor, (const char*)l_sBackGroundColor.c_str(),
				-1);
		};
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

typedef struct _SConfiguration
{
	_SConfiguration(void)
		:m_eNoGui(CommandLineFlag_None)
		, m_eNoCheckColorDepth(CommandLineFlag_None)
		, m_eNoManageSession(CommandLineFlag_None)
	{
	}

	OpenViBEDesigner::ECommandLineFlag getFlags(void)
	{
		return OpenViBEDesigner::ECommandLineFlag(m_eNoGui | m_eNoCheckColorDepth | m_eNoManageSession);
	}

	std::vector < std::pair < ECommandLineFlag, std::string > > m_vFlag;
	OpenViBEDesigner::ECommandLineFlag m_eNoGui;
	OpenViBEDesigner::ECommandLineFlag m_eNoCheckColorDepth;
	OpenViBEDesigner::ECommandLineFlag m_eNoManageSession;
} SConfiguration;

static char backslash_to_slash(char c)
{
	return c == '\\' ? '/' : c;
}

/** ------------------------------------------------------------------------------------------------------------------------------------
* Use Mutex to ensure that only one instance with GUI of Studio runs at the same time
* if another instance exists, sends a message to it so that it opens a scenario or get the focus back
* \param sMode: play, play-fast or open
* \param sScenarioPath: name of the scenario to open
------------------------------------------------------------------------------------------------------------------------------------**/
static bool ensureOneInstanceOfDesigner(SConfiguration& pConfiguration, ILogManager& l_rLogManager)
{
#if defined _NDEBUG
	try
	{
		// If the mutex cannot be opened, it's the first instance of Designer, go to catch
		boost::interprocess::named_mutex l_oMutex(boost::interprocess::open_only, MUTEX_NAME);

		// If the mutex was opened, then an instance of designer is already running, we send it a message before dying
		// The message contains the command to send: sMode: open, play, play-fast a scenario, sScenarioPath: path of the scenario
		boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(l_oMutex);
		std::string l_sMessage="";
		int32 l_i32Mode;
		if(pConfiguration.m_vFlag.size() == 0)
		{
			char l_s32Mode[32];
			sprintf(l_s32Mode, "%d", MessageType_NoArguments);
			l_sMessage = std::string(l_s32Mode) + ": ;";
		}
		for(size_t i = 0; i<pConfiguration.m_vFlag.size(); i++)
		{
			std::string l_sFileName = pConfiguration.m_vFlag[i].second;
			std::transform(l_sFileName.begin(), l_sFileName.end(), l_sFileName.begin(), backslash_to_slash);
			switch(pConfiguration.m_vFlag[i].first)
			{
			case CommandLineFlag_Open:
				l_i32Mode = MessageType_OpenScenario;
				break;
			case CommandLineFlag_Play:
				l_i32Mode = MessageType_PlayScenario;
				break;
			case CommandLineFlag_PlayFast:
				l_i32Mode = MessageType_PlayFastScenario;
				break;
			default:
				l_i32Mode = MessageType_NoArguments;
				break;
			}
			char l_s32Mode[32];
			sprintf(l_s32Mode, "%d", l_i32Mode);
			l_sMessage = l_sMessage + l_s32Mode + ": <" + l_sFileName + "> ; ";
		}
		const char* l_sFinalMessage = l_sMessage.c_str(); 
		l_rLogManager << LogLevel_Trace << "There is already an instance of studio running. " << l_sFinalMessage << " \n";
		size_t l_sizeMessage = (strlen(l_sFinalMessage) * sizeof(char));

		boost::interprocess::message_queue l_oMessageToFirstInstance(boost::interprocess::open_or_create, MESSAGE_NAME, l_sizeMessage, l_sizeMessage);
		l_oMessageToFirstInstance.send(l_sFinalMessage, l_sizeMessage, 0);

		return false;
	}
	catch(boost::interprocess::interprocess_exception&)
	{
		//Create the named mutex to catch the potential next instance of studio that could open
		l_rLogManager << LogLevel_Trace << "EnsureOneInstanceOfDesigner- This is the only instance of studio with a gui, open it normally.\n";
		boost::interprocess::named_mutex l_oMutex(boost::interprocess::create_only, MUTEX_NAME);
		return true;
	}
#else
	return true;
#endif
}

OpenViBE::boolean parse_arguments(int argc, char** argv, SConfiguration& rConfiguration)
{
	SConfiguration l_oConfiguration;

	int i;
	std::vector < std::string > l_vArgValue;
	std::vector < std::string >::const_iterator it;
	for(i = 1; i<argc; i++)
	{
		l_vArgValue.push_back(argv[i]);
	}
	l_vArgValue.push_back("");

	for(it = l_vArgValue.begin(); it != l_vArgValue.end(); ++it)
	{
		if(*it == "")
		{
		}
		else if(*it == "-h" || *it == "--help")
		{
			return false;
		}
		else if(*it == "-o" || *it == "--open")
		{
			l_oConfiguration.m_vFlag.push_back(std::make_pair(CommandLineFlag_Open, *++it));
		}
		else if(*it == "-p" || *it == "--play")
		{
			l_oConfiguration.m_vFlag.push_back(std::make_pair(CommandLineFlag_Play, *++it));
		}
		else if(*it == "-pf" || *it == "--play-fast")
		{
			l_oConfiguration.m_vFlag.push_back(std::make_pair(CommandLineFlag_PlayFast, *++it));
		}
		else if(*it == "--no-gui")
		{
			l_oConfiguration.m_eNoGui = CommandLineFlag_NoGui;
			l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
			l_oConfiguration.m_eNoManageSession = CommandLineFlag_NoManageSession;
		}
		else if(*it == "--no-check-color-depth")
		{
			l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
		}
		else if(*it == "--no-session-management")
		{
			l_oConfiguration.m_eNoManageSession = CommandLineFlag_NoManageSession;
		}
		//		else if(*it=="--define")
		//		{
		//			l_oConfiguration.m_vFlag.push_back(std::make_pair(Flag_NoGui, *++it));
		//		}
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
	rConfiguration.m_bCheckColorDepth=l_oConfiguration.m_bCheckColorDepth;
	rConfiguration.m_bShowGui=l_oConfiguration.m_bShowGui;
#else
	rConfiguration=l_oConfiguration;
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

void message(const char* sTitle, const char* sMessage, GtkMessageType eType)
{
	::GtkWidget* l_pDialog=::gtk_message_dialog_new(
		NULL,
		::GtkDialogFlags(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
		eType,
		GTK_BUTTONS_OK,
		"%s", sTitle);
	::gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(l_pDialog), "%s", sMessage);
	::gtk_window_set_icon_from_file(GTK_WINDOW(l_pDialog), (OpenViBE::Directories::getDataDir() + CString("/applications/designer/designer.ico")).toASCIIString(), NULL);
	::gtk_window_set_title(GTK_WINDOW(l_pDialog), sTitle);
	::gtk_dialog_run(GTK_DIALOG(l_pDialog));
	::gtk_widget_destroy(l_pDialog);
}

int go(int argc, char ** argv)
{
	/*
	{ 0,     0,     0,     0 },
	{ 0, 16383, 16383, 16383 },
	{ 0, 32767, 32767, 32767 },
	{ 0, 49151, 49151, 49151 },
	{ 0, 65535, 65535, 65535 },
	*/
#define gdk_color_set(c, r, g, b) { c.pixel=0; c.red=r; c.green=g; c.blue=b; }
	gdk_color_set(g_vColors[Color_BackgroundPlayerStarted], 32767, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundSelected], 65535, 65535, 49151);
	gdk_color_set(g_vColors[Color_BoxBackgroundMissing], 49151, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundDisabled], 46767, 46767, 59151);
	gdk_color_set(g_vColors[Color_BoxBackgroundDeprecated], 65535, 50000, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundNeedsUpdate], 57343, 57343, 57343);
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

	CKernelLoader l_oKernelLoader;

	cout << "[  INF  ] Created kernel loader, trying to load kernel module" << "\n";
	CString l_sError;
#if defined TARGET_OS_Windows
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll";
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
		IKernelDesc* l_pKernelDesc = NULL;
		IKernelContext* l_pKernelContext = NULL;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if (!l_pKernelDesc)
		{
			cout << "[ FAILED ] No kernel descriptor" << "\n";
		}
		else
		{
			cout << "[  INF  ] Got kernel descriptor, trying to create kernel" << "\n";

			l_pKernelContext = l_pKernelDesc->createKernel("designer", OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			l_pKernelContext->initialize();
			l_pKernelContext->getConfigurationManager().addConfigurationFromFile(OpenViBE::Directories::getDataDir() + "/applications/designer/designer.conf");
			OpenViBE::CString l_sAppConfigFile = l_pKernelContext->getConfigurationManager().expand("${Designer_CustomConfigurationFile}");

			l_pKernelContext->getConfigurationManager().addConfigurationFromFile(l_sAppConfigFile);
			if (!l_pKernelContext)
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
				ILogManager& l_rLogManager = l_pKernelContext->getLogManager();

				SConfiguration l_oConfiguration;
				OpenViBE::boolean bArgParseResult = parse_arguments(argc, argv, l_oConfiguration);
				l_rLogManager << LogLevel_Info << "Syntax :\n";

				l_pKernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));

				//FIXME : set locale only when needed
				CString l_sLocale = l_rConfigurationManager.expand("${Designer_Locale}");
				if (l_sLocale == CString(""))
				{
					l_sLocale = "C";
				}
				setlocale(LC_ALL, l_sLocale.toASCIIString());

				if(!bArgParseResult)
				{
					l_rLogManager << LogLevel_Info << "Syntax : " << argv[0] << " [ switches ]\n";
					l_rLogManager << LogLevel_Info << "Possible switches :\n";
					l_rLogManager << LogLevel_Info << "  --help                  : displays this help message and exits\n";
					l_rLogManager << LogLevel_Info << "  --open filename         : opens a scenario (see also --no-session-management)\n";
					l_rLogManager << LogLevel_Info << "  --play filename         : plays the opened scenario (see also --no-session-management)\n";
					l_rLogManager << LogLevel_Info << "  --play-fast filename    : plays fast forward the opened scenario (see also --no-session-management)\n";
					l_rLogManager << LogLevel_Info << ("  --no-gui                : hides the " + std::string(STUDIO_NAME) + " graphical user interface (assumes --no-color-depth-test)\n").c_str();
					l_rLogManager << LogLevel_Info << "  --no-check-color-depth  : does not check 24/32 bits color depth\n";
					l_rLogManager << LogLevel_Info << "  --no-session-management : neither restore last used scenarios nor saves them at exit\n";
					//					l_rLogManager << LogLevel_Info << "  --define                : defines a variable in the configuration manager\n";
				}
				else
				{
					if ((l_rConfigurationManager.expandAsBoolean("${Kernel_WithGUI}", true) == false) && ((l_oConfiguration.getFlags()&CommandLineFlag_NoGui) == 0))
					{
						l_rLogManager << LogLevel_ImportantWarning << "${Kernel_WithGUI} is set to false and --no-gui flag not set. Forcing the --no-gui flag\n";
						l_oConfiguration.m_eNoGui = CommandLineFlag_NoGui;
						l_oConfiguration.m_eNoCheckColorDepth = CommandLineFlag_NoCheckColorDepth;
						l_oConfiguration.m_eNoManageSession = CommandLineFlag_NoManageSession;
					}
	
					if(l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui && !ensureOneInstanceOfDesigner(l_oConfiguration, l_rLogManager))
					{
						l_rLogManager << LogLevel_Trace << "An instance of Studio is already running.\n";
						return 0;
					}

					{
						::CApplication app(*l_pKernelContext);
						app.initialize(l_oConfiguration.getFlags());

						// FIXME is it necessary to keep next line uncomment ?
						//boolean l_bIsScreenValid=true;
						if (!l_oConfiguration.m_eNoCheckColorDepth)
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
						for (size_t i = 0; i<l_oConfiguration.m_vFlag.size(); i++)
						{
							std::string l_sFileName = l_oConfiguration.m_vFlag[i].second;
							std::transform(l_sFileName.begin(), l_sFileName.end(), l_sFileName.begin(), backslash_to_slash);
							switch (l_oConfiguration.m_vFlag[i].first)
							{
							case CommandLineFlag_Open:
								l_rLogManager << LogLevel_Info << "Opening scenario [" << CString(l_sFileName.c_str()) << "]\n";
								app.openScenario(l_sFileName.c_str());
								break;
							case CommandLineFlag_Play:
								l_rLogManager << LogLevel_Info << "Opening and playing scenario [" << CString(l_sFileName.c_str()) << "]\n";
								if (app.openScenario(l_sFileName.c_str()))
								{
									app.playScenarioCB();
								}
								break;
							case CommandLineFlag_PlayFast:
								l_rLogManager << LogLevel_Info << "Opening and fast playing scenario [" << CString(l_sFileName.c_str()) << "]\n";
								if (app.openScenario(l_sFileName.c_str()))
								{
									app.forwardScenarioCB();
								}
								break;
								//								case CommandLineFlag_Define:
								//									break;
							default:
								break;
							}
						}

						if (app.m_vInterfacedScenario.empty())
						{
							app.newScenarioCB();
						}

						{
							CPluginObjectDescCollector cb_collector1(*l_pKernelContext);
							CPluginObjectDescCollector cb_collector2(*l_pKernelContext);
							CPluginObjectDescLogger cb_logger(*l_pKernelContext);
							cb_logger.enumeratePluginObjectDesc();
							cb_collector1.enumeratePluginObjectDesc(OV_ClassId_Plugins_BoxAlgorithmDesc);
							cb_collector2.enumeratePluginObjectDesc(OV_ClassId_Plugins_AlgorithmDesc);
							insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, cb_collector1.getPluginObjectDescMap(), app.m_pBoxAlgorithmTreeModel, app.m_vNewBoxes, app.m_vUpdatedBoxes, app.m_bIsNewVersion);
							insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, cb_collector2.getPluginObjectDescMap(), app.m_pAlgorithmTreeModel, app.m_vNewBoxes, app.m_vUpdatedBoxes);

							l_pKernelContext->getMetaboxManager().addMetaboxesFromFiles(l_pKernelContext->getConfigurationManager().expand("${Kernel_Metabox}"));
							std::map<std::string, const OpenViBE::Plugins::IPluginObjectDesc*> metaboxDescMap;
							CIdentifier l_oIdentifier;
							while ((l_oIdentifier = l_pKernelContext->getMetaboxManager().getNextMetaboxObjectDescIdentifier(l_oIdentifier)) != OV_UndefinedIdentifier)
							{
								metaboxDescMap[std::string(l_oIdentifier.toString())] = l_pKernelContext->getMetaboxManager().getMetaboxObjectDesc(l_oIdentifier);
							}
							insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, metaboxDescMap, app.m_pBoxAlgorithmTreeModel, app.m_vNewBoxes, app.m_vUpdatedBoxes, app.m_bIsNewVersion);

							l_pKernelContext->getLogManager() << LogLevel_Info << "Initialization took " << l_pKernelContext->getConfigurationManager().expand("$Core{real-time}") << " ms\n";
							// If the application is a newly launched version, and not launched without GUI -> display changelog
							if(app.m_bIsNewVersion && l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui)
							{
								app.displayChangelogWhenAvailable();
							}
							try
							{
								gtk_main();
							}
							catch (DesignerException ex)
							{
								std::cerr << "Caught designer exception" << std::endl;
								::GtkWidget* errorDialog = gtk_message_dialog_new(
								            NULL,
								            GTK_DIALOG_MODAL,
								            GTK_MESSAGE_ERROR,
								            GTK_BUTTONS_CLOSE,
								            "%s",
								            ex.getErrorString().c_str()
								            );
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
				if (l_oConfiguration.m_eNoGui != CommandLineFlag_NoGui )
				{
					boost::interprocess::named_mutex::remove("openvibe_designer_mutex");
				}
			}
		}
		l_oKernelLoader.uninitialize();
		l_oKernelLoader.unload();
	}

	return 0;
}

int main(int argc, char ** argv)
{
	// Remove mutex at startup, as the main loop regenerates frequently this mutex,
	// if another instance is running, it should have the time to regenerate it
	// Avoids that after crashing, a mutex stays blocking
	boost::interprocess::named_mutex::remove("openvibe_designer_mutex");
	int l_iRet = -1;
//	try
	{
		l_iRet = go(argc, argv);
	}
	/*
	catch (...)
	{
		std::cout << "Caught an exception at the very top...\nLeaving application!\n";
	}
	*/
	return l_iRet;
}

#if defined TARGET_OS_Windows
// Should be used once we get rid of the .cmd launchers
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int windowStyle)
{
	return main(__argc, __argv);
}
#endif //defined TARGET_OS_Windows
