#pragma once

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <visualization-toolkit/ovviz_all.h>
#include <gtk/gtk.h>

#include <vector>
#include <map>

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		class CKeyboardStimulator : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			CKeyboardStimulator(void);

			virtual void release(void) { delete this; }

			virtual bool initialize();
			virtual bool uninitialize();

			virtual unsigned long long getClockFrequency() { return (32LL << 32); }

			virtual bool processClock(OpenViBE::CMessageClock &rMessageClock);

			virtual bool process();

			/**
			 * \brief Called when a key has been pressed.
			 * \param uiKey The gdk value to the pressed key.
			 */
			virtual void processKey(guint uiKey, bool bState);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_KeyboardStimulator)
		
		private:
			/**
			 * \brief Parse the configuration file and creates the Key/Stimulation associations.
			 * \param pFilename The name of the configuration file.
			 * \return True if the file was correctly parsed.
			 */
			bool parseConfigurationFile(const char* pFilename);

		private:

			OpenViBEToolkit::TStimulationEncoder<CKeyboardStimulator> m_Encoder;

			::GtkWidget* m_Widget;

			typedef struct
			{
				unsigned long long m_StimulationPress;
				unsigned long long m_StimulationRelease;
				bool m_Status;
			} SKey;

			//! Stores keyvalue/stimulation couples
			std::map<guint, SKey > m_KeyToStimulation;

			//! Vector of the stimulations to send when possible
			std::vector<unsigned long long> m_StimulationToSend;

			//! Plugin's previous activation date
			unsigned long long m_PreviousActivationTime;

		private:
			bool m_UnknownKeyPressed;
			unsigned int m_UnknownKeyCode;
			OpenViBEVisualizationToolkit::IVisualizationContext* m_VisualizationContext;
		};

		/**
		 * Plugin's description
		 */
		class CKeyboardStimulatorDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const { return OpenViBE::CString("Keyboard stimulator"); }
			virtual OpenViBE::CString getAuthorName(void) const { return OpenViBE::CString("Bruno Renier"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const { return OpenViBE::CString("Stimulation generator"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Sends stimulations according to key presses"); }
			virtual OpenViBE::CString getCategory(void) const { return OpenViBE::CString("Stimulation"); }
			virtual OpenViBE::CString getVersion(void) const { return OpenViBE::CString("0.1"); }
			virtual void release(void) {}
			virtual OpenViBE::CIdentifier getCreatedClass(void) const { return OVP_ClassId_KeyboardStimulator; }
			virtual OpenViBE::Plugins::IPluginObject* create(void) { return new OpenViBEPlugins::SimpleVisualisation::CKeyboardStimulator(); }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addOutput("Outgoing Stimulations", OV_TypeId_Stimulations);

				rPrototype.addSetting("Filename", OV_TypeId_Filename, "${Path_Data}/plugins/visualization/simple-visualisation/share/simple-keyboard-to-stimulations.txt");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_KeyboardStimulatorDesc)
		};
	};
};
