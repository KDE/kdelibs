    /*

    Copyright (C) 2000 Stefan Westerfeld
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "iomanager.h"
#include "dispatcher.h"
#include "notification.h"
#include "thread.h"
#include <stdio.h>
#include <fcntl.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif
// WABA: NOTE!
// sys/select.h is needed on e.g. AIX to define "fd_set".
// However, we can not include config.h in a header file.
// The right solution would be not to use "fd_set" in the
// header file but to use it only in a private datastructure 
// defined in the .cc file.

using namespace std;
using namespace Arts;

/*
 * Enable this if you want to debug how long certain plugins / operations
 * take to perform. You'll get the times between two select() calls that are
 * done by the IOManager, which is equivalent to the time the input/output
 * remains unserved. For apps like artsd, it gives the minimum audio latency
 * users will need to specify to avoid dropouts.
 */
#undef IOMANAGER_DEBUG_LATENCY

#ifdef IOMANAGER_DEBUG_LATENCY
static timeval iomanager_debug_latency_time = { 0, 0 };

static void iomanager_debug_latency_end()
{
	if(iomanager_debug_latency_time.tv_sec)
	{
		timeval end;
		gettimeofday(&end,0);

		float diff = (end.tv_usec-iomanager_debug_latency_time.tv_usec)/1000.0
				   + (end.tv_sec-iomanager_debug_latency_time.tv_sec)*1000.0;

		/* change this value if you get your screen filled up with messages */
		if(diff >= 1.5)
			fprintf(stderr,"IOManager: latency for operation: %2.3f ms\n",diff);
	}
}

static void iomanager_debug_latency_start()
{
	gettimeofday(&iomanager_debug_latency_time,0);
}
#else
static inline void iomanager_debug_latency_end()
{
}

static inline void iomanager_debug_latency_start()
{
}
#endif

IOWatchFD::IOWatchFD(int fd, int types, IONotify *notify)
{
	_fd = fd;
	_types = types;
	_notify = notify;
}

StdIOManager::StdIOManager()
{
	// force initialization of the fd_set's
	fdListChanged = true;
	timeListChanged = false;
	level = 0;
}

void StdIOManager::processOneEvent(bool blocking)
{
	assert(SystemThreads::the()->isMainThread());

	level++;

	// we release and acquire the lock only on level 1
	if(level == 1)
		Dispatcher::lock();

	// notifications not carried out reentrant
	if(level == 1)
		NotificationManager::the()->run();

	// FIXME: timers *could* change the file descriptors to select...
	//---
	if(fdListChanged)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
		FD_ZERO(&reentrant_readfds);
		FD_ZERO(&reentrant_writefds);
		FD_ZERO(&reentrant_exceptfds);

		maxfd = 0;

		list<IOWatchFD *>::iterator i;
		for(i = fdList.begin(); i != fdList.end(); i++)
		{
			IOWatchFD *w = *i;

			if(w->types() & IOType::read)     FD_SET(w->fd(),&readfds);
			if(w->types() & IOType::write)    FD_SET(w->fd(),&writefds);
			if(w->types() & IOType::except)   FD_SET(w->fd(),&exceptfds);

			if(w->types() & IOType::reentrant)
			{
				if(w->types() & IOType::read)
					FD_SET(w->fd(),&reentrant_readfds);
				if(w->types() & IOType::write)
					FD_SET(w->fd(),&reentrant_writefds);
				if(w->types() & IOType::except)
					FD_SET(w->fd(),&reentrant_exceptfds);
			}

			if(w->types() && w->fd() > maxfd) maxfd = w->fd();
		}

		fdListChanged = false;
	}
	fd_set rfd,wfd,efd;
	if(level == 1)
	{
		rfd = readfds;
		wfd = writefds;
		efd = exceptfds;
	}
	else
	{
		// watch out, this is reentrant I/O
		rfd = reentrant_readfds;
		wfd = reentrant_writefds;
		efd = reentrant_exceptfds;
	}

	/* default timeout 5 seconds */
	long selectabs;

	if(blocking)
		selectabs = 5000000;
	else
		selectabs = 0;

	/* prepare timers - only at level 1 */
	if(level == 1 && timeList.size())
	{
		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<TimeWatcher *>::iterator ti;

		timeListChanged = false;
		ti = timeList.begin();
		while(ti != timeList.end())
		{
			TimeWatcher *w = *ti++;
			timeval timertime = w->advance(currenttime);

			// if that may happen in the next ten seconds
			if(timertime.tv_sec < currenttime.tv_sec+10)
			{
				long timerabs = (timertime.tv_sec - currenttime.tv_sec)*1000000;
				timerabs += (timertime.tv_usec - currenttime.tv_usec);

				if(timerabs < selectabs) selectabs = timerabs;
			}

			if(timeListChanged) 
			{
		        ti = timeList.begin();
				timeListChanged = false;
			}
		}
	}

	timeval select_timeout;
	select_timeout.tv_sec = selectabs / 1000000;
	select_timeout.tv_usec = selectabs % 1000000;

	if(level == 1) iomanager_debug_latency_end();

	// we release and acquire the lock only on level 1
	if(level == 1)
		Dispatcher::unlock();

	int retval = select(maxfd+1,&rfd,&wfd,&efd,&select_timeout);

	// we release and acquire the lock only on level 1
	if(level == 1)
		Dispatcher::lock();

	if(level == 1) iomanager_debug_latency_start();

	if(retval > 0)
	{
		/*
		 * the problem is, that objects that are being notified may change
		 * the watch list, add fds, remove fds, remove objects and whatever
		 * else
		 *
		 * so we can' notify them from the loop - but we can make a stack
		 * of "notifications to do" and send them as soon as we looked up
		 * in the list what to send
		 */
		long tonotify = 0;

		list<IOWatchFD *>::iterator i;
		for(i = fdList.begin(); i != fdList.end(); i++) {
			IOWatchFD *w = *i;
			int match = 0;

			if(FD_ISSET(w->fd(),&rfd) && (w->types() & IOType::read))
				match |= IOType::read;

			if(FD_ISSET(w->fd(),&wfd) && (w->types() & IOType::write))
				match |= IOType::write;

			if(FD_ISSET(w->fd(),&efd) && (w->types() & IOType::except))
				match |= IOType::except;

			if((w->types() & IOType::reentrant) == 0 && level != 1)
				match = 0;

			if(match) {
				w->notify()->notifyIO(w->fd(),match);
				break;
			}
/*!!!
			if(match) {
				tonotify++;
				notifyStack.push(make_pair(w,match));
			}*/
		}
		
		while(tonotify != 0)
		{
			IOWatchFD *w = notifyStack.top().first;
			int match = notifyStack.top().second;

			w->notify()->notifyIO(w->fd(),match);

			notifyStack.pop();
			tonotify--;
		}
	}
	/* handle timers - only at level 1 */
	if(level == 1 && timeList.size())
	{
		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<TimeWatcher *>::iterator ti;

		timeListChanged = false;
		ti = timeList.begin();
		while(ti != timeList.end())
		{
			TimeWatcher *w = *ti++;
			w->advance(currenttime);
			if (timeListChanged) 
			{
		        ti = timeList.begin();
				timeListChanged = false;
			}
		}
	}

	// notifications not carried out reentrant
	if(level == 1)
		NotificationManager::the()->run();

	// we release and acquire the lock only on level 1
	if(level == 1)
		Dispatcher::unlock();

	level--;
}

