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

#include "kprocess.h"
#include "kprocctrl.h"

template class QPtrList<KProcess>;

KProcessController *KProcessController::theKProcessController = 0;

KProcessController::KProcessController()
{
  struct sigaction act;

  // initialize theKProcessList
  processList = new QPtrList<KProcess>();
  Q_CHECK_PTR(processList);

  if (0 > pipe(fd))
	printf(strerror(errno));

  notifier = new QSocketNotifier(fd[0], QSocketNotifier::Read);
  notifier->setEnabled(true);
  QObject::connect(notifier, SIGNAL(activated(int)),
				   this, SLOT(slotDoHousekeeping(int)));

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

  sigaction( SIGCHLD, &act, 0L);
  act.sa_handler=SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGPIPE);
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L);
}

void KProcessController::theSigCHLDHandler(int )
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
  int bytes_read = 0;
  pid_t pid;
  int status;

  // read pid and status from the pipe.

  int len = sizeof(pid_t) + sizeof(int), errcnt = 0;
  unsigned char buf[sizeof(pid_t) + sizeof(int)];
  while (bytes_read < len && errcnt < 50) {
      int r = ::read(fd[0], buf + bytes_read, len - bytes_read);
      if (r > 0) bytes_read += r;
      else if (r < 0) errcnt++;
  }
  if (errcnt >= 50) {
	fprintf(stderr,
	       "Error: Max. error count for pipe read "
               "exceed in KProcessController::slotDoHousekeeping\n");
	return;           // it makes no sense to continue here!
  }
  if (bytes_read != len) {
	fprintf(stderr,
	       "Error: Could not read info from signal handler %d <> %d!\n",
	       bytes_read, len);
	return;           // it makes no sense to continue here!
  }
  pid    = *reinterpret_cast<pid_t *>(buf);
  status = *reinterpret_cast<int *>(buf + sizeof(pid_t));

//   bool found = false;

  proc = processList->first();

  while (0L != proc) {
	if (proc->pid() == pid) {
// 	  found = true;
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
	}
	proc = processList->next();
  }
}

KProcessController::~KProcessController()
{
  struct sigaction act;

  notifier->setEnabled(false);

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
