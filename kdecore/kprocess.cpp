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

#ifdef __sgi
#define __svr4__
#endif

#if defined(HAVE_GRANTPT) && defined(HAVE_PTSNAME) && defined(HAVE_UNLOCKPT) && !defined(_XOPEN_SOURCE) && !defined(__svr4__)
#define _XOPEN_SOURCE // make stdlib.h offer the above fcts
#endif

/* for NSIG */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifdef __osf__
#define _OSF_SOURCE
#include <float.h>
#endif

#ifdef _AIX
#define _ALL_SOURCE
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_STROPTS_H
#include <sys/stropts.h>
#define _NEW_TTY_CTRL
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <grp.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#ifdef HAVE_LIBUTIL_H
        #include <libutil.h>
        #define USE_LOGIN
#elif defined(HAVE_UTIL_H)
        #include <util.h>
        #define USE_LOGIN
#endif

#ifdef USE_LOGIN
        #include <utmp.h>
#endif

#ifdef HAVE_TERMIOS_H
/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
#include <termios.h>
}
#endif

#if !defined(__osf__)
#ifdef HAVE_TERMIO_H
/* needed at least on AIX */
#include <termio.h>
#endif
#endif

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif

#if defined(HAVE_PTY_H)
#include <pty.h>
#endif

#include <qfile.h>
#include <qsocketnotifier.h>
#include <qregexp.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#define TTY_GROUP "tty"

///////////////////////
// private functions //
///////////////////////

class KProcess_Utmp : public KProcess
{
public:
   int commSetupDoneC()
   {
     dup2(cmdFd, 0);
     dup2(cmdFd, 1);
     dup2(cmdFd, 3);
     return 1;
   }
   int cmdFd;
};

#define PTY_FILENO 3
#define BASE_CHOWN "konsole_grantpty"

static int kprocess_chownpty(int fd, bool grant)
// param fd: the fd of a master pty.
// param grant: true to grant, false to revoke
// returns 1 on success 0 on fail
{
  struct sigaction newsa, oldsa;
  newsa.sa_handler = SIG_DFL;
  newsa.sa_mask = sigset_t();
  newsa.sa_flags = 0;
  sigaction(SIGCHLD, &newsa, &oldsa);

  pid_t pid = fork();
  if (pid < 0)
  {
    // restore previous SIGCHLD handler
    sigaction(SIGCHLD, &oldsa, NULL);

    return 0;
  }
  if (pid == 0)
  {
    /* We pass the master pseudo terminal as file descriptor PTY_FILENO. */
    if (fd != PTY_FILENO && dup2(fd, PTY_FILENO) < 0) exit(1);
    QString path = locate("exe", BASE_CHOWN);
    execle(path.ascii(), BASE_CHOWN, grant?"--grant":"--revoke", NULL, NULL);
    exit(1); // should not be reached
  }

  if (pid > 0) {
    int w;

retry:
    int rc = waitpid(pid, &w, 0);
    if ((rc == -1) && (errno == EINTR))
      goto retry;

    // restore previous SIGCHLD handler
    sigaction(SIGCHLD, &oldsa, NULL);

    return (rc != -1 && WIFEXITED(w) && WEXITSTATUS(w) == 0);
  }

  return 0; //dummy.
}


//////////////////
// private data //
//////////////////

class KProcessPrivate {
public:
   KProcessPrivate() : 
     useShell(false), usePty(false), addUtmp(false), ptyXonXoff(false),
     ptyNeedGrantPty(false),
     ptyMasterFd(-1), ptySlaveFd(-1)
   { 
     memset(&ptySize, 0, sizeof(ptySize));
     ptySize.ws_row = 25;
     ptySize.ws_col = 80;
   }

   bool useShell : 1;
   bool usePty : 1;
   bool addUtmp : 1;
   bool ptyXonXoff : 1;
   bool ptyNeedGrantPty : 1;
   int ptyMasterFd;
   int ptySlaveFd;
   struct winsize ptySize;
   
   QMap<QString,QString> env;
   QString wd;
   QCString shell;
   QCString ptySlaveName;
   QCString ptyMasterName;
   QCString executable;
};

/////////////////////////////
// public member functions //
/////////////////////////////

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

