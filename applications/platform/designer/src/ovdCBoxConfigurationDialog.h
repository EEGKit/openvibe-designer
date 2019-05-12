#pragma once

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
		virtual ~CBoxConfigurationDialog();
		virtual bool run();
		virtual void update(OpenViBE::CObservable& o, void* data);

		void saveConfiguration();
		void loadConfiguration();
		void onOverrideBrowse();

		void storeState();
		void restoreState();

		virtual const OpenViBE::CIdentifier getBoxID() const;
		virtual GtkWidget* getWidget();
	protected:

		void generateSettingsTable();
		bool addSettingsToView(uint32_t ui32SettingIndex, uint32_t ui32TableIndex);
		void updateSize();
		void settingChange(uint32_t ui32SettingIndex);
		void addSetting(uint32_t settingIndex);

		void clearSettingWrappersVector();
		void removeSetting(uint32_t ui32SettingIndex, bool bShift = true);

		int32_t getTableIndex(uint32_t ui32SettingIndex);

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;

		Setting::CSettingViewFactory m_oSettingFactory;
		std::vector<Setting::CAbstractSettingView*> m_vSettingViewVector;
		GtkTable* m_pSettingsTable = nullptr;
		GtkViewport* m_pViewPort = nullptr;
		GtkScrolledWindow* m_pScrolledWindow = nullptr;
		GtkEntry* m_pOverrideEntry = nullptr;
		GtkWidget* m_pOverrideEntryContainer = nullptr;
		GtkWidget* m_pSettingDialog = nullptr;
		GtkCheckButton* m_pFileOverrideCheck = nullptr;
		bool m_bIsScenarioRunning = false;

		std::vector<OpenViBE::CString> m_SettingsMemory;
	};
};
