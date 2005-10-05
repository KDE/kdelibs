/*
   This file is part of the KDE libraries
   Copyright (C) 2005 Andreas Roth <aroth@arsoft-online.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <stdio.h>
#include <qeventloop.h>
#include <qapplication.h>
#include "qeventloopex.h"


// Local defined structures and classes (needed only for QEventLoopEx implementation)
struct QSockNotEx
{
	QSocketNotifier *obj;
	int fd;
	fd_set *queue;
};

class QSockNotTypeEx
{
public:
	QSockNotTypeEx();
	~QSockNotTypeEx();

	QPtrList<QSockNotEx> *list;
	fd_set select_fds;
	fd_set enabled_fds;
	fd_set pending_fds;

};

class QEventLoopExPrivate
{
public:
	QEventLoopExPrivate()
	{
		reset();
	}

	void reset() {
		m_bStopped = false;
		m_hThread = NULL;
		m_sockUpdate = 0;
		m_evPendingListEmpty = NULL;
		sn_highest = 0;
	}
	bool m_bStopped;

	HANDLE m_hThread;
	SOCKET m_sockUpdate;

	// pending socket notifiers list
	QPtrList<QSockNotEx> sn_pending_list;
	HANDLE m_evPendingListEmpty;
	CRITICAL_SECTION m_csPendingList;

	// highest fd for all socket notifiers
	int sn_highest;
	// 3 socket notifier types - read, write and exception
	QSockNotTypeEx sn_vec[3];
	CRITICAL_SECTION m_csVec;
};


/*****************************************************************************
 Socket notifier type
 *****************************************************************************/
QSockNotTypeEx::QSockNotTypeEx()
	: list( 0 )
{
	FD_ZERO( &select_fds );
	FD_ZERO( &enabled_fds );
	FD_ZERO( &pending_fds );
}

QSockNotTypeEx::~QSockNotTypeEx()
{
	delete list;
	list = 0;
}

QEventLoopEx::QEventLoopEx( QObject *parent, const char *name) : 
	QEventLoop(parent,name)
{
	DWORD dwThreadId;

#ifdef _DEBUG_EVENTLOOPEX
	qDebug( "QEventLoopEx::QEventLoopEx enter");
#endif
	d = new QEventLoopExPrivate;

	InitializeCriticalSection(&d->m_csVec);
	InitializeCriticalSection(&d->m_csPendingList);

	d->m_evPendingListEmpty = CreateEvent(NULL,TRUE,FALSE,NULL);

	d->m_sockUpdate = socket(AF_INET,SOCK_DGRAM,0);
	d->m_hThread = CreateThread(NULL,0,ThreadProc,this,0,&dwThreadId);
#ifdef _DEBUG_EVENTLOOPEX
	qDebug( "QEventLoopEx::QEventLoopEx leave");
#endif	
}

QEventLoopEx::~QEventLoopEx()
{
#ifdef _DEBUG_EVENTLOOPEX	
	qDebug( "QEventLoopEx::~QEventLoopEx enter");
#endif
	d->m_bStopped = true;
	// Ensure that you thread gets unblocked and can terminate gracefully
	SetEvent(d->m_evPendingListEmpty);
	
	closesocket(d->m_sockUpdate);

	WaitForSingleObject(d->m_hThread,INFINITE);
	CloseHandle(d->m_hThread);

	CloseHandle(d->m_evPendingListEmpty);
	DeleteCriticalSection(&d->m_csVec);
	DeleteCriticalSection(&d->m_csPendingList);
	
	delete d;
	
#ifdef _DEBUG_EVENTLOOPEX
	qDebug( "QEventLoopEx::~QEventLoopEx leave");
#endif
}



/*****************************************************************************
 QEventLoopEx implementations for Windows (for synchronous socket calls)
 *****************************************************************************/
