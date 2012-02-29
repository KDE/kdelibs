/*  -*- C++ -*-
 *  Copyright (C) 2003-2005 Thiago Macieira <thiago@kde.org>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <config.h>
#include <config-network.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <limits.h>
#include <unistd.h>		// only needed for pid_t

#ifdef HAVE_RES_INIT
# include <sys/stat.h>
extern "C" {
#   include <arpa/nameser.h>
}
# include <time.h>
# include <resolv.h>
#endif

#include <QByteArray>
#include <QCoreApplication>
#include <QList>
#include <QMutableListIterator>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>

#include <QThread>
#include <QTimer>
#include <QWaitCondition>

#include <kde_file.h>
#include <kdebug.h>
#include "k3resolver.h"
#include "k3resolver_p.h"
#include "k3resolverworkerbase.h"
#include "k3resolverstandardworkers_p.h"

using namespace KNetwork;
using namespace KNetwork::Internal;

/*
 * Explanation on how the resolver system works

   When KResolver::start is called, it calls KResolverManager::enqueue to add
   an entry to the queue. KResolverManager::enqueue will verify the availability
   of a worker thread: if one is available, it will dispatch the request to it.
   If no threads are available, it will then decide whether to launch a thread
   or to queue for the future.

   (This process is achieved by always queuing the new request, starting a
   new thread if necessary and then notifying of the availability of data
   to all worker threads).

 * Worker thread
   A new thread, when started, will enter its event loop
   immediately. That is, it'll first try to acquire new data to
   process, which means it will lock and unlock the manager mutex in
   the process.

   If it finds no new data, it'll wait on the feedWorkers condition
   for a certain maximum time. If that time expires and there's still
   no data, the thread will exit, in order to save system resources.

   If it finds data, however, it'll set up and call the worker class
   that has been selected by the manager. Once that worker is done,
   the thread releases the data through KResolverManager::releaseData.

 * Data requesting/releasing
   A worker thread always calls upon functions on the resolver manager
   in order to acquire and release data.

   When data is being requested, the KResolverManager::requestData
   function will look the currentRequests list and return the first
   Queued request it finds, while marking it to be InProgress.

   When the worker class has returned, the worker thread will release
   that data through the KResolverManager::releaseData function. If the
   worker class has requested no further data (nRequests == 0), the
   request's status is marked to be Done. It'll then look at the
   requestor for that data: if it was requested by another worker,
   it'll decrement the requests count for that one and add the results
   to a list. And, finally, if the requests count for the requestor
   becomes 0, it'll repeat this process for the requestor as well
   (change status to Done, check for a requestor).
 */

namespace
{

/*
 * This class is used to control the access to the
 * system's resolver API.
 *
 * It is necessary to periodically poll /etc/resolv.conf and reload
 * it if any changes are noticed. This class does exactly that.
 *
 * However, there's also the problem of reloading the structure while
 * some threads are in progress. Therefore, we keep a usage reference count.
 */
class ResInitUsage
{
public:

#ifdef HAVE_RES_INIT
  time_t mTime;
  int useCount;

# ifndef RES_INIT_THREADSAFE
  QWaitCondition cond;
  QMutex mutex;
# endif

  bool shouldResInit()
  {
    // check if /etc/resolv.conf has changed
    KDE_struct_stat st;
    if (KDE_stat("/etc/resolv.conf", &st) != 0)
      return false;

    if (mTime != st.st_mtime)
      {
	kDebug(179) << "shouldResInit: /etc/resolv.conf updated";
	return true;
      }
    return false;
  }

  void callResInit()
  {
    if (mTime != 0)
      {
	// don't call it the first time
	// let it be initialized naturally
	kDebug(179) << "callResInit: calling res_init()";
	res_init();
      }

    KDE_struct_stat st;
    if (KDE_stat("/etc/resolv.conf", &st) == 0)
      mTime = st.st_mtime;
  }

  ResInitUsage()
    : mTime(0), useCount(0)
  { }

