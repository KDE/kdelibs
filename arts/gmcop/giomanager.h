    /*

    Copyright (C) 2001 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef ARTS_GIOMANAGER_H
#define ARTS_GIOMANAGER_H

#include <glib.h>
#include <iomanager.h>
#include <list>
#include <stack>
#include <map>

namespace Arts {

class GIOWatch;
class GIOManagerPrivate;
struct GIOManagerSource;
class GIOTimeWatch;
class GIOManagerBlocking;

/**
 * GIOManager performs MCOP I/O inside the Glib event loop. This way, you will
 * be able to receive requests and notifications inside Glib application. The
 * usual way to set it up is:
 *
 * <pre>
 * GMainLoop *main_loop = g_main_new(FALSE);  // as usual
 *
 * Arts::GIOManager iomanager;
 * Arts::Dispatcher dispatcher(&iomanager);
 *
 * g_main_run(main_loop);                     // as usual
 * </pre>
 */
class GIOManager : public IOManager {
private:
	GIOManagerPrivate *d;

protected:
	std::list<GIOWatch *> fdList;
	std::list<GIOTimeWatch *> timeList;
	int level;
	bool _blocking;
	bool fileDescriptorsNeedRecheck;
	GMainContext *context;
	GIOManagerSource *source;
	GIOManagerBlocking *gioManagerBlocking;

public:
	GIOManager(GMainContext *context = 0);
	~GIOManager();

	void processOneEvent(bool blocking);
	void run();
	void terminate();
	void watchFD(int fd, int types, IONotify *notify);
	void remove(IONotify *notify, int types);
	void addTimer(int milliseconds, TimeNotify *notify);
	void removeTimer(TimeNotify *notify);

	/**
	 * This controls what GIOManager will do while waiting for the result
	 * of an MCOP request, the possibilities are:
	 *
	 * @li block until the request is completed (true)
	 * @li open a local event loop (false)
	 *
	 * It is much easier to write working and reliable code with blocking
	 * enabled, so this is the default. If you disable blocking, you have
	 * to deal with the fact that timers, user interaction and similar
	 * "unpredictable" things will possibly influence your code in all
	 * places where you make a remote MCOP call (which is quite often in
	 * MCOP applications).
	 */
	void setBlocking(bool blocking);

	/* GSource stuff: */

	/**
	 *  - internal -
	 *
	 *  (implements the GSource prepare)
	 */
	gboolean prepare(gint *timeout);

	/**
	 *  - internal -
	 *
	 *  (implements the GSource check)
	 */
	gboolean check();

	/**
	 *  - internal -
	 *
	 *  (implements the GSource dispatch)
	 */
	gboolean dispatch(GSourceFunc callback, gpointer user_data);
};

};

#endif /* ARTS_GIOMANAGER_H */
