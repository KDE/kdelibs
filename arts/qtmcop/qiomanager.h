    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#ifndef QIOMANAGER_H
#define QIOMANAGER_H

#include "iomanager.h"
#include <qobject.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <list>

class QIOWatch : public QObject {
	Q_OBJECT
protected:
	QSocketNotifier *qsocketnotify;
	IONotify *_client;
	int fd;
	int _type;

public:
	QIOWatch(int fd, int type, IONotify *notify, QSocketNotifier::Type qtype);

	inline IONotify *client() { return _client; }
	inline int type() { return _type; }
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

class QIOManager : public IOManager {
protected:
	list<QIOWatch *> fdList;
	list<QTimeWatch *> timeList;

public:
	QIOManager();

	void processOneEvent(bool blocking);
	void run();
	void terminate();
	void watchFD(int fd, int types, IONotify *notify);
	void remove(IONotify *notify, int types);
	void addTimer(int milliseconds, TimeNotify *notify);
	void removeTimer(TimeNotify *notify);
};

#endif
