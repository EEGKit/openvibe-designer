#ifndef __OpenViBEKernel_Kernel_Plugins_CMetaboxLoader_H__
#define __OpenViBEKernel_Kernel_Plugins_CMetaboxLoader_H__


#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
//#include "ovd_base.h"r

#include <fs/IEntryEnumerator.h>
#include <fs/Files.h>

#include <map>
#include <vector>
#include <string>

#define OVD_ScenarioimportContext_OnLoadMetaboxImport OpenViBE::CIdentifier(0xED48480A, 0x9F6DBAA7)


/*
 * Enumerator for loading scenarios
 *
 */
namespace Mensia
{
	class CMetaboxLoader;

	class CMetaboxLoaderEntryEnumeratorCallBack : public FS::IEntryEnumeratorCallBack
	{
	public:

		CMetaboxLoaderEntryEnumeratorCallBack(
		        const OpenViBE::Kernel::IKernelContext& rKernelContext,
		        CMetaboxLoader& rMetaboxLoader)
		    : m_rMetaboxLoader(rMetaboxLoader)
		{
		}

		FS::boolean callback(FS::IEntryEnumerator::IEntry& rEntry, FS::IEntryEnumerator::IAttributes& rAttributes);

	protected:
		CMetaboxLoader& m_rMetaboxLoader;

	};


	/**
	 * @brief The CBoxAlgorithmMetaboxDesc virtual BoxAlgorithmDesc for metaboxes
	 *
	 * This class provides a virtual algorithm descriptor for metaboxes. Each metabox-scenario
	 * will result in one of these descriptors. The prototype is created from scenario inputs,
	 * outputs and settings.
	 *
	 * Variables such as name, author etc are pulled from scenario information.
	 */
	class CBoxAlgorithmMetaboxDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
	{
	public:
		CBoxAlgorithmMetaboxDesc()
		{
		}

		CBoxAlgorithmMetaboxDesc(
		        const OpenViBE::CString& rMetaboxDescriptor,
		        const OpenViBE::CString& rName,
		        const OpenViBE::CString& rAuthorName,
		        const OpenViBE::CString& rAuthorCompanyName,
		        const OpenViBE::CString& rShortDescription,
		        const OpenViBE::CString& rDetailedDescription,
		        const OpenViBE::CString& rCategory,
		        const OpenViBE::CString& rVersion,
		        const OpenViBE::CString& rStockItemName,
				const OpenViBE::CString& rAddedSoftwareVersion,
				const OpenViBE::CString& rUpdatedSoftwareVersion)
		    : m_sMetaboxDescriptor(rMetaboxDescriptor),
		      m_sName(rName),
		      m_sAuthorName(rAuthorName),
		      m_sAuthorCompanyName(rAuthorCompanyName),
		      m_sShortDescription(rShortDescription),
		      m_sDetailedDescription(rDetailedDescription),
		      m_sCategory(rCategory),
		      m_sVersion(rVersion),
		      m_sStockItemName(rStockItemName),
			  m_sAddedSoftwareVersion(rAddedSoftwareVersion),
			  m_sUpdatedSoftwareVersion(rUpdatedSoftwareVersion)
		{
		}

		virtual void release(void) { }

		OpenViBE::CString getMetaboxDescriptor(void) const                 { return m_sMetaboxDescriptor; }

		virtual OpenViBE::CString getName(void) const                   { return m_sName; }
		virtual OpenViBE::CString getAuthorName(void) const             { return m_sAuthorName; }
		virtual OpenViBE::CString getAuthorCompanyName(void) const      { return m_sAuthorCompanyName; }
		virtual OpenViBE::CString getShortDescription(void) const       { return m_sShortDescription; }
		virtual OpenViBE::CString getDetailedDescription(void) const    { return m_sDetailedDescription; }
		virtual OpenViBE::CString getCategory(void) const               { return m_sCategory; }
		virtual OpenViBE::CString getVersion(void) const                { return m_sVersion; }
		virtual OpenViBE::CString getStockItemName(void) const          { return m_sStockItemName; }
		virtual OpenViBE::CString getAddedSoftwareVersion(void) const   { return m_sAddedSoftwareVersion; }
		virtual OpenViBE::CString getUpdatedSoftwareVersion(void) const { return m_sUpdatedSoftwareVersion; }

		virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_Metabox; }
		virtual OpenViBE::Plugins::IPluginObject* create(void)       { return NULL; }

		// Handling of the virtual prototype

		// Since we have to construct a prototype on the fly, the special Metabox descriptor
		// will also hold the information about the settings, inputs and outputs of the box
		typedef struct _SIOStream
		{
			_SIOStream()
			    : m_sName(""),
			      m_oTypeIdentifier(OV_UndefinedIdentifier)
			{}

			_SIOStream(const OpenViBE::CString& rName, const OpenViBE::CIdentifier& rTypeIdentifier)
			    : m_sName(rName),
			      m_oTypeIdentifier(rTypeIdentifier)
			{}

			OpenViBE::CString m_sName;
			OpenViBE::CIdentifier m_oTypeIdentifier;

		} SIOStream;

		typedef struct _SSetting
		{
			_SSetting()
			    : m_sName(""),
			      m_oTypeIdentifier(OV_UndefinedIdentifier),
			      m_sDefaultValue("")
			{}

			_SSetting(const OpenViBE::CString& rName, const OpenViBE::CIdentifier& rTypeIdentifier, const OpenViBE::CString& rDefaultValue)
			    : m_sName(rName),
			      m_oTypeIdentifier(rTypeIdentifier),
			      m_sDefaultValue(rDefaultValue)
			{}

			OpenViBE::CString m_sName;
			OpenViBE::CIdentifier m_oTypeIdentifier;
			OpenViBE::CString m_sDefaultValue;

		} SSetting;

		void addInput(const OpenViBE::CString& rName, const OpenViBE::CIdentifier& rTypeIdentifier)
		{
			m_vInput.push_back(SIOStream(rName, rTypeIdentifier));
		}

		void addOutput(const OpenViBE::CString& rName, const OpenViBE::CIdentifier& rTypeIdentifier)
		{
			m_vOutput.push_back(SIOStream(rName, rTypeIdentifier));
		}

		void addSetting(const OpenViBE::CString& rName, const OpenViBE::CIdentifier& rTypeIdentifier, const OpenViBE::CString& rDefaultValue)
		{
			m_vSetting.push_back(SSetting(rName, rTypeIdentifier, rDefaultValue));
		}

		/*
		virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmDifferentialIntegralListener; }
		virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
		*/
		virtual OpenViBE::boolean getBoxPrototype(
		        OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
		{
			//for (auto& oInput : m_vInput)
			for (auto itInput = m_vInput.cbegin(); itInput != m_vInput.cend(); itInput++)
			{
				const SIOStream& oInput = *itInput;
				rBoxAlgorithmPrototype.addInput(oInput.m_sName, oInput.m_oTypeIdentifier);
			}

			//for (auto& oOutput : m_vOutput)
			for (auto itOutput = m_vOutput.cbegin(); itOutput != m_vOutput.cend(); itOutput++)
			{
				const SIOStream& oOutput = *itOutput;
				rBoxAlgorithmPrototype.addOutput(oOutput.m_sName, oOutput.m_oTypeIdentifier);
			}

			//for (auto& oSetting : m_vSetting)
			for (auto itSetting = m_vSetting.cbegin(); itSetting != m_vSetting.cend(); itSetting++)
			{
				const SSetting& oSetting = *itSetting;
				rBoxAlgorithmPrototype.addSetting(oSetting.m_sName, oSetting.m_oTypeIdentifier, oSetting.m_sDefaultValue);
			}


			return true;
		}

		_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MetaboxDesc);
	private:
		OpenViBE::CString m_sMetaboxDescriptor;

