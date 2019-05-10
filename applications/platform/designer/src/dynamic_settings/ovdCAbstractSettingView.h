#ifndef __OpenViBE_Designer_Setting_CAbstractSettingView_H__
#define __OpenViBE_Designer_Setting_CAbstractSettingView_H__

#include "../ovd_base.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CAbstractSettingView
		{
		public:
			virtual ~CAbstractSettingView();

			//Store the value of the setting in rValue
			virtual void getValue(OpenViBE::CString& rValue) const = 0;
			//Set the view with the value contains in rValue
			virtual void setValue(const OpenViBE::CString& rValue) = 0;

			//Get the label which contains the name of the setting
			virtual GtkWidget* getNameWidget();
			//Get the table of widget which display the value of the setting (the entry and all interaction buttons)
			virtual GtkWidget* getEntryWidget();

			//This function is use to update the setting index when a setting is suppressed or inserted
			virtual void setSettingIndex(uint32_t m_ui32NewIndex);
			//Get the index of the setting
			virtual uint32_t getSettingIndex();

		protected:
			//Initialize the common part of all view. If sBuilderName and sWidgetName are not nullptr, the entryTable and the label
			//will be set according to these informations.
			//If there are nullptr, name and entry widget will have to be set after with corresponding setter.
			CAbstractSettingView(OpenViBE::Kernel::IBox& rBox, uint32_t ui32Index, const char* sBuilderName, const char* sWidgetName);

			//Return the box which contains the setting
			virtual OpenViBE::Kernel::IBox& getBox();

			//Set the pWidget as the new widget name
			virtual void setNameWidget(GtkWidget* pWidget);
			//Set the pWidget as the new widget name
			virtual void setEntryWidget(GtkWidget* pWidget);

			//Set the setting view with the current value of the setting
			virtual void initializeValue();

			//Return a vector which contains the list of the widget contains int the widget pWidget
			virtual void extractWidget(GtkWidget* pWidget, std::vector<GtkWidget *>& rVector);

			//Return the part of the entry table which is not revert or default button
			virtual GtkWidget* getEntryFieldWidget();

		private:
			//Generate the label of the setting
			virtual void generateNameWidget();
			//Generate the table of widget which display the value of the setting (the entry and all interaction buttons)
			virtual GtkWidget* generateEntryWidget();


			OpenViBE::Kernel::IBox& m_rBox;
			uint32_t m_ui32Index = 0;
			OpenViBE::CString m_sSettingWidgetName;
			GtkWidget* m_pNameWidget = nullptr;
			GtkWidget* m_pEntryNameWidget = nullptr;
			GtkWidget* m_pEntryFieldWidget = nullptr;

			//If we don't store the builder, the setting name will be free when we'll unref the builder
			GtkBuilder* m_pBuilder = nullptr;
			bool m_bOnValueSetting = false;
		};
	}
}

#endif // __OpenViBE_Designer_Setting_CAbstractSettingView_H__
