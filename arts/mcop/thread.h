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

#ifndef ARTS_MCOP_THREAD_H
#define ARTS_MCOP_THREAD_H

namespace Arts {

class Thread_impl;
class Mutex_impl;
class Thread;

/**
 * Encapsulates the operating system threading facilities
 */
class SystemThreads {
public:
	static SystemThreads *the();
	static bool init(SystemThreads *the);

	/**
	 * Check whether there is threading support available
	 *
	 * If there is no real threading support, the Threading classes try to
	 * gracefully degrade the performance. For instance, locking a mutex will
	 * do * nothing, and calling the start() function of a Thread will execute
	 * it's run function.
	 *
	 * @returns true if there are real threads 
	 */
	static bool supported();

	virtual bool isMainThread() = 0;
	virtual Mutex_impl *createMutex_impl() = 0;
	virtual Thread_impl *createThread_impl(Thread *thread) = 0;
	virtual ~SystemThreads();
};

/**
 * Base class for platform specific thread code
 */
class Thread_impl
{
public:
	virtual void start() = 0;
	virtual void waitDone() = 0;
	virtual ~Thread_impl();
};

/**
 * Base class for platform specific mutex code
 */
class Mutex_impl {
public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual ~Mutex_impl();
};

/**
 * A thread of execution
 *
 * Example for implementing a thread:
 *
 * <pre>
 * class Counter : public Arts::Thread 
 * {
 * public:
 *   void run() {
 *     for(int i = 0;i < 10;i++)
 *     {
 *       printf("%d\n",i+1);
 *       sleep(1);
 *     }
 *   }
 * };            // start the thread with Counter c; c.start();
 * </pre>
 */
class Thread {
private:
	Thread_impl *impl;
		 
public:
	Thread() : impl(SystemThreads::the()->createThread_impl(this))
	{
	}
	
	virtual ~Thread();

	/**
	 * starts the run() method in a thread
	 */
	inline void start()
	{
		impl->start();
	}

	/**
	 * waits until the thread is executed completely
	 */
	inline void waitDone()
	{
		impl->waitDone();
	}

	/**
	 * implement this method, if you want to create an own thread - then
	 * you can simply call thread.start() to start execution of run() in
	 * a seperate thread
	 */
	virtual void run() = 0;
};

/**
 * A mutex
 *
 * To protect a critical section, you can use a mutex, which will ensure that
 * only one thread at a time can lock it. Here is an example for a thread-safe
 * random number generator:
 *
 * <pre>
 * class RandomGenerator {
 *   Arts::Mutex mutex;
 *   long seed;
 * public:
 *   long get() {
 *     mutex.lock();
 *     // do complicated calculation with seed here
 *     mutex.unlock();
 *     return seed;
 *   }
 * };
 * </pre>
 */
class Mutex {
private:
	Mutex_impl *impl;

public:
	/**
	 * constructor
	 */
	inline Mutex() {
		impl = SystemThreads::the()->createMutex_impl();
	}

	/**
	 * destructor
	 */
	virtual ~Mutex();

	/**
	 * locks the mutex
	 */
	inline void lock() {
		impl->lock();
	}

	/**
	 * unlocks the mutex
	 */
	inline void unlock() {
		impl->unlock();
	}
};

};

#endif /* ARTS_MCOP_THREAD_H */
