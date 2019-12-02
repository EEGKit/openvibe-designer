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

		CBoxConfigurationDialog(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBE::Kernel::IBox& box, const char* sGUIFilename,
								const char* sGUISettingsFilename, const bool isScenarioRunning = false);
		virtual ~CBoxConfigurationDialog();
		virtual bool run();
		void update(OpenViBE::CObservable& o, void* data) override;

		void saveConfig() const;
		void loadConfig() const;
		void onOverrideBrowse() const;

		void storeState();
		void restoreState();

		virtual OpenViBE::CIdentifier getBoxID() const { return m_box.getIdentifier(); }
		virtual GtkWidget* getWidget() { return m_pSettingDialog; }
	protected:

		void generateSettingsTable();
		bool addSettingsToView(const uint32_t settingIndex, const uint32_t tableIndex);
		void updateSize() const;
		void settingChange(const uint32_t settingIndex);
		void addSetting(const uint32_t settingIndex);

		void clearSettingWrappersVector();
		void removeSetting(const uint32_t settingIndex, bool shift = true);

		int getTableIndex(const uint32_t settingIndex);

		const OpenViBE::Kernel::IKernelContext& m_kernelCtx;
		OpenViBE::Kernel::IBox& m_box;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;

		Setting::CSettingViewFactory m_oSettingFactory;
		std::vector<Setting::CAbstractSettingView*> m_vSettingViewVector;
		GtkTable* m_pSettingsTable           = nullptr;
		GtkViewport* m_pViewPort             = nullptr;
		GtkScrolledWindow* m_pScrolledWindow = nullptr;
		GtkEntry* m_pOverrideEntry           = nullptr;
		GtkWidget* m_pOverrideEntryContainer = nullptr;
		GtkWidget* m_pSettingDialog          = nullptr;
		GtkCheckButton* m_pFileOverrideCheck = nullptr;
		bool m_isScenarioRunning            = false;

		std::vector<OpenViBE::CString> m_SettingsMemory;
	};
} // namespace OpenViBEDesigner
