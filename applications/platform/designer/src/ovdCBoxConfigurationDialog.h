#ifndef __OpenViBEDesigner_CBoxConfigurationDialog_H__
#define __OpenViBEDesigner_CBoxConfigurationDialog_H__

#include "ovd_base.h"

#include <string>

#include "dynamic_settings/ovdCAbstractSettingView.h"
#include "dynamic_settings/ovdCSettingViewFactory.h"

namespace OpenViBEDesigner
{
	class CBoxConfigurationDialog : public OpenViBE::IObserver
	{
	public:

		CBoxConfigurationDialog(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox, const char* sGUIFilename, const char* sGUISettingsFilename, bool isScenarioRunning = false);
		virtual ~CBoxConfigurationDialog(void);
		virtual bool run(void);
		virtual void update(OpenViBE::CObservable &o, void* data);

		void saveConfiguration();
		void loadConfiguration();

		void storeState(void);
		void restoreState(void);

		virtual const OpenViBE::CIdentifier getBoxID() const;
		virtual ::GtkWidget* getWidget();
	protected:

		void generateSettingsTable();
		bool addSettingsToView(uint32_t ui32SettingIndex, uint32_t ui32TableIndex);
		void updateSize();
		void settingChange(uint32_t ui32SettingIndex);
		void addSetting(uint32_t ui32SettingIndex);

		void clearSettingWrappersVector(void);
		void removeSetting(uint32_t ui32SettingIndex, bool bShift = true);

		int32_t getTableIndex(uint32_t ui32SettingIndex);

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;

		Setting::CSettingViewFactory m_oSettingFactory;
		std::vector<Setting::CAbstractSettingView* > m_vSettingViewVector;
		::GtkTable *m_pSettingsTable;
		::GtkViewport *m_pViewPort;
		::GtkScrolledWindow * m_pScrolledWindow;
		::GtkWidget* m_pOverrideEntry;
		::GtkWidget* m_pSettingDialog;
		::GtkCheckButton* m_pFileOverrideCheck;
		bool m_bIsScenarioRunning;

		std::vector<OpenViBE::CString> m_SettingsMemory;
	};
};

#endif // __OpenViBEDesigner_CBoxConfigurationDialog_H__
