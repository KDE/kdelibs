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

#include <config.h>

#include <qfile.h>
#include <qsocketnotifier.h>
#include <qregexp.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_INITGROUPS
#include <grp.h>
#endif
#include <pwd.h>

#include <qapplication.h>
#include <kdebug.h>

/////////////////////////////
// public member functions //
/////////////////////////////

class KProcessPrivate {
public:
   KProcessPrivate() : useShell(false) { }

   bool useShell;
   QMap<QString,QString> env;
   QString wd;
   QCString shell;
};


KProcess::KProcess()
  : QObject(),
    run_mode(NotifyOnExit),
    runs(false),
    pid_(0),
    status(0),
    keepPrivs(false),
    innot(0),
    outnot(0),
    errnot(0),
    communication(NoCommunication),
    input_data(0),
    input_sent(0),
    input_total(0),
    d(0)
{
  if (0 == KProcessController::theKProcessController) {
        (void) new KProcessController();
        Q_CHECK_PTR(KProcessController::theKProcessController);
  }

  KProcessController::theKProcessController->addKProcess(this);
  out[0] = out[1] = -1;
  in[0] = in[1] = -1;
  err[0] = err[1] = -1;
}

void
KProcess::setEnvironment(const QString &name, const QString &value)
{
   if (!d)
      d = new KProcessPrivate;
   d->env.insert(name, value);
}

void
KProcess::setWorkingDirectory(const QString &dir)
{
   if (!d)
      d = new KProcessPrivate;
   d->wd = dir;   
} 

void 
KProcess::setupEnvironment()
{
   if (d)
   {
      QMap<QString,QString>::Iterator it;
      for(it = d->env.begin(); it != d->env.end(); ++it)
         setenv(QFile::encodeName(it.key()).data(),
                QFile::encodeName(it.data()).data(), 1);
      if (!d->wd.isEmpty())
         chdir(QFile::encodeName(d->wd).data());
   }
}

void
KProcess::setRunPrivileged(bool keepPrivileges)
{
   keepPrivs = keepPrivileges;
}

bool
KProcess::runPrivileged() const
{
   return keepPrivs;
}


KProcess::~KProcess()
{
  // destroying the KProcess instance sends a SIGKILL to the
  // child process (if it is running) after removing it from the
  // list of valid processes (if the process is not started as
  // "DontCare")

  KProcessController::theKProcessController->removeKProcess(this);
  // this must happen before we kill the child
  // TODO: block the signal while removing the current process from the process list

  if (runs && (run_mode != DontCare))
    kill(SIGKILL);

  // Clean up open fd's and socket notifiers.
  closeStdin();
  closeStdout();
  closeStderr();

  // TODO: restore SIGCHLD and SIGPIPE handler if this is the last KProcess
  delete d;
}

void KProcess::detach()
{
  KProcessController::theKProcessController->removeKProcess(this);

  runs = false;
  pid_ = 0;

  // Clean up open fd's and socket notifiers.
  closeStdin();
  closeStdout();
  closeStderr();
}

bool KProcess::setExecutable(const QString& proc)
{
  if (runs) return false;

  if (proc.isEmpty())  return false;

  if (!arguments.isEmpty())
     arguments.remove(arguments.begin());
  arguments.prepend(QFile::encodeName(proc));

  return true;
}

KProcess &KProcess::operator<<(const QStringList& args)
{
  QStringList::ConstIterator it = args.begin();
  for ( ; it != args.end() ; ++it )
      arguments.append(QFile::encodeName(*it));
  return *this;
}

KProcess &KProcess::operator<<(const QCString& arg)
{
  return operator<< (arg.data());
}

KProcess &KProcess::operator<<(const char* arg)
{
  arguments.append(arg);
  return *this;
}

KProcess &KProcess::operator<<(const QString& arg)
{
  arguments.append(QFile::encodeName(arg));
  return *this;
}

void KProcess::clearArguments()
{
  arguments.clear();
}