void QEventLoopEx::registerSocketNotifier( QSocketNotifier *notifier )
{
	int sockfd = notifier->socket();
	int type = notifier->type();
	u_long	n;
	DWORD dw;
#ifdef _DEBUG_EVENTLOOPEX
	qDebug( "QSocketNotifier::registerSocketNotifier %p", notifier );
#endif
	if(ioctlsocket(sockfd,FIONREAD,&n) == SOCKET_ERROR)
	{
#ifdef _DEBUG_EVENTLOOPEX
		qDebug( "QSocketNotifier::registerSocketNotifier %p not a socket", notifier );
#endif
		dw = WSAGetLastError();
		QEventLoop::registerSocketNotifier(notifier);
		return;
	}

	if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) 
	{
#if defined(QT_CHECK_RANGE)
		qWarning( "QSocketNotifier: Internal error" );
#endif
		return;
	}

	EnterCriticalSection(&d->m_csVec);

	QPtrList<QSockNotEx>  *list = d->sn_vec[type].list;
	fd_set *fds  = &d->sn_vec[type].enabled_fds;
	QSockNotEx *sn;

	if ( ! list ) {
		// create new list, the QSockNotType destructor will delete it for us
		list = new QPtrList<QSockNotEx>;
		Q_CHECK_PTR( list );
		list->setAutoDelete( TRUE );
		d->sn_vec[type].list = list;
	}

	sn = new QSockNotEx;
	Q_CHECK_PTR( sn );
	sn->obj = notifier;
	sn->fd = sockfd;
	sn->queue = &d->sn_vec[type].pending_fds;

	if ( list->isEmpty() ) {
		list->insert( 0, sn );
	} else { // sort list by fd, decreasing
		QSockNotEx *p = list->first();
		while ( p && p->fd > sockfd )
			p = list->next();
		if ( p )
			list->insert( list->at(), sn );
		else
			list->append( sn );
	}

	FD_SET( sockfd, fds );

	d->sn_highest = QMAX( d->sn_highest, sockfd );
	LeaveCriticalSection(&d->m_csVec);
	
#ifdef _DEBUG_EVENTLOOPEX
	qDebug( "QSocketNotifier::signal update socket");
#endif	
	closesocket(d->m_sockUpdate);
}

void QEventLoopEx::unregisterSocketNotifier( QSocketNotifier *notifier )
{
	int sockfd = notifier->socket();
	int type = notifier->type();
	if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
		qWarning( "QSocketNotifier: Internal error" );
#endif
		return;
	}
#ifdef _DEBUG_EVENTLOOPEX	
	qDebug( "QSocketNotifier::unregisterSocketNotifier %p", notifier );
#endif

	EnterCriticalSection(&d->m_csVec);
	QPtrList<QSockNotEx> *list = d->sn_vec[type].list;
	fd_set *fds  =  &d->sn_vec[type].enabled_fds;
	QSockNotEx *sn;
	if ( ! list ) {
		LeaveCriticalSection(&d->m_csVec);
		QEventLoop::unregisterSocketNotifier(notifier);
		return;
	}
	sn = list->first();
	while ( sn && !(sn->obj == notifier && sn->fd == sockfd) )
	sn = list->next();
	if ( !sn ) {// not found
		LeaveCriticalSection(&d->m_csVec);
		QEventLoop::unregisterSocketNotifier(notifier);
		return;
	}

	FD_CLR( sockfd, fds ); // clear fd bit
	FD_CLR( sockfd, sn->queue );

	EnterCriticalSection(&d->m_csPendingList);
	d->sn_pending_list.removeRef( sn );		// remove from activation list
	bool bNowEmpty = (d->sn_pending_list.count() == 0);
	LeaveCriticalSection(&d->m_csPendingList);
	if(bNowEmpty)
		SetEvent(d->m_evPendingListEmpty);
	list->remove(); // remove notifier found above

	if ( d->sn_highest == sockfd ) {// find highest fd
		d->sn_highest = -1;
		for ( int i=0; i<3; i++ ) {
			if ( d->sn_vec[i].list && ! d->sn_vec[i].list->isEmpty() )
			d->sn_highest = QMAX( d->sn_highest,  // list is fd-sorted
				d->sn_vec[i].list->getFirst()->fd );
		}
	}
	LeaveCriticalSection(&d->m_csVec);
#ifdef _DEBUG_EVENTLOOPEX	
	qDebug( "QSocketNotifier::signal update socket");
#endif
	closesocket(d->m_sockUpdate);
}

bool QEventLoopEx::processEvents( ProcessEventsFlags flags )
{
	if(!QEventLoop::processEvents(flags))
		return false;

	activateSocketNotifiers();
	return true;
}

void QEventLoopEx::setSocketNotifierPending( QSocketNotifier *notifier )
{
	int sockfd = notifier->socket();
	int type = notifier->type();
	if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) 
	{
#if defined(QT_CHECK_RANGE)
		qWarning( "QSocketNotifier: Internal error" );
#endif
		return;
	}
#ifdef _DEBUG_EVENTLOOPEX	
	qDebug( "QSocketNotifier::setSocketNotifierPending %p",notifier );