void StdIOManager::run()
{
	assert(SystemThreads::the()->isMainThread());
	assert(level == 0);

	// FIXME: this might not be threadsafe, as there is no lock here!
	terminated = false;
	while(!terminated)
		processOneEvent(true);
}

void StdIOManager::terminate()
{
	terminated = true;
	Dispatcher::wakeUp();
}

void StdIOManager::watchFD(int fd, int types, IONotify *notify)
{
	/*
	IOWatchFD *watchfd = findWatch(fd,notify);
	if(watchfd)
	{
		watchfd->add(types);
	}
	else
	{
		fdList.push_back(new IOWatchFD(fd,types,notify));
	}
	*/

	// FIXME: might want to reuse old watches
	fdList.push_back(new IOWatchFD(fd,types,notify));
	fdListChanged = true;
	Dispatcher::wakeUp();
}

void StdIOManager::remove(IONotify *notify, int types)
{
	list<IOWatchFD *>::iterator i;

	i = fdList.begin();

	while(i != fdList.end())
	{
		IOWatchFD *w = *i;

		if(w->notify() == notify) w->remove(types);

		// nothing left to watch?
		if(w->types() == 0 || w->types() == IOType::reentrant)	
		{
			i = fdList.erase(i);
			delete w;		// FIXME: shouldn't we have a destroy() similar
			                // to the one for timers
		}
		else i++;
	}
	fdListChanged = true;
}

void StdIOManager::addTimer(int milliseconds, TimeNotify *notify)
{
	timeList.push_back(new TimeWatcher(milliseconds,notify));
	timeListChanged = true;
	Dispatcher::wakeUp();
}

void StdIOManager::removeTimer(TimeNotify *notify)
{
	list<TimeWatcher *>::iterator i;

	i = timeList.begin();

	while(i != timeList.end())
	{
		TimeWatcher *w = *i;

		if(w->notify() == notify)
		{
			i = timeList.erase(i);
			timeListChanged = true;
			w->destroy();
		}
		else i++;
	}
}

TimeWatcher::TimeWatcher(int _milliseconds, TimeNotify *notify)
	: milliseconds(_milliseconds),_notify(notify),active(false),destroyed(false)
{
	gettimeofday(&nextNotify,0);

	nextNotify.tv_usec += (milliseconds%1000)*1000;
	nextNotify.tv_sec += (milliseconds/1000)+(nextNotify.tv_usec/1000000);
	nextNotify.tv_usec %= 1000000;
}

timeval TimeWatcher::advance(const timeval& currentTime)
{
	active = true;
	while(earlier(currentTime))
	{
		nextNotify.tv_usec += (milliseconds%1000)*1000;
		nextNotify.tv_sec += (milliseconds/1000)+(nextNotify.tv_usec/1000000);
		nextNotify.tv_usec %= 1000000;

		_notify->notifyTime();

		if(destroyed)
		{
			delete this;
		
			struct timeval never = { 0xffffffff, 0 };
			return never;
		}
	}
	active = false;
	return nextNotify;
}

bool TimeWatcher::earlier(const timeval& reference)
{
	if(nextNotify.tv_sec > reference.tv_sec) return false;
	if(nextNotify.tv_sec < reference.tv_sec) return true;

	return (nextNotify.tv_usec < reference.tv_usec);
}

void TimeWatcher::destroy()
{
	if(active)
	{
		destroyed = true;
	}
	else
	{
		delete this;
	}
}
