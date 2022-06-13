#pragma once

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include <gtk/gtk.h>

#include <gdk/gdk.h>

#include "ovd_defines.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#define MUTEX_NAME "openvibe_designer_mutex"
#define MESSAGE_NAME "openvibe_designer_message"

#define G_CALLBACK2(x) G_CALLBACK_AUTOCAST(G_CALLBACK(x))

class G_CALLBACK_AUTOCAST
{
public:
	typedef union
	{
		GCallback fp;
		gpointer p;
	} data_t;

	data_t m_Data;

	G_CALLBACK_AUTOCAST(const gpointer p) { m_Data.p = p; }
	G_CALLBACK_AUTOCAST(const GCallback fp) { m_Data.fp = fp; }

	operator gpointer() const { return m_Data.p; }
	operator GCallback() const { return m_Data.fp; }

private:
	G_CALLBACK_AUTOCAST();
};
