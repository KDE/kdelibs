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

#include "qiomanager.h"
#include "qiomanager_p.h"
#include <qsocketnotifier.h>
#include <qapplication.h>
#include "debug.h"
#include "dispatcher.h"
#include "thread.h"

using namespace std;
using namespace Arts;

/* 
 * Collected incompatibilities of QIOManager (compared against StdIOManager):
 *
 * - StdIOManager catches up timers (i.e. if a 100ms timer hasn't been
 *   notified for half a second, it will get notified five times to
 *   catch up the lost time
 * - StdIOManager is able to watch the same filedescriptor twice for reading
 * - StdIOManager sends notifications soon after they have been produced,
 *   whereas we use a hackish 50ms timer to deliver them
 */

/*
 * Fallback for the case where we should perform blocking
 */
namespace Arts {
class QIOManagerBlocking : public StdIOManager {
public:
	void setLevel(int newLevel) { level = newLevel; }
};
};

/*
 * QIOManager is a singleton (or at least supposed to be used only once at
 * most, and the behaviour would pretty undefined if you violate this), so we
 * use static variables for private data here.
 */
static int qioLevel;
static QIOManager *qioManager = 0;
static QIOManagerBlocking *qioManagerBlocking = 0;
static bool qioBlocking;

/*
 * QIOWatch:
 */
QIOWatch::QIOWatch(int fd, int type, IONotify *notify,
	QSocketNotifier::Type qtype, bool reentrant)
	: _fd(fd), _type(type), _client(notify), _reentrant(reentrant)
{
	qsocketnotify = new QSocketNotifier(fd,qtype,this);
	connect(qsocketnotify,SIGNAL(activated(int)),this,SLOT(notify(int)));
}

void QIOWatch::notify(int socket)
{
	arts_assert(socket == _fd);
	qioManager->dispatch(this);
}

/*
 * QTimeWatch:
 */
QTimeWatch::QTimeWatch(int milliseconds, TimeNotify *notify)
{
	timer = new QTimer(this);
	connect( timer, SIGNAL(timeout()), this, SLOT(notify()) );
	timer->start(milliseconds);
	_client = notify;
}

void QTimeWatch::notify()
{
	qioManager->dispatch(this);
}

/*
 * Handle NotificationManager::the()->run() from time to time
 */
namespace Arts {

class HandleNotifications : public TimeNotify {
public:
	HandleNotifications()
	{
		Arts::Dispatcher::the()->ioManager()->addTimer(50, this);
	}
	void notifyTime()
	{
		NotificationManager::the()->run();
	}
	virtual ~HandleNotifications()
	{
		Arts::Dispatcher::the()->ioManager()->removeTimer(this);
	}
};

class HandleNotificationsStartup :public StartupClass
{
public:
	void startup()	{ h = new HandleNotifications(); }
	void shutdown()	{ delete h; }
private:
	HandleNotifications *h;
};
static HandleNotificationsStartup handleNotifications;

};

/*
 * QIOManager:
 */
QIOManager::QIOManager()
{
	assert(!qioManager);
	qioManager = this;
	qioLevel = 0;
	qioBlocking = true;
	qioManagerBlocking = new QIOManagerBlocking();
}

QIOManager::~QIOManager()
{
	assert(qioManager);
	qioManager = 0;

	delete qioManagerBlocking;
	qioManagerBlocking = 0;
}

void QIOManager::processOneEvent(bool blocking)
{
	assert(SystemThreads::the()->isMainThread());

	if(qioBlocking)
	{
		qioLevel++;
		if(qioLevel == 1)
			Dispatcher::lock();

		/*
		 * we explicitely take the level to qioManagerBlocking, so that it
		 * will process reentrant watchFDs only
		 */
		qioManagerBlocking->setLevel(qioLevel);
		qioManagerBlocking->processOneEvent(blocking);

		if(qioLevel == 1)
			Dispatcher::unlock();
		qioLevel--;
	}
	else
	{
		if(blocking)
			qApp->processOneEvent();
		else
			qApp->processEvents(0);
	}
}

void QIOManager::run()
{
	arts_warning("QIOManager::run() not implemented.");
}

void QIOManager::terminate()
{
	arts_warning("QIOManager::terminate() not implemented.");
}

void QIOManager::watchFD(int fd, int types, IONotify *notify)
{
	bool r = (types & IOType::reentrant) != 0;

	if(types & IOType::read)
	{
		fdList.push_back(
			new QIOWatch(fd, IOType::read, notify, QSocketNotifier::Read, r)
		);
	}
	if(types & IOType::write)
	{
		fdList.push_back(
			new QIOWatch(fd, IOType::write, notify, QSocketNotifier::Write, r)
		);
	}
	if(types & IOType::except)
	{
		fdList.push_back(
			new QIOWatch(fd, IOType::except, notify, QSocketNotifier::Exception,
						 r)
		);
	}
	if(r) qioManagerBlocking->watchFD(fd, types, notify);
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
	qioManagerBlocking->remove(notify, types);
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

void QIOManager::dispatch(QIOWatch *ioWatch)
{
	qioLevel++;
	if(qioLevel == 1)
		Dispatcher::lock();

	/*
	 * FIXME: there is main loop pollution for (qioBlocking == false) here:
	 *
	 * As QIOManager will never disable the socket notifiers that are not
	 * to be carried out reentrant, these will (maybe) fire again and again,
	 * so that CPU is wasted.
	 */
	if(qioLevel == 1 || ioWatch->reentrant())
		ioWatch->client()->notifyIO(ioWatch->fd(),ioWatch->type());
	
	if(qioLevel == 1)
		Dispatcher::unlock();
	qioLevel--;
}

void QIOManager::dispatch(QTimeWatch *timeWatch)
{
	qioLevel++;
	if(qioLevel == 1)
		Dispatcher::lock();

	// timers are never done reentrant
	if(qioLevel == 1)
		timeWatch->client()->notifyTime();
	
	if(qioLevel == 1)
		Dispatcher::unlock();
	qioLevel--;
}

bool QIOManager::blocking()
{
	return qioBlocking;
}

void QIOManager::setBlocking(bool blocking)
{
	qioBlocking = blocking;
}

#include "qiomanager_p.moc"
