/*

   $Id$

   This file is part of the KDE libraries
   Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)

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

   $Log$
   Revision 1.29.4.4  1999/07/12 10:26:42  porten
   kprocess deadlock fix from Alex Hayward (bug #1002 and #1513)

   Revision 1.29.4.3  1999/07/04 17:48:09  porten
   check for Stdin flag in writeStdin

   Revision 1.29.4.2  1999/05/10 19:33:18  gehrmab
   Fixed memory leak in KProcess

   Revision 1.29.4.1  1999/04/07 15:39:11  porten
   restore SIGPIPE handler

   Revision 1.29  1999/01/18 10:56:22  kulow
   .moc files are back in kdelibs. Built fine here using automake 1.3

   Revision 1.28  1999/01/15 09:30:40  kulow
   it's official - kdelibs builds with srcdir != builddir. For this I
   automocifized it, the generated rules are easier to maintain than
   selfwritten rules. I have to fight with some bugs of this tool, but
   generally it's better than keeping them updated by hand.

   Revision 1.27  1998/10/28 19:00:15  ettrich
   small fix (thanks to Christian Stoeckl)

   Revision 1.26  1998/10/19 08:09:05  ettrich
   small pseudo fix to kshellprocess reverted

   Revision 1.25  1998/09/22 18:03:13  ettrich
   Matthias: small updates to kaccel

   Revision 1.24  1998/09/17 19:22:09  radej
   sven: made it work with egcs-1.1

   Revision 1.23  1998/09/01 20:21:29  kulow
   I renamed all old qt header files to the new versions. I think, this looks
   nicer (and gives the change in configure a sense :)

   Revision 1.22  1998/07/29 10:14:28  kulow
   porting to a virtual plattform called "tajsandmineansiplatform" :)

   Revision 1.21  1998/07/29 09:07:50  ssk
   Fixed a whole lot of -Wall -ansi -pedantic warnings.

   Revision 1.20  1998/03/10 18:59:22  mario
   Mario: fixed a memory leak in KShellProcess (shell not freed)

   Revision 1.19  1998/03/08 17:21:40  wuebben
   Bernd: Fixed the segfault problem in 'KShellProcess::start()'.


*/


//
//  KPROCESS -- A class for handling child processes in KDE without
//  having to take care of Un*x specific implementation details
//
//  version 0.3.1, Jan 8th 1998
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//
// Changes:
//
// March 2nd, 1998: Changed parameter list for KShellProcess:
//   Arguments are now placed in a single string so that
//   <shell> -c <commandstring> is passed to the shell
//   to make the use of "operator<<" consistent with KProcess

#include "kprocess.h"
#define _MAY_INCLUDE_KPROCESSCONTROLLER_
#include "kprocctrl.h"

#include <qapplication.h>

// to define kstrdup
#include <config.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>

/////////////////////////////
// public member functions //
/////////////////////////////


KProcess::KProcess()
{
  if (0 == theKProcessController) {
	theKProcessController= new KProcessController();
	CHECK_PTR(theKProcessController);
  }

  run_mode = NotifyOnExit;
  runs = FALSE;
  pid = 0;
  status = 0;
  innot = outnot = errnot = 0;
  communication = NoCommunication;
  input_data = 0;
  input_sent = 0;
  input_total = 0;

  theKProcessController->processList->append(this);
}



KProcess::~KProcess()
{
  // destroying the KProcess instance sends a SIGKILL to the
  // child process (if it is running) after removing it from the
  // list of valid processes (if the process is not started as
  // "DontCare")

  theKProcessController->processList->remove(this);
  // this must happen before we kill the child
  // TODO: block the signal while removing the current process from the process list

  if (runs && (run_mode != DontCare))
    kill(SIGKILL);

  // TODO: restore SIGCHLD and SIGPIPE handler if this is the last KProcess
}



bool KProcess::setExecutable(const char *proc)
{
  if (runs) return FALSE;

  arguments.removeFirst();
  if (0 != proc) {
    arguments.insert(0,proc);
  }

  return TRUE;
}






KProcess &KProcess::operator<<(const char *arg)
{
  CHECK_PTR(arg);
  arguments.append(arg);
  return *this;
}



void KProcess::clearArguments()
{
  if (0 != arguments.first()) {
    while (arguments.remove())
      ;
  }
}



