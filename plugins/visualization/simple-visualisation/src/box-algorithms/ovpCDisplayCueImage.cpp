#include "ovpCDisplayCueImage.h"

#include <cmath>
#include <iostream>
#include <cstdlib>

#include <algorithm>

#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
#include <unistd.h>
#endif

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		gboolean DisplayCueImage_SizeAllocateCallback(GtkWidget *widget, GtkAllocation *allocation, gpointer data)
		{
			reinterpret_cast<CDisplayCueImage*>(data)->resize((uint32)allocation->width, (uint32)allocation->height);
			return FALSE;
		}

		gboolean DisplayCueImage_RedrawCallback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
		{
			reinterpret_cast<CDisplayCueImage*>(data)->redraw();
			return TRUE;
		}

		CDisplayCueImage::CDisplayCueImage(void) :
			m_BuilderInterface(NULL),
			m_MainWindow(NULL),
			m_DrawingArea(NULL),
			m_ImageRequested(false),
			m_RequestedImageID(-1),
			m_ImageDrawn(false),
			m_DrawnImageID(-1),
			m_IsFullScreen(false),
			m_LastOutputChunkDate(0)
		{
			m_BackgroundColor.pixel = 0;
			m_BackgroundColor.red = 0;
			m_BackgroundColor.green = 0;
			m_BackgroundColor.blue = 0;

			m_ForegroundColor.pixel = 0;
			m_ForegroundColor.red = 0xFFFF;
			m_ForegroundColor.green = 0xFFFF;
			m_ForegroundColor.blue = 0xFFFF;
		}

		boolean CDisplayCueImage::initialize()
		{
			//>>>> Reading Settings:

			//Number of Cues:
			m_NumberOfCue = getStaticBoxContext().getSettingCount() / 2 - 1;

			//Do we display the images in full screen?
			m_IsFullScreen = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);

			//Clear screen stimulation:
			m_ClearScreenStimulation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

			//Stimulation ID and images file names for each cue
			m_ImageNames.resize(m_NumberOfCue);
			m_StimulationsId.resize(m_NumberOfCue);

			for (uint32 i = 0; i < m_NumberOfCue; i++)
			{
				m_ImageNames[i] = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2 * i + 2);
				m_StimulationsId[i] = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2 * i + 3);
			}

			//>>>> Initialisation
			m_StimulationDecoder.initialize(*this, 0);
			m_StimulationEncoder.initialize(*this, 0);

			//load the gtk builder interface
			m_BuilderInterface = gtk_builder_new();
			gtk_builder_add_from_file(m_BuilderInterface, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualisation/openvibe-simple-visualisation-DisplayCueImage.ui", NULL);

			OV_ERROR_UNLESS_KRF(
				m_BuilderInterface,
				"Couldn't load the interface",
				ErrorType::ResourceNotFound);

			gtk_builder_connect_signals(m_BuilderInterface, NULL);

			m_DrawingArea = GTK_WIDGET(gtk_builder_get_object(m_BuilderInterface, "DisplayCueImageDrawingArea"));
			g_signal_connect(G_OBJECT(m_DrawingArea), "expose_event", G_CALLBACK(DisplayCueImage_RedrawCallback), this);
			g_signal_connect(G_OBJECT(m_DrawingArea), "size-allocate", G_CALLBACK(DisplayCueImage_SizeAllocateCallback), this);

			//set widget bg color
			gtk_widget_modify_bg(m_DrawingArea, GTK_STATE_NORMAL, &m_BackgroundColor);
			gtk_widget_modify_bg(m_DrawingArea, GTK_STATE_PRELIGHT, &m_BackgroundColor);
			gtk_widget_modify_bg(m_DrawingArea, GTK_STATE_ACTIVE, &m_BackgroundColor);

			gtk_widget_modify_fg(m_DrawingArea, GTK_STATE_NORMAL, &m_ForegroundColor);
			gtk_widget_modify_fg(m_DrawingArea, GTK_STATE_PRELIGHT, &m_ForegroundColor);
			gtk_widget_modify_fg(m_DrawingArea, GTK_STATE_ACTIVE, &m_ForegroundColor);

			//Load the pictures:
			m_OriginalPicture.resize(m_NumberOfCue, NULL);
			m_ScaledPicture.resize(m_NumberOfCue, NULL);

			m_StartingWidth = 64;
			m_StartingHeight = 64;

			for (uint32 i = 0; i < m_NumberOfCue; i++)
			{
				m_OriginalPicture[i] = gdk_pixbuf_new_from_file_at_size(m_ImageNames[i].toASCIIString(), -1, -1, NULL);

				OV_ERROR_UNLESS_KRF(m_OriginalPicture[i] != NULL,
									"Error couldn't load ressource file: " << m_ImageNames[i],
									ErrorType::ResourceNotFound);

				const uint32 l_ui32ImageWidth = gdk_pixbuf_get_width(m_OriginalPicture[i]);
				const uint32 l_ui32ImageHeight = gdk_pixbuf_get_height(m_OriginalPicture[i]);

				m_StartingWidth = (l_ui32ImageWidth > m_StartingWidth) ? l_ui32ImageWidth : m_StartingWidth;
				m_StartingHeight = (l_ui32ImageHeight > m_StartingHeight) ? l_ui32ImageHeight : m_StartingHeight;

				m_ScaledPicture[i] = 0;
			}

			gtk_widget_set_size_request(m_DrawingArea, m_StartingWidth, m_StartingHeight);

			m_VisualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
			m_VisualizationContext->setWidget(*this, m_DrawingArea);

			return true;
		}

		boolean CDisplayCueImage::uninitialize()
		{
			m_StimulationDecoder.uninitialize();
			m_StimulationEncoder.uninitialize();

			//destroy drawing area
			if (m_DrawingArea)
			{
				gtk_widget_destroy(m_DrawingArea);
				m_DrawingArea = NULL;
			}

			// unref the xml file as it's not needed anymore
			if (m_BuilderInterface)
			{
				g_object_unref(G_OBJECT(m_BuilderInterface));
				m_BuilderInterface = NULL;
			}

			m_StimulationsId.clear();
			m_ImageNames.clear();

			for (::GdkPixbuf* originalPicture : m_OriginalPicture)
			{
				if (originalPicture)
				{
					g_object_unref(G_OBJECT(originalPicture));
				}
			}

			m_OriginalPicture.clear();

			for (::GdkPixbuf* scaledPicture : m_ScaledPicture)
			{
				if (scaledPicture)
				{
					g_object_unref(G_OBJECT(scaledPicture));
				}
			}

			m_ScaledPicture.clear();

			return true;
		}

		boolean CDisplayCueImage::processClock(CMessageClock& rMessageClock)
		{
			IBoxIO* l_pBoxIO = getBoxAlgorithmContext()->getDynamicBoxContext();
			m_StimulationEncoder.getInputStimulationSet()->clear();

			if (m_ImageDrawn)
			{
				// this is first redraw() for that image or clear screen
				// we send a stimulation to signal it.

				if (m_DrawnImageID >= 0)
				{
					// it was a image
					m_StimulationEncoder.getInputStimulationSet()->appendStimulation(
						m_StimulationsId[m_DrawnImageID],
						this->getPlayerContext().getCurrentTime(),
						0);
				}
				else
				{
					// it was a clear_screen
					m_StimulationEncoder.getInputStimulationSet()->appendStimulation(
						m_ClearScreenStimulation,
						this->getPlayerContext().getCurrentTime(),
						0);
				}

				m_ImageDrawn = false;

				if (m_DrawnImageID != m_RequestedImageID)
				{
					// We must be late...
					OV_WARNING_K("One image may have been skipped => we must be late...");
				}
			}

			m_StimulationEncoder.encodeBuffer();
			l_pBoxIO->markOutputAsReadyToSend(0, m_LastOutputChunkDate, this->getPlayerContext().getCurrentTime());
			m_LastOutputChunkDate = this->getPlayerContext().getCurrentTime();

			// We check if some images must be display
			for (uint64 stim = 0; stim < m_PendingStimulationSet.getStimulationCount();)
			{
				const uint64 stimDate = m_PendingStimulationSet.getStimulationDate(stim);
				const uint64 time = this->getPlayerContext().getCurrentTime();

				if (stimDate < time)
				{
					float delay = static_cast<float>(((time - stimDate) >> 16) / 65.5360); //delay in ms

					if (delay > 50.f)
					{
						OV_WARNING_K("Image was late: " << delay << " ms");
					}

					const uint64 stimID = m_PendingStimulationSet.getStimulationIdentifier(stim);

					if (stimID == m_ClearScreenStimulation)
					{
						if (m_ImageRequested)
						{
							OV_WARNING_K("One image was skipped => Not enough time between two images!");
						}

						m_ImageRequested = true;
						m_RequestedImageID = -1;
					}
					else
					{
						for (uint32 i = 0; i <= m_NumberOfCue; i++)
						{
							if (stimID == m_StimulationsId[i])
							{
								if (m_ImageRequested)
								{
									OV_WARNING_K("One image was skipped => Not enough time between two images!");
								}

								m_ImageRequested = true;
								m_RequestedImageID = i;
								break;
							}
						}
					}

					m_PendingStimulationSet.removeStimulation(stim);

					if (GTK_WIDGET(m_DrawingArea)->window)
					{
						gdk_window_invalidate_rect(GTK_WIDGET(m_DrawingArea)->window, NULL, true);
						// it will trigger the callback redraw()
					}
				}
				else
				{
					stim++;
				}
			}

			return true;
		}

		boolean CDisplayCueImage::processInput(uint32 inputIndex)
		{
			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}

		boolean CDisplayCueImage::process()
		{
			IBoxIO* boxIO = getBoxAlgorithmContext()->getDynamicBoxContext();

			// We decode and save the received stimulations.
			for (uint32 input = 0; input < getBoxAlgorithmContext()->getStaticBoxContext()->getInputCount(); input++)
			{
				for (uint32 chunk = 0; chunk < boxIO->getInputChunkCount(input); chunk++)
				{
					m_StimulationDecoder.decode(chunk, true);

					if (m_StimulationDecoder.isHeaderReceived())
					{
						m_LastOutputChunkDate = this->getPlayerContext().getCurrentTime();
						m_StimulationEncoder.encodeHeader();
						boxIO->markOutputAsReadyToSend(0, 0, m_LastOutputChunkDate);
					}

					if (m_StimulationDecoder.isBufferReceived())
					{
						for (uint32 stim = 0; stim < m_StimulationDecoder.getOutputStimulationSet()->getStimulationCount(); stim++)
						{
							const uint64 stimID = m_StimulationDecoder.getOutputStimulationSet()->getStimulationIdentifier(stim);
							boolean addStim = false;

							if (stimID == m_ClearScreenStimulation)
							{
								addStim = true;
							}
							else
							{
								if (std::find(m_StimulationsId.begin(), m_StimulationsId.end(), stimID) != std::end(m_StimulationsId))
								{
									addStim = true;
								}
							}

							if (addStim)
							{
								const uint64 stimDate = m_StimulationDecoder.getOutputStimulationSet()->getStimulationDate(stim);
								const uint64 stimDuration = m_StimulationDecoder.getOutputStimulationSet()->getStimulationDuration(stim);
								const uint64 time = this->getPlayerContext().getCurrentTime();

								if (stimDate < time)
								{
									float delay = static_cast<float>(((time - stimDate) >> 16) / 65.5360); //delay in ms

									if (delay > 50.f)
									{
										OV_WARNING_K("Stimulation was received late: " << delay << " ms");
									}
								}

								if (stimDate < boxIO->getInputChunkStartTime(input, chunk))
								{
									OV_WARNING_K("Input Stimulation Date before beginning of the buffer");
								}

								m_PendingStimulationSet.appendStimulation(
									stimID,
									stimDate,
									stimDuration);
							}
						}
					}

					boxIO->markInputAsDeprecated(input, chunk);
				}
			}

			return true;
		}

		//Callback called by GTK
		void CDisplayCueImage::redraw()
		{
			if (m_RequestedImageID >= 0)
			{
				drawCuePicture(m_RequestedImageID);
			}

			if (m_ImageRequested)
			{
				m_ImageRequested = false;
				m_ImageDrawn = true;
				m_DrawnImageID = m_RequestedImageID;
			}
		}

		void CDisplayCueImage::drawCuePicture(OpenViBE::uint32 cueID)
		{
			if (m_IsFullScreen)
			{
				gdk_draw_pixbuf(m_DrawingArea->window, NULL, m_ScaledPicture[cueID], 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			}
			else
			{
				gint x = (m_DrawingArea->allocation.width / 2) - gdk_pixbuf_get_width(m_ScaledPicture[cueID]) / 2;
				gint y = (m_DrawingArea->allocation.height / 2) - gdk_pixbuf_get_height(m_ScaledPicture[cueID]) / 2;;
				gdk_draw_pixbuf(m_DrawingArea->window, NULL, m_ScaledPicture[cueID], 0, 0, x, y, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			}
		}

		void CDisplayCueImage::resize(uint32 width, uint32 height)
		{
			for (uint32 i = 0; i < m_NumberOfCue; i++)
			{
				if (m_ScaledPicture[i])
				{
					g_object_unref(G_OBJECT(m_ScaledPicture[i]));
				}
			}

			if (m_IsFullScreen)
			{
				for (uint32 i = 0; i < m_NumberOfCue; i++)
				{
					m_ScaledPicture[i] = gdk_pixbuf_scale_simple(m_OriginalPicture[i], width, height, GDK_INTERP_BILINEAR);
				}
			}
			else
			{
				for (uint32 i = 0; i < m_NumberOfCue; i++)
				{
					m_ScaledPicture[i] = gdk_pixbuf_scale_simple(m_OriginalPicture[i],
																 gdk_pixbuf_get_width(m_OriginalPicture[i]),
																 gdk_pixbuf_get_height(m_OriginalPicture[i]),
																 GDK_INTERP_BILINEAR);
				}
			}
		}
	};
};
