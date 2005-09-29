/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Czezakte (e9025461@student.tuwien.ac.at)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kprocess.h"
#include "kprocctrl.h"

#include <config.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <qsocketnotifier.h>

KProcessController *KProcessController::theKProcessController;
int KProcessController::refCount;

void KProcessController::ref()
{
  if( !refCount ) {
    theKProcessController = new KProcessController;
    setupHandlers();
  }
  refCount++;
}

void KProcessController::deref()
{
  refCount--;
  if( !refCount ) {
    resetHandlers();
    delete theKProcessController;
    theKProcessController = 0;
  }
}

KProcessController::KProcessController()
  : needcheck( false )
{
  if( pipe( fd ) )
  {
    perror( "pipe" );
    abort();
  }

  fcntl( fd[0], F_SETFL, O_NONBLOCK ); // in case slotDoHousekeeping is called without polling first
  fcntl( fd[1], F_SETFL, O_NONBLOCK ); // in case it fills up
  fcntl( fd[0], F_SETFD, FD_CLOEXEC );
  fcntl( fd[1], F_SETFD, FD_CLOEXEC );

  notifier = new QSocketNotifier( fd[0], QSocketNotifier::Read );
  notifier->setEnabled( true );
  QObject::connect( notifier, SIGNAL(activated(int)),
                    SLOT(slotDoHousekeeping()));
}

KProcessController::~KProcessController()
{
  delete notifier;

  close( fd[0] );
  close( fd[1] );
}


extern "C" {
static void theReaper( int num )
{
  KProcessController::theSigCHLDHandler( num );
}
}

#ifdef Q_OS_UNIX
struct sigaction KProcessController::oldChildHandlerData;
#endif
bool KProcessController::handlerSet = false;

void KProcessController::setupHandlers()
{
  if( handlerSet )
      return;
  handlerSet = true;

#ifdef Q_OS_UNIX
  struct sigaction act;
  sigemptyset( &act.sa_mask );

  act.sa_handler = SIG_IGN;
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L );

  act.sa_handler = theReaper;
  act.sa_flags = SA_NOCLDSTOP;
  // CC: take care of SunOS which automatically restarts interrupted system
  // calls (and thus does not have SA_RESTART)
#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART;
#endif
  sigaction( SIGCHLD, &act, &oldChildHandlerData );

  sigaddset( &act.sa_mask, SIGCHLD );
  // Make sure we don't block this signal. gdb tends to do that :-(
  sigprocmask( SIG_UNBLOCK, &act.sa_mask, 0 );
#else
  //TODO: win32
#endif
}

void KProcessController::resetHandlers()
{
  if( !handlerSet )
      return;
  handlerSet = false;

#ifdef Q_OS_UNIX
  sigaction( SIGCHLD, &oldChildHandlerData, 0 );
#else
  //TODO: win32
#endif
  // there should be no problem with SIGPIPE staying SIG_IGN
}

// the pipe is needed to sync the child reaping with our event processing,
// as otherwise there are race conditions, locking requirements, and things
// generally get harder
void KProcessController::theSigCHLDHandler( int arg )
{
  int saved_errno = errno;

  char dummy = 0;
  ::write( theKProcessController->fd[1], &dummy, 1 );

#ifdef Q_OS_UNIX
  if( oldChildHandlerData.sa_handler != SIG_IGN &&
      oldChildHandlerData.sa_handler != SIG_DFL )
     oldChildHandlerData.sa_handler( arg ); // call the old handler
#else
  //TODO: win32
#endif

  errno = saved_errno;
}

int KProcessController::notifierFd() const
{
  return fd[0];
}

void KProcessController::unscheduleCheck()
{
  char dummy[16]; // somewhat bigger - just in case several have queued up
  if( ::read( fd[0], dummy, sizeof(dummy) ) > 0 )
    needcheck = true;
}

void
KProcessController::rescheduleCheck()
{
  if( needcheck )
  {
    needcheck = false;
    char dummy = 0;
    ::write( fd[1], &dummy, 1 );
  }
}

void KProcessController::slotDoHousekeeping()
{
  char dummy[16]; // somewhat bigger - just in case several have queued up
  ::read( fd[0], dummy, sizeof(dummy) );

  int status;
 again:
  QValueListIterator<KProcess*> it( kProcessList.begin() );
  QValueListIterator<KProcess*> eit( kProcessList.end() );
  while( it != eit )
  {
    KProcess *prc = *it;
    if( prc->runs && waitpid( prc->pid_, &status, WNOHANG ) > 0 )
    {
      prc->processHasExited( status );
      // the callback can nuke the whole process list and even 'this'
      if (!theKProcessController)
        return;
      goto again;
    }
    ++it;
  }
  QValueListIterator<int> uit( unixProcessList.begin() );
  QValueListIterator<int> ueit( unixProcessList.end() );
  while( uit != ueit )
  {
    if( waitpid( *uit, 0, WNOHANG ) > 0 )
    {
      uit = unixProcessList.remove( uit );
      deref(); // counterpart to addProcess, can invalidate 'this'
    } else
      ++uit;
  }
}

bool KProcessController::waitForProcessExit( int timeout )
{
#ifdef Q_OS_UNIX
  for(;;)
  {
    struct timeval tv, *tvp;
    if (timeout < 0)
      tvp = 0;
    else
    {
      tv.tv_sec = timeout;
      tv.tv_usec = 0;
      tvp = &tv;
    }

    fd_set fds;
    FD_ZERO( &fds );
    FD_SET( fd[0], &fds );

    switch( select( fd[0]+1, &fds, 0, 0, tvp ) )
    {
    case -1:
      if( errno == EINTR )
        continue;
      // fall through; should never happen
    case 0:
      return false;
    default:
      slotDoHousekeeping();
      return true;
    }
  }
#else
  //TODO: win32
  return false;
#endif
}

void KProcessController::addKProcess( KProcess* p )
{
  kProcessList.append( p );
}

void KProcessController::removeKProcess( KProcess* p )
{
  kProcessList.remove( p );
}

void KProcessController::addProcess( int pid )
{
  unixProcessList.append( pid );
  ref(); // make sure we stay around when the KProcess goes away
}

#include "kprocctrl.moc"
