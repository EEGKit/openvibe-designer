#include "ovpCBoxAlgorithmMatrixDisplay.h"

#include <string>
#include <sstream>
#include <iomanip>

#include <cstdlib>
#include <cmath>
#include <system/ovCMemory.h>
#include <visualization-toolkit/ovvizColorGradient.h>

using namespace std;

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace SimpleVisualization;

#define uint64_t uint64_t

namespace
{
	void show_values_toggle_button_cb(GtkToggleToolButton* pButton, gpointer pUserData)
	{
		auto* l_pMatrixDisplay = reinterpret_cast<CBoxAlgorithmMatrixDisplay*>(pUserData);
		l_pMatrixDisplay->m_bShowValues = (gtk_toggle_tool_button_get_active(pButton) != 0);
	}

	void show_colors_toggle_button_cb(GtkToggleToolButton* pButton, gpointer pUserData)
	{
		auto* l_pMatrixDisplay = reinterpret_cast<CBoxAlgorithmMatrixDisplay*>(pUserData);
		l_pMatrixDisplay->m_bShowColors = (gtk_toggle_tool_button_get_active(pButton) != 0);
		l_pMatrixDisplay->resetColors();
	}
}  // namespace;

bool CBoxAlgorithmMatrixDisplay::resetColors()

{
	if (m_bShowColors)
	{
		//we take colors from cache and re-put it in the table
		auto it = m_vEventBoxCache.begin();

		for (; it != m_vEventBoxCache.end(); ++it)
		{
			gtk_widget_modify_bg((*it).first, GTK_STATE_NORMAL, &(*it).second);
		}
	}
	else
	{
		vector<pair<GtkWidget*, GdkColor>>::iterator it = m_vEventBoxCache.begin();

		for (; it != m_vEventBoxCache.end(); ++it)
		{
			GdkColor l_ColorWhite;
			l_ColorWhite.red = 65535;
			l_ColorWhite.green = 65535;
			l_ColorWhite.blue = 65535;
			gtk_widget_modify_bg((*it).first, GTK_STATE_NORMAL, &l_ColorWhite);
		}
	}

	return true;
}

bool CBoxAlgorithmMatrixDisplay::initialize()

{
	//IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//targets decoder
	m_pMatrixDecoder = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
	m_pMatrixDecoder->initialize();

	//IO for the targets MemoryBuffer -> StreamedMatrix
	ip_pMemoryBuffer.initialize(m_pMatrixDecoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pMatrix.initialize(m_pMatrixDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));

	//widgets
	m_pMainWidgetInterface = gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", "matrix-display-table", nullptr);
	m_pToolbarWidgetInterface = gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", "matrix-display-toolbar", nullptr);
	gtk_builder_add_from_file(m_pMainWidgetInterface, Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", nullptr);
	gtk_builder_add_from_file(m_pToolbarWidgetInterface, Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", nullptr);

	gtk_builder_connect_signals(m_pMainWidgetInterface, nullptr);
	gtk_builder_connect_signals(m_pToolbarWidgetInterface, nullptr);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pToolbarWidgetInterface, "show-values-toggle-button")), "toggled", G_CALLBACK(::show_values_toggle_button_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pToolbarWidgetInterface, "show-colors-toggle-button")), "toggled", G_CALLBACK(::show_colors_toggle_button_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pToolbarWidgetInterface, "matrix-display-toolbar")), "delete_event", G_CALLBACK(gtk_widget_hide), nullptr);

	m_pMainWidget = GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "matrix-display-table"));
	m_pToolbarWidget = GTK_WIDGET(gtk_builder_get_object(m_pToolbarWidgetInterface, "matrix-display-toolbar"));

	if (!this->canCreatePluginObject(OVP_ClassId_Plugin_VisualizationContext))
	{
		this->getLogManager() << LogLevel_Error << "Visualization framework is not loaded" << "\n";
		return false;
	}

	m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
	m_visualizationContext->setWidget(*this, m_pMainWidget);
	m_visualizationContext->setToolbar(*this, m_pToolbarWidget);

	m_bShowValues = (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pToolbarWidgetInterface, "show-values-toggle-button"))) != 0);
	m_bShowColors = (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pToolbarWidgetInterface, "show-colors-toggle-button"))) != 0);

	CString l_sColorGradientSetting;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sColorGradientSetting);
	OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(m_MatrixColorGradient, l_sColorGradientSetting);

	CString l_sGradientStepsSetting;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, l_sGradientStepsSetting);
	m_GradientSteps = strtol(l_sGradientStepsSetting, nullptr, 10);
	OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(m_MatrixInterpolatedColorGardient, m_MatrixColorGradient, m_GradientSteps);
	m_f64MaxValue = 0;
	m_f64MinValue = 0;

	CString l_sSymetricMinMaxSetting;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(2, l_sSymetricMinMaxSetting);
	m_bSymetricMinMax = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);

	CString l_sRealTimeMinMaxSetting;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3, l_sRealTimeMinMaxSetting);
	m_bRealTimeMinMax = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 3);

	return true;
}