void KProcess::setBinaryExecutable(const char *filename)
{
   if (!d)
      d = new KProcessPrivate;
   
   d->executable = filename;
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
  {
      kdDebug(175) << "Could not setup Communication!\n";
      return false;
  }

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
     fd[0] = fd[1] = -1; // Pipe failed.. continue
  }

  runs = true;

  QApplication::flushX();

  // WABA: Note that we use fork() and not vfork() because
  // vfork() has unclear semantics and is not standardized.
  pid_ = fork();

  if (0 == pid_) {
        if (fd[0] >= 0)
           close(fd[0]);

        // The child process
        if(!commSetupDoneC())
          kdDebug(175) << "Could not finish comm setup in child!" << endl;

        // reset all signal handlers
        for (int sig = 1; sig < NSIG; sig++)
        {
          struct sigaction act;
          sigemptyset(&(act.sa_mask));
          sigaddset(&(act.sa_mask), sig);
          act.sa_handler = SIG_DFL;
          act.sa_flags = 0;
          sigaction(sig, &act, 0L);
        }

        if (!runPrivileged())
        {
           setgid(gid);
#if defined( HAVE_INITGROUPS)
	   if(pw)
              initgroups(pw->pw_name, pw->pw_gid);
#endif
           setuid(uid);
        }
          
        setupEnvironment();

        // Matthias
        if (run_mode == DontCare)
          setpgid(0,0);

        // We set the close on exec flag.
        // Closing of fd[1] indicates that the execvp succeeded!
        if (fd[1])
          fcntl(fd[1], F_SETFD, FD_CLOEXEC);

        const char *executable = arglist[0];
        if (d && !d->executable.isEmpty())
           executable = d->executable.data();

        execvp(executable, arglist);

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
        if (fd[0] >= 0) for(;;)
        {
           char resultByte;
           int n = ::read(fd[0], &resultByte, 1);
           if (n == 1)
           {
               // Error
               runs = false;
               close(fd[0]);
               free(arglist);
               ::waitpid( pid_, 0, 0 );
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
        if (fd[0] >= 0)
           close(fd[0]);

        if (!commSetupDoneP())  // finish communication socket setup for the parent
          kdDebug(175) << "Could not finish comm setup in parent!" << endl;

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

void KProcess::setUseShell(bool useShell, const char *shell)
{
  if (!d)
    d = new KProcessPrivate;
  d->useShell = useShell;
  d->shell = (shell && *shell) ? shell : "/bin/sh";
}

void KProcess::setUsePty(bool usePty, bool addUtmp)
{
  if (!d)
    d = new KProcessPrivate;
  d->usePty = usePty;
  d->addUtmp = addUtmp;  
}
  
void KProcess::setPtySize(int lines, int columns)
{
  if (!d)
    d = new KProcessPrivate;
  d->ptySize.ws_row = (unsigned short)lines;
  d->ptySize.ws_col = (unsigned short)columns;
  if(d->ptyMasterFd < 0) return;
  ioctl(d->ptyMasterFd, TIOCSWINSZ,(char *)&(d->ptySize));
   
}
  
void KProcess::setPtyXonXoff(bool useXonXoff)
{
  if (!d)
    d = new KProcessPrivate;
  d->ptyXonXoff = useXonXoff;
}

const char *KProcess::ptyMasterName()
{
    return d ? d->ptyMasterName.data() : 0;
}

const char *KProcess::ptySlaveName()
{
    return d ? d->ptySlaveName.data() : 0;
}

int KProcess::ptyMasterFd()
{
    return d ? d->ptyMasterFd : -1;
}

int KProcess::ptySlaveFd()
{
    return d ? d->ptySlaveFd : -1;
}

QString KProcess::quote(const QString &arg)
{
    QString res = arg;
    res.replace(QString::fromLatin1("\'"), QString::fromLatin1("'\\''"));
    res.prepend('\'');
    res.append('\'');
    return res;
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

void KProcess::openMasterPty()
{
  if (!d)
    d = new KProcessPrivate;

  d->ptyNeedGrantPty = true;

  // Find a master pty that we can open ////////////////////////////////

  // Because not all the pty animals are created equal, they want to
  // be opened by several different methods.

  // We try, as we know them, one by one.
#if defined(HAVE_OPENPTY)
  if (d->ptyMasterFd < 0) 
  {
    int master_fd, slave_fd;
    if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL) == 0)
    {
      d->ptyMasterFd = master_fd;
      d->ptySlaveFd = slave_fd;
#ifdef HAVE_PTSNAME
      d->ptyMasterName = ptsname(master_fd);
#else
      // Just a guess, maybe ttyname with return nothing.
      d->ptyMasterName = ttyname(master_fd);
#endif
      d->ptySlaveName = ttyname(slave_fd);

      d->ptyNeedGrantPty = false;

      /* Get the group ID of the special `tty' group.  */
      struct group* p = getgrnam(TTY_GROUP);    /* posix */
      gid_t gid = p ? p->gr_gid : getgid ();    /* posix */

      if (fchown(slave_fd, (uid_t) -1, gid) < 0)
      {
         int e = errno;
         d->ptyNeedGrantPty = true;
         kdWarning(175) << "Cannot chown " << d->ptySlaveName << endl;
         kdWarning(175) << "Reason " << strerror(e) << endl;
      }
      else if (fchmod(slave_fd, S_IRUSR|S_IWUSR|S_IWGRP) < 0)
      {
         int e = errno;
         d->ptyNeedGrantPty = true;
         kdWarning(175) << "Cannot chmod " << d->ptySlaveName << endl;
         kdWarning(175) << "Reason " << strerror(e) << endl;
      }
    }
  }
#endif

//#if defined(__sgi__) || defined(__osf__) || defined(__svr4__)
#if defined(HAVE_GRANTPT) && defined(HAVE_PTSNAME)
  if (d->ptyMasterFd < 0)
  {
#ifdef _AIX
    d->ptyMasterFd = open("/dev/ptc",O_RDWR);
#else
    d->ptyMasterFd = open("/dev/ptmx",O_RDWR);
#endif
    if (d->ptyMasterFd >= 0)
    {
      char *ptsn = ptsname(d->ptyMasterFd);
      if (ptsn) {
          d->ptySlaveName = ptsname(d->ptyMasterFd);
          grantpt(d->ptyMasterFd);
          d->ptyNeedGrantPty = false;
      } else {
      	  perror("ptsname");
	  close(d->ptyMasterFd);
	  d->ptyMasterFd = -1;
      }
    }
  }
#endif

#if defined(_SCO_DS) || defined(__USLC__) // SCO OSr5 and UnixWare, might be obsolete
  if (d->ptyMasterFd < 0)
  { 
    for (int idx = 0; idx < 256; idx++)
    { 
      d->ptyMasterName.sprintf("/dev/ptyp%d", idx);
      d->ptySlaveName.sprintf("/dev/ttyp%d", idx);
      if (access(d->ptySlaveName, F_OK) < 0) 
      { 
        idx = 256; 
        break; 
      }
     
      d->ptyMasterFd = open( d->ptyMasterName, O_RDWR);
      
      if (d->ptyMasterFd >= 0)
      {
        if (access (d->ptySlaveName, R_OK|W_OK) == 0) 
          break; // Success!!
        close(d->ptyMasterFd); 
        d->ptyMasterFd = -1;
      }
    }
  }
#endif

  if (d->ptyMasterFd < 0) // Linux device names, FIXME: Trouble on other systems?
  { 
    for (const char* s3 = "pqrstuvwxyzabcdefghijklmno"; *s3 != 0; s3++)
    { 
      for (const char* s4 = "0123456789abcdefghijklmnopqrstuvwxyz"; *s4 != 0; s4++)
      { 
        d->ptyMasterName.sprintf("/dev/pty%c%c", *s3, *s4);
        d->ptySlaveName.sprintf("/dev/tty%c%c", *s3, *s4);
           
        d->ptyMasterFd = open(d->ptyMasterName, O_RDWR);
        if (d->ptyMasterFd >= 0)
        {
          if (geteuid() == 0 || access(d->ptySlaveName,R_OK|W_OK) == 0)
            break; // Success !!
          close(d->ptyMasterFd);
          d->ptyMasterFd = -1;
        }
      }
      if (d->ptyMasterFd >= 0) 
        break; // Success, break out of loop
    }
  }

  if (d->ptyMasterFd < 0)
  {
    kdWarning(175) << "Can't open a pseudo teletype" << endl;
    return;
  }

  if (d->ptyNeedGrantPty && !kprocess_chownpty(d->ptyMasterFd, true))
  {
    kdWarning(175) << "chownpty failed for device " << d->ptyMasterName << 
                   "::" << d->ptySlaveName << endl;
    kdWarning(175) << "This mean the communication can be eavesdropped." << endl;
  }

  fcntl(d->ptyMasterFd,F_SETFL,O_NDELAY);

}

void KProcess::openSlavePty()
{
  if (!d)
    d = new KProcessPrivate;

  if (d->ptyMasterFd < 0) // no master pty could be opened
  {
    fprintf(stdout,"opening master pty failed.\n");
    exit(1);
  }

#ifdef HAVE_UNLOCKPT
  unlockpt(d->ptyMasterFd);
#endif

  // open and set all standard files to slave tty
  if (d->ptySlaveFd < 0) // Not yet opened?
    d->ptySlaveFd = open(d->ptySlaveName, O_RDWR);

  if (d->ptySlaveFd < 0) // the slave pty could not be opened
  {
    fprintf(stdout,"opening slave pty failed.\n");
    exit(1);
  }

#if (defined(__svr4__) || defined(__sgi__))
  // Solaris
  ioctl(d->ptySlaveFd, I_PUSH, "ptem");
  ioctl(d->ptySlaveFd, I_PUSH, "ldterm");
#endif

  // Stamp utmp/wtmp if we have and want them
#ifdef HAVE_UTEMPTER
  if (d->addUtmp)
  {
     KProcess_Utmp utmp;
     utmp.cmdFd = d->ptyMasterFd;
     utmp << "/usr/sbin/utempter" << "-a" << d->ptySlaveName << "";
     utmp.start(KProcess::Block);
  }
#endif
#ifdef USE_LOGIN
  char *str_ptr;
  struct utmp l_struct;
  memset(&l_struct, 0, sizeof(struct utmp));

  if (! (str_ptr=getlogin()) ) {
    if ( ! (str_ptr=getenv("LOGNAME"))) {
      abort();
    }
  }
  strlcpy(l_struct.ut_name, str_ptr, UT_NAMESIZE);

  if (gethostname(l_struct.ut_host, UT_HOSTSIZE) == -1) {
     if (errno != ENOMEM)
        abort();
     l_struct.ut_host[UT_HOSTSIZE]=0;
  }

  str_ptr = ttyname(d->ptySlaveFd);
  if (!str_ptr)
     str_ptr = "unknown";
  if (strncmp(str_ptr, "/dev/", 5) == 0)
     str_ptr += 5;
  strlcpy(l_struct.ut_line, str_ptr, UT_LINESIZE);
  time(&l_struct.ut_time);

  login(&l_struct);
#endif
}


int KProcess::setupCommunication(Communication comm)
{
  // PTY stuff //
  if (d && d->usePty)
  {
    openMasterPty();
    if (d->ptyMasterFd < 0)
       return 0;

    out[0] = d->ptyMasterFd;
    out[1] = dup(2); // Dummy
    communication = (Communication) (comm | Stdout & !Stdin & !Stderr);
    return 1;
  }

  communication = comm;

  if ((comm & Stdin) && (socketpair(AF_UNIX, SOCK_STREAM, 0, in) < 0))
     comm = (Communication) (comm & ~Stdin);

  if ((comm & Stdout) && (socketpair(AF_UNIX, SOCK_STREAM, 0, out) < 0))
     comm = (Communication) (comm & ~Stdout);

  if ((comm & Stderr) && (socketpair(AF_UNIX, SOCK_STREAM, 0, err) < 0))
     comm = (Communication) (comm & ~Stderr);
  
  if (communication != comm)
  {
     if (comm & Stdin)
     {
        close(in[0]);
        close(in[1]);        
     }
     if (comm & Stdout)
     {
        close(out[0]);
        close(out[1]);        
     }
     if (comm & Stderr)
     {
        close(err[0]);
        close(err[1]);        
     }
     communication = NoCommunication;
     return 0; // Error
  }

  return 1; // Ok
}



int KProcess::commSetupDoneP()
{
  int ok = 1;
  // PTY stuff //
  if (d && d->usePty)
  {
    if (d->ptySlaveFd >= 0)
    {
       close(d->ptySlaveFd);
       d->ptySlaveFd = -1;
    }
  }

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
  // PTY stuff //
  if (d && d->usePty)
  {
    openSlavePty();
    dup2(d->ptySlaveFd, STDIN_FILENO);
    dup2(d->ptySlaveFd, STDOUT_FILENO);
    dup2(d->ptySlaveFd, STDERR_FILENO);

    // Setup job control //////////////////////////////////

    // This is pretty obscure stuff which makes the session
    // to be the controlling terminal of a process group.
    setsid();

#if defined(TIOCSCTTY)
    ioctl(0, TIOCSCTTY, 0);
#endif

    // The following sequence is necessary for event propagation
    // Omitting this is not noticeable with all clients (bash,vi). 
    // Because bash heals this, use '-e' to test it.
    int pgrp = getpid();                 
#ifdef _AIX
    tcsetpgrp (0, pgrp);
#else
    ioctl(0, TIOCSPGRP, (char *)&pgrp);
#endif
    setpgid(0,0);
    close(open(d->ptySlaveName, O_WRONLY, 0));
    setpgid(0,0);
    // End event propagation //


    // without the '::' some version of HP-UX thinks, this declares
    // the struct in this class, in this method, and fails to find 
    // the correct t[gc]etattr 
    static struct ::termios ttmode;

#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__)
    ioctl(0,TIOCGETA,(char *)&ttmode);
#else
# if defined (_HPUX_SOURCE) || defined(__Lynx__)
    tcgetattr(0, &ttmode);
# else
    ioctl(0,TCGETS,(char *)&ttmode);
# endif
#endif

#undef CTRL
#define CTRL(c) ((c) - '@')
    if (!d->ptyXonXoff)
      ttmode.c_iflag &= ~(IXOFF | IXON);
    ttmode.c_cc[VINTR] = CTRL('C');
    ttmode.c_cc[VQUIT] = CTRL('\\');
    ttmode.c_cc[VERASE] = 0177;

#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__)
    ioctl(0,TIOCSETA,(char *)&ttmode);
#else
# ifdef _HPUX_SOURCE
    tcsetattr(0, TCSANOW, &ttmode);
# else
    ioctl(0,TCSETS,(char *)&ttmode);
# endif
#endif

    // set screen size
    ioctl(0,TIOCSWINSZ,(char *)&(d->ptySize));  
  }
  // End of PTY stuff //
  else
  {
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
  }
  
#if 0
  // Skip fd[1]
  // Skip fd == 3 when using ptys

  // close all fds
  const int open_max = sysconf( _SC_OPEN_MAX );
  for( int i = 0;
       i < open_max;
       ++i )
  {
      if( i != STDIN_FILENO && i != STDOUT_FILENO && i != STDERR_FILENO )
      {
          close( i );
      }
  }
#endif
          
  return ok;
}



void KProcess::commClose()
{
  // PTY stuff //
  if (d && d->usePty)
  {
#ifdef HAVE_UTEMPTER
     if (d->addUtmp)
     {
        KProcess_Utmp utmp;
        utmp.cmdFd = d->ptyMasterFd;
        utmp << "/usr/sbin/utempter" << "-d" << d->ptySlaveName;
        utmp.start(KProcess::Block);
     }
#elif defined(USE_LOGIN)
     char *tty_name=ttyname(0);
     if (tty_name)
     {
  	if (strncmp(tty_name, "/dev/", 5) == 0)
	    tty_name += 5;
        logout(tty_name);
     }
#endif
     if (d->ptyNeedGrantPty) 
        kprocess_chownpty(d->ptyMasterFd, false);
  }
  // End of PTY stuff //

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

        if (communication & Stdin) {
	    communication = (Communication) (communication & ~Stdin);
            close(in[1]);
        }
        if (communication & Stdout) {
	    communication = (Communication) (communication & ~Stdout);
            close(out[0]);
        }
        if (communication & Stderr) {
	    communication = (Communication) (communication & ~Stderr);
            close(err[0]);
        }
  }
}


void KProcess::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////////
// CC: Class KShellProcess
///////////////////////////

KShellProcess::KShellProcess(const char *shellname):
  KProcess()
{
  setUseShell( true, shellname ? shellname : getenv("SHELL") );
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
