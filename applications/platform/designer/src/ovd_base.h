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

#define M_Functionality_IsMensia OpenViBE::CIdentifier(0x8FF0F98F, 0x2917E3BB)

class G_CALLBACK_AUTOCAST
{
public:

	typedef union
	{
		GCallback fp;
		gpointer p;
	} _data;

	_data m_pData;

	G_CALLBACK_AUTOCAST(gpointer p) { m_pData.p = p; }
	G_CALLBACK_AUTOCAST(GCallback fp) { m_pData.fp = fp; }

	operator gpointer() { return m_pData.p; }
	operator GCallback() { return m_pData.fp; }

private:

	G_CALLBACK_AUTOCAST();
};
