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
 * BC - Status (2001-02-23): QIOWatch, QTimeWatch.
 *
 * These classes provide implementation details and not meant to be used
 * in any way.
 */

#ifndef QIOMANAGER_P_H
#define QIOMANAGER_P_H

#include "iomanager.h"
#include <qobject.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <list>

namespace Arts {

class QIOWatch : public QObject {
	Q_OBJECT
protected:
	QSocketNotifier *qsocketnotify;
	int _fd;
	int _type;
	IONotify *_client;
	bool _reentrant;

public:
	QIOWatch(int fd, int type, IONotify *notify, QSocketNotifier::Type qtype,
				bool reentrant);

	inline IONotify *client() { return _client; }
	inline int type() { return _type; }
	inline int fd() { return _fd; }
	inline bool reentrant() { return _reentrant; }
public slots:
	void notify(int socket);
};

class QTimeWatch : public QObject {
	Q_OBJECT
protected:
	QTimer *timer;
	TimeNotify *_client;
public:
	QTimeWatch(int milliseconds, TimeNotify *notify);

	inline TimeNotify *client() { return _client; }
public slots:
	void notify();
};

};

#endif
