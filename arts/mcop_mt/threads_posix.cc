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

#include "thread.h"

namespace Arts {
namespace PosixThreads {

class Mutex_impl : public Arts::Mutex_impl {
protected:
	pthread_mutex_t mutex;
public:
	Mutex_impl()
	{
		pthread_mutex_init(&mutex,0);
	}
	void lock()
	{
		pthread_mutex_lock(&mutex);
	}
	void unlock()
	{
		pthread_mutex_unlock(&mutex);
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
	};
	Arts::Thread_impl *createThread_impl(Arts::Thread *thread) {
		return new Thread_impl(thread);
	};
};

// set posix threads on startup
static class SetSystemThreads {
private:
	PosixThreads posixThreads;

public:
	SetSystemThreads() { SystemThreads::init(&posixThreads); }
	~SetSystemThreads() { SystemThreads::init(0); }
} initOnStartup;

};
};
#endif