  /*
   * Marks the end of usage to the resolver tools
   */
  void release()
  {
# ifndef RES_INIT_THREADSAFE
    QMutexLocker locker(&mutex);
    if (--useCount == 0)
      {
	if (shouldResInit())
	  callResInit();

	// we've reached 0, wake up anyone that's waiting to call res_init
	cond.wakeAll();
      }
# else
    // do nothing
# endif
  }

  /*
   * Marks the beginning of usage of the resolver API
   */
  void acquire()
  {
# ifndef RES_INIT_THREADSAFE
    mutex.lock();

    if (shouldResInit())
      {
	if (useCount)
	  {
	    // other threads are already using the API, so wait till
	    // it's all clear
	    // the thread that emits this condition will also call res_init
	    //qDebug("ResInitUsage: waiting for libresolv to be clear");
	    cond.wait(&mutex);
	  }
	else
	  // we're clear
	  callResInit();
      }
    useCount++;
    mutex.unlock();

# else
    if (shouldResInit())
      callResInit();

# endif
  }

#else
  ResInitUsage()
  { }

  bool shouldResInit()
  { return false; }

  void acquire()
  { }

  void release()
  { }
#endif

} resInit;

} // anonymous namespace

/*
 * parameters
 */
// a thread will try maxThreadRetries to get data, waiting at most
// maxThreadWaitTime milliseconds between each attempt. After that, it'll
// exit
static const int maxThreadWaitTime = 2000; // 2 seconds
static const int maxThreads = 5;

static pid_t pid;		// FIXME -- disable when everything is ok

KResolverThread::KResolverThread()
  : data(0L)
{
}

// remember! This function runs in a separate thread!
void KResolverThread::run()
{
  // initialization
  // enter the loop already

  //qDebug("KResolverThread(thread %u/%p): started", pid, (void*)QThread::currentThread());
  KResolverManager::manager()->registerThread(this);
  while (true)
    {
      data = KResolverManager::manager()->requestData(this, ::maxThreadWaitTime);
      //qDebug("KResolverThread(thread %u/%p) got data %p", KResolverManager::pid,
      //       (void*)QThread::currentThread(), (void*)data);
      if (data)
	{
	  // yes, we got data
	  // process it!

	  // 1) set up
	  ;

	  // 2) run it
	  data->worker->run();

	  // 3) release data
	  KResolverManager::manager()->releaseData(this, data);

	  // now go back to the loop
	}
      else
	break;
    }

  KResolverManager::manager()->unregisterThread(this);
  //qDebug("KResolverThread(thread %u/%p): exiting", pid, (void*)QThread::currentThread());
}

bool KResolverThread::checkResolver()
{
  return resInit.shouldResInit();
}

void KResolverThread::acquireResolver()
{
#if defined(NEED_MUTEX) && !defined(Q_OS_FREEBSD)
  getXXbyYYmutex.lock();
#endif

  resInit.acquire();
}

void KResolverThread::releaseResolver()
{
#if defined(NEED_MUTEX) && !defined(Q_OS_FREEBSD)
  getXXbyYYmutex.unlock();
#endif

  resInit.release();
}

static KResolverManager *globalManager;

KResolverManager* KResolverManager::manager()
{
  if (globalManager == 0L)
    new KResolverManager();
  return globalManager;
}

KResolverManager::KResolverManager()
  : runningThreads(0), availableThreads(0)
{
  globalManager = this;
  initStandardWorkers();

  pid = getpid();
}

KResolverManager::~KResolverManager()
{
  // this should never be called

  // kill off running threads
  foreach (KResolverThread* worker, workers)
    worker->terminate();
}

void KResolverManager::registerThread(KResolverThread* )
{
}

void KResolverManager::unregisterThread(KResolverThread*)
{
  runningThreads--;
}

// this function is called by KResolverThread::run
RequestData* KResolverManager::requestData(KResolverThread *th, int maxWaitTime)
{
  /////
  // This function is called in a worker thread!!
  /////

  // lock the mutex, so that the manager thread or other threads won't
  // interfere.
  QMutexLocker locker(&mutex);
  RequestData *data = findData(th);

  if (data)
    // it found something, that's good
    return data;

  // nope, nothing found; sleep for a while
  availableThreads++;
  feedWorkers.wait(&mutex, maxWaitTime);
  availableThreads--;

  data = findData(th);
  return data;
}