#endif
	EnterCriticalSection(&d->m_csVec);

	QPtrList<QSockNotEx> *list = d->sn_vec[type].list;
	QSockNotEx *sn;
	if ( ! list )
	{
#ifdef _DEBUG_EVENTLOOPEX	
		qDebug( "QSocketNotifier::setSocketNotifierPending %p: no list",notifier );
#endif
		LeaveCriticalSection(&d->m_csVec);
		return;
	}
	sn = list->first();
	while ( sn && !(sn->obj == notifier && sn->fd == sockfd) )
	sn = list->next();
	if ( ! sn ) { // not found
#ifdef _DEBUG_EVENTLOOPEX	
		qDebug( "QSocketNotifier::setSocketNotifierPending %p: not found",notifier );
#endif
		LeaveCriticalSection(&d->m_csVec);
		return;
	}

	// We choose a random activation order to be more fair under high load.
	// If a constant order is used and a peer early in the list can
	// saturate the IO, it might grab our attention completely.
	// Also, if we're using a straight list, the callback routines may
	// delete other entries from the list before those other entries are
	// processed.
	EnterCriticalSection(&d->m_csPendingList);
	if ( ! FD_ISSET( sn->fd, sn->queue ) ) {
		d->sn_pending_list.insert( (rand() & 0xff) %
				   (d->sn_pending_list.count()+1), sn );
		FD_SET( sn->fd, sn->queue );
	}
	LeaveCriticalSection(&d->m_csPendingList);
	LeaveCriticalSection(&d->m_csVec);
}

int QEventLoopEx::activateSocketNotifiers()
{
	if ( d->sn_pending_list.isEmpty() )
		return 0; // nothing to do

	int n_act = 0;
	QEvent event( QEvent::SockAct );

	EnterCriticalSection(&d->m_csVec);			// Avoid deaklock
	EnterCriticalSection(&d->m_csPendingList);
	QPtrListIterator<QSockNotEx> it( d->sn_pending_list );
	QSockNotEx *sn;
	while ( (sn=it.current()) ) {
		++it;
		d->sn_pending_list.removeRef( sn );
		if ( FD_ISSET(sn->fd, sn->queue) ) {
			FD_CLR( sn->fd, sn->queue );
#ifdef _DEBUG_EVENTLOOPEX	
			qDebug("QEventLoopEx:activateSocketNotifiers %p to object %p",sn, sn->obj);
#endif
			QApplication::sendEvent( sn->obj, &event );
			n_act++;
		}
	}

	LeaveCriticalSection(&d->m_csPendingList);
	LeaveCriticalSection(&d->m_csVec);			// Avoid deaklock

#ifdef _DEBUG_EVENTLOOPEX	
	qDebug( "QSocketNotifier::activateSocketNotifiers set m_evPendingListEmpty");
#endif
	SetEvent(d->m_evPendingListEmpty);

	return n_act;
}

DWORD QEventLoopEx::ThreadProc(void * p)
{
	QEventLoopEx * pEventLoopEx = static_cast<QEventLoopEx *>(p);
	pEventLoopEx->run();
	return 0;
}