		OpenViBE::CString m_sName;
		OpenViBE::CString m_sAuthorName;
		OpenViBE::CString m_sAuthorCompanyName;
		OpenViBE::CString m_sShortDescription;
		OpenViBE::CString m_sDetailedDescription;
		OpenViBE::CString m_sCategory;
		OpenViBE::CString m_sVersion;
		OpenViBE::CString m_sStockItemName;
		OpenViBE::CString m_sAddedSoftwareVersion;
		OpenViBE::CString m_sUpdatedSoftwareVersion;

		std::vector<SIOStream> m_vInput;
		std::vector<SIOStream> m_vOutput;
		std::vector<SSetting> m_vSetting;

	};


	/*
 * Loader for metaboxes from scenarios
 */

	class CMetaboxLoader
	{
	public:
		/// a map of opened scenarios for easier access
		typedef struct _SMetaboxScenario
		{
			_SMetaboxScenario()
			{}

			void assignMetaboxDesc(const Mensia::CBoxAlgorithmMetaboxDesc& rMetaboxDesc)
			{
				m_oMetaboxDesc = rMetaboxDesc;
			}

			void assignVirtualBoxIdentifier(const char* sVirtualBoxIdentifier)
			{
				m_sVirtualBoxIdentifier = std::string(sVirtualBoxIdentifier);
			}

			void assignMetaboxScenarioPath(const char* sMetaboxScenarioPath)
			{
				m_sMetaboxScenarioPath = std::string(sMetaboxScenarioPath);
			}

			void assignMetaboxHash(const OpenViBE::CIdentifier& rHash)
			{
				m_oHash = rHash;
			}


			/// The BoxAlgorithmDesc specific to a metabox
			Mensia::CBoxAlgorithmMetaboxDesc m_oMetaboxDesc;

			/// Identifier of the box inside designer tree-view Category/Name
			std::string m_sVirtualBoxIdentifier;

			/// path to the scenario containing the description of the metabox
			std::string m_sMetaboxScenarioPath;
			// TODO_JL add information about modification time etc

			/// the hash of the metabox prototype
			OpenViBE::CIdentifier m_oHash;
		} SMetaboxScenario;

	public:
		CMetaboxLoader(OpenViBE::Kernel::IKernelContext const& rKernelContext)
		    : m_rKernelContext(rKernelContext),
		      m_rScenarioManager(rKernelContext.getScenarioManager())
		{
			// Register the default XML importer for xml metaboxes
			m_rScenarioManager.registerScenarioImporter(OVD_ScenarioimportContext_OnLoadMetaboxImport, ".mxb", OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
		}

		OpenViBE::boolean loadPluginDescriptorsFromWildcard(OpenViBE::CString const& rFileNameWildCard);
		OpenViBE::CIdentifier openScenario(const char* sFileName);
		void closeScenario(const OpenViBE::CIdentifier& rIdentifier);

		const OpenViBE::Kernel::IKernelContext& getKernelContext() const { return m_rKernelContext; }

		/// get information about one metabox using its identifier
		SMetaboxScenario& getMetaboxInfo(const std::string& rMetaboxIdentifier) { return m_mMetaboxScenarioInfo[rMetaboxIdentifier]; }

		/// map of Category/Name -> IPluginObjectDesc
		std::map<std::string, const OpenViBE::Plugins::IPluginObjectDesc*>& getPluginObjectDescMap() { return m_mPluginObjectDesc; }

		/// map of identifier -> path
		std::map<std::string, std::string> getMetaboxPathMap();

		//std::map<std::string, SMetaboxScenario>& getMetaboxInfoMap() { return m_mMetaboxScenarioInfo; }
	private:
		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IScenarioManager& m_rScenarioManager;

		/// contains virtual plugin object descriptors
		std::map<std::string, const OpenViBE::Plugins::IPluginObjectDesc*> m_mPluginObjectDesc;

		/// contains scenario information indexed by filename
		std::map<std::string, SMetaboxScenario> m_mMetaboxScenarioInfo;

	};
}


#endif //__OpenViBEKernel_Kernel_Plugins_CMetaboxLoader_H__