RequestData* KResolverManager::findData(KResolverThread* th)
{
  /////
  // This function is called by requestData() above and must
  // always be called with a locked mutex
  /////

  // now find data to be processed
  QMutableListIterator<RequestData*> it(newRequests);
  while (it.hasNext())
    {
      RequestData *curr = it.next();
      if (!curr->worker->m_finished)
	{
	  // found one
	  if (curr->obj)
	    curr->obj->status = KResolver::InProgress;
	  curr->worker->th = th;

	  // move it to the currentRequests list
	  it.remove();
	  currentRequests.append(curr);

	  return curr;
	}
    }

  // found nothing!
  return 0L;
}

// this function is called by KResolverThread::run
void KResolverManager::releaseData(KResolverThread *, RequestData* data)
{
  /////
  // This function is called in a worker thread!!
  /////

  //qDebug("KResolverManager::releaseData(%u/%p): %p has been released", pid,
//	 (void*)QThread::currentThread(), (void*)data);

  if (data->obj)
    {
      data->obj->status = KResolver::PostProcessing;
    }

  data->worker->m_finished = true;
  data->worker->th = 0L;	// this releases the object

  // handle finished requests
  handleFinished();
}

// this function is called by KResolverManager::releaseData above
void KResolverManager::handleFinished()
{
  bool redo = false;
  QQueue<RequestData*> doneRequests;

  mutex.lock();
  if (currentRequests.isEmpty())
    {
      mutex.unlock();
      return;
    }

  // loop over all items on the currently running list
  // we loop from the last to the first so that we catch requests
  // with "requestors" before we catch the requestor itself.
  QMutableListIterator<RequestData*> it(currentRequests);
  it.toBack();
  while (it.hasPrevious())
    {
      RequestData *curr = it.previous();
      if (curr->worker->th == 0L)
	{
	  if (handleFinishedItem(curr))
	    {
	      it.remove();
	      doneRequests.enqueue(curr);

	      if (curr->requestor &&
		  curr->requestor->nRequests == 0 &&
		  curr->requestor->worker->m_finished)
		// there's a requestor that is now finished
		redo = true;
	    }
	}
    }

  //qDebug("KResolverManager::handleFinished(%u): %d requests to notify", pid, doneRequests.count());
  while (!doneRequests.isEmpty())
    doNotifying(doneRequests.dequeue());

  mutex.unlock();

  if (redo)
    {
      //qDebug("KResolverManager::handleFinished(%u): restarting processing to catch requestor",
	//     pid);
      handleFinished();
    }
}

// This function is called by KResolverManager::handleFinished above
bool KResolverManager::handleFinishedItem(RequestData* curr)

{
  // for all items that aren't currently running, remove from the list
  // this includes all finished or canceled requests

  if (curr->worker->m_finished && curr->nRequests == 0)
    {
      // this one has finished
      if (curr->obj)
	curr->obj->status = KResolver::PostProcessing; // post-processing is run in doNotifying()

      if (curr->requestor)
	--curr->requestor->nRequests;

      //qDebug("KResolverManager::handleFinishedItem(%u): removing %p since it's done",
	//     pid, (void*)curr);
      return true;
    }
  return false;
}



void KResolverManager::registerNewWorker(KResolverWorkerFactoryBase *factory)
{
  workerFactories.append(factory);
}

KResolverWorkerBase* KResolverManager::findWorker(KResolverPrivate* p)
{
  /////
  // this function can be called on any user thread
  /////

  // this function is called with an unlocked mutex and it's expected to be
  // thread-safe!
  // but the factory list is expected not to be changed asynchronously

  // This function is responsible for finding a suitable worker for the given
  // input. That means we have to do a costly operation to create each worker
  // class and call their preprocessing functions. The first one that
  // says they can process (i.e., preprocess() returns true) will get the job.

  foreach (KResolverWorkerFactoryBase *factory, workerFactories)
    {
      KResolverWorkerBase *worker = factory->create();

      // set up the data the worker needs to preprocess
      worker->input = &p->input;

      if (worker->preprocess())
	{
	  // good, this one says it can process
	  if (worker->m_finished)
	    p->status = KResolver::PostProcessing;
	  else
	    p->status = KResolver::Queued;
	  return worker;
	}

      // no, try again
      delete worker;
    }

  // found no worker
  return 0L;
}

