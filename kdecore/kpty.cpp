/*

   $Id$

   This file is part of the KDE libraries
   Copyright (C) 1997-2002 The Konsole Developers
   Copyright (C) 2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>

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


#include "kpty.h"
#include "kprocess.h"

#include <config.h>

#ifdef __sgi
#define __svr4__
#endif

#ifdef __osf__
#define _OSF_SOURCE
#include <float.h>
#endif

#ifdef _AIX
#define _ALL_SOURCE
#endif

// __USE_XOPEN isn't defined by default in ICC
// (needed for ptsname(), grantpt() and unlockpt())
#ifdef __INTEL_COMPILER
#  ifndef __USE_XOPEN
#    define __USE_XOPEN
#  endif
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
//#include <sys/socket.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_STROPTS_H
# include <sys/stropts.h>	// Defines I_PUSH
# define _NEW_TTY_CTRL
#endif
//#ifdef HAVE_SYS_SELECT_H
//#include <sys/select.h>
//#endif

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>

#ifdef HAVE_LIBUTIL_H
# include <libutil.h>
# define USE_LOGIN
#elif defined(HAVE_UTIL_H)
# include <util.h>
# define USE_LOGIN
#endif

#ifdef USE_LOGIN
# include <utmp.h>
#endif

#ifdef HAVE_TERMIOS_H
/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
# include <termios.h>
}
#endif

#if !defined(__osf__)
# ifdef HAVE_TERMIO_H
/* needed at least on AIX */
#  include <termio.h>
# endif
#endif

#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__)
# define _tcgetattr(fd, ttmode) ioctl(fd, TIOCGETA, (char *)ttmode)
#else
# if defined(_HPUX_SOURCE) || defined(__Lynx__)
#  define _tcgetattr(fd, ttmode) tcgetattr(fd, ttmode)
# else
#  define _tcgetattr(fd, ttmode) ioctl(fd, TCGETS, (char *)ttmode)
# endif
#endif

#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__)
# define _tcsetattr(fd, ttmode) ioctl(fd, TIOCSETA, (char *)ttmode)
#else
# ifdef _HPUX_SOURCE
#  define _tcsetattr(fd, ttmode) tcsetattr(fd, TCSANOW, ttmode)
# else
#  define _tcsetattr(fd, ttmode) ioctl(fd, TCSETS, (char *)ttmode)
# endif
#endif

#if defined (_HPUX_SOURCE)
# define _TERMIOS_INCLUDED
# include <bsdtty.h>
#endif

#if defined(HAVE_PTY_H)
# include <pty.h>
#endif

//#include <qfile.h>
//#include <qapplication.h>

#include <kdebug.h>
#include <kstandarddirs.h>	// locate

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
#define BASE_CHOWN "kgrantpty"



//////////////////
// private data //
//////////////////

struct KPtyPrivate {
   KPtyPrivate() : 
     xonXoff(false), needGrantPty(false),
     masterFd(-1), slaveFd(-1)
   { 
     memset(&winSize, 0, sizeof(winSize));
     winSize.ws_row = 24;
     winSize.ws_col = 80;
   }

   bool xonXoff : 1;
   bool needGrantPty : 1;
   int masterFd;
   int slaveFd;
   struct winsize winSize;

   QCString ttyName;
};

/////////////////////////////
// public member functions //
/////////////////////////////

KPty::KPty()
{
  d = new KPtyPrivate;
}

KPty::~KPty()
{
  close();
  delete d;
}
  
