/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>
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

#ifndef QRESOLVER_P_H
#define QRESOLVER_P_H

#include <config.h>

#include <qstring.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qptrqueue.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qsemaphore.h>
#include <qevent.h>

#include "qtaddon.h"
#include "qresolver.h"

/* decide whether we need a mutex */
#if !defined(HAVE_GETPROTOBYNAME_R) || !defined(HAVE_GETSERVBYNAME_R) || !defined(HAVE_GETHOSTBYNAME_R)
# define NEED_MUTEX
extern QMutex getXXbyYYmutex;
#endif

ADDON_START

// defined in network/qresolverworkerbase.h
class QResolverWorkerBase;
class QResolverWorkerFactoryBase;

class QResolverPrivate
{
public:
  // parent class. Should never be changed!
  QResolver* parent;
  bool deleteWhenDone : 1;
  bool waiting : 1;
  mutable bool emitSignal : 1;

  // class status. Should not be changed by worker threads!
  volatile int status;
  volatile int errorcode, syserror;

  // input data. Should not be changed by worker threads!
  struct InputData
  {
    QString node, service;
    QCString protocolName;
    int flags;
    int familyMask;
    int socktype;
    int protocol;
  } input;

  // mutex
  QMutex mutex;

  // output data
  QResolverResults results;

  QResolverPrivate(QResolver* _parent,
		   const QString& _node = QString::null, 
		   const QString& _service = QString::null)
    : parent(_parent), deleteWhenDone(false), waiting(false), emitSignal(true),
      status(0), errorcode(0), syserror(0)
  {
    input.node = _node;
    input.service = _service;
    input.flags = 0;
    input.familyMask = QResolver::AnyFamily;
    input.socktype = 0;
    input.protocol = 0;

    results.setAddress(_node, _service);
  }
};

namespace Internal
{
  class QResolverManager;
  class QResolverThread;

  struct RequestData
  {
    // worker threads should not change values in the input data
    QResolverPrivate *obj;
    const QResolverPrivate::InputData *input;
    QResolverWorkerBase *worker; // worker class
    RequestData *requestor; // class that requested us

    volatile int nRequests; // how many requests that we made we still have left
  };

  /*
   * @internal
   * This class is the resolver manager
   */
  class QResolverManager
  {
  public:
    enum EventTypes
    { ResolutionCompleted = 1576 }; // arbitrary value;

    /*
     * This wait condition is used to notify wait states (QResolver::wait) that
     * the resolver manager has finished processing one or more objects. All
     * objects in wait state will be woken up and will check if they are done.
     * If they aren't, they will go back to sleeping.
     */
    QWaitCondition notifyWaiters;

  private:
    /*
     * This variable is used to count the number of threads that are running
     */
    volatile unsigned short runningThreads;

    /*
     * This variable is used to count the number of threads that are currently
     * waiting for data.
     */
    unsigned short availableThreads;

    /*
     * This wait condition is used to notify worker threads that there is new
     * data available that has to be processed. All worker threads wait on this
     * waitcond for a limited amount of time.
     */
    QWaitCondition feedWorkers;

    // this mutex protects the data in this object
    QMutex mutex;

    // hold a list of all the current threads we have
    QPtrList<QResolverThread> workers;

    // hold a list of all the new requests we have
    QPtrList<RequestData> newRequests;

    // hold a list of all the requests in progress we have
    QPtrList<RequestData> currentRequests;

    // hold a list of all the workers we have
    QPtrList<QResolverWorkerFactoryBase> workerFactories;

    // private constructor
    QResolverManager();

  public:
    static QResolverManager* manager();	// creates and returns the global manager

    // destructor
    ~QResolverManager();

    /*
     * Register this thread in the pool
     */
    void registerThread(QResolverThread* id);

    /*
     * Unregister this thread from the pool
     */
    void unregisterThread(QResolverThread* id);

    /*
     * Requests new data to work on.
     *
     * This function should only be called from a worker thread. This function
     * is thread-safe.
     *
     * If there is data to be worked on, this function will return it. If there is
     * none, this function will return a null pointer.
     */
    RequestData* requestData(QResolverThread* id, int maxWaitTime);

    /*
     * Releases the resources and returns the resolved data.
     *
     * This function should only be called from a worker thread. It is
     * thread-safe. It does not post the event to the manager.
     */
    void releaseData(QResolverThread *id, RequestData* data);

    /*
     * Registers a new worker class by way of its factory.
     *
     * This function is NOT thread-safe.
     */
    void registerNewWorker(QResolverWorkerFactoryBase *factory);

    /*
     * Enqueues new resolutions.
     */
    void enqueue(QResolver *obj, RequestData* requestor);

    /*
     * Dispatch a new request
     */
    void dispatch(RequestData* data);

    /*
     * Dequeues a resolution.
     */
    void dequeue(QResolver *obj);

    /*
     * Notifies the manager that the given resolution is about to
     * be deleted. This function should only be called by the
     * QResolver destructor.
     */
    void aboutToBeDeleted(QResolver *obj);

    /*
     * Notifies the manager that new events are ready.
     */
    void newEvent();

    /*
     * This function is called by the manager to receive a new event. It operates
     * on the @ref eventSemaphore semaphore, which means it will block till there
     * is at least one event to go.
     */
    void receiveEvent();

  private:
    /*
     * finds a suitable worker for this request
     */
    QResolverWorkerBase *findWorker(QResolverPrivate *p);

    /*
     * finds data for this request
     */
    RequestData* findData(QResolverThread*);

    /*
     * Handle completed requests.
     *
     * This function is called by releaseData above
     */
    void handleFinished();

    /*
     * Handle one completed request.
     *
     * This function is called by handleFinished above.
     */
    bool handleFinishedItem(RequestData* item);

    /*
     * Notifies the parent class that this request is done.
     *
     * This function deletes the request
     */
    void doNotifying(RequestData *p);

    /*
     * Dequeues and notifies an object that is in Queued state
     * Returns true if the object is no longer queued; false if it could not 
     * be dequeued (i.e., it's running)
     */
    bool dequeueNew(QResolver* obj);
  };

  /*
   * @internal
   * This class is a worker thread in the resolver system.
   * This class must be thread-safe.
   */
  class QResolverThread: public QThread
  {
  public:
    // private constructor. Only the manager can create worker threads
    QResolverThread();
    RequestData* data;
  
  protected:
    virtual void run();		// here the thread starts

    friend class QResolverManager;
    friend class QResolverWorkerBase;
  };

}

ADDON_END

#endif
