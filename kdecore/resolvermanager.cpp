/*  -*- C++ -*-
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <qapplication.h>
#include <qstring.h>
#include <qcstring.h>
#include <qptrlist.h>
#include <qmutex.h>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qsemaphore.h>

#include "resolver.h"		// knetwork/resolver.h
#include "resolver_p.h"
#include "resolverworkerbase.h"	// knetwork/resolverworkerbase.h

using namespace KDE::Network;
using namespace KDE::Internal::Resolver;

/*
 * Explanation on how the resolver system works

 * User-called events:
   When Resolver::startAsync is called, it calls Manager::queue to add
   an entry to the queue. Manager::queue must lock the mutex in order
   to add an entry to the queue newRequests, then unlock it. It then
   notifies the manager through a posted event.

 * Manager:
   The manager thread will then wake up during the event processing
   cycle. The first thing it does is lock its internal mutex. It will
   find out that there are new items to be processed and will then  
   launch a new request. That is, it will dequeue the item from the
   newRequests queue and will add one request to the currentRequests
   list. 
  
   When processing a new queued request, the manager will look at its
   table of available workers and choose the first one that claims to
   be able to do the job.
  
   It will then loop over all items on the currentRequests list. For
   items whose state is Queued, it'll try and start a worker thread. For
   all other items whose state isn't InProgress, it'll remove from the
   list. 
  
   If there are no running threads, the manager will create as many as
   needed for the current number of requests, but will not create more
   than a certain maximum. It'll then start them and signal new data
   through a wake-up-all call on the feedWorkers condition.

   Only then will the manager release its hold on the mutex.

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
   the thread posts an event to the manager.

 * Data requesting/releasing
   A worker thread always calls upon functions on the resolver manager
   in order to acquire and release data.

   When data is being requested, the ResolverManager::requestData
   function will look the currentRequests list and return the first
   Queued request it finds, while marking it to be InProgress.

   When the worker class has returned, the worker thread will release
   that data through the ResolverManager::releaseData function. If the
   worker class has requested no further data (nRequests == 0), the
   request's status is marked to be Done. It'll then look at the
   requestor for that data: if it was requested by another worker,
   it'll decrement the requests count for that one and add the results
   to a list. And, finally, if the requests count for the requestor
   becomes 0, it'll repeat this process for the requestor as well
   (change status to Done, check for a requestor).
 */

/*
 * parameters
 */
// a thread will try maxThreadRetries to get data, waiting at most
// maxThreadWaitTime milliseconds between each attempt. After that, it'll
// exit
static const int maxThreadWaitTime = 20000; // 20 seconds
static const int maxThreadRetries = 2;
static const int maxThreads = 5;


WorkerThread::WorkerThread()
  : data(0L)
{
}

// remember! This function runs in a separate thread!
void WorkerThread::run()
{
  // initialisation
  // enter the loop already
  int trycount = maxThreadRetries;

  while (trycount)
    {
      data = Manager::manager()->requestData(this);
      if (data)
	{
	  // yes, we got data
	  // process it!

	  // 1) set up
	  data->worker->th = this;

	  // 2) run it
	  data->worker->run();

	  // 3) release data
	  data->worker->th = 0L;
	  Manager::manager()->releaseData(this, data);

	  // now go back to the loop
	  trycount = maxThreadRetries; // reset it
	}
      else
	trycount--;

      // Wait for more data
      Manager::manager()->feedWorkers.wait(maxThreadWaitTime);
    }
}

static Manager *globalManager;

Manager* Manager::manager()
{
  if (globalManager == 0L)
    new Manager();
  return globalManager;
}

Manager::Manager()
  : QObject(qApp, "Resolver manager object"),
    workers(new WorkerThread[maxThreads])
{
  globalManager = this;
  currentRequests.setAutoDelete(true);
}

Manager::~Manager()
{
  // this should never be called

  // kill off running threads
  for (int i = 0; i <= maxThreads; i++)
    workers[i].terminate();
}

bool Manager::event(QEvent */*e*/)
{
  QPtrQueue<RequestData> doneRequests;
  // ok, here goes all the processing

  // 1) lock the mutex
  mutex.lock();

  // 2) check if there are new items in queue
  while (ResolverPrivate *p = newRequests.dequeue())
    {
      // 2a) for each item, we dequeue it and add
      // a request to the currentRequests

      RequestData *newrequest = new RequestData;
      newrequest->input = p;

      // 2b) when processing a new request, find the most
      // suitable worker
      if ((newrequest->worker = findWorker(p)) == 0L)
	{
	  // oops, problem
	  // cannot find a worker class for this guy
	  p->status = KDE::Network::Resolver::Failed;
	  p->errorcode = KDE::Network::Resolver::UnsupportedFamily;
	  p->syserror = 0;

	  doneRequests.enqueue(newrequest);
	}
      else
	{
	  // no, queue it
	  p->status = KDE::Network::Resolver::Queued;
	  currentRequests.append(newrequest);
	}
    }

  // 3) loop over all items on the current list
  int wantedThreads = 0;
  RequestData *curr = currentRequests.first(); 
  while (curr)
    if (curr->input->status == KDE::Network::Resolver::Queued)
      {
	// 3a) count all items whose status is Queued
	wantedThreads++;
	curr = currentRequests.next();
      }
    else if (curr->input->status != KDE::Network::Resolver::InProgress)
      {
	// 3b) for all items whose status isn't InProgress, 
	// remove from the list
	doneRequests.enqueue(currentRequests.take());
      }
      
  // 4) the manager will not create more than a maximum
  // number of threads
  if (wantedThreads > maxThreads)
    wantedThreads = maxThreads;

  // 4a) iterate over the available threads and start as many
  // as needed
  for (int i = 0; i < maxThreads && wantedThreads; i++)
    {
      if (!workers[i].running())
	workers[i].start();
      wantedThreads--;
    }

  // 4b) signal new data available
  feedWorkers.wakeAll();

  // 5) release the mutex
  mutex.unlock();

  // 6) extra processing: the done requests
  // we can do it now because the mutex is no longer locked
  while (RequestData *d = doneRequests.dequeue())
    doNotifying(d);

  return true;
}

RequestData* Manager::requestData(WorkerThread*)
{
}

void Manager::releaseData(WorkerThread *id, RequestData* data)
{
}

ResolverWorkerBase* Manager::findWorker(ResolverPrivate*)
{
}

void Manager::doNotifying(RequestData *p)
{
}

#include "resolver_p.moc"
