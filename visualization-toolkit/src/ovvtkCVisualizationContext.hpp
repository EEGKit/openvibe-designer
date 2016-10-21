#pragma once

#include <map>
#include <memory>

#include <openvibe/plugins/ovIPluginObjectDesc.h>
#include <visualization-toolkit/ovvtk_all.h>
#include <gtk/gtk.h>
#include "ovvtkIVisualizationManager.h"
#include "ovvtkIVisualizationContext.h"

#define OVP_ClassId_Plugin_VisualizationContext OpenViBE::CIdentifier(0x05A7171D, 0x78E4FE3C)
#define OVP_ClassId_Plugin_VisualizationContextDesc OpenViBE::CIdentifier(0x35A11438, 0x764F72E8)


namespace OpenViBEVisualizationToolkit
{
	/**
	 * @brief The CVisualizationContext class is a singleton used for passing visualization related information between the application
	 * and visualization plugins.
	 */
	class CVisualizationContext final : public OpenViBEVisualizationToolkit::IVisualizationContext
	{
	public:

		/**
		 * The release function is neutralized. The object is only allocated once in the descriptor as a unique_ptr
		 * and will be released at its destruction.
		 */
		void release(void) override
		{
		}

		bool setManager(OpenViBEVisualizationToolkit::IVisualizationManager* visualizationManager) override
		{
			m_VisualizationManager = visualizationManager;
			return true;
		}

		bool setWidget(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* widget) override;

		bool setToolbar(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& box, GtkWidget* toolbarWidget) override;

		bool isDerivedFromClass(const OpenViBE::CIdentifier& classIdentifier) const override
		{
			return ((classIdentifier == OVP_ClassId_Plugin_VisualizationContext)
			        || OpenViBEVisualizationToolkit::IVisualizationContext::isDerivedFromClass(classIdentifier));
		}

		OpenViBE::CIdentifier getClassIdentifier(void) const override
		{
			return OVP_ClassId_Plugin_VisualizationContext;
		}

		CVisualizationContext()
		{
		}

	private:
		OpenViBEVisualizationToolkit::IVisualizationManager* m_VisualizationManager;
	};

	class CVisualizationContextDesc final : public OpenViBE::Plugins::IPluginObjectDesc
	{
	public:

		CVisualizationContextDesc()
		    : m_VisualizationContext(new CVisualizationContext())
		{

		}

		void release(void) override
		{
		}

		OpenViBE::CString getName(void) const override { return OpenViBE::CString("Visualization Context"); }
		OpenViBE::CString getAuthorName(void) const override { return OpenViBE::CString("Jozef Legény"); }
		OpenViBE::CString getAuthorCompanyName(void) const override { return OpenViBE::CString("Mensia Technologies"); }
		OpenViBE::CString getShortDescription(void) const override { return OpenViBE::CString(""); }
		OpenViBE::CString getDetailedDescription(void) const override { return OpenViBE::CString(""); }
		OpenViBE::CString getCategory(void) const override { return OpenViBE::CString(""); }
		OpenViBE::CString getVersion(void) const override { return OpenViBE::CString("1.0"); }

		OpenViBE::CIdentifier getCreatedClass(void) const override { return OVP_ClassId_Plugin_VisualizationContext; }

		/**
		 * The create function usage is different from standard plugins. As we need to be able to pass data between
		 * the application and the plugins, we need a permanent object that can be accessed by both. We achieve this
		 * by saving the object within the plugin descriptor and returning the pointer to the same object to all
		 * plugins.
		 *
		 * @return The singleton visualizationContext object
		 */
		OpenViBE::Plugins::IPluginObject* create(void) override { return m_VisualizationContext.get(); }

		bool isDerivedFromClass(const OpenViBE::CIdentifier& classIdentifier) const override
		{
			return ((classIdentifier == OVP_ClassId_Plugin_VisualizationContextDesc)
			        || OpenViBE::Plugins::IPluginObjectDesc::isDerivedFromClass(classIdentifier));
		}

		OpenViBE::CIdentifier getClassIdentifier(void) const override
		{
			return OVP_ClassId_Plugin_VisualizationContextDesc;
		}
	private:
		std::unique_ptr<CVisualizationContext> m_VisualizationContext;
	};
}

