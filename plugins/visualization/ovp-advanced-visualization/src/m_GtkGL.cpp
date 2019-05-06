/*********************************************************************
 * Software License Agreement (AGPL-3 License)
 *
 * OpenViBE Designer
 * Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
 * Copyright (C) Inria, 2015-2017,V1.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "m_GtkGL.hpp"
#include "m_defines.hpp"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#if defined TARGET_OS_Windows
#include <windows.h>
#include <gdk/gdkwin32.h>
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
#include <gdk/gdkx.h>
#include <GL/glx.h>
#else
#error unsupported platform
#endif

// ###########################################################################################################################################################
// ###########################################################################################################################################################
//
// GtkGL implementation
//
// ###########################################################################################################################################################
// ###########################################################################################################################################################

#define GtkGL_RenderingContextName "GL Rendering Context"
#define GtkGL_DeviceContextName "Device Context"
#define GtkGL_Debug(s) // g_debug("GtkGL : "#s);
#define GtkGL_Warning(s) g_warning("GtkGL : "#s);

#if defined TARGET_OS_Windows

namespace
{
	typedef bool (*wglSwapIntervalEXT_t)(int);
	wglSwapIntervalEXT_t wglSwapIntervalEXT = nullptr;
} // namespace

#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS

namespace
{
	typedef void (*glXSwapIntervalEXT_t)(Display*, GLXDrawable, int);
	glXSwapIntervalEXT_t glXSwapIntervalEXT = nullptr;
}

#endif

// ##  WINDOWS  ##############################################################################################################################################

#if defined TARGET_OS_Windows

namespace
{
	void on_realize_cb(GtkWidget* pWidget, void*)
	{
		GtkGL_Debug("realize-callback");

		gdk_window_ensure_native(gtk_widget_get_window(pWidget));

		HWND l_pWindow = HWND(GDK_WINDOW_HWND(::gtk_widget_get_window(pWidget)));
		HDC l_pDrawingContext = GetDC(l_pWindow);

		PIXELFORMATDESCRIPTOR l_oPixelFormatDescriptor;
		l_oPixelFormatDescriptor.nSize = sizeof(l_oPixelFormatDescriptor);
		l_oPixelFormatDescriptor.nVersion = 1;
		l_oPixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		l_oPixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
		l_oPixelFormatDescriptor.cColorBits = 24;
		l_oPixelFormatDescriptor.cAlphaBits = 8;
		l_oPixelFormatDescriptor.cDepthBits = 32;
		l_oPixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

		int l_iPixelFormatIdentifier = ChoosePixelFormat(l_pDrawingContext, &l_oPixelFormatDescriptor);

		if (l_iPixelFormatIdentifier == 0)
		{
			GtkGL_Warning("ChoosePixelFormat failed");
			GtkGL_Debug("realize-callback::failed");
			return;
		}

		if (SetPixelFormat(l_pDrawingContext, l_iPixelFormatIdentifier, &l_oPixelFormatDescriptor) == 0)
		{
			GtkGL_Warning("SetPixelFormat failed");
			GtkGL_Debug("realize-callback::failed");
			return;
		}

		HGLRC l_pGLRenderingContext = wglCreateContext(l_pDrawingContext);
		if (l_pGLRenderingContext == nullptr)
		{
			GtkGL_Warning("wglCreateContext failed");
			GtkGL_Debug("realize-callback::failed");
			return;
		}

		g_object_set_data(G_OBJECT(pWidget), GtkGL_RenderingContextName, l_pGLRenderingContext);
		g_object_set_data(G_OBJECT(pWidget), GtkGL_DeviceContextName, l_pDrawingContext);

		gtk_widget_queue_resize(pWidget);
		gtk_widget_set_double_buffered(pWidget, FALSE);

		wglSwapIntervalEXT = (wglSwapIntervalEXT_t)wglGetProcAddress("wglSwapIntervalEXT");

		GtkGL_Debug("realize-callback::success");
	}
}  // namespace

void Mensia::AdvancedVisualization::GtkGL::initialize(GtkWidget* pWidget)
{
	GtkGL_Debug("initialize");

	g_signal_connect(pWidget, "realize", G_CALLBACK(on_realize_cb), nullptr);

	GtkGL_Debug("initialize::success");
}

void Mensia::AdvancedVisualization::GtkGL::uninitialize(GtkWidget* pWidget)
{
	GtkGL_Debug("uninitialize");

	HWND l_pWindow = (HWND)GDK_WINDOW_HWND(::gtk_widget_get_window(pWidget));

	auto l_pGLRenderingContext = (HGLRC)g_object_get_data(G_OBJECT(pWidget), GtkGL_RenderingContextName);
	wglDeleteContext(l_pGLRenderingContext);

	HDC l_pDrawingContext = (HDC)g_object_get_data(G_OBJECT(pWidget), GtkGL_DeviceContextName);
	ReleaseDC(l_pWindow, l_pDrawingContext);

	GtkGL_Debug("uninitialize::success");
}

void Mensia::AdvancedVisualization::GtkGL::preRender(GtkWidget* pWidget, bool bVerticalSync)
{
	GtkGL_Debug("pre-render");

	HWND l_pWindow = (HWND)GDK_WINDOW_HWND(::gtk_widget_get_window(pWidget));
	HDC l_pDrawingContext = (HDC)g_object_get_data(G_OBJECT(pWidget), GtkGL_DeviceContextName);
	auto l_pGLRenderingContext = (HGLRC)g_object_get_data(G_OBJECT(pWidget), GtkGL_RenderingContextName);

	if (l_pGLRenderingContext == nullptr)
	{
		GtkGL_Debug("Rendering context not ready");
		GtkGL_Debug("pre-render::failed");
		return;
	}

	if (wglMakeCurrent(l_pDrawingContext, l_pGLRenderingContext) == 0)
	{
		GtkGL_Warning("wglMakeCurrent failed");
		GtkGL_Debug("pre-render::failed");
		return;
	}

	// Enable / Disable vsync
	if (wglSwapIntervalEXT != nullptr)
	{
		wglSwapIntervalEXT(bVerticalSync ? 1 : 0);
	}

	GtkGL_Debug("pre-render::success");
}

void Mensia::AdvancedVisualization::GtkGL::postRender(GtkWidget* pWidget)
{
	GtkGL_Debug("post-render");

	HDC l_pDrawingContext = (HDC)g_object_get_data(G_OBJECT(pWidget), GtkGL_DeviceContextName);
	HGLRC l_pGLRenderingContext = (HGLRC)g_object_get_data(G_OBJECT(pWidget), GtkGL_RenderingContextName);

	if (l_pDrawingContext == nullptr)
	{
		GtkGL_Debug("Rendering context not ready");
		GtkGL_Debug("post-render::failed");
		return;
	}

	if (SwapBuffers(l_pDrawingContext) == 0)
	{
		GtkGL_Warning("SwapBuffers failed");
		GtkGL_Debug("post-render::failed");
		return;
	}

	if (wglMakeCurrent(l_pDrawingContext, nullptr) == 0)
	{
		GtkGL_Warning("wglMakeCurrent failed");
		GtkGL_Debug("post-render::failed");
		return;
	}

	GtkGL_Debug("post-render::success");
}

// ##  WINDOWS  ##############################################################################################################################################

#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS

// ##  LINUX  ################################################################################################################################################

void Mensia::AdvancedVisualization::GtkGL::initialize(::GtkWidget * pWidget)
{
	GtkGL_Debug("initialize");

	// ::gdk_window_ensure_native(gtk_widget_get_window(pWidget));

	::GdkScreen* l_pScreen = ::gdk_screen_get_default();
	::Display* l_pDisplay = GDK_SCREEN_XDISPLAY(l_pScreen);
	::gint l_iScreenNumber = GDK_SCREEN_XNUMBER(l_pScreen);

	int l_vVisualInfoAttributes[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE,    1,
		GLX_GREEN_SIZE,  1,
		GLX_BLUE_SIZE,   1,
		GLX_ALPHA_SIZE,  1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE,  1,
		None
	};

	if (!::glXQueryVersion(l_pDisplay, nullptr, nullptr))
	{
		GtkGL_Warning("initialize::failed");
		return;
	}

	::XVisualInfo* l_pVisualInfo = ::glXChooseVisual(l_pDisplay, l_iScreenNumber, l_vVisualInfoAttributes);
	::GLXContext l_pGLRenderingContext = glXCreateContext(l_pDisplay, l_pVisualInfo, nullptr, True);
	g_object_set_data(G_OBJECT(pWidget), GtkGL_RenderingContextName, l_pGLRenderingContext);

#if 1
	/* Fix up colormap */
	::GdkVisual* l_pVisual = ::gdk_x11_screen_lookup_visual(l_pScreen, l_pVisualInfo->visualid);
	::GdkColormap* l_pColorMap = ::gdk_colormap_new(l_pVisual, FALSE);
	::gtk_widget_set_colormap(pWidget, l_pColorMap);
