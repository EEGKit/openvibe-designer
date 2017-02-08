#include "ovpCKeyboardStimulator.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <fs/Files.h>

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace OpenViBEToolkit;

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		/**
		 * \brief Called when a key is pressed on the keyboard.
		 */
		gboolean KeyboardStimulator_KeyPressCallback(GtkWidget* widget, GdkEventKey* thisEvent, gpointer data)
		{
			reinterpret_cast<CKeyboardStimulator*>(data)->processKey(thisEvent->keyval, true);
			return true;
		}

		/**
		 * \brief Called when a key is released on the keyboard
		 */
		gboolean KeyboardStimulator_KeyReleaseCallback(GtkWidget* widget, GdkEventKey* thisEvent, gpointer data)
		{
			reinterpret_cast<CKeyboardStimulator*>(data)->processKey(thisEvent->keyval, false);
			return true;
		}

		void CKeyboardStimulator::processKey(guint key, bool state)
		{
			//if there is one entry, adds the stimulation to the list of stims to be sent
			if (m_KeyToStimulation.count(key) != 0 && state != m_KeyToStimulation[key].m_Status)
			{
				if (state)
				{
					m_StimulationToSend.push_back(m_KeyToStimulation[key].m_StimulationPress);
				}
				else
				{
					m_StimulationToSend.push_back(m_KeyToStimulation[key].m_StimulationRelease);
				}

				m_KeyToStimulation[key].m_Status = state;
			}
			else
			{
				m_UnknownKeyPressed = true;
				m_UnknownKeyCode = static_cast<uint32>(key);
			}
		}

		bool CKeyboardStimulator::parseConfigurationFile(const char * filename)
		{
			std::ifstream file;
			FS::Files::openIFStream(file, filename);

			OV_ERROR_UNLESS_KRF(
				file.is_open(),
				"Configuration file not found: " << filename,
				ErrorType::BadFileRead);

			//reads all the couples key name/stim
			while (!file.eof() && !file.fail())
			{
				std::string keyName;
				std::string stimulationPress;
				std::string stimulationRelease;

				file >> keyName >> stimulationPress >> stimulationRelease;

				SKey key{ 0, 0, false };

				// MAY CAUSE ENDIANNESS PROBLEMS !
				sscanf(stimulationPress.c_str(), "0x%08Lx", &key.m_StimulationPress);
				sscanf(stimulationRelease.c_str(), "0x%08Lx", &key.m_StimulationRelease);

				m_KeyToStimulation[gdk_keyval_from_name(keyName.c_str())] = key;
			}

			file.close();

			return true;
		}

		CKeyboardStimulator::CKeyboardStimulator(void) :
			m_Widget(NULL),
			m_PreviousActivationTime(0),
			m_UnknownKeyPressed(false),
			m_UnknownKeyCode(0),
			m_VisualizationContext(nullptr)
		{
		}

		bool CKeyboardStimulator::initialize()
		{
			const CString fileName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);

			OV_ERROR_UNLESS_KRF(
				parseConfigurationFile(fileName.toASCIIString()), 
				"Problem while parsing configuration file", 
				ErrorType::BadFileParsing);

			std::string userInterfacePath = std::string(OpenViBE::Directories::getDataDir().toASCIIString()) + "/plugins/simple-visualization/keyboard-stimulator.ui";
			
			OV_ERROR_UNLESS_KRF(
				FS::Files::fileExists(userInterfacePath.c_str()),
				"User interface file not found: " << userInterfacePath.c_str(),
				ErrorType::BadFileRead);

			m_Encoder.initialize(*this, 0);

			const std::string redColorCode("#602020");
			const std::string greenColorCode("#206020");
			const std::string blueColorCode("#202060");

			std::stringstream ss;
			ss << "\nUse your keyboard to send stimulations\nAvailable keys are :\n\n";

			for (std::map<guint, SKey>::const_iterator i = m_KeyToStimulation.begin(); i != m_KeyToStimulation.end(); i++)
			{
				ss << "<span size=\"smaller\">\t";
				ss << "<span style=\"italic\" foreground=\"" << greenColorCode << "\">" << gdk_keyval_name(i->first) << "</span>";
				ss << "\t";
				ss << "Pressed : <span style=\"italic\" foreground=\"" << blueColorCode << "\">" << this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, i->second.m_StimulationPress) << "</span>";
				ss << "\t";
				ss << "Released : <span style=\"italic\" foreground=\"" << blueColorCode << "\">" << this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, i->second.m_StimulationRelease) << "</span>";
				ss << "\t</span>\n";
			}

			::GtkBuilder* l_pBuilder = gtk_builder_new();
			
			gtk_builder_add_from_file(l_pBuilder, userInterfacePath.c_str(), NULL);
			gtk_builder_connect_signals(l_pBuilder, NULL);

			m_Widget = GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "keyboard_stimulator-eventbox"));

			OV_ERROR_UNLESS_KRF(
				m_Widget != NULL,
				"Failed to create user interface",
				ErrorType::BadFileRead);

			gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(l_pBuilder, "keyboard_stimulator-label")), ss.str().c_str());

			g_signal_connect(m_Widget, "key-press-event", G_CALLBACK(KeyboardStimulator_KeyPressCallback), this);
			g_signal_connect(m_Widget, "key-release-event", G_CALLBACK(KeyboardStimulator_KeyReleaseCallback), this);
			g_object_unref(l_pBuilder);

			m_VisualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
			m_VisualizationContext->setWidget(*this, m_Widget);

			m_Encoder.encodeHeader();

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, 0, 0);

			return true;
		}

		bool CKeyboardStimulator::uninitialize()
		{
			m_Encoder.uninitialize();

			if (m_VisualizationContext != nullptr)
			{
				this->releasePluginObject(m_VisualizationContext);
				m_VisualizationContext = nullptr;
			}

			if (m_Widget != nullptr)
			{
				g_object_unref(m_Widget);
				m_Widget = NULL;
			}

			return true;
		}

		bool CKeyboardStimulator::processClock(CMessageClock& messageClock)
		{
			if (m_UnknownKeyPressed)
			{
				OV_WARNING_K("Unhandled key code " << m_UnknownKeyCode);
				m_UnknownKeyPressed = false;
			}

			const uint64 currentTime = messageClock.getTime();

			if (currentTime != m_PreviousActivationTime)
			{
				IBoxIO* boxIO = getBoxAlgorithmContext()->getDynamicBoxContext();

				IStimulationSet* stimulationSet = m_Encoder.getInputStimulationSet();
				stimulationSet->clear(); // The encoder may retain the buffer from the previous round, clear it

				for (const unsigned long long stimulation : m_StimulationToSend)
				{
					stimulationSet->appendStimulation(stimulation, currentTime, 0);
				}

				m_StimulationToSend.clear();

				m_Encoder.encodeBuffer();

				boxIO->markOutputAsReadyToSend(0, m_PreviousActivationTime, currentTime);
				getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			}

			m_PreviousActivationTime = currentTime;
			return true;
		}

		bool CKeyboardStimulator::process()
		{
			return true;
		}
	};
};