void KResolverManager::doNotifying(RequestData *p)
{
  /////
  // This function may be called on any thread
  // any thread at all: user threads, GUI thread, manager thread or worker thread
  /////

  // Notification and finalisation
  //
  // Once a request has finished the normal processing, we call the
  // post processing function.
  //
  // After that is done, we will consolidate all results in the object's
  // KResolverResults and then post an event indicating that the signal
  // be emitted
  //
  // In case we detect that the object is waiting for completion, we do not
  // post the event, for KResolver::wait will take care of emitting the
  // signal.
  //
  // Once we release the mutex on the object, we may no longer reference it
  // for it might have been deleted.

  // "User" objects are those that are not created by the manager. Note that
  // objects created by worker threads are considered "user" objects. Objects
  // created by the manager are those created for KResolver::resolveAsync.
  // We should delete them.

  if (p->obj)
    {
      // lock the object
      p->obj->mutex.lock();
      KResolver* parent = p->obj->parent; // is 0 for non-"user" objects
      KResolverResults& r = p->obj->results;

      if (p->obj->status == KResolver::Canceled)
	{
	  p->obj->status = KResolver::Canceled;
	  p->obj->errorcode = KResolver::Canceled;
	  p->obj->syserror = 0;
	  r.setError(KResolver::Canceled, 0);
	}
      else if (p->worker)
	{
	  // post processing
	  p->worker->postprocess();	// ignore the result

	  // copy the results from the worker thread to the final
	  // object
	  r = p->worker->results;

	  // reset address
	  r.setAddress(p->input->node, p->input->service);

	  //qDebug("KResolverManager::doNotifying(%u/%p): for %p whose status is %d and has %d results",
		 //pid, (void*)QThread::currentThread(), (void*)p, p->obj->status, r.count());

	  p->obj->errorcode = r.error();
	  p->obj->syserror = r.systemError();
	  p->obj->status = !r.isEmpty() ?
	    KResolver::Success : KResolver::Failed;
	}
      else
	{
	  r.empty();
	  r.setError(p->obj->errorcode, p->obj->syserror);
	}

      // check whether there's someone waiting
      if (!p->obj->waiting && parent)
	// no, so we must post an event requesting that the signal be emitted
	// sorry for the C-style cast, but neither static nor reintepret cast work
	// here; I'd have to do two casts
	QCoreApplication::postEvent(parent, new QEvent((QEvent::Type)(ResolutionCompleted)));

      // release the mutex
      p->obj->mutex.unlock();
    }
  else
    {
      // there's no object!
      if (p->worker)
	p->worker->postprocess();
    }

  delete p->worker;

  // ignore p->requestor and p->nRequests
  // they have been dealt with by the main loop

  delete p;

  // notify any objects waiting in KResolver::wait
  notifyWaiters.wakeAll();
}

// enqueue a new request
// this function is called from KResolver::start and
// from KResolverWorkerBase::enqueue
void KResolverManager::enqueue(KResolver *obj, RequestData *requestor)
{
  RequestData *newrequest = new RequestData;
  newrequest->nRequests = 0;
  newrequest->obj = obj->d;
  newrequest->input = &obj->d->input;
  newrequest->requestor = requestor;

  // when processing a new request, find the most
  // suitable worker
  if ((newrequest->worker = findWorker(obj->d)) == 0L)
    {
      // oops, problem
      // cannot find a worker class for this guy
      obj->d->status = KResolver::Failed;
      obj->d->errorcode = KResolver::UnsupportedFamily;
      obj->d->syserror = 0;

      doNotifying(newrequest);
      return;
    }

  // no, queue it
  // p->status was set in findWorker!
  if (requestor)
    requestor->nRequests++;

  if (!newrequest->worker->m_finished)
    dispatch(newrequest);
  else if (newrequest->nRequests > 0)
    {
      mutex.lock();
      currentRequests.append(newrequest);
      mutex.unlock();
    }
  else
    // already done
    doNotifying(newrequest);
}

