/*

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include "kpty.h"
#include "kprocess.h"

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
#include <sys/stat.h>
#include <sys/param.h>

#ifdef HAVE_SYS_STROPTS_H
# include <sys/stropts.h>	// Defines I_PUSH
# define _NEW_TTY_CTRL
#endif

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
# if defined(_HPUX_SOURCE) || defined(__Lynx__) || defined (__CYGWIN__)
#  define _tcgetattr(fd, ttmode) tcgetattr(fd, ttmode)
# else
#  define _tcgetattr(fd, ttmode) ioctl(fd, TCGETS, (char *)ttmode)
# endif
#endif

#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__)
# define _tcsetattr(fd, ttmode) ioctl(fd, TIOCSETA, (char *)ttmode)
#else
# if defined(_HPUX_SOURCE) || defined(__CYGWIN__)
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

#include <kdebug.h>
#include <kstandarddirs.h>	// locate

// not defined on HP-UX for example
#ifndef CTRL
# define CTRL(x) ((x) & 037)
#endif

#define TTY_GROUP "tty"

///////////////////////
// private functions //
///////////////////////

#ifdef HAVE_UTEMPTER
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
#endif

#define BASE_CHOWN "kgrantpty"



//////////////////
// private data //
//////////////////

struct KPtyPrivate {
   KPtyPrivate() :
     xonXoff(false),
     logged(false),
     masterFd(-1), slaveFd(-1)
   {
     memset(&winSize, 0, sizeof(winSize));
     winSize.ws_row = 24;
     winSize.ws_col = 80;
   }

   bool xonXoff : 1;
   bool utf8    : 1;
   bool logged  : 1;
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

  // Find a master pty that we can open ////////////////////////////////

  // Because not all the pty animals are created equal, they want to
  // be opened by several different methods.

  // We try, as we know them, one by one.

#if defined(HAVE_PTSNAME) && defined(HAVE_GRANTPT)
#ifdef _AIX
  d->masterFd = ::open("/dev/ptc",O_RDWR);
#else
  d->masterFd = ::open("/dev/ptmx",O_RDWR);
#endif
  if (d->masterFd >= 0)
  {
    char *ptsn = ptsname(d->masterFd);
    if (ptsn) {
        grantpt(d->masterFd);
        d->ttyName = ptsn;
        goto gotpty;
    } else {
       ::close(d->masterFd);
       d->masterFd = -1;
    }
  }
#endif

  // Linux device names, FIXME: Trouble on other systems?
  for (const char* s3 = "pqrstuvwxyzabcdefghijklmno"; *s3; s3++)
  {
    for (const char* s4 = "0123456789abcdefghijklmnopqrstuvwxyz"; *s4; s4++)
    {
      ptyName.sprintf("/dev/pty%c%c", *s3, *s4);
      d->ttyName.sprintf("/dev/tty%c%c", *s3, *s4);

      d->masterFd = ::open(ptyName.data(), O_RDWR);
      if (d->masterFd >= 0)
      {
#ifdef __sun
        /* Need to check the process group of the pty.
         * If it exists, then the slave pty is in use,
         * and we need to get another one.
         */
        int pgrp_rtn;
        if (ioctl(d->masterFd, TIOCGPGRP, &pgrp_rtn) == 0 || errno != EIO) {
          ::close(d->masterFd);
          d->masterFd = -1;
          continue;
        }
#endif /* sun */
        if (!access(d->ttyName.data(),R_OK|W_OK)) // checks availability based on permission bits
        {
          if (!geteuid())
          {
            struct group* p = getgrnam(TTY_GROUP);
            if (!p)
              p = getgrnam("wheel");
            gid_t gid = p ? p->gr_gid : getgid ();

            chown(d->ttyName.data(), getuid(), gid);
            chmod(d->ttyName.data(), S_IRUSR|S_IWUSR|S_IWGRP);
          }
          goto gotpty;
        }
        ::close(d->masterFd);
        d->masterFd = -1;
      }
    }
  }

  kdWarning(175) << "Can't open a pseudo teletype" << endl;
  return false;

 gotpty:
  struct stat st;
  if (stat(d->ttyName.data(), &st))
    return false; // this just cannot happen ... *cough*  Yeah right, I just
                  // had it happen when pty #349 was allocated.  I guess
                  // there was some sort of leak?  I only had a few open.
  if (((st.st_uid != getuid()) ||
       (st.st_mode & (S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH))) &&
      !chownpty(true))
  {
    kdWarning(175)
      << "chownpty failed for device " << ptyName << "::" << d->ttyName
      << "\nThis means the communication can be eavesdropped." << endl;
  }

#ifdef BSD
  revoke(d->ttyName.data());
#endif

#ifdef HAVE_UNLOCKPT
  unlockpt(d->masterFd);
#endif

  d->slaveFd = ::open(d->ttyName.data(), O_RDWR | O_NOCTTY);
  if (d->slaveFd < 0)
  {
    kdWarning(175) << "Can't open slave pseudo teletype" << endl;
    ::close(d->masterFd);
    d->masterFd = -1;
    return false;
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

#ifdef IUTF8
  if (!d->utf8)
    ttmode.c_iflag &= ~IUTF8;
  else
    ttmode.c_iflag |= IUTF8;
#endif

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
   logout();
   // don't bother resetting unix98 pty, it will go away after closing master anyway.
   if (memcmp(d->ttyName.data(), "/dev/pts/", 9)) {
      if (!geteuid()) {
         struct stat st;
         if (!stat(d->ttyName.data(), &st)) {
            chown(d->ttyName.data(), 0, st.st_gid == getgid() ? 0 : -1);
            chmod(d->ttyName.data(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	 }
      } else {
         fcntl(d->masterFd, F_SETFD, 0);
         chownpty(false);
      }
   }
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
    if (d->logged)
        return; // print a warning?
    d->logged = true;
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

    // Handle 64-bit time_t properly, where it may be larger
    // than the integral type of ut_time.
    {
        time_t ut_time_temp;
        time(&ut_time_temp);
        l_struct.ut_time=ut_time_temp;
    }

    ::login(&l_struct);
#else
    Q_UNUSED(user);
    Q_UNUSED(remotehost);
#endif
}

void KPty::logout()
{
    if (!d->logged)
        return;
    d->logged = false;
#ifdef HAVE_UTEMPTER
    KProcess_Utmp utmp;
    utmp.cmdFd = d->masterFd;
    utmp << "/usr/sbin/utempter" << "-d" << d->ttyName;
    utmp.start(KProcess::Block);
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

void KPty::setUtf8Mode(bool useUtf8)
{
  d->utf8 = useUtf8;
#ifdef IUTF8
  if (d->masterFd >= 0) {
    // without the '::' some version of HP-UX thinks, this declares
    // the struct in this class, in this method, and fails to find
    // the correct tc[gs]etattr
    struct ::termios ttmode;

    _tcgetattr(d->masterFd, &ttmode);

    if (!useUtf8)
      ttmode.c_iflag &= ~IUTF8;
    else
      ttmode.c_iflag |= IUTF8;

    _tcsetattr(d->masterFd, &ttmode);
  }
#endif
}

const char *KPty::ttyName() const
{
    return d->ttyName.data();
}

int KPty::masterFd() const
{
    return d->masterFd;
}

int KPty::slaveFd() const
{
    return d->slaveFd;
}

// private
bool KPty::chownpty(bool grant)
{
  KProcess proc;
  proc << locate("exe", BASE_CHOWN) << (grant?"--grant":"--revoke") << QString::number(d->masterFd);
  return proc.start(KProcess::Block) && proc.normalExit() && !proc.exitStatus();
}