#endif

	::gtk_widget_queue_resize(pWidget);
	::gtk_widget_set_double_buffered(pWidget, FALSE);

	glXSwapIntervalEXT = (glXSwapIntervalEXT_t) ::glXGetProcAddressARB(reinterpret_cast <const unsigned char*>("glXSwapIntervalEXT"));

	GtkGL_Debug("initialize::success");
}

void Mensia::AdvancedVisualization::GtkGL::uninitialize(::GtkWidget * pWidget)
{
	GtkGL_Debug("uninitialize");

	::Display* l_pDisplay = GDK_SCREEN_XDISPLAY(::gtk_widget_get_screen(pWidget));
	::GLXContext l_pGLRenderingContext = (::GLXContext) g_object_get_data(G_OBJECT(pWidget), GtkGL_RenderingContextName);
	if (!l_pDisplay || !l_pGLRenderingContext)
	{
		GtkGL_Warning("uninitialize::failed");
		return;
	}
	::glXMakeCurrent(l_pDisplay, None, nullptr);
	::glXDestroyContext(l_pDisplay, l_pGLRenderingContext);

	GtkGL_Debug("uninitialize::success");
}

void Mensia::AdvancedVisualization::GtkGL::preRender(::GtkWidget * pWidget, bool bVerticalSync)
{
	GtkGL_Debug("pre-render");

	::Display* l_pDisplay = GDK_SCREEN_XDISPLAY(::gtk_widget_get_screen(pWidget));
	::Window l_pWindow = GDK_WINDOW_XID(gtk_widget_get_window(pWidget));
	::GLXContext l_pGLRenderingContext = (::GLXContext) g_object_get_data(G_OBJECT(pWidget), GtkGL_RenderingContextName);
	if (!l_pDisplay || !l_pGLRenderingContext)
	{
		GtkGL_Warning("pre-render::failed");
		return;
	}
	::glXMakeCurrent(l_pDisplay, l_pWindow, l_pGLRenderingContext);

	// Enable / Disable vsync
	if (glXSwapIntervalEXT)
	{
		glXSwapIntervalEXT(l_pDisplay, ::glXGetCurrentDrawable(), bVerticalSync ? 1 : 0);
	}

	GtkGL_Debug("pre-render::success");
}

void Mensia::AdvancedVisualization::GtkGL::postRender(::GtkWidget * pWidget)
{
	GtkGL_Debug("post-render");

	::Display* l_pDisplay = GDK_SCREEN_XDISPLAY(::gtk_widget_get_screen(pWidget));
	::Window l_pWindow = GDK_WINDOW_XID(gtk_widget_get_window(pWidget));
	if (!l_pDisplay)
	{
		GtkGL_Warning("post-render::failed");
		return;
	}
	::glXSwapBuffers(l_pDisplay, l_pWindow);

	GtkGL_Debug("post-render::success");
}

// ##  LINUX  ################################################################################################################################################

#endif