bool KProcess::start(RunMode runmode, Communication comm)
{
  uint i;
  uint n = arguments.count();
  char **arglist;

  if (runs || (0 == n)) {
        return false;  // cannot start a process that is already running
        // or if no executable has been assigned
  }
  run_mode = runmode;
  status = 0;

  QCString shellCmd;
  if (d && d->useShell)
  {
      if (d->shell.isEmpty())
      {
          kdDebug() << "Could not find a valid shell\n" << endl;
              return false;
      }
      
      arglist = static_cast<char **>(malloc( (4)*sizeof(char *)));
      for (i=0; i < n; i++) {
          shellCmd += arguments[i];
          shellCmd += " "; // CC: to separate the arguments
      }

      arglist[0] = d->shell.data();
      arglist[1] = (char *) "-c";
      arglist[2] = shellCmd.data();
      arglist[3] = 0;
  }
  else
  {
      arglist = static_cast<char **>(malloc( (n+1)*sizeof(char *)));
      for (i=0; i < n; i++)
         arglist[i] = arguments[i].data();
      arglist[n]= 0;
  }

  if (!setupCommunication(comm))
      kdDebug() << "Could not setup Communication!\n";

  // We do this in the parent because if we do it in the child process
  // gdb gets confused when the application runs from gdb.
  uid_t uid = getuid();
  gid_t gid = getgid();
#ifdef HAVE_INITGROUPS
  struct passwd *pw = getpwuid(uid);
#endif

  int fd[2];
  if (0 > pipe(fd))
  {
     fd[0] = fd[1] = 0; // Pipe failed.. continue
  }

  runs = true;

  QApplication::flushX();

  // WABA: Note that we use fork() and not vfork() because
  // vfork() has unclear semantics and is not standardized.
  pid_ = fork();

  if (0 == pid_) {
        if (fd[0])
           close(fd[0]);
        if (!runPrivileged())
        {
           setgid(gid);
#if defined( HAVE_INITGROUPS)
	   if(pw)
              initgroups(pw->pw_name, pw->pw_gid);
#endif
           setuid(uid);
        }
        // The child process
        if(!commSetupDoneC())
          kdDebug() << "Could not finish comm setup in child!" << endl;
          
        setupEnvironment();

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

        // We set the close on exec flag.
        // Closing of fd[1] indicates that the execvp succeeded!
        if (fd[1])
          fcntl(fd[1], F_SETFD, FD_CLOEXEC);
        execvp(arglist[0], arglist);
        char resultByte = 1;
        if (fd[1])
          write(fd[1], &resultByte, 1);
        _exit(-1);
  } else if (-1 == pid_) {
        // forking failed

        runs = false;
        free(arglist);
        return false;
  } else {
        if (fd[1])
          close(fd[1]);
        // the parent continues here

        // Discard any data for stdin that might still be there
        input_data = 0;

        // Check whether client could be started.
        if (fd[0]) for(;;)
        {
           char resultByte;
           int n = ::read(fd[0], &resultByte, 1);
           if (n == 1)
           {
               // Error
               runs = false;
               close(fd[0]);
               free(arglist);
               pid_ = 0;
               return false;
           }
           if (n == -1)
           {
              if ((errno == ECHILD) || (errno == EINTR))
                 continue; // Ignore
           }
           break; // success
        }
        if (fd[0])
           close(fd[0]);

        if (!commSetupDoneP())  // finish communication socket setup for the parent
          kdDebug() << "Could not finish comm setup in parent!" << endl;

        if (run_mode == Block) {
          commClose();

          // The SIGCHLD handler of the process controller will catch
          // the exit and set the status
          while(runs)
          {
             KProcessController::theKProcessController->
                  waitForProcessExit(10);
          }
          runs = FALSE;
          emit processExited(this);
        }
  }
  free(arglist);
  return true;
}



bool KProcess::kill(int signo)
{
  bool rv=false;

  if (0 != pid_)
    rv= (-1 != ::kill(pid_, signo));
  // probably store errno somewhere...
  return rv;
}



bool KProcess::isRunning() const
{
  return runs;
}



pid_t KProcess::pid() const
{
  return pid_;
}



bool KProcess::normalExit() const
{
  int _status = status;
  return (pid_ != 0) && (!runs) && (WIFEXITED((_status)));
}



int KProcess::exitStatus() const
{
  int _status = status;
  return WEXITSTATUS((_status));
}