void QEventLoopEx::run()
{
	do 
	{
		EnterCriticalSection(&d->m_csVec);
		// return the highest fd we can wait for input on
		if ( d->sn_highest >= 0 ) { // has socket notifier(s)
			if ( d->sn_vec[0].list && ! d->sn_vec[0].list->isEmpty() )
				d->sn_vec[0].select_fds = d->sn_vec[0].enabled_fds;
			else
				FD_ZERO( &d->sn_vec[0].select_fds );

			if ( d->sn_vec[1].list && ! d->sn_vec[1].list->isEmpty() )
				d->sn_vec[1].select_fds = d->sn_vec[1].enabled_fds;
			else
				FD_ZERO( &d->sn_vec[1].select_fds );

			if ( d->sn_vec[2].list && ! d->sn_vec[2].list->isEmpty() )
				d->sn_vec[2].select_fds = d->sn_vec[2].enabled_fds;
			else
				FD_ZERO( &d->sn_vec[2].select_fds );
		} 
		else {
			FD_ZERO( &d->sn_vec[0].select_fds );
			FD_ZERO( &d->sn_vec[1].select_fds );
			FD_ZERO( &d->sn_vec[2].select_fds );
		}

		FD_SET(d->m_sockUpdate,&d->sn_vec[0].select_fds);
		d->sn_highest = QMAX(d->sn_highest,(int)d->m_sockUpdate);
//		FD_SET(m_sockUpdate,&sn_vec[1].select_fds);
//		FD_SET(m_sockUpdate,&sn_vec[2].select_fds);

		LeaveCriticalSection(&d->m_csVec);

#ifdef _DEBUG_EVENTLOOPEX	
		qDebug("QEventLoopEx: select(%d,%d, %d, %d)",sn_highest,sn_vec[0].select_fds.fd_count,sn_vec[1].select_fds.fd_count,sn_vec[2].select_fds.fd_count);
#endif
		int nsel = select( d->sn_highest,
				&d->sn_vec[0].select_fds,
				&d->sn_vec[1].select_fds,
				&d->sn_vec[2].select_fds,
				NULL );
#ifdef _DEBUG_EVENTLOOPEX	
		qDebug("QEventLoopEx: select returned %d",nsel);
#endif
		if (nsel == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAENOTSOCK) {

				
				// it seems a socket notifier has a bad fd... find out
				// which one it is and disable it
				fd_set fdset;
				struct timeval zerotm;
				int ret;
				zerotm.tv_sec = zerotm.tv_usec = 0l;

				FD_ZERO(&fdset);
				FD_SET(d->m_sockUpdate, &fdset);

				ret = select(d->m_sockUpdate + 1, &fdset, 0, 0, &zerotm);
				if(ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK)
				{
					// Update the waiting sockets
					d->m_sockUpdate = socket(AF_INET,SOCK_DGRAM,0);
				}
				else
				{
					EnterCriticalSection(&d->m_csVec);
					
					for (int type = 0; type < 3; ++type)
					{
						QPtrList<QSockNotEx> *list = d->sn_vec[type].list;
						if (!list) 
							continue;

						QSockNotEx *sn = list->first();
						while (sn) {
							FD_ZERO(&fdset);
							FD_SET(sn->fd, &fdset);

							
							do {
								switch (type) {
								case 0: // read
									ret = select(sn->fd + 1, &fdset, 0, 0, &zerotm);
									break;
								case 1: // write
									ret = select(sn->fd + 1, 0, &fdset, 0, &zerotm);
									break;
								case 2: // except
									ret = select(sn->fd + 1, 0, 0, &fdset, &zerotm);
									break;
								}
							} while (ret == SOCKET_ERROR && WSAGetLastError() != WSAENOTSOCK);

							if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK)
							{
								// disable the invalid socket notifier
								static const char *t[] = { "Read", "Write", "Exception" };
								qWarning("QSocketNotifier: invalid socket %d and type '%s', disabling...",
									sn->fd, t[type]);
								sn->obj->setEnabled(FALSE);
							}

							sn = list->next();
						}
					}
					LeaveCriticalSection(&d->m_csVec);
				}
			} else {
				// EINVAL... shouldn't happen, so let's complain to stderr
				// and hope someone sends us a bug report
				DWORD dw = WSAGetLastError();
				qWarning("QEventLoopEx: select failed with error %i\n",dw);
			}
		}
		else
		{
			EnterCriticalSection(&d->m_csVec);

			if(FD_ISSET( d->m_sockUpdate, &d->sn_vec[0].select_fds))
			{
				d->m_sockUpdate = socket(AF_INET,SOCK_DGRAM,0);
#ifdef _DEBUG_EVENTLOOPEX	
				qDebug("QEventLoopEx: update socket signaled -> recreate it %i",d->m_sockUpdate);
#endif
			}

			// if select says data is ready on any socket, then set the socket notifier
			// to pending
			int i;
			for ( i=0; i<3; i++ ) 
			{
				if ( ! d->sn_vec[i].list )
					continue;

				QPtrList<QSockNotEx> *list = d->sn_vec[i].list;
				QSockNotEx *sn = list->first();
				while ( sn ) {
					if ( FD_ISSET( sn->fd, &d->sn_vec[i].select_fds ) )
					{
						QEvent event( QEvent::SockAct );
						setSocketNotifierPending( sn->obj );
					}
					sn = list->next();
				}
			}
			LeaveCriticalSection(&d->m_csVec);
		}
		if(!d->sn_pending_list.isEmpty() )
		{
			QApplication::eventLoop()->wakeUp();
#ifdef _DEBUG_EVENTLOOPEX	
			qDebug("QEventLoopEx: wake up main event loop and wait for pending list empty");
#endif
			WaitForSingleObject(d->m_evPendingListEmpty,INFINITE);
#ifdef _DEBUG_EVENTLOOPEX	
			qDebug("QEventLoopEx: pending list now empty again");
#endif
			ResetEvent(d->m_evPendingListEmpty);
		}
	}
	while(!d->m_bStopped);
}

#include "qeventloopex.moc"
