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

#ifndef KRESOLVER_P_H
#define KRESOLVER_P_H

#include <qstring.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qptrqueue.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qevent.h>
//#include "knetwork/resolver.h"

namespace KDE
{
  namespace Network
  {

    // defined in network/resolverworkerbase.h
    class ResolverWorkerBase;

    class ResolverPrivate
    {
    public:
      // parent class. Should never be changed!
      Resolver* parent;
      bool deleteWhenDone;

      // class status. Should not be changed by worker threads!
      volatile int status;
      int errorcode, syserror;

      // input data. Should not be changed by worker threads!
      QString node, service;
      QCString protocolName;
      int flags;
      int familyMask;
      int socktype;
      int protocol;

      // output data
      // only the manager may change these (no need for a mutex)
      volatile KDE::Network::ResolverResults results;
      //QMutex mutex;

      ResolverPrivate(Resolver* _parent,
		      const QString& _node = QString::null, 
		      const QString& _service = QString::null)
	: parent(_parent), deleteWhenDone(false),
	  status(0), errorcode(0), syserror(0),
	  node(_node), service(_service),
	  familyMask(Resolver::AnyFamily), socktype(0), protocol(0)
      {
      }
    };

  } // namespace KDE::Network

  namespace Internal
  {
    namespace Resolver
    {
      class Manager;
      class WorkerThread;

      struct RequestData
      {
	// worker threads should not change values in the input data
	const KDE::Network::ResolverPrivate *input;
	KDE::Network::ResolverWorkerBase *worker; // worker class
	KDE::Network::ResolverWorkerBase *requestor; // class that requested us

	int nRequests;		// how many requests that we made we still have left
	QValueList<KDE::Network::ResolverResults> resultList;

	RequestData() :
	  input(0L), worker(0L), requestor(0L), nRequests(0)
	{ }
      };

      /*
       * @internal
       * This class is the resolver manager
       */
      class Manager: public QObject
      {
	Q_OBJECT
      public:
	QWaitCondition feedWorkers;

      private:
	// this mutex protects the data in this object
	QMutex mutex;

	// hold a list of all the current threads we have
	WorkerThread* workers;

	// hold a list of all the requests we have
	QPtrList<RequestData> currentRequests;
	QPtrQueue<KDE::Network::ResolverPrivate> newRequests;

	// private constructor
	Manager();

      public:
	static Manager* manager();	// creates and returns the global manager

	// destructor
	virtual ~Manager();

	// the actual handler
	virtual bool event(QEvent *e);

	/*
	 * Requests new data to work on
	 *
	 * This function should only be called from a worker thread. This function
	 * is thread-safe.
	 *
	 * If there is data to be worked on, this function will return it. If there is
	 * none, this function will return a null pointer.
	 */
	RequestData* requestData(WorkerThread* id);

	/*
	 * Releases the resources and returns the resolved data.
	 *
	 * This function should only be called from a worker thread. It is
	 * thread-safe. It does not post the event to the manager.
	 */
	void releaseData(WorkerThread *id, RequestData* data);

      private:
	/*
	 * finds a suitable worker for this request
	 */
	KDE::Network::ResolverWorkerBase *findWorker(KDE::Network::ResolverPrivate *p);

	/*
	 * Notifies the parent class that this request is done.
	 *
	 * This function deletes the request
	 */
        void doNotifying(RequestData *p);
      };

      /*
       * @internal
       * This class is a worker thread in the resolver system.
       * This class must be thread-safe.
       */
      class WorkerThread: public QThread
      {
      public:
	// private constructor. Only the manager can create worker threads
	WorkerThread();

      private:
	RequestData* data;
  
      protected:
	virtual void run();		// here the thread starts

	friend class Manager;
	friend class KDE::Network::ResolverWorkerBase;
      };

    }
  }
}


#endif
