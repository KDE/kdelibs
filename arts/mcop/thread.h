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

class Mutex_impl;
class Thread_impl;
class ThreadCondition_impl;
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

	/**
	 * Check wether the current thread is the main thread
	 *
	 * The main thread is the thread that the application's main() was
	 * executed in. The IOManager event loop will only run in the main
	 * thread.
	 */
	virtual bool isMainThread() = 0;

	virtual Mutex_impl *createMutex_impl() = 0;
	virtual Mutex_impl *createRecMutex_impl() = 0;
	virtual Thread_impl *createThread_impl(Thread *thread) = 0;
	virtual ThreadCondition_impl *createThreadCondition_impl() = 0;
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
	virtual bool tryLock() = 0;
	virtual void unlock() = 0;
	virtual ~Mutex_impl();
};

/**
 * Base class for platform specific thread condition code
 */
class ThreadCondition_impl {
public:
	virtual void wakeOne() = 0;
	virtual void wakeAll() = 0;
	virtual void wait(Mutex_impl *impl) = 0;
	virtual ~ThreadCondition_impl();
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
	friend class ThreadCondition;

public:
	/**
	 * constructor
	 *
	 * @param recursive whether to create a recursive mutex (may be locked by
	 *                  the same thread more than once), or a normal mutex
	 */
	inline Mutex(bool recursive = false)
		: impl(recursive?SystemThreads::the()->createRecMutex_impl()
						:SystemThreads::the()->createMutex_impl())
	{
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
	 * tries to lock the mutex, returning immediately in any case (even if
	 * mutex is locked by another thread)
	 *
	 * @returns true if successful (mutex locked), false otherwise
	 */
	inline bool tryLock() {
		return impl->tryLock();
	}

	/**
	 * unlocks the mutex
	 */
	inline void unlock() {
		impl->unlock();
	}
};

/**
 * A thread condition
 *
 * Thread conditions are used to let a different thread know that a certain
 * condition might have changed. For instance, if you have a thread that
 * waits until a counter exceeds a limit, the thread would look like this:
 *
 * <pre>
 * class WaitCounter : public Arts::Thread 
 * {
 *   int counter;
 *   Arts::Mutex mutex;
 *   Arts::ThreadCondition cond;
 * 
 * public:
 *   WaitCounter() : counter(0) {}
 *
 *   void run() {  // run will terminate once the counter reaches 20
 *     mutex.lock();
 *     while(counter < 20)
 *       cond.wait(mutex);
 *     mutex.unlock();
 *   }
 *
 *   void inc() {  // inc will increment the counter and indicate the change
 *     mutex.lock();
 *     counter++;
 *     cond.wakeOne();
 *     mutex.unlock();
 *   }
 * };
 * </pre>
 */
class ThreadCondition {
private:
	ThreadCondition_impl *impl;
		 
public:
	ThreadCondition()
		: impl(SystemThreads::the()->createThreadCondition_impl())
	{
	}
	
	virtual ~ThreadCondition();

	/**
	 * wakes one waiting thread
	 */
	inline void wakeOne()
	{
		impl->wakeOne();
	}

	/**
	 * wakes all waiting threads
	 */
	inline void wakeAll()
	{
		impl->wakeAll();
	}

	/**
	 * Waits until the condition changes. You will need to lock the mutex
	 * before calling this. Internally it will unlock the mutex (to let
	 * others change the condition), and relock it once the wait succeeds.
	 */
	inline void wait(Mutex& mutex)
	{
		impl->wait(mutex.impl);
	}
};


};

#endif /* ARTS_MCOP_THREAD_H */
