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

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <stdio.h>
#include <config.h>
#include "kprocess.h"

#include "kprocctrl.h"

KProcessController *theKProcessController = 0;

KProcessController::KProcessController()
{
  struct sigaction act;

  // initialize theKProcessList
  processList = new QList<KProcess>();
  CHECK_PTR(processList);
 
  if (0 > pipe(fd))
	printf(strerror(errno));
  
  if (-1 == fcntl(fd[0], F_SETFL, O_NONBLOCK))
	printf(strerror(errno));

  notifier = new QSocketNotifier(fd[0], QSocketNotifier::Read);
  notifier->setEnabled(TRUE);
  QObject::connect(notifier, SIGNAL(activated(int)),
				   this, SLOT(slotDoHousekeeping(int)));
 		 
  act.sa_handler=theSigCHLDHandler;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGCHLD);
  act.sa_flags = SA_NOCLDSTOP;

  // CC: take care of SunOS which automatically restarts interrupted system
  // calls (and thus does not have SA_RESTART)

#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART;
#endif

  sigaction( SIGCHLD, &act, 0L); 
  act.sa_handler=SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGPIPE);
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L);
}

#ifdef __sgi__
void KProcessController::theSigCHLDHandler()
#else
void KProcessController::theSigCHLDHandler(int )
#endif
{
  int status;
  pid_t this_pid;
  int saved_errno;

  saved_errno = errno;
  // since waitpid and write change errno, we have to save it and restore it
  // (Richard Stevens, Advanced programming in the Unix Environment)

  // Waba: Check for multiple childs exiting at the same time
  do
  {
    this_pid = waitpid(-1, &status, WNOHANG);
    // J6t: theKProcessController might be already destroyed
    if ((this_pid > 0) && (theKProcessController != 0)) {
      ::write(theKProcessController->fd[1], &this_pid, sizeof(this_pid));
      ::write(theKProcessController->fd[1], &status, sizeof(status));
    }
  }
  while (this_pid > 0); 

  errno = saved_errno;
}



void KProcessController::slotDoHousekeeping(int )
{
  KProcess *proc;
  int bytes_read;
  pid_t pid;
  int status;

  bytes_read  = ::read(fd[0], &pid, sizeof(pid_t));
  bytes_read += ::read(fd[0], &status, sizeof(int));

  if (bytes_read != sizeof(int)+sizeof(pid_t))
	fprintf(stderr,"Error: Could not read info from signal handler!\n");
 
  proc = processList->first();

  while (0L != proc) {
	if (proc->pid == pid) {
	  // process has exited, so do emit the respective events
	  proc->processHasExited(status);
	}
	proc = processList->next();
  }
}

KProcessController::~KProcessController()
{
  struct sigaction act;

  notifier->setEnabled(FALSE);

  // Turn off notification for processes that have exited
  act.sa_handler=SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGCHLD);
  act.sa_flags = 0;
  sigaction( SIGCHLD, &act, 0L);
  
  close(fd[0]);
  close(fd[1]);
  delete processList;
  delete notifier;
}
#include "kprocctrl.moc"
