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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//
//  KPROCESSCONTROLLER -- A helper class for KProcess
//
//  version 0.3.1, Jan, 8th 1997
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <qsocketnotifier.h>
#include "kprocess.h"
#include "kprocctrl.h"

KProcessController *KProcessController::theKProcessController = 0;

struct sigaction KProcessController::oldChildHandlerData;
bool KProcessController::handlerSet = false;

KProcessController::KProcessController()
{
  assert( theKProcessController == 0 );

  if (0 > pipe(fd))
	printf(strerror(errno));

  notifier = new QSocketNotifier(fd[0], QSocketNotifier::Read);
  notifier->setEnabled(true);
  QObject::connect(notifier, SIGNAL(activated(int)),
				   this, SLOT(slotDoHousekeeping(int)));
  connect( &delayedChildrenCleanupTimer, SIGNAL( timeout()),
      SLOT( delayedChildrenCleanup()));

  theKProcessController = this;

  setupHandlers();
}


void KProcessController::setupHandlers()
{
  if( handlerSet )
      return;
  struct sigaction act;
  act.sa_handler=theSigCHLDHandler;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGCHLD);
  // Make sure we don't block this signal. gdb tends to do that :-(
  sigprocmask(SIG_UNBLOCK, &(act.sa_mask), 0);

  act.sa_flags = SA_NOCLDSTOP;

  // CC: take care of SunOS which automatically restarts interrupted system
  // calls (and thus does not have SA_RESTART)

#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART;
#endif

  sigaction( SIGCHLD, &act, &oldChildHandlerData );

  act.sa_handler=SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGPIPE);
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L);
  handlerSet = true;
}

void KProcessController::resetHandlers()
{
  if( !handlerSet )
      return;
  sigaction( SIGCHLD, &oldChildHandlerData, 0 );
  // there should be no problem with SIGPIPE staying SIG_IGN
  handlerSet = false;
}

// block SIGCHLD handler, because it accesses processList
void KProcessController::addKProcess( KProcess* p )
{
  sigset_t newset, oldset;
  sigemptyset( &newset );
  sigaddset( &newset, SIGCHLD );
  sigprocmask( SIG_BLOCK, &newset, &oldset );
  processList.append( p );
  sigprocmask( SIG_SETMASK, &oldset, 0 );
}

void KProcessController::removeKProcess( KProcess* p )
{
  sigset_t newset, oldset;
  sigemptyset( &newset );
  sigaddset( &newset, SIGCHLD );
  sigprocmask( SIG_BLOCK, &newset, &oldset );
  processList.remove( p );
  sigprocmask( SIG_SETMASK, &oldset, 0 );
}

void KProcessController::theSigCHLDHandler(int arg)
{
  int status;
  pid_t this_pid;
  int saved_errno;

  saved_errno = errno;
  // since waitpid and write change errno, we have to save it and restore it
  // (Richard Stevens, Advanced programming in the Unix Environment)

  bool found = false;
  if( theKProcessController != 0 ) {
      // iterating the list doesn't perform any system call
      for( QValueList<KProcess*>::ConstIterator it = theKProcessController->processList.begin();
           it != theKProcessController->processList.end();
           ++it )
      {
        if( !(*it)->isRunning())
            continue;
        this_pid = waitpid( (*it)->pid(), &status, WNOHANG );
        if ( this_pid > 0 ) {
          ::write(theKProcessController->fd[1], &this_pid, sizeof(this_pid));
          ::write(theKProcessController->fd[1], &status, sizeof(status));
          found = true;
        }
      }
  }
  if( !found && oldChildHandlerData.sa_handler != SIG_IGN
          && oldChildHandlerData.sa_handler != SIG_DFL )
        oldChildHandlerData.sa_handler( arg ); // call the old handler
  // handle the rest
  if( theKProcessController != 0 ) {
      pid_t dummy_pid = 0; // delayed waitpid()
      int dummy_status = 0;
      ::write(theKProcessController->fd[1], &dummy_pid, sizeof(dummy_pid));
      ::write(theKProcessController->fd[1], &dummy_status, sizeof(dummy_status));
  } else {
      int dummy;
      while( waitpid( -1, &dummy, WNOHANG ) > 0 )
          ;
  }

  errno = saved_errno;
}



void KProcessController::slotDoHousekeeping(int )
{
  int bytes_read = 0;

  // read pid and status from the pipe.

  const int len = sizeof(pid_t) + sizeof(int);
  int errcnt = 0;
  unsigned char buf[len];
  while (bytes_read < len && errcnt < 50) {
      int r = ::read(fd[0], buf + bytes_read, len - bytes_read);
      if (r > 0) bytes_read += r;
      else if (r < 0) errcnt++;
  }
  if (errcnt >= 50) {
	fprintf(stderr,
	       "Error: Max. error count for pipe read "
               "exceeded in KProcessController::slotDoHousekeeping\n");
	return;           // it makes no sense to continue here!
  }
  if (bytes_read != len) {
	fprintf(stderr,
	       "Error: Could not read info from signal handler %d <> %d!\n",
	       bytes_read, len);
	return;           // it makes no sense to continue here!
  }
  pid_t pid    = *reinterpret_cast<pid_t *>(buf);
  int status = *reinterpret_cast<int *>(buf + sizeof(pid_t));

  if( pid == 0 ) { // special case, see delayedChildrenCleanup()
      delayedChildrenCleanupTimer.start( 1000, true );
      return;
  }

  for( QValueList<KProcess*>::ConstIterator it = processList.begin();
       it != processList.end();
       ++it ) {
        KProcess* proc = *it;
	if (proc->pid() == pid) {
	  // process has exited, so do emit the respective events
	  if (proc->run_mode == KProcess::Block) {
	    // If the reads are done blocking then set the status in proc
	    // but do nothing else because KProcess will perform the other
	    // actions of processHasExited.
	    proc->status = status;
            proc->runs = false;
	  } else {
	    proc->processHasExited(status);
	  }
        return;
	}
  }
}

// this is needed e.g. for popen(), which calls waitpid() checking
// for its forked child, if we did waitpid() directly in the SIGCHLD
// handler, popen()'s waitpid() call would fail
void KProcessController::delayedChildrenCleanup()
{
  int status;
  pid_t pid;
  while(( pid = waitpid( -1, &status, WNOHANG ) ) > 0 ) {
      for( QValueList<KProcess*>::ConstIterator it = processList.begin();
           it != processList.end();
           ++it )
      {
        if( !(*it)->isRunning() || (*it)->pid() != pid )
            continue;
        // it's KProcess, handle it
          ::write(fd[1], &pid, sizeof(pid));
          ::write(fd[1], &status, sizeof(status));
        break;
      }
  }
}

KProcessController::~KProcessController()
{
  assert( theKProcessController == this );
  resetHandlers();

  notifier->setEnabled(false);

  close(fd[0]);
  close(fd[1]);

  delete notifier;
  theKProcessController = 0;
}

#include "kprocctrl.moc"
