#pragma once

#include "../ovd_base.h"
#include <cstdlib>	// size_t for unix

namespace OpenViBE
{
	namespace Designer
	{
		namespace Setting
		{
			class CAbstractSettingView
			{
			public:
				virtual ~CAbstractSettingView();

				//Store the value of the setting in value
				virtual void getValue(CString& value) const = 0;
				//Set the view with the value contains in value
				virtual void setValue(const CString& value) = 0;

				//Get the label which contains the name of the setting
				virtual GtkWidget* getNameWidget() { return m_nameWidget; }
				//Get the table of widget which display the value of the setting (the entry and all interaction buttons)
				virtual GtkWidget* getEntryWidget() { return m_entryNameWidget; }

				//This function is use to update the setting index when a setting is suppressed or inserted
				virtual void setSettingIndex(const size_t index) { m_index = index; }

				//Get the index of the setting
				virtual size_t getSettingIndex() { return m_index; }

			protected:
				//Initialize the common part of all view. If builderName and widgetName are not nullptr, the entryTable and the label
				//will be set according to these informations.
				//If there are nullptr, name and entry widget will have to be set after with corresponding setter.
				CAbstractSettingView(Kernel::IBox& box, const size_t index, const char* builderName, const char* widgetName);

				//Return the box which contains the setting
				virtual Kernel::IBox& getBox() { return m_box; }


				//Set the widget as the new widget name
				virtual void setNameWidget(GtkWidget* widget);
				//Set the widget as the new widget name
				virtual void setEntryWidget(GtkWidget* widget);

				//Set the setting view with the current value of the setting
				virtual void initializeValue();

				//Return a vector which contains the list of the widget contains int the widget widget
				virtual void extractWidget(GtkWidget* widget, std::vector<GtkWidget *>& widgets);

				//Return the part of the entry table which is not revert or default button
				virtual GtkWidget* getEntryFieldWidget() { return m_entryFieldWidget; }

			private:
				//Generate the label of the setting
				virtual void generateNameWidget();
				//Generate the table of widget which display the value of the setting (the entry and all interaction buttons)
				virtual GtkWidget* generateEntryWidget();


				Kernel::IBox& m_box;
				size_t m_index = 0;
				CString m_settingWidgetName;
				GtkWidget* m_nameWidget       = nullptr;
				GtkWidget* m_entryNameWidget  = nullptr;
				GtkWidget* m_entryFieldWidget = nullptr;

				//If we don't store the builder, the setting name will be free when we'll unref the builder
				GtkBuilder* m_builder = nullptr;
			};
		} // namespace Setting
	}  // namespace Designer
}  // namespace OpenViBE
