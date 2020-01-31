#pragma once

#include "ovd_base.h"

#include <string>

#include "dynamic_settings/ovdCAbstractSettingView.h"
#include "dynamic_settings/ovdCSettingViewFactory.h"

namespace OpenViBE
{
	namespace Designer
	{
		class CBoxConfigurationDialog final : public IObserver
		{
		public:

			CBoxConfigurationDialog(const Kernel::IKernelContext& ctx, Kernel::IBox& box, const char* guiFilename,
									const char* guiSettingsFilename, const bool isScenarioRunning = false);
			~CBoxConfigurationDialog() override;

			bool run();
			void update(CObservable& o, void* data) override;

			void saveConfig() const;
			void loadConfig() const;
			void onOverrideBrowse() const;

			void storeState();
			void restoreState();

			CIdentifier getBoxID() const { return m_box.getIdentifier(); }
			GtkWidget* getWidget() const { return m_settingDialog; }
		protected:

			void generateSettingsTable();
			bool addSettingsToView(const size_t settingIdx, const size_t tableIdx);
			void updateSize() const;
			void settingChange(const size_t index);
			void addSetting(const size_t index);
			void removeSetting(const size_t index, bool shift = true);
			int getTableIndex(const size_t index);

			const Kernel::IKernelContext& m_kernelCtx;
			Kernel::IBox& m_box;
			CString m_guiFilename;
			CString m_guiSettingsFilename;

			Setting::CSettingViewFactory m_settingFactory;
			std::vector<Setting::CAbstractSettingView*> m_settingViews;
			GtkTable* m_settingsTable           = nullptr;
			GtkViewport* m_viewPort             = nullptr;
			GtkScrolledWindow* m_scrolledWindow = nullptr;
			GtkEntry* m_overrideEntry           = nullptr;
			GtkWidget* m_overrideEntryContainer = nullptr;
			GtkWidget* m_settingDialog          = nullptr;
			GtkCheckButton* m_fileOverrideCheck = nullptr;
			bool m_isScenarioRunning            = false;

			std::vector<CString> m_settingsMemory;
		};
	}  // namespace Designer
}  // namespace OpenViBE
