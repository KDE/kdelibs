    /*

    Copyright (C) 1999-2001 Stefan Westerfeld
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

/*
 * BC - Status (2001-02-23): QIOManager.
 *
 * QIOManager is kept binary compatible.
 */

#ifndef QIOMANAGER_H
#define QIOMANAGER_H

#include "iomanager.h"
#include <qobject.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <list>

namespace Arts {

class QIOWatch;
class QTimeWatch;

/**
 * QIOManager performs MCOP I/O inside the Qt event loop. This way, you will
 * be able to receive requests and notifications inside Qt application. The
 * usual way to set it up is:
 *
 * <pre>
 * KApplication app(argc, argv);    // as usual
 *
 * Arts::QIOManager qiomanager;
 * Arts::Dispatcher dispatcher(&qiomanager);
 * ...
 * return app.exec();               // as usual
 * </pre>
 */

class QIOManager : public IOManager {
protected:
	friend class QIOWatch;
	friend class QTimeWatch;

	std::list<QIOWatch *> fdList;
	std::list<QTimeWatch *> timeList;

	void dispatch(QIOWatch *ioWatch);
	void dispatch(QTimeWatch *timeWatch);

public:
	QIOManager();
	~QIOManager();

	void processOneEvent(bool blocking);
	void run();
	void terminate();
	void watchFD(int fd, int types, IONotify *notify);
	void remove(IONotify *notify, int types);
	void addTimer(int milliseconds, TimeNotify *notify);
	void removeTimer(TimeNotify *notify);

	/**
	 * This controls what QIOManager will do while waiting for the result
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

	/**
	 * Query whether blocking is enabled.
	 */
	bool blocking();
};

};

#endif
