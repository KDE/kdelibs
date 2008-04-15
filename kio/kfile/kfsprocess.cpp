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

#include "kfsprocess.h"

#include <config.h>

#include <kdebug.h>
#include <kstandarddirs.h>
//#include <kuser.h>

#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

KfsProcessController *KfsProcessController::s_instance = 0;
int KfsProcessController::s_refCount = 0;

void KfsProcessController::ref()
{
    if ( !s_refCount ) {
        s_instance = new KfsProcessController;
        setupHandlers();
    }
    s_refCount++;
}

void KfsProcessController::deref()
{
    s_refCount--;
    if( !s_refCount ) {
        resetHandlers();
        delete s_instance;
        s_instance = 0;
    }
}

KfsProcessController* KfsProcessController::instance()
{
    return s_instance;
}

KfsProcessController::KfsProcessController()
{
  if( pipe( m_fd ) )
  {
    perror( "pipe" );
    abort();
  }

  fcntl( m_fd[0], F_SETFL, O_NONBLOCK ); // in case slotDoHousekeeping is called without polling first
  fcntl( m_fd[1], F_SETFL, O_NONBLOCK ); // in case it fills up
  fcntl( m_fd[0], F_SETFD, FD_CLOEXEC );
  fcntl( m_fd[1], F_SETFD, FD_CLOEXEC );

  m_notifier = new QSocketNotifier( m_fd[0], QSocketNotifier::Read );
  m_notifier->setEnabled( true );
  QObject::connect( m_notifier, SIGNAL(activated(int)),
                    SLOT(slotDoHousekeeping()));
}

KfsProcessController::~KfsProcessController()
{
#ifndef Q_OS_MAC
/* not sure why, but this is causing lockups */
  close( m_fd[0] );
  close( m_fd[1] );
#else
#warning FIXME: why does close() freeze up destruction?
#endif
}


extern "C" {
static void theReaper( int num )
{
  KfsProcessController::theSigCHLDHandler( num );
}
}

struct sigaction KfsProcessController::s_oldChildHandlerData;
bool KfsProcessController::s_handlerSet = false;

void KfsProcessController::setupHandlers()
{
  if( s_handlerSet )
      return;
  s_handlerSet = true;

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
  sigaction( SIGCHLD, &act, &s_oldChildHandlerData );

  sigaddset( &act.sa_mask, SIGCHLD );
  // Make sure we don't block this signal. gdb tends to do that :-(
  sigprocmask( SIG_UNBLOCK, &act.sa_mask, 0 );
}

void KfsProcessController::resetHandlers()
{
  if( !s_handlerSet )
      return;
  s_handlerSet = false;

  sigset_t mask, omask;
  sigemptyset( &mask );
  sigaddset( &mask, SIGCHLD );
  sigprocmask( SIG_BLOCK, &mask, &omask );

  struct sigaction act;
  sigaction( SIGCHLD, &s_oldChildHandlerData, &act );
  if (act.sa_handler != theReaper) {
     sigaction( SIGCHLD, &act, 0 );
     s_handlerSet = true;
  }

  sigprocmask( SIG_SETMASK, &omask, 0 );
  // there should be no problem with SIGPIPE staying SIG_IGN
}

// the pipe is needed to sync the child reaping with our event processing,
// as otherwise there are race conditions, locking requirements, and things
// generally get harder
void KfsProcessController::theSigCHLDHandler( int arg )
{
  int saved_errno = errno;

  char dummy = 0;
  ::write( instance()->m_fd[1], &dummy, 1 );

    if ( s_oldChildHandlerData.sa_handler != SIG_IGN &&
         s_oldChildHandlerData.sa_handler != SIG_DFL ) {
        s_oldChildHandlerData.sa_handler( arg ); // call the old handler
    }

  errno = saved_errno;
}