bool KProcess::writeStdin(const char *buffer, int buflen)
{
  bool rv;

  // if there is still data pending, writing new data
  // to stdout is not allowed (since it could also confuse
  // kprocess...
  if (0 != input_data)
    return false;

  if (runs && (communication & Stdin)) {
    input_data = buffer;
    input_sent = 0;
    input_total = buflen;
    slotSendData(0);
    innot->setEnabled(true);
    rv = true;
  } else
    rv = false;
  return rv;
}

void KProcess::suspend()
{
  if ((communication & Stdout) && outnot)
     outnot->setEnabled(false);
}

void KProcess::resume()
{
  if ((communication & Stdout) && outnot)
     outnot->setEnabled(true);
}

bool KProcess::closeStdin()
{
  bool rv;

  if (communication & Stdin) {
    communication = (Communication) (communication & ~Stdin);
    delete innot;
    innot = 0;
    close(in[1]);
    rv = true;
  } else
    rv = false;
  return rv;
}

bool KProcess::closeStdout()
{
  bool rv;

  if (communication & Stdout) {
    communication = (Communication) (communication & ~Stdout);
    delete outnot;
    outnot = 0;
    close(out[0]);
    rv = true;
  } else
    rv = false;
  return rv;
}

bool KProcess::closeStderr()
{
  bool rv;

  if (communication & Stderr) {
    communication = static_cast<Communication>(communication & ~Stderr);
    delete errnot;
    errnot = 0;
    close(err[0]);
    rv = true;
  } else
    rv = false;
  return rv;
}


/////////////////////////////
// protected slots         //
/////////////////////////////



void KProcess::slotChildOutput(int fdno)
{
  if (!childOutput(fdno))
     closeStdout();
}


void KProcess::slotChildError(int fdno)
{
  if (!childError(fdno))
     closeStderr();
}


void KProcess::slotSendData(int)
{
  if (input_sent == input_total) {
    innot->setEnabled(false);
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
  if (runs)
  {
    runs = false;
    status = state;

    commClose(); // cleanup communication sockets

    // also emit a signal if the process was run Blocking
    if (DontCare != run_mode)
    {
      emit processExited(this);
    }
  }
}



int KProcess::childOutput(int fdno)
{
  if (communication & NoRead) {
     int len = -1;
     emit receivedStdout(fdno, len);
     errno = 0; // Make sure errno doesn't read "EAGAIN"
     return len;
  }
  else
  {
     char buffer[1025];
     int len;

     len = ::read(fdno, buffer, 1024);
     
     if ( 0 < len) {
        buffer[len] = 0; // Just in case.
        emit receivedStdout(this, buffer, len);
     }
     return len;
  }
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
//        ok &= (-1 != fcntl(in[1], F_SETFL, O_NONBLOCK));
          innot =  new QSocketNotifier(in[1], QSocketNotifier::Write, this);
          Q_CHECK_PTR(innot);
          innot->setEnabled(false); // will be enabled when data has to be sent
          QObject::connect(innot, SIGNAL(activated(int)),
                                           this, SLOT(slotSendData(int)));
        }

        if (communication & Stdout) {
//        ok &= (-1 != fcntl(out[0], F_SETFL, O_NONBLOCK));
          outnot = new QSocketNotifier(out[0], QSocketNotifier::Read, this);
          Q_CHECK_PTR(outnot);
          QObject::connect(outnot, SIGNAL(activated(int)),
                                           this, SLOT(slotChildOutput(int)));
          if (communication & NoRead)
              suspend();
        }

        if (communication & Stderr) {
//        ok &= (-1 != fcntl(err[0], F_SETFL, O_NONBLOCK));
          errnot = new QSocketNotifier(err[0], QSocketNotifier::Read, this );
          Q_CHECK_PTR(errnot);
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
  memset(&so, 0, sizeof(so));

  if (communication & Stdin)
    close(in[1]);
  if (communication & Stdout)
    close(out[0]);
  if (communication & Stderr)
    close(err[0]);

  if (communication & Stdin)
    ok &= dup2(in[0],  STDIN_FILENO) != -1;
  else {
    int null_fd = open( "/dev/null", O_RDONLY );
    ok &= dup2( null_fd, STDIN_FILENO ) != -1;
    close( null_fd );
  }
  if (communication & Stdout) {
    ok &= dup2(out[1], STDOUT_FILENO) != -1;
    ok &= !setsockopt(out[1], SOL_SOCKET, SO_LINGER, (char*)&so, sizeof(so));
  }
  else {
    int null_fd = open( "/dev/null", O_WRONLY );
    ok &= dup2( null_fd, STDOUT_FILENO ) != -1;
    close( null_fd );
  }
  if (communication & Stderr) {
    ok &= dup2(err[1], STDERR_FILENO) != -1;
    ok &= !setsockopt(err[1], SOL_SOCKET, SO_LINGER, reinterpret_cast<char *>(&so), sizeof(so));
  }
  else {
    int null_fd = open( "/dev/null", O_WRONLY );
    ok &= dup2( null_fd, STDERR_FILENO ) != -1;
    close( null_fd );
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
            outnot = 0;
          }
          if (b_err) {
            fcntl(err[0], F_SETFL, O_NONBLOCK);
            if (err[0] > max_fd)
              max_fd = err[0];
            delete errnot;
            errnot = 0;
          }


          while (b_out || b_err) {
            // * If the process is still running we block until we
            // receive data. (p_timeout = 0, no timeout)
            // * If the process has already exited, we only check
            // the available data, we don't wait for more.
            // (p_timeout = &timeout, timeout immediately)
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            struct timeval *p_timeout = runs ? 0 : &timeout;

            FD_ZERO(&rfds);
            if (b_out)
              FD_SET(out[0], &rfds);

            if (b_err)
              FD_SET(err[0], &rfds);

            fds_ready = select(max_fd+1, &rfds, 0, 0, p_timeout);
            if (fds_ready <= 0) break;

            if (b_out && FD_ISSET(out[0], &rfds)) {
              int ret = 1;
              while (ret > 0) ret = childOutput(out[0]);
              if ((ret == -1 && errno != EAGAIN) || ret == 0)
                 b_out = false;
            }

            if (b_err && FD_ISSET(err[0], &rfds)) {
              int ret = 1;
              while (ret > 0) ret = childError(err[0]);
              if ((ret == -1 && errno != EAGAIN) || ret == 0)
                 b_err = false;
            }
          }
        }

        if (b_in) {
	    communication = (Communication) (communication & ~Stdin);
            close(in[1]);
        }
        if (b_out) {
	    communication = (Communication) (communication & ~Stdout);
            close(out[0]);
        }
        if (b_err) {
	    communication = (Communication) (communication & ~Stderr);
            close(err[0]);
        }
  }
}