bool KProcess::start(RunMode runmode, Communication comm)
{
  uint i;
  uint n = arguments.count();
  char **arglist;

  if (runs || (0 == n)) {
	return FALSE;  // cannot start a process that is already running
	// or if no executable has been assigned
  }
  run_mode = runmode;
  status = 0;

  arglist = (char **)malloc( (n+1)*sizeof(char *));
  CHECK_PTR(arglist);
  for (i=0; i < n; i++)
    arglist[i] = arguments.at(i);
  arglist[n]= 0;

  if (!setupCommunication(comm))
    debug("Could not setup Communication!");

  runs = TRUE;
  pid = fork();

  if (0 == pid) {
	// The child process
	if(!commSetupDoneC())
	  debug("Could not finish comm setup in child!");

	// Matthias
	if (run_mode == DontCare)
          setpgid(0,0);

        // restore default SIGPIPE handler (Harri)
        struct sigaction act;
        sigemptyset(&(act.sa_mask));
        sigaddset(&(act.sa_mask), SIGPIPE);
        act.sa_handler = SIG_DFL;
        act.sa_flags = 0;
        sigaction(SIGPIPE, &act, 0L);

	execvp(arglist[0], arglist);
	exit(-1);

  } else if (-1 == pid) {
	// forking failed

	runs = FALSE;
	free(arglist);
	return FALSE;

  } else {
	// the parent continues here
	if (!commSetupDoneP())  // finish communication socket setup for the parent
	  debug("Could not finish comm setup in parent!");

	// Discard any data for stdin that might still be there
	input_data = 0;

	if (run_mode == Block) {
	  commClose();

	  // Its possible that the child's exit was caught by the SIGCHLD handler
	  // which will have set status for us.
	  if (waitpid(pid, &status, 0) != -1) this->status = status;

	  runs = FALSE;
	  emit processExited(this);
	}
  }
  free(arglist);
  return TRUE;
}



bool KProcess::kill(int signo)
{
  bool rv=FALSE;

  if (0 != pid)
    rv= (-1 != ::kill(pid, signo));
  // probably store errno somewhere...
  return rv;
}



bool KProcess::isRunning()
{
  return runs;
}



pid_t KProcess::getPid()
{
  return pid;
}



bool KProcess::normalExit()
{
  int _status = status;
  return (pid != 0) && (!runs) && (WIFEXITED((_status)));
}



int KProcess::exitStatus()
{
  int _status = status;
  return WEXITSTATUS((_status));
}



bool KProcess::writeStdin(char *buffer, int buflen)
{
  bool rv;

  // if there is still data pending, writing new data
  // to stdout is not allowed (since it could also confuse
  // kprocess...
  if (0 != input_data)
    return FALSE;

  if (runs && (communication & Stdin)) {
    input_data = buffer;
    input_sent = 0;
    input_total = buflen;
    slotSendData(0);
    innot->setEnabled(TRUE);
    rv = TRUE;
  } else
    rv = FALSE;
  return rv;
}



bool KProcess::closeStdin()
{
  bool rv;

  if (communication & Stdin) {
    innot->setEnabled(FALSE);
    close(in[1]);
    rv = TRUE;
  } else
    rv = FALSE;
  return rv;
}


/////////////////////////////
// protected slots         //
/////////////////////////////



void KProcess::slotChildOutput(int fdno)
{
  if (!childOutput(fdno)) {
	outnot->setEnabled(FALSE);	
  }
}



void KProcess::slotChildError(int fdno)
{
  if (!childError(fdno)) {
    errnot->setEnabled(FALSE);
  }
}



void KProcess::slotSendData(int)
{
  if (input_sent == input_total) {
    innot->setEnabled(FALSE);
    input_data = 0;
    emit wroteStdin(this);
  } else
    input_sent += ::write(in[1], input_data+input_sent, input_total-input_sent);
}



//////////////////////////////
// private member functions //
//////////////////////////////



void KProcess::processHasExited(int state)
{
  runs = FALSE;
  status = state;

  commClose(); // cleanup communication sockets

  // also emit a signal if the process was run Blocking
  if (DontCare != run_mode)
    emit processExited(this);
}



int KProcess::childOutput(int fdno)
{
  char buffer[1024];
  int len;

  len = ::read(fdno, buffer, 1024);

  if ( 0 < len) {
	emit receivedStdout(this, buffer, len);
  }
  return len;
}



int KProcess::childError(int fdno)
{
  char buffer[1024];
  int len;

  len = ::read(fdno, buffer, 1024);

  if ( 0 < len)
	emit receivedStderr(this, buffer, len);
  return len;
}



int KProcess::setupCommunication(Communication comm)
{
  int ok;

  communication = comm;

  ok = 1;
  if (comm & Stdin)
	ok &= socketpair(AF_UNIX, SOCK_STREAM, 0, in) >= 0;

  if (comm & Stdout)
	ok &= socketpair(AF_UNIX, SOCK_STREAM, 0, out) >= 0;

  if (comm & Stderr)
	ok &= socketpair(AF_UNIX, SOCK_STREAM, 0, err) >= 0;

  return ok;
}



