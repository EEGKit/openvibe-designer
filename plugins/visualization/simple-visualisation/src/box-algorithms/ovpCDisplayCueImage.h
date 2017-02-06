#pragma once

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include <gtk/gtk.h>

#include <vector>
#include <string>
#include <map>
#include <deque>

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		class CDisplayCueImage :
			public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CDisplayCueImage(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::uint64 getClockFrequency(void) { return (128LL << 32); }
			virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
			virtual OpenViBE::boolean process();
			virtual void redraw(void);
			virtual void resize(OpenViBE::uint32 width, OpenViBE::uint32 height);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_DisplayCueImage)

		protected:
			virtual void drawCuePicture(OpenViBE::uint32 cueID);

			//The Builder handler used to create the interface
			::GtkBuilder* m_BuilderInterface;
			::GtkWidget*  m_MainWindow;
			::GtkWidget*  m_DrawingArea;

			OpenViBEToolkit::TStimulationDecoder<CDisplayCueImage> m_StimulationDecoder;
			OpenViBEToolkit::TStimulationEncoder<CDisplayCueImage> m_StimulationEncoder;

			// For the display of the images:
			OpenViBE::boolean m_ImageRequested;        //when true: a new image must be drawn
			OpenViBE::int32   m_RequestedImageID;  //ID of the requested image. -1 => clear the screen

			OpenViBE::boolean m_ImageDrawn;            //when true: the new image has been drawn
			OpenViBE::int32   m_DrawnImageID;      //ID of the drawn image. -1 => clear the screen

			std::vector<::GdkPixbuf*> m_OriginalPicture;
			std::vector<::GdkPixbuf*> m_ScaledPicture;

			::GdkColor m_BackgroundColor;
			::GdkColor m_ForegroundColor;

			//Settings
			OpenViBE::uint32   m_NumberOfCue;
			std::vector<OpenViBE::uint64>  m_StimulationsId;
			std::vector<OpenViBE::CString> m_ImageNames;
			OpenViBE::uint64   m_ClearScreenStimulation;
			OpenViBE::boolean  m_IsFullScreen;

			//Start and end time of the last buffer
			OpenViBE::uint64 m_StartTime;
			OpenViBE::uint64 m_EndTime;
			OpenViBE::uint64 m_LastOutputChunkDate;

			//We save the received stimulations
			OpenViBE::CStimulationSet m_PendingStimulationSet;

			// Size of the window
			OpenViBE::uint32 m_StartingWidth;
			OpenViBE::uint32 m_StartingHeight;

		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_VisualizationContext;
		};

		class CDisplayCueImageListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual OpenViBE::boolean onSettingAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				OpenViBE::CString l_sDefaultName = OpenViBE::Directories::getDataDir() + "/plugins/simple-visualisation/p300-magic-card/bomberman.png";

				rBox.setSettingDefaultValue(ui32Index, l_sDefaultName.toASCIIString());
				rBox.setSettingValue(ui32Index, l_sDefaultName.toASCIIString());

				char l_sName[1024];
				sprintf(l_sName, "OVTK_StimulationId_Label_%02X", ui32Index / 2);
				rBox.addSetting("", OV_TypeId_Stimulation, l_sName);
				rBox.setSettingDefaultValue(ui32Index + 1, l_sName);
				rBox.setSettingValue(ui32Index + 1, l_sName);

				this->checkSettingNames(rBox);
				return true;
			}

			virtual OpenViBE::boolean onSettingRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				rBox.removeSetting((ui32Index / 2) * 2);
				this->checkSettingNames(rBox);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);

		private:

			OpenViBE::boolean checkSettingNames(OpenViBE::Kernel::IBox& rBox)
			{
				char l_sName[1024];
				for (OpenViBE::uint32 i = 2; i < rBox.getSettingCount() - 1; i += 2)
				{
					sprintf(l_sName, "Cue Image %i", i / 2);
					rBox.setSettingName(i, l_sName);
					rBox.setSettingType(i, OV_TypeId_Filename);
					sprintf(l_sName, "Stimulation %i", i / 2);
					rBox.setSettingName(i + 1, l_sName);
					rBox.setSettingType(i + 1, OV_TypeId_Stimulation);
				}
				return true;
			}
		};

		/**
		 * Plugin's description
		 */
		class CDisplayCueImageDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const { return OpenViBE::CString("Display cue image"); }
			virtual OpenViBE::CString getAuthorName(void) const { return OpenViBE::CString("Joan Fruitet"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const { return OpenViBE::CString("INRIA Sophia"); }
			virtual OpenViBE::CString getShortDescription(void) const { return OpenViBE::CString("Display cue images when receiving stimulations"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Display cue images when receiving specified stimulations and a fixation cross for OVTK_GDF_Cross_On_Screen"); }
			virtual OpenViBE::CString getCategory(void) const { return OpenViBE::CString("Visualization/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const { return OpenViBE::CString("1.1"); }
			virtual void release(void) {}
			virtual OpenViBE::CIdentifier getCreatedClass(void) const { return OVP_ClassId_DisplayCueImage; }

			virtual OpenViBE::CString getStockItemName(void) const { return OpenViBE::CString("gtk-fullscreen"); }
			virtual OpenViBE::Plugins::IPluginObject* create(void) { return new OpenViBEPlugins::SimpleVisualisation::CDisplayCueImage(); }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const { return new CDisplayCueImageListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Stimulations", OV_TypeId_Stimulations);
				rPrototype.addOutput("Stimulations", OV_TypeId_Stimulations);

				rPrototype.addSetting("Display images in full screen", OV_TypeId_Boolean, "false");
				rPrototype.addSetting("Clear screen Stimulation", OV_TypeId_Stimulation, "OVTK_StimulationId_VisualStimulationStop");
				rPrototype.addSetting("Cue Image 1", OV_TypeId_Filename, "${Path_Data}/plugins/simple-visualisation/p300-magic-card/mario.png");
				rPrototype.addSetting("Stimulation 1", OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				
				rPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddSetting);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_DisplayCueImageDesc)
		};
	};
};