void KProcess::setUseShell(bool useShell, const char *shell)
{
  if (!d)
    d = new KProcessPrivate;
  d->useShell = useShell;
  d->shell = shell;
  if (d->shell.isEmpty())
     d->shell = searchShell();
}

QString KProcess::quote(const QString &arg)
{
    QString res = arg;
    res.replace(QRegExp(QString::fromLatin1("\'")),
                QString::fromLatin1("'\"'\"'"));
    res.prepend('\'');
    res.append('\'');
    return res;
}

QCString KProcess::searchShell()
{
  QCString tmpShell = QCString(getenv("SHELL")).stripWhiteSpace();
  if (!isExecutable(tmpShell))
  {
     tmpShell = "/bin/sh";
  }

  return tmpShell;
}

bool KProcess::isExecutable(const QCString &filename)
{
  struct stat fileinfo;

  if (filename.isEmpty()) return false;

  // CC: we've got a valid filename, now let's see whether we can execute that file

  if (-1 == stat(filename.data(), &fileinfo)) return false;
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
    return false;
  }

  // CC: now check for permission to execute the file
  if (access(filename.data(), X_OK) != 0) return false;

  // CC: we've passed all the tests...
  return true;
}

void KProcess::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////////
// CC: Class KShellProcess
///////////////////////////

KShellProcess::KShellProcess(const char *shellname):
  KProcess()
{
  setUseShell(true, shellname);
}


KShellProcess::~KShellProcess() {
}

QString KShellProcess::quote(const QString &arg)
{
    return KProcess::quote(arg);
}

bool KShellProcess::start(RunMode runmode, Communication comm)
{
  return KProcess::start(runmode, comm);
}

void KShellProcess::virtual_hook( int id, void* data )
{ KProcess::virtual_hook( id, data ); }

#include "kprocess.moc"