void KfsProcessController::slotDoHousekeeping()
{
  char dummy[16]; // somewhat bigger - just in case several have queued up
  ::read( m_fd[0], dummy, sizeof(dummy) );

 again:
  QList<KfsProcess*>::iterator it( m_kProcessList.begin() );
  QList<KfsProcess*>::iterator eit( m_kProcessList.end() );
  while( it != eit )
  {
    KfsProcess *prc = *it;
    if( prc->runs && waitpid( prc->pid_, 0, WNOHANG ) > 0 )
    {
      prc->processHasExited();
      // the callback can nuke the whole process list and even 'this'
      if (!instance())
        return;
      goto again;
    }
    ++it;
  }
  QList<int>::iterator uit( m_unixProcessList.begin() );
  QList<int>::iterator ueit( m_unixProcessList.end() );
  while( uit != ueit )
  {
    if( waitpid( *uit, 0, WNOHANG ) > 0 )
    {
      uit = m_unixProcessList.erase( uit );
      deref(); // counterpart to addProcess, can invalidate 'this'
    } else
      ++uit;
  }
}

void KfsProcessController::addKProcess( KfsProcess* p )
{
  m_kProcessList.append( p );
}

void KfsProcessController::removeKProcess( KfsProcess* p )
{
  m_kProcessList.removeAll( p );
}

void KfsProcessController::addProcess( int pid )
{
  m_unixProcessList.append( pid );
  ref(); // make sure we stay around when the KfsProcess goes away
}

/////////////////////////////
// public member functions //
/////////////////////////////

KfsProcess::KfsProcess( QObject* parent )
  : QObject( parent ),
    runs(false),
    pid_(0)
{
  KfsProcessController::ref();
  KfsProcessController::instance()->addKProcess(this);
}

KfsProcess::~KfsProcess()
{
  KfsProcessController::instance()->removeKProcess(this);
  KfsProcessController::deref();
}

void KfsProcess::detach()
{
  if (runs) {
    KfsProcessController::instance()->addProcess(pid_);
    runs = false;
    pid_ = 0;
  }
}

KfsProcess &KfsProcess::operator<<(const char* arg)
{
  arguments.append(QByteArray(arg));
  return *this;
}

KfsProcess &KfsProcess::operator<<(const QString& arg)
{
  arguments.append(QFile::encodeName(arg));
  return *this;
}

bool KfsProcess::start()
{
  if (runs) {
    kDebug(175) << "Attempted to start an already running process";
    return false;
  }

  uint n = arguments.count();
  if (n == 0) {
    kDebug(175) << "Attempted to start a process without arguments";
    return false;
  }
  char **arglist = static_cast<char **>(malloc( (n + 1) * sizeof(char *)));
  for (uint i = 0; i < n; i++)
     arglist[i] = arguments[i].data();
  arglist[n] = 0;

  int fd[2];
  if (pipe(fd))
     fd[0] = fd[1] = -1; // Pipe failed.. continue

  pid_ = fork();
  if (pid_ == 0) {
        // The child process

        close(fd[0]);
        // Closing of fd[1] indicates that the execvp() succeeded!
        fcntl(fd[1], F_SETFD, FD_CLOEXEC);

        // reset all signal handlers
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_handler = SIG_DFL;
        act.sa_flags = 0;
        for (int sig = 1; sig < NSIG; sig++)
          sigaction(sig, &act, 0L);

        setsid();
        execvp(arglist[0], arglist);

        char resultByte = 1;
        write(fd[1], &resultByte, 1);
        _exit(-1);
  } else if (pid_ == -1) {
        // forking failed

        pid_ = 0;
        free(arglist);
        return false;
  }
  // the parent continues here
  free(arglist);

  // Check whether client could be started.
  close(fd[1]);
  for(;;)
  {
     char resultByte;
     int n = ::read(fd[0], &resultByte, 1);
     if (n == 1)
     {
         // exec() failed
         close(fd[0]);
         waitpid(pid_, 0, 0);
         pid_ = 0;
         return false;
     }
     if (n == -1)
     {
        if (errno == EINTR)
           continue; // Ignore
     }
     break; // success
  }
  close(fd[0]);

  runs = true;
  return true;
}

void KfsProcess::processHasExited()
{
    // only successfully run processes ever get here
    runs = false;
    emit processExited();
}

#include "kfsprocess.moc"
