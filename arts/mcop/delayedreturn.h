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

#ifndef ARTS_MCOP_DELAYEDRETURN_H
#define ARTS_MCOP_DELAYEDRETURN_H

namespace Arts {

class AnyConstRef;
class Buffer;
class Connection;
class DelayedReturnPrivate;

/**
 * The "DelayedReturn" class implements delayed returning from functions.
 * 
 * For instance, suppose you would like to return from a wait() operation
 * only after a certain time has passed, without blocking other clients
 * from using your interface during this time.
 *
 * Your implementation (in the skel file) of wait should look like this
 *
 * <pre>
 * static DelayedReturn *delayedReturn = 0;
 *
 * bool TimeServer::waitUntil(long time)
 * {
 *     if(time(0) < time)
 *     {
 *     	   // ... initialize timer here ...
 *         delayedReturn = Dispatcher::the()->delayReturn();
 *     }
 *     return false; 
 * }
 *
 * void timerEvent()
 * {
 * 	   delayedReturn->doReturn(true);
 * }
 * </pre>
 *
 * NOTE: this class breaks for local use (i.e. you can only use it if the
 * call was a remote call), so it is strongly recommended not to use it
 * unless there is no alternative (and you should really know what you are
 * doing, too).
 */
class DelayedReturn {
friend class Dispatcher;
private:
	DelayedReturnPrivate *d;

protected:
	Buffer *buffer;
	Connection *connection;

	DelayedReturn();
	void initialize(Connection *connection, Buffer *buffer);
	~DelayedReturn();

public:
	/**
	 * call this only once to make the request really return
	 */
	void doReturn(const AnyConstRef& value);

	/**
	 * return without value (for void functions)
	 */
	void doReturn();
};

};

#endif /* ARTS_MCOP_DELAYEDRETURN_H */
