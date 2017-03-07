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
	namespace SimpleVisualization
	{
		class CBoxAlgorithmDisplayCueImage :
			public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CBoxAlgorithmDisplayCueImage(void);

			virtual void release(void) { delete this; }

			virtual bool initialize();
			virtual bool uninitialize();
			virtual bool processInput(unsigned intui32InputIndex);
			virtual uint64_t getClockFrequency(void) { return (128LL << 32); }
			virtual bool processClock(OpenViBE::CMessageClock& rMessageClock);
			virtual bool process();
			virtual void redraw(void);
			virtual void resize(unsigned int width, unsigned int height);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_DisplayCueImage)

		protected:
			virtual bool drawCuePicture(unsigned int cueID);

			//The Builder handler used to create the interface
			::GtkBuilder* m_BuilderInterface;
			::GtkWidget*  m_MainWindow;
			::GtkWidget*  m_DrawingArea;

			OpenViBEToolkit::TStimulationDecoder<CBoxAlgorithmDisplayCueImage> m_StimulationDecoder;
			OpenViBEToolkit::TStimulationEncoder<CBoxAlgorithmDisplayCueImage> m_StimulationEncoder;

			// For the display of the images:
			bool m_ImageRequested;        //when true: a new image must be drawn
			int m_RequestedImageID;  //ID of the requested image. -1 => clear the screen

			bool m_ImageDrawn;            //when true: the new image has been drawn
			int m_DrawnImageID;      //ID of the drawn image. -1 => clear the screen

			std::vector<::GdkPixbuf*> m_OriginalPicture;
			std::vector<::GdkPixbuf*> m_ScaledPicture;

			::GdkColor m_BackgroundColor;
			::GdkColor m_ForegroundColor;

			//Settings
			unsigned int   m_NumberOfCue;
			std::vector<uint64_t>  m_StimulationsId;
			std::vector<OpenViBE::CString> m_ImageNames;
			uint64_t  m_ClearScreenStimulation;
			bool m_IsFullScreen;

			//Start and end time of the last buffer
			uint64_t m_StartTime;
			uint64_t m_EndTime;
			uint64_t m_LastOutputChunkDate;

			//We save the received stimulations
			OpenViBE::CStimulationSet m_PendingStimulationSet;

			// Size of the window
			unsigned int m_StartingWidth;
			unsigned int m_StartingHeight;

		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_VisualizationContext;
		};

		class CBoxAlgorithmDisplayCueImageListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual bool onSettingAdded(OpenViBE::Kernel::IBox& box, const unsigned int index)
			{
				OpenViBE::CString defaultName = OpenViBE::Directories::getDataDir() + "/scenarios/box-tutorials/openvibe-logo.png";

				box.setSettingDefaultValue(index, defaultName.toASCIIString());
				box.setSettingValue(index, defaultName.toASCIIString());

				char name[1024];
				sprintf(name, "OVTK_StimulationId_Label_%02X", index / 2);
				box.addSetting("", OV_TypeId_Stimulation, name);
				box.setSettingDefaultValue(index + 1, name);
				box.setSettingValue(index + 1, name);

				this->checkSettingNames(box);
				return true;
			}

			virtual bool onSettingRemoved(OpenViBE::Kernel::IBox& box, const unsigned int index)
			{
				box.removeSetting((index / 2) * 2);
				this->checkSettingNames(box);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);

		private:

			bool checkSettingNames(OpenViBE::Kernel::IBox& box)
			{
				char name[1024];
				for (unsigned int i = 2; i < box.getSettingCount() - 1; i += 2)
				{
					sprintf(name, "Cue Image %i", i / 2);
					box.setSettingName(i, name);
					box.setSettingType(i, OV_TypeId_Filename);
					sprintf(name, "Stimulation %i", i / 2);
					box.setSettingName(i + 1, name);
					box.setSettingType(i + 1, OV_TypeId_Stimulation);
				}
				return true;
			}
		};

		/**
		 * Plugin's description
		 */
		class CBoxAlgorithmDisplayCueImageDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
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
			virtual OpenViBE::Plugins::IPluginObject* create(void) { return new OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmDisplayCueImage(); }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const { return new CBoxAlgorithmDisplayCueImageListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}

			virtual bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Stimulations", OV_TypeId_Stimulations);
				rPrototype.addOutput("Stimulations", OV_TypeId_Stimulations);

				rPrototype.addSetting("Display images in full screen", OV_TypeId_Boolean, "false");
				rPrototype.addSetting("Clear screen Stimulation", OV_TypeId_Stimulation, "OVTK_StimulationId_VisualStimulationStop");
				rPrototype.addSetting("Cue Image 1", OV_TypeId_Filename, "${Path_Data}/scenarios/box-tutorials/openvibe-logo.png");
				rPrototype.addSetting("Stimulation 1", OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				
				rPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddSetting);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_DisplayCueImageDesc)
		};
	};
};
