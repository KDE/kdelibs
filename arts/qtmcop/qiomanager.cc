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

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "qiomanager.h"
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <assert.h>

using namespace std;
using namespace Arts;

QIOWatch::QIOWatch(int fd, int type, IONotify *notify,
	QSocketNotifier::Type qtype)
{
	this->fd = fd;
	_type = type;
	_client = notify;
	qsocketnotify = new QSocketNotifier(fd,qtype,this);

	connect(qsocketnotify,SIGNAL(activated(int)),this,SLOT(notify(int)));
}

void QIOWatch::notify(int socket)
{
	_client->notifyIO(socket,_type);
}

QTimeWatch::QTimeWatch(int milliseconds, TimeNotify *notify)
{
	timer = new QTimer(this);
	connect( timer, SIGNAL(timeout()), this, SLOT(notify()) );
	timer->start(milliseconds);
	_client = notify;
}

void QTimeWatch::notify()
{
	_client->notifyTime();
}

QIOManager::QIOManager()
{
}


void QIOManager::processOneEvent(bool blocking)
{
	assert(blocking);
	qApp->processOneEvent();
}

void QIOManager::run()
{
}

void QIOManager::terminate()
{
}

void QIOManager::watchFD(int fd, int types, IONotify *notify)
{
	if(types & IOType::read)
	{
		fdList.push_back(
			new QIOWatch(fd, IOType::read, notify, QSocketNotifier::Read)
		);
	}
	if(types & IOType::write)
	{
		fdList.push_back(
			new QIOWatch(fd, IOType::write, notify, QSocketNotifier::Write)
		);
	}
	if(types & IOType::except)
	{
		fdList.push_back(
			new QIOWatch(fd, IOType::except, notify, QSocketNotifier::Exception)
		);
	}
}

void QIOManager::remove(IONotify *notify, int types)
{
	list<QIOWatch *>::iterator i;

	i = fdList.begin();
	while(i != fdList.end())
	{
		QIOWatch *w = *i;

		if(w->type() & types && w->client() == notify)
		{
			delete w;
			fdList.erase(i);
			i = fdList.begin();
		}
		else i++;
	}
}

void QIOManager::addTimer(int milliseconds, TimeNotify *notify)
{
	timeList.push_back(new QTimeWatch(milliseconds,notify));
}

void QIOManager::removeTimer(TimeNotify *notify)
{
	list<QTimeWatch *>::iterator i;

	i = timeList.begin();
	while(i != timeList.end())
	{
		QTimeWatch *w = *i;

		if(w->client() == notify)
		{
			delete w;
			timeList.erase(i);
			i = timeList.begin();
		}
		else i++;
	}
}

#include "qiomanager.moc"