// a new request has been created
// dispatch it
void KResolverManager::dispatch(RequestData *data)
{
  // As stated in the beginning of the file, this function
  // is supposed to verify the availability of threads, start
  // any if necessary

  QMutexLocker locker(&mutex);

  // add to the queue
  newRequests.append(data);

  // check if we need to start a new thread
  //
  // we depend on the variables availableThreads and runningThreads to
  // know if we are supposed to start any threads:
  // - if availableThreads > 0, then there is at least one thread waiting,
  //    blocked in KResolverManager::requestData. It can't unblock
  //    while we are holding the mutex locked, therefore we are sure that
  //    our event will be handled
  // - if availableThreads == 0:
  //   - if runningThreads < maxThreads
  //     we will start a new thread, which will certainly block in
  //     KResolverManager::requestData because we are holding the mutex locked
  //   - if runningThreads == maxThreads
  //     This situation generally means that we have already maxThreads running
  //     and that all of them are processing. We will not start any new threads,
  //     but will instead wait for one to finish processing and request new data
  //
  //     There's a possible race condition here, which goes unhandled: if one of
  //     threads has timed out waiting for new data and is in the process of
  //     exiting. In that case, availableThreads == 0 and runningThreads will not
  //     have decremented yet. This means that we will not start a new thread
  //     that we could have. However, since there are other threads working, our
  //     event should be handled soon.
  //     It won't be handled if and only if ALL threads are in the process of
  //     exiting. That situation is EXTREMELY unlikely and is not handled either.
  //
  if (availableThreads == 0 && runningThreads < maxThreads)
    {
      // yes, a new thread should be started

      // find if there's a finished one
      KResolverThread *th = 0L;
      for (int i = 0; i < workers.size(); ++i)
	if (!workers[i]->isRunning())
	  {
	    th = workers[i];
	    break;
	  }

      if (th == 0L)
	{
	  // no, create one
	  th = new KResolverThread;
	  workers.append(th);
	}

      th->start();
      runningThreads++;
    }

  feedWorkers.wakeAll();

  // clean up idle threads
  QMutableListIterator<KResolverThread*> it(workers);
  while (it.hasNext())
    {
      KResolverThread *worker = it.next();
      if (!worker->isRunning())
	{
	  it.remove();
	  delete worker;
	}
    }
}

// this function is called by KResolverManager::dequeue
bool KResolverManager::dequeueNew(KResolver* obj)
{
  // This function must be called with a locked mutex
  // Deadlock warning:
  // always lock the global mutex first if both mutexes must be locked

  KResolverPrivate *d = obj->d;

  // check if it's in the new request list
  for (QMutableListIterator<RequestData*> it(newRequests);
       it.hasNext(); )
    {
      RequestData *curr = it.next();
      if (curr->obj == d)
	{
	  // yes, this object is still in the list
	  // but it has never been processed
	  d->status = KResolver::Canceled;
	  d->errorcode = KResolver::Canceled;
	  d->syserror = 0;
	  it.remove();

	  delete curr->worker;
	  delete curr;

	  return true;
	}
    }

  // check if it's running
  for (int i = 0; i < currentRequests.size(); ++i)
    {
      RequestData* curr = currentRequests[i];
      if (curr->obj == d)
	{
	  // it's running. We cannot simply take it out of the list.
	  // it will be handled when the thread that is working on it finishes
	  d->mutex.lock();

	  d->status = KResolver::Canceled;
	  d->errorcode = KResolver::Canceled;
	  d->syserror = 0;

	  // disengage from the running threads
	  curr->obj = 0L;
	  curr->input = 0L;
	  if (curr->worker)
	    curr->worker->input = 0L;

	  d->mutex.unlock();
	}
    }

  return false;
}

// this function is called by KResolver::cancel
// it's expected to be thread-safe
void KResolverManager::dequeue(KResolver *obj)
{
  QMutexLocker locker(&mutex);
  dequeueNew(obj);
}
