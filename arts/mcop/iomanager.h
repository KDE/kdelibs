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

#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <list>
#include <stack>

// constants:

struct IOType {
	enum { read = 1, write = 2, except = 4, all = 7 };
};

class IONotify {
public:
	virtual void notifyIO(int fd, int type) = 0;
};

class IOWatchFD {
	int _fd, _types;
	IONotify *_notify;

public:
	IOWatchFD(int fd, int types, IONotify *notify);

	inline int fd() { return _fd; };
	inline int types() { return _types; };
	inline IONotify *notify() { return _notify; };
	inline void remove(int types) { _types &= ~types; }
};

class TimeNotify {
public:
	virtual void notifyTime() = 0;
};

class TimeWatcher {
	int milliseconds;
	TimeNotify *_notify;
	struct timeval _nextNotify;

public:
	TimeWatcher(int milliseconds, TimeNotify *notify);
	inline struct timeval nextNotify() { return _nextNotify; }
	inline TimeNotify *notify() { return _notify; };
	bool earlier(struct timeval reference);
	void doTick();
};

class IOManager {
public:
	virtual ~IOManager() {};

	/**
	 * processes exactly one io event
	 */
	virtual void processOneEvent(bool blocking) = 0;

	/**
	 * enters a loop which processes io events, until terminate is called
	 *
	 * may only be called once (use processOneEvent for other purposes)
	 */
	virtual void run() = 0;

	/**
	 * terminates the io event loop (which was started with run)
	 */
	virtual void terminate() = 0;

	/**
	 * starts watching one filedescriptor for certain types of operations
	 *
	 * notifies the notify object when e.g. the fd requires (allows) reading
	 * and types contained IOType::read.
	 */
	virtual void watchFD(int fd, int types, IONotify *notify) = 0;

	/**
	 * stops watching a filedescriptor
	 */
	virtual void remove(IONotify *notify, int types) = 0;

	/**
	 * starts a periodic timer
	 */
	virtual void addTimer(int milliseconds, TimeNotify *notify) = 0;

	/**
	 * stops the timer
	 */
	virtual void removeTimer(TimeNotify *notify) = 0;
};

class StdIOManager : public IOManager {
protected:
	list<IOWatchFD *> fdList;
	list<TimeWatcher *> timeList;
	stack< pair<IOWatchFD *,int> > notifyStack;

	bool terminated;

	bool fdListChanged;	// causes the fd_sets to be rebuilt before using them
	fd_set readfds, writefds, exceptfds;
	int maxfd;

public:
	StdIOManager();

	void processOneEvent(bool blocking);
	void run();
	void terminate();
	void watchFD(int fd, int types, IONotify *notify);
	void remove(IONotify *notify, int types);
	void addTimer(int milliseconds, TimeNotify *notify);
	void removeTimer(TimeNotify *notify);
};

#endif