int KProcess::commSetupDoneP()
{
  int ok = 1;

  if (communication != NoCommunication) {
	if (communication & Stdin)
	  close(in[0]);
	if (communication & Stdout)
	  close(out[1]);
	if (communication & Stderr)
	  close(err[1]);

	// Don't create socket notifiers and set the sockets non-blocking if
	// blocking is requested.
	if (run_mode == Block) return ok;

	if (communication & Stdin) {
	  ok &= (-1 != fcntl(in[1], F_SETFL, O_NONBLOCK));
	  innot =  new QSocketNotifier(in[1], QSocketNotifier::Write, this);
	  CHECK_PTR(innot);
	  innot->setEnabled(FALSE); // will be enabled when data has to be sent
	  QObject::connect(innot, SIGNAL(activated(int)),
					   this, SLOT(slotSendData(int)));
	}

	if (communication & Stdout) {
	  ok &= (-1 != fcntl(out[0], F_SETFL, O_NONBLOCK));
	  outnot = new QSocketNotifier(out[0], QSocketNotifier::Read, this);
	  CHECK_PTR(outnot);
	  QObject::connect(outnot, SIGNAL(activated(int)),
					   this, SLOT(slotChildOutput(int)));
	}

	if (communication & Stderr) {
	  ok &= (-1 != fcntl(err[0], F_SETFL, O_NONBLOCK));
	  errnot = new QSocketNotifier(err[0], QSocketNotifier::Read, this );
	  CHECK_PTR(errnot);
	  QObject::connect(errnot, SIGNAL(activated(int)),
					   this, SLOT(slotChildError(int)));
	}
  }
  return ok;
}



int KProcess::commSetupDoneC()
{
  int ok = 1;
  struct linger so;

  if (communication != NoCommunication) {
	if (communication & Stdin)
	  close(in[1]);
	if (communication & Stdout)
	  close(out[0]);
	if (communication & Stderr)
	  close(err[0]);

	if (communication & Stdin)
	  ok &= dup2(in[0],  STDIN_FILENO) != -1;
	if (communication & Stdout) {
	  ok &= dup2(out[1], STDOUT_FILENO) != -1;
	  ok &= !setsockopt(out[1], SOL_SOCKET, SO_LINGER, (char*)&so, sizeof(so));
	}
	if (communication & Stderr) {
	  ok &= dup2(err[1], STDERR_FILENO) != -1;
	  ok &= !setsockopt(err[1], SOL_SOCKET, SO_LINGER, (char*)&so, sizeof(so));
	}
  }
  return ok;
}



void KProcess::commClose()
{
  if (NoCommunication != communication) {
        bool b_in = (communication & Stdin);
        bool b_out = (communication & Stdout);
        bool b_err = (communication & Stderr);
	if (b_in)
		delete innot;

	if (b_out || b_err) {
	  // If both channels are being read we need to make sure that one socket buffer
	  // doesn't fill up whilst we are waiting for data on the other (causing a deadlock).
	  // Hence we need to use select.

	  // Once one or other of the channels has reached EOF (or given an error) go back
	  // to the usual mechanism.

	  int fds_ready = 1;
	  fd_set rfds;

          int max_fd = 0;
          if (b_out) {
	    fcntl(out[0], F_SETFL, O_NONBLOCK);
            if (out[0] > max_fd)
              max_fd = out[0];
            delete outnot;
          }
          if (b_err) {
	    fcntl(err[0], F_SETFL, O_NONBLOCK);
            if (err[0] > max_fd)
              max_fd = err[0];
            delete errnot;
          }
           

	  while (1) {
	    FD_ZERO(&rfds);
            if (b_out) 
	      FD_SET(out[0], &rfds);

            if (b_err) 
	      FD_SET(err[0], &rfds);

	    fds_ready = select(max_fd+1, &rfds, 0, 0, 0);
	    if (fds_ready <= 0) break;

	    if (b_out && FD_ISSET(out[0], &rfds)) {
	      int ret = 1;
	      while (ret > 0) ret = childOutput(out[0]);
	      if ((ret == -1 && errno != EAGAIN) || ret == 0) break;
	    }
                               
	    if (b_err && FD_ISSET(err[0], &rfds)) {
	      int ret = 1;
	      while (ret > 0) ret = childError(err[0]);
	      if ((ret == -1 && errno != EAGAIN) || ret == 0) break;
	    }
	  }
	}

	if (b_in)
	    close(in[1]);
	if (b_out)
	    close(out[0]);
	if (b_err)
	    close(err[0]);
	
	communication = NoCommunication;
  }
}




