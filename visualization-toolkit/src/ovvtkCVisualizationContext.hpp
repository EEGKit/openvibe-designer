#pragma once

#include <map>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovvtk_all.h>
#include <gtk/gtk.h>
#include "ovvtkIVisualizationManager.h"
#include "ovvtkIVisualizationContext.h"

#define OVP_ClassId_Plugin_VisualizationContext OpenViBE::CIdentifier(0x05A7171D, 0x78E4FE3C)
#define OVP_ClassId_Plugin_VisualizationContextDesc OpenViBE::CIdentifier(0x35A11438, 0x764F72E8)


namespace OpenViBEToolkit
{
	class CVisualizationContext final : public OpenViBEVisualizationToolkit::IVisualizationContext
	{
	public:

		void release(void) override
		{
		}

		bool setManager(OpenViBEVisualizationToolkit::IVisualisationManager* visualizationManager) override
		{
			m_visualizationManager = visualizationManager;
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
		    : data(0)
		{
		}

	private:
		int data;
		OpenViBEVisualizationToolkit::IVisualisationManager* m_visualizationManager;
	};

	class CVisualizationContextDesc final : public OpenViBE::Plugins::IPluginObjectDesc
	{
	public:

		CVisualizationContextDesc()
		    : visualizationContext(new CVisualizationContext())
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
		OpenViBE::Plugins::IPluginObject* create(void) override { return visualizationContext.get(); }

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
		std::unique_ptr<CVisualizationContext> visualizationContext;
	};
}