bool KPty::open()
{
  if (d->masterFd >= 0)
    return true;

  QCString ptyName;

  d->needGrantPty = true;

  // Find a master pty that we can open ////////////////////////////////

  // Because not all the pty animals are created equal, they want to
  // be opened by several different methods.

  // We try, as we know them, one by one.
#if defined(HAVE_OPENPTY)
    if (openpty(&d->masterFd, &d->slaveFd, NULL, NULL, NULL) == 0)
    {
      d->ttyName = ttyname(d->slaveFd);

      d->needGrantPty = false;

      /* Get the group ID of the special `tty' group.  */
      struct group* p = getgrnam(TTY_GROUP);    /* posix */
      gid_t gid = p ? p->gr_gid : getgid ();    /* posix */

      if (fchown(d->slaveFd, (uid_t) -1, gid) < 0)
      {
         int e = errno;
         d->needGrantPty = true;
         kdWarning(175) << "Cannot chown " << d->ttyName << endl
                        << "Reason " << strerror(e) << endl;
      }
      else if (fchmod(d->slaveFd, S_IRUSR|S_IWUSR|S_IWGRP) < 0)
      {
         int e = errno;
         d->needGrantPty = true;
         kdWarning(175) << "Cannot chmod " << d->ttyName << endl
                        << "Reason " << strerror(e) << endl;
      }
      goto gotpty;
    }
#endif

//#if defined(__sgi__) || defined(__osf__) || defined(__svr4__)
#if defined(HAVE_GRANTPT) && defined(HAVE_PTSNAME)
#ifdef _AIX
    d->masterFd = ::open("/dev/ptc",O_RDWR);
#else
    d->masterFd = ::open("/dev/ptmx",O_RDWR);
#endif
    if (d->masterFd >= 0)
    {
      char *ptsn = ptsname(d->masterFd);
      if (ptsn) {
          d->ttyName = ptsn;
          grantpt(d->masterFd);	// XXX could this fail?
          d->needGrantPty = false;
          goto gotpty;
      } else {
	  ::close(d->masterFd);
	  d->masterFd = -1;
      }
    }
#endif

#if defined(_SCO_DS) || defined(__USLC__) // SCO OSr5 and UnixWare, might be obsolete
    for (int idx = 0; idx < 256; idx++)
    { 
      ptyName.sprintf("/dev/ptyp%d", idx);
      d->ttyName.sprintf("/dev/ttyp%d", idx);
      if (access(d->ttyName.data(), F_OK) < 0) 
        break; // no such device ...
     
      d->masterFd = ::open(ptyName.data(), O_RDWR);
      if (d->masterFd >= 0)
      {
        if (geteuid() == 0 || access (d->ttyName.data(), R_OK|W_OK) == 0)
          goto gotpty;
        ::close(d->masterFd); 
        d->masterFd = -1;
      }
    }
#endif

    // Linux device names, FIXME: Trouble on other systems?
    for (const char* s3 = "pqrstuvwxyzabcdefghijklmno"; *s3 != 0; s3++)
    { 
      for (const char* s4 = "0123456789abcdefghijklmnopqrstuvwxyz"; *s4 != 0; s4++)
      { 
        ptyName.sprintf("/dev/pty%c%c", *s3, *s4);
        d->ttyName.sprintf("/dev/tty%c%c", *s3, *s4);
        //if (access(d->ttyName.data(), F_OK) < 0) 
        //  break; // no such device ...
           
        d->masterFd = ::open(ptyName.data(), O_RDWR);
        if (d->masterFd >= 0)
        {
          if (geteuid() == 0 || access(d->ttyName.data(),R_OK|W_OK) == 0)
            goto gotpty; // Success !!
          ::close(d->masterFd);
          d->masterFd = -1;
        }
      }
    }

    kdWarning(175) << "Can't open a pseudo teletype" << endl;
    return false;

 gotpty:
  if (!chownpty(true))
  {
    kdWarning(175)
      << "chownpty failed for device " << ptyName << "::" << d->ttyName
      << "\nThis means the communication can be eavesdropped." << endl;
  }

//  fcntl(d->masterFd,F_SETFL,O_NDELAY);

#ifdef HAVE_UNLOCKPT
  unlockpt(d->masterFd);
#endif

  if (d->slaveFd < 0) { // slave pty not yet opened?
    d->slaveFd = ::open(d->ttyName, O_RDWR);
    if (d->slaveFd < 0)
    {
      kdWarning(175) << "Can't open slave pseudo teletype" << endl;
      ::close(d->masterFd);
      d->masterFd = -1;
      return false;
    }
  }

#if (defined(__svr4__) || defined(__sgi__))
  // Solaris
  ioctl(d->slaveFd, I_PUSH, "ptem");
  ioctl(d->slaveFd, I_PUSH, "ldterm");
#endif

    // set xon/xoff & control keystrokes
  // without the '::' some version of HP-UX thinks, this declares
  // the struct in this class, in this method, and fails to find 
  // the correct tc[gs]etattr 
  struct ::termios ttmode;

  _tcgetattr(d->slaveFd, &ttmode);

  if (!d->xonXoff)
    ttmode.c_iflag &= ~(IXOFF | IXON);
  else
    ttmode.c_iflag |= (IXOFF | IXON);

  ttmode.c_cc[VINTR] = CTRL('C' - '@');
  ttmode.c_cc[VQUIT] = CTRL('\\' - '@');
  ttmode.c_cc[VERASE] = 0177;

  _tcsetattr(d->slaveFd, &ttmode);

  // set screen size
  ioctl(d->slaveFd, TIOCSWINSZ, (char *)&d->winSize);

  fcntl(d->masterFd, F_SETFD, FD_CLOEXEC);
  fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);

  return true;
}

void KPty::close()
{
   if (d->masterFd < 0)
      return;
   chownpty(false);
   ::close(d->slaveFd);
   ::close(d->masterFd);
   d->masterFd = d->slaveFd = -1;
}

