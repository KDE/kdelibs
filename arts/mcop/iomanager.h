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

#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <list>
#include <stack>

namespace Arts {
// constants:

/**
 * What does the reentrant flag do?
 *
 * The IOManager offers a processOneEvent call. This means, that you can ask
 * that I/O is handled, even while in a routine that handles I/O. As a
 * practical example: you may have got a remote invocation for the function
 * foo. Now you are in function foo() and call function bar() on a remote
 * server. When you wait for the result, you obviously will again require
 * the IOManager to wait for it. Thus this is a case where you need reentrant
 * I/O handling.
 *
 * That way, you get a multiple level stack:
 *
 *    [...]
 *      |
 * [ Hander for I/O ]
 *      |
 * [ IOManager ]              level 2
 *      |
 * [ Some other function ]
 *      |
 * [ Hander for I/O ]
 *      |
 * [ IOManager ]              level 1
 *      |
 * [ main() ]
 *
 * What reentrant does, is to allow that IO Watch to be activated at levels
 * higher than one.
 *
 * Timers and notifications, on the other hand will only be carried out at
 * level 1.
 */
struct IOType {
	enum { read = 1, write = 2, except = 4, reentrant = 8, all = 15 };
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
	timeval nextNotify;
	bool active, destroyed;

	bool earlier(const timeval& reference);
public:
	TimeWatcher(int milliseconds, TimeNotify *notify);

	inline TimeNotify *notify() { return _notify; };
	timeval advance(const timeval& currentTime);
	void destroy();
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
	std::list<IOWatchFD *> fdList;
	std::list<TimeWatcher *> timeList;
	std::stack< std::pair<IOWatchFD *,int> > notifyStack;

	bool terminated;

	bool fdListChanged;	// causes the fd_sets to be rebuilt before using them
	fd_set readfds, writefds, exceptfds;
	fd_set reentrant_readfds, reentrant_writefds, reentrant_exceptfds;
	int maxfd;

	int level;

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

};

#endif
