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

#include "iomanager.h"
#include <stack>

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
}

void StdIOManager::processOneEvent(bool blocking)
{
	if(fdListChanged)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		maxfd = 0;

		list<IOWatchFD *>::iterator i;
		for(i = fdList.begin(); i != fdList.end(); i++)
		{
			IOWatchFD *w = *i;

			if(w->types() & IOType::read)     FD_SET(w->fd(),&readfds);
			if(w->types() & IOType::write)    FD_SET(w->fd(),&writefds);
			if(w->types() & IOType::except)   FD_SET(w->fd(),&exceptfds);

			if(w->types() && w->fd() > maxfd) maxfd = w->fd();
		}

		fdListChanged = false;
	}
	fd_set rfd = readfds;
	fd_set wfd = writefds;
	fd_set efd = exceptfds;

	/* default timeout 5 seconds */
	timeval select_timeout;
	select_timeout.tv_sec = 5;
	select_timeout.tv_usec = 0;

	/* prepare timers */
	if(timeList.size())
	{
		long selectabs = 5000000;

		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<TimeWatcher *>::iterator ti;
		for(ti = timeList.begin(); ti != timeList.end(); ti++)
		{
			while((*ti)->earlier(currenttime))
				(*ti)->doTick();

			timeval timertime = (*ti)->nextNotify();

			// if that may happen in the next ten seconds
			if(timertime.tv_sec < currenttime.tv_sec+10)
			{
				long timerabs = (timertime.tv_sec - currenttime.tv_sec)*1000000;
				timerabs += (timertime.tv_usec - currenttime.tv_usec);

				if(timerabs < selectabs) selectabs = timerabs;
			}
		}

		select_timeout.tv_sec = selectabs / 1000000;
		select_timeout.tv_usec = selectabs % 1000000;
	}

	int retval = select(maxfd+1,&rfd,&wfd,&efd,&select_timeout);

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
		stack< pair<IOWatchFD *,int> > notifyStack;

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

			if(match) notifyStack.push(make_pair(w,match));
		}
		
		while(!notifyStack.empty())
		{
			IOWatchFD *w = notifyStack.top().first;
			int match = notifyStack.top().second;

			w->notify()->notifyIO(w->fd(),match);

			notifyStack.pop();
		}
	}
	/* handle timers */
	if(timeList.size())
	{
		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<TimeWatcher *>::iterator ti;
		for(ti = timeList.begin(); ti != timeList.end(); ti++)
		{
			while((*ti)->earlier(currenttime))
				(*ti)->doTick();
		}
	}
}

void StdIOManager::run()
{
	terminated = false;
	while(!terminated)
		processOneEvent(true);
}

void StdIOManager::terminate()
{
	terminated = true;
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
}

void StdIOManager::remove(IONotify *notify, int types)
{
	list<IOWatchFD *>::iterator i;

	i = fdList.begin();

	while(i != fdList.end())
	{
		IOWatchFD *w = *i;

		if(w->notify() == notify) w->remove(types);

		if(!w->types())		// nothing left to watch
		{
			fdList.erase(i);
			delete w;

			i = fdList.begin();
		}
		else i++;
	}
	fdListChanged = true;
}

void StdIOManager::addTimer(int milliseconds, TimeNotify *notify)
{
	timeList.push_back(new TimeWatcher(milliseconds,notify));
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
			timeList.erase(i);
			delete w;

			i = timeList.begin();
		}
		else i++;
	}
}

TimeWatcher::TimeWatcher(int milliseconds, TimeNotify *notify)
{
	_notify = notify;
	this->milliseconds = milliseconds;

	gettimeofday(&_nextNotify,0);

	_nextNotify.tv_usec += (milliseconds%1000)*1000;
	_nextNotify.tv_sec += (milliseconds/1000)+(_nextNotify.tv_usec/1000000);
	_nextNotify.tv_usec %= 1000000;
}

void TimeWatcher::doTick()
{
	_nextNotify.tv_usec += (milliseconds%1000)*1000;
	_nextNotify.tv_sec += (milliseconds/1000)+(_nextNotify.tv_usec/1000000);
	_nextNotify.tv_usec %= 1000000;

	_notify->notifyTime();
}

bool TimeWatcher::earlier(struct timeval reference)
{
	if(_nextNotify.tv_sec > reference.tv_sec) return false;
	if(_nextNotify.tv_sec < reference.tv_sec) return true;

	return (_nextNotify.tv_usec < reference.tv_usec);
}