bool CBoxAlgorithmMatrixDisplay::uninitialize()

{
	op_pMatrix.uninitialize();
	ip_pMemoryBuffer.uninitialize();

	//decoders
	m_pMatrixDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pMatrixDecoder);

	//widgets
	g_object_unref(m_pToolbarWidgetInterface);
	m_pToolbarWidgetInterface = nullptr;

	g_object_unref(m_pMainWidgetInterface);
	m_pMainWidgetInterface = nullptr;

	this->releasePluginObject(m_visualizationContext);

	return true;
}

bool CBoxAlgorithmMatrixDisplay::processInput(uint32_t /*ui32InputIndex*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

bool CBoxAlgorithmMatrixDisplay::process()

{
	IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();

	for (uint32_t i = 0; i < l_rDynamicBoxContext.getInputChunkCount(0); ++i)
	{
		ip_pMemoryBuffer = l_rDynamicBoxContext.getInputChunk(0, i);
		m_pMatrixDecoder->process();

		if (m_pMatrixDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			//header received
			//adding the event  to the window
			GtkTable* l_pTable = GTK_TABLE(gtk_builder_get_object(m_pMainWidgetInterface, "matrix-display-table"));
			uint32_t l_ui32RowCount, l_ui32ColumnCount;
			if (op_pMatrix->getDimensionCount() == 1)
			{
				//getLogManager() << LogLevel_Warning<< "The streamed matrix received has 1 dimensions (found "<< op_pMatrix->getDimensionCount() <<" dimensions)\n";
				l_ui32RowCount = 1;
				l_ui32ColumnCount = op_pMatrix->getDimensionSize(0);
				//return false;
			}
			else if (op_pMatrix->getDimensionCount() != 2)
			{
				getLogManager() << LogLevel_Error << "The streamed matrix received has more than 2 dimensions (found " << op_pMatrix->getDimensionCount() << " dimensions)\n";
				return false;
			}
			else
			{
				l_ui32RowCount = op_pMatrix->getDimensionSize(0);
				l_ui32ColumnCount = op_pMatrix->getDimensionSize(1);
			}

			gtk_table_resize(l_pTable, l_ui32RowCount + 1, l_ui32ColumnCount + 1);

			//first line : labels
			uint32_t row = 0;
			for (uint32_t c = 1; c < l_ui32ColumnCount + 1; c++)
			{
				GtkWidget* l_pWidgetLabel = gtk_label_new("");
				gtk_widget_set_visible(l_pWidgetLabel, 1);
				gtk_table_attach(l_pTable, l_pWidgetLabel, c, c + 1, row, row + 1, GtkAttachOptions(GTK_EXPAND | GTK_FILL), GtkAttachOptions(GTK_EXPAND | GTK_FILL), 0, 0);
				//g_object_unref(l_pGtkBuilderLabel);

				stringstream ss;
				ss << c;
				gtk_label_set_label(GTK_LABEL(l_pWidgetLabel), ss.str().c_str());
				m_vColumnLabelCache.emplace_back(GTK_LABEL(l_pWidgetLabel), ss.str().c_str());
			}

			//first column : labels
			uint32_t col = 0;
			for (uint32_t r = 1; r < l_ui32RowCount + 1; r++)
			{
				//::GtkBuilder* l_pGtkBuilderLabel=gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", "matrix-value-label", nullptr);
				//gtk_builder_add_from_file(l_pGtkBuilderLabel, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", nullptr);

				//::GtkWidget* l_pWidgetLabel=GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "matrix-value-label"));
				GtkWidget* l_pWidgetLabel = gtk_label_new("");
				gtk_widget_set_visible(l_pWidgetLabel, 1);
				//gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidgetLabel)), l_pWidgetLabel);
				gtk_table_attach(
					l_pTable, l_pWidgetLabel,
					col, col + 1, r, r + 1,
					GtkAttachOptions(GTK_EXPAND | GTK_FILL),
					GtkAttachOptions(GTK_EXPAND | GTK_FILL),
					0, 0);
				//g_object_unref(l_pGtkBuilderLabel);

				stringstream ss;
				ss << char(r - 1 + int('A'));
				gtk_label_set_label(GTK_LABEL(l_pWidgetLabel), ss.str().c_str());
				m_vRowLabelCache.emplace_back(GTK_LABEL(l_pWidgetLabel), ss.str().c_str());
			}

			for (uint32_t r = 1; r < l_ui32RowCount + 1; r++)
			{
				for (uint32_t c = 1; c < l_ui32ColumnCount + 1; c++)
				{
					//::GtkBuilder* l_pGtkBuilderEventBox=gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", "matrix-value-eventbox", nullptr);
					//gtk_builder_add_from_file(l_pGtkBuilderEventBox, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-MatrixDisplay.ui", nullptr);

					//::GtkWidget* l_pWidgetEventBox=GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "matrix-value-eventbox"));
					GtkWidget* l_pWidgetEventBox = gtk_event_box_new();
					gtk_widget_set_visible(l_pWidgetEventBox, 1);
					//::GtkWidget* l_pWidgetLabel=GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "matrix-value-label"));
					GtkWidget* l_pWidgetLabel = gtk_label_new("");
					gtk_widget_set_visible(l_pWidgetLabel, 1);
					gtk_container_add(GTK_CONTAINER(l_pWidgetEventBox), l_pWidgetLabel);
					//gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidgetEventBox)), l_pWidgetEventBox);
					gtk_table_attach(
						l_pTable, l_pWidgetEventBox,
						c, c + 1, r, r + 1,
						GtkAttachOptions(GTK_EXPAND | GTK_FILL),
						GtkAttachOptions(GTK_EXPAND | GTK_FILL),
						0, 0);
					//g_object_unref(l_pGtkBuilderEventBox);

					GdkColor l_ColorWhite;
					l_ColorWhite.red = 65535;
					l_ColorWhite.green = 65535;
					l_ColorWhite.blue = 65535;
					gtk_widget_modify_bg(l_pWidgetEventBox, GTK_STATE_NORMAL, &l_ColorWhite);
					m_vEventBoxCache.emplace_back(l_pWidgetEventBox, l_ColorWhite);

					gtk_label_set_label(GTK_LABEL(l_pWidgetLabel), "X");
					m_vLabelCache.emplace_back(GTK_LABEL(l_pWidgetLabel), "X");
				}
			}
		}

		if (m_pMatrixDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			//buffer received
			//2-dimension-matrix values
			uint32_t l_ui32RowCount, l_ui32ColumnCount;
			if (op_pMatrix->getDimensionCount() == 1)
			{
				l_ui32RowCount = 1;
				l_ui32ColumnCount = op_pMatrix->getDimensionSize(0);
			}
			else
			{
				l_ui32RowCount = op_pMatrix->getDimensionSize(0);
				l_ui32ColumnCount = op_pMatrix->getDimensionSize(1);
			}

			if (m_bRealTimeMinMax || // we need recompute the min max at each loop call
				(m_f64MaxValue == 0 && m_f64MinValue == 0)) // we have never computed the min max values.
			{
				if (op_pMatrix->getBufferElementCount() != 0) // if the matrix is not empty.
				{
					m_f64MaxValue = op_pMatrix->getBuffer()[0];
					m_f64MinValue = op_pMatrix->getBuffer()[0];
				}
			}

			// MIN-MAX computation
			for (uint32_t r = 0; r < l_ui32RowCount; r++)
			{
				for (uint32_t c = 0; c < l_ui32ColumnCount; c++)
				{
					double l_f64Value = op_pMatrix->getBuffer()[r * l_ui32ColumnCount + c];
					m_f64MaxValue = (l_f64Value > m_f64MaxValue ? l_f64Value : m_f64MaxValue);
					m_f64MinValue = (l_f64Value < m_f64MinValue ? l_f64Value : m_f64MinValue);

					if (m_bSymetricMinMax)
					{
						double l_f64MaxAbsValue = (fabs(m_f64MaxValue) > fabs(m_f64MinValue) ? fabs(m_f64MaxValue) : fabs(m_f64MinValue));
						m_f64MaxValue = l_f64MaxAbsValue;
						m_f64MinValue = -l_f64MaxAbsValue;
					}
				}
			}

			for (uint32_t r = 0; r < l_ui32RowCount; r++)
			{
				for (uint32_t c = 0; c < l_ui32ColumnCount; c++)
				{
					double l_f64Value = op_pMatrix->getBuffer()[r * l_ui32ColumnCount + c];
					if (m_f64MaxValue != 0 || m_f64MinValue != 0) // if the first value ever sent is 0, both are 0, and we dont want to divide by 0 :)
					{
						double l_f64Step = ((l_f64Value - m_f64MinValue) / (m_f64MaxValue - m_f64MinValue)) * (m_GradientSteps - 1);
						uint32_t l_ui32Step = uint32_t(l_f64Step);

						// gtk_widget_modify_bg uses 16bit colors, the interpolated gradients gives 8bits colors.
						GdkColor l_ColorEventBox;
						l_ColorEventBox.red = uint16_t(m_MatrixInterpolatedColorGardient[l_ui32Step * 4 + 1] * 65535. / 100.);
						l_ColorEventBox.green = uint16_t(m_MatrixInterpolatedColorGardient[l_ui32Step * 4 + 2] * 65535. / 100.);
						l_ColorEventBox.blue = uint16_t(m_MatrixInterpolatedColorGardient[l_ui32Step * 4 + 3] * 65535. / 100.);

						if (!System::Memory::compare(&(m_vEventBoxCache[r * l_ui32ColumnCount + c].second), &l_ColorEventBox, sizeof(GdkColor)) && m_bShowColors)
						{
							gtk_widget_modify_bg(m_vEventBoxCache[r * l_ui32ColumnCount + c].first, GTK_STATE_NORMAL, &l_ColorEventBox);
						}
						m_vEventBoxCache[r * l_ui32ColumnCount + c].second = l_ColorEventBox;

						std::stringstream ss;
						ss << std::fixed;
						ss << std::setprecision(2);
						if (m_bShowValues)
						{
							ss << l_f64Value;
						}

						if (ss.str() != m_vLabelCache[r * l_ui32ColumnCount + c].second)
						{
							gtk_label_set_label(m_vLabelCache[r * l_ui32ColumnCount + c].first, ss.str().c_str());
						}
						m_vLabelCache[r * l_ui32ColumnCount + c].second = ss.str();
					}
				}
			}

			if (op_pMatrix->getDimensionCount() != 1)
			{
				//first line : labels
				for (uint32_t c = 0; c < l_ui32ColumnCount; c++)
				{
					if (m_vColumnLabelCache[c].second != op_pMatrix->getDimensionLabel(1, c) && !string(op_pMatrix->getDimensionLabel(1, c)).empty())
					{
						gtk_label_set_label(GTK_LABEL(m_vColumnLabelCache[c].first), op_pMatrix->getDimensionLabel(1, c));
						m_vColumnLabelCache[c].second = op_pMatrix->getDimensionLabel(1, c);
					}
				}

				//first column : labels
				for (uint32_t r = 0; r < l_ui32RowCount; r++)
				{
					if (m_vRowLabelCache[r].second != op_pMatrix->getDimensionLabel(0, r) && !string(op_pMatrix->getDimensionLabel(0, r)).empty())
					{
						gtk_label_set_label(GTK_LABEL(m_vRowLabelCache[r].first), op_pMatrix->getDimensionLabel(0, r));
						m_vRowLabelCache[r].second = op_pMatrix->getDimensionLabel(0, r);
					}
				}
			}
			else
			{
				//first line : labels
				for (uint32_t c = 0; c < l_ui32ColumnCount; c++)
				{
					if (m_vColumnLabelCache[c].second != op_pMatrix->getDimensionLabel(0, c) && !string(op_pMatrix->getDimensionLabel(0, c)).empty())
					{
						gtk_label_set_label(GTK_LABEL(m_vColumnLabelCache[c].first), op_pMatrix->getDimensionLabel(0, c));
						m_vColumnLabelCache[c].second = op_pMatrix->getDimensionLabel(0, c);
					}
				}
			}
		}

		/*if(m_pMatrixDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd)) { }*/

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	return true;
}