///////////////////////////
// CC: Class KShellProcess
///////////////////////////

KShellProcess::KShellProcess(const char *shellname):
  KProcess()
{
  if (0 != shellname)
    shell = kstrdup(shellname);
  else
    shell = 0;
}


KShellProcess::~KShellProcess() {
  if(shell)
    free(shell);
}

bool KShellProcess::start(RunMode runmode, Communication comm)
{
  uint i;
  uint n = arguments.count();
  char *arglist[4];
  QString cmd;

  if (runs || (0 == n)) {
	return FALSE;  // cannot start a process that is already running
	// or if no executable has been assigned
  }

  run_mode = runmode;
  status = 0;

  if (0 == shell)
    shell = searchShell();
  if (0 == shell) {
    debug("Could not find a valid shell\n");
    return FALSE;
  }

  // CC: Changed the way the parameter was built up
  // CC: Arglist for KShellProcess is now always:
  // CC: <shell> -c <command>

  arglist[0] = shell;
  arglist[1] = "-c";

  for (i=0; i < n; i++) {
    cmd += arguments.at(i);
    cmd += " "; // CC: to separate the arguments
  }

//   // execution in background
//   cmd.stripWhiteSpace();
//   if (cmd[cmd.length()-1] != '&')
//       cmd += '&';
  arglist[2] = cmd.data();
  arglist[3] = 0;

  if (!setupCommunication(comm))
    debug("Could not setup Communication!");

  runs = TRUE;
  pid = fork();

  if (0 == pid) {
	// The child process

	if(!commSetupDoneC())
	  debug("Could not finish comm setup in child!");

	// Matthias
	if (run_mode == DontCare)
          setpgid(0,0);

        // restore default SIGPIPE handler (Harri)
        struct sigaction act;
        sigemptyset(&(act.sa_mask));
        sigaddset(&(act.sa_mask), SIGPIPE);
        act.sa_handler = SIG_DFL;
        act.sa_flags = 0;
        sigaction(SIGPIPE, &act, 0L);

	execvp(arglist[0], arglist);
	exit(-1);

  } else if (-1 == pid) {
	// forking failed

	runs = FALSE;
	return FALSE;

  } else {
	// the parent continues here

	if (!commSetupDoneP())  // finish communication socket setup for the parent
	  debug("Could not finish comm setup in parent!");

	// Discard any data for stdin that might still be there
	input_data = 0;

	if (run_mode == Block) {
	  commClose();

	  // Its possible that the child's exit was caught by the SIGCHLD handler
	  // which will have set status for us.
	  if (waitpid(pid, &status, 0) != -1) this->status = status;

	  runs = FALSE;
	  emit processExited(this);
	}
  }
  return TRUE;
}



char *KShellProcess::searchShell()
{
  char *hlp = 0;
  char *copy = 0;


  // CC: now get the name of the shell we have to use
  hlp = getenv("SHELL");
  if (isExecutable(hlp)) {
    copy = kstrdup(hlp);
    CHECK_PTR(copy);
  }

  if (0 == copy) {
    // CC: hmm, invalid $SHELL in environment -- maybe there are whitespaces to be stripped?
    QString stmp = QString(shell);
    QString shell_stripped = stmp.stripWhiteSpace();
    if (isExecutable(shell_stripped.data())) {
      copy = kstrdup(shell_stripped.data());
      CHECK_PTR(copy);
    }
  }
  return copy;
}




bool KShellProcess::isExecutable(const char *fname)
{
  struct stat fileinfo;

  if ((0 == fname) || (strlen(fname) == 0)) return FALSE;
  // CC: filename is invalid

  // CC: we've got a valid filename, now let's see whether we can execute that file

  if (-1 == stat(fname, &fileinfo)) return FALSE;
  // CC: return false if the file does not exist

  // CC: anyway, we cannot execute directories, block/character devices, fifos or sockets
  if ( (S_ISDIR(fileinfo.st_mode))  ||
       (S_ISCHR(fileinfo.st_mode))  ||
       (S_ISBLK(fileinfo.st_mode))  ||
#ifdef S_ISSOCK
       // CC: SYSVR4 systems don't have that macro
       (S_ISSOCK(fileinfo.st_mode)) ||
#endif
       (S_ISFIFO(fileinfo.st_mode)) ||
       (S_ISDIR(fileinfo.st_mode)) ) {
    return FALSE;
  }

  // CC: now check for permission to execute the file
  if (access(fname, X_OK) != 0) return FALSE;

  // CC: we've passed all the tests...
  return TRUE;
}
#include "kprocess.moc"