void KPty::setCTty()
{
    // Setup job control //////////////////////////////////

    // Become session leader, process group leader,
    // and get rid of the old controlling terminal.
    setsid();

    // make our slave pty the new controlling terminal.
#ifdef TIOCSCTTY
    ioctl(d->slaveFd, TIOCSCTTY, 0);
#else
    // SVR4 hack: the first tty opened after setsid() becomes controlling tty
    ::close(::open(d->ttyName, O_WRONLY, 0));
#endif

    // make our new process group the foreground group on the pty
    int pgrp = getpid();
#if defined(_POSIX_VERSION) || defined(__svr4__)
    tcsetpgrp (d->slaveFd, pgrp);
#elif defined(TIOCSPGRP)
    ioctl(d->slaveFd, TIOCSPGRP, (char *)&pgrp);
#endif
}

void KPty::login(const char *user, const char *remotehost)
{
#ifdef HAVE_UTEMPTER
    KProcess_Utmp utmp;
    utmp.cmdFd = d->masterFd;
    utmp << "/usr/sbin/utempter" << "-a" << d->ttyName << "";
    utmp.start(KProcess::Block);
    Q_UNUSED(user);
    Q_UNUSED(remotehost);
#elif defined(USE_LOGIN)
    const char *str_ptr;
    struct utmp l_struct;
    memset(&l_struct, 0, sizeof(struct utmp));
    // note: strncpy without terminators _is_ correct here. man 4 utmp

    if (user)
      strncpy(l_struct.ut_name, user, UT_NAMESIZE);

    if (remotehost)
      strncpy(l_struct.ut_host, remotehost, UT_HOSTSIZE);

# ifndef __GLIBC__
    str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5))
        str_ptr += 5;
    strncpy(l_struct.ut_line, str_ptr, UT_LINESIZE);
# endif

    time(&l_struct.ut_time);

    ::login(&l_struct);
#else
    Q_UNUSED(user);
    Q_UNUSED(remotehost);
#endif
}

void KPty::logout()
{
#ifdef HAVE_UTEMPTER
    KProcess_Utmp utmp;
    utmp.cmdFd = d->masterFd;
    utmp << "/usr/sbin/utempter" << "-d" << d->ttyName;
    utmp.start(KPty::Block);
#elif defined(USE_LOGIN)
    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5))
        str_ptr += 5;
# ifdef __GLIBC__
    else {
        const char *sl_ptr = strrchr(str_ptr, '/');
        if (sl_ptr)
            str_ptr = sl_ptr + 1;
    }
# endif
    ::logout(str_ptr);
#endif
}

void KPty::setWinSize(int lines, int columns)
{
  d->winSize.ws_row = (unsigned short)lines;
  d->winSize.ws_col = (unsigned short)columns;
  if (d->masterFd >= 0)
    ioctl( d->masterFd, TIOCSWINSZ, (char *)&d->winSize );
}
  
void KPty::setXonXoff(bool useXonXoff)
{
  d->xonXoff = useXonXoff;
  if (d->masterFd >= 0) {
    // without the '::' some version of HP-UX thinks, this declares
    // the struct in this class, in this method, and fails to find 
    // the correct tc[gs]etattr 
    struct ::termios ttmode;

    _tcgetattr(d->masterFd, &ttmode);

    if (!useXonXoff)
      ttmode.c_iflag &= ~(IXOFF | IXON);
    else
      ttmode.c_iflag |= (IXOFF | IXON);

    _tcsetattr(d->masterFd, &ttmode);
  }
}

const char *KPty::ttyName()
{
    return d->ttyName.data();
}

int KPty::masterFd()
{
    return d->masterFd;
}

int KPty::slaveFd()
{
    return d->slaveFd;
}

// private
bool KPty::chownpty(bool grant)
{
  if (!d->needGrantPty)
    return true;

  // TODO: change to kprocess?
  pid_t pid = fork();
  if (pid == 0)
  {
    /* We pass the master pseudo terminal as file descriptor PTY_FILENO. */
    if (d->masterFd != PTY_FILENO && 
        dup2(d->masterFd , PTY_FILENO) < 0)
      exit(1);
    QString path = locate("exe", BASE_CHOWN);
    execle(path.ascii(), BASE_CHOWN, grant?"--grant":"--revoke", (void *)0, 
    	NULL);
    exit(1); // should not be reached
  }
  else if (pid > 0)
  {
    int w;

retry:
    int rc = waitpid(pid, &w, 0);
    if ((rc == -1) && (errno == EINTR))
      goto retry;

    return (rc != -1 && WIFEXITED(w) && WEXITSTATUS(w) == 0);
  }

  return false;
}

