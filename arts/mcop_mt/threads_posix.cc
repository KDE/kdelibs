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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* only compile this if we have libpthread available */
#ifdef HAVE_LIBPTHREAD

#include <pthread.h>
#include <debug.h>
#include <string.h>

#include "thread.h"

/*
 * define this if you want to protect mutexes against being locked twice by
 * the same thread
 */
#define PTHREAD_DEBUG 1

namespace Arts {
namespace PosixThreads {

class ThreadCondition_impl;

class Mutex_impl : public Arts::Mutex_impl {
protected:
	friend class ThreadCondition_impl;
	pthread_mutex_t mutex;

#ifdef PTHREAD_DEBUG
	pthread_t owner;
#endif

public:
	Mutex_impl()
	{
		pthread_mutex_init(&mutex, 0);
		owner = 0;
	}
	void lock()
	{
#ifdef PTHREAD_DEBUG
		pthread_t self = pthread_self();
		arts_assert(owner != self);
#endif

		pthread_mutex_lock(&mutex);

#ifdef PTHREAD_DEBUG
		arts_assert(!owner);
		owner = self;
#endif
	}
	bool tryLock()
	{
#ifdef PTHREAD_DEBUG
		pthread_t self = pthread_self();
		arts_assert(owner != self);
#endif

		int result = pthread_mutex_trylock(&mutex);

#ifdef PTHREAD_DEBUG
		if(result == 0)
		{
			arts_assert(!owner);
			owner = self;
		}
#endif
		return(result == 0);
	}
	void unlock()
	{
#ifdef PTHREAD_DEBUG
		arts_assert(owner == pthread_self());
		owner = 0;
#endif

		pthread_mutex_unlock(&mutex);
	}
};

class RecMutex_impl : public Arts::Mutex_impl {
protected:
	friend class ThreadCondition_impl;
	pthread_mutex_t mutex;
	pthread_t owner;
	int count;

public:
	RecMutex_impl()
	{
		pthread_mutex_init(&mutex, 0);
		owner = 0;
		count = 0;
	}
	void lock()
	{
		pthread_t self = pthread_self();
		if(owner != self)
		{
			pthread_mutex_lock(&mutex);
#ifdef PTHREAD_DEBUG
			arts_assert(count == 0);
			arts_assert(!owner);
#endif
			owner = self;
		}
		count++;
	}
	bool tryLock()
	{
		pthread_t self = pthread_self();
		if(owner != self)
		{
			int result = pthread_mutex_trylock(&mutex);
			if(result != 0)
				return false;

#ifdef PTHREAD_DEBUG
			arts_assert(count == 0);
			arts_assert(!owner);
#endif
			owner = self;
		}
		count++;
	}
	void unlock()
	{
#ifdef PTHREAD_DEBUG
		arts_assert(owner == pthread_self());
		arts_assert(count > 0);
#endif

		count--;
		if(count == 0)
		{
			owner = 0;
			pthread_mutex_unlock(&mutex);
		}
	}
};


static void *threadStartInternal(void *object)
{
    ((Thread *)object)->run();
    return 0;
}

class Thread_impl : public Arts::Thread_impl {
protected:
	pthread_t pthread;
	Thread *thread;

public:
	Thread_impl(Thread *thread) : thread(thread) {
	}
	void start() {
		pthread_attr_t attrs;
			 
		pthread_attr_init(&attrs);
	    pthread_create(&pthread,&attrs,threadStartInternal,thread);
	}
	void waitDone() {
    	void *foo;
    	pthread_join(pthread,&foo);
	}
};

class ThreadCondition_impl : public Arts::ThreadCondition_impl {
protected:
	pthread_cond_t cond;

public:
	ThreadCondition_impl() {
		pthread_cond_init(&cond, 0);
	}
	~ThreadCondition_impl() {
		pthread_cond_destroy(&cond);
	}
	void wakeOne() {
		pthread_cond_signal(&cond);
	}
	void wakeAll() {
		pthread_cond_broadcast(&cond);
	}
	void wait(Arts::Mutex_impl *mutex) {
#ifdef PTHREAD_DEBUG
		pthread_t self = pthread_self();
		arts_assert(((Mutex_impl *)mutex)->owner == self);
		((Mutex_impl *)mutex)->owner = 0;
#endif

		pthread_cond_wait(&cond, &((Mutex_impl*)mutex)->mutex);

#ifdef PTHREAD_DEBUG
		arts_assert(((Mutex_impl *)mutex)->owner == 0);
		((Mutex_impl *)mutex)->owner = self;
#endif
	}
};


class PosixThreads : public SystemThreads {
private:
	pthread_t mainThread;
public:
	PosixThreads() {
		mainThread = pthread_self();
	}
	bool isMainThread() {
		return pthread_equal(pthread_self(), mainThread);
	}
	Arts::Mutex_impl *createMutex_impl() {
		return new Mutex_impl();
	}
	Arts::Mutex_impl *createRecMutex_impl() {
		return new RecMutex_impl();
	}
	Arts::Thread_impl *createThread_impl(Arts::Thread *thread) {
		return new Thread_impl(thread);
	}
	Arts::ThreadCondition_impl *createThreadCondition_impl() {
		return new ThreadCondition_impl();
	}
	void getCurrentThread(void *id) {
		pthread_t *result = static_cast<pthread_t*>(id);
		*result = pthread_self();
	}
};

// set posix threads on startup
static class SetSystemThreads {
private:
	PosixThreads posixThreads;

public:
	SetSystemThreads() { SystemThreads::init(&posixThreads); }
	~SetSystemThreads() { SystemThreads::init(0); }
} initOnStartup;

}

};

#endif
