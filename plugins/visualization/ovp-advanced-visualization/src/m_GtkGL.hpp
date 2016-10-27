/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#ifndef __OpenViBEPlugins_GtkGL_H__
#define __OpenViBEPlugins_GtkGL_H__

#include <gtk/gtk.h>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		namespace GtkGL
		{
			void initialize(::GtkWidget* pWidget);
			void uninitialize(::GtkWidget* pWidget);

			void preRender(::GtkWidget* pWidget, bool bVerticalSync=false);
			void postRender(::GtkWidget* pWidget);
		};
	};
};

#endif // __OpenViBEPlugins_GtkGL_H__
