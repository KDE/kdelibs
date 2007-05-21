/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This file contains code from TEShell.C of the KDE konsole.
 * Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * pty.cpp: Access to PTY's on different systems a la UNIX98.
 */

#include "kdesu_pty.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* Needed for getpt, ptsname in glibc 2.1.x systems */
#endif

#include <config.h>
#include <config-pty.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>

#ifdef HAVE__PTY_H
	#include <pty.h>
#endif

#ifdef HAVE_LIBUTIL_H
	#include <libutil.h>
#elif defined(HAVE_UTIL_H)
	#include <util.h>
#endif

#include <QtCore/QBool>
#include <QtCore/QFile>

#include <kdebug.h>
#include <kstandarddirs.h>


// stdlib.h is meant to declare the prototypes but doesn't :(
#ifndef __THROW
#define __THROW
#endif

#ifdef HAVE_GRANTPT
extern "C" int grantpt(int fd) __THROW;
#endif

#ifdef HAVE_PTSNAME
extern "C" char * ptsname(int fd) __THROW;
#endif

#ifdef HAVE_UNLOCKPT
extern "C" int unlockpt(int fd) __THROW;
#endif

#ifdef HAVE__GETPTY
extern "C" char *_getpty(int *, int, mode_t, int);
#endif

class PTY::PTYPrivate
{
public:
    PTYPrivate() : ptyfd(-1) {}
    int ptyfd;
    QString ptyname;
    QString ttyname;
};

PTY::PTY()
    : d( new PTYPrivate )
{
}

PTY::~PTY()
{
    if (d->ptyfd >= 0)
        close(d->ptyfd);
    delete d;
}


// Opens a pty master and returns its filedescriptor.

int PTY::getpt()
{

#if defined(HAVE_GETPT) && defined(HAVE_PTSNAME)

    // 1: UNIX98: preferred way
    d->ptyfd = ::getpt();
    d->ttyname = ::ptsname(d->ptyfd);
    return d->ptyfd;

#elif defined(HAVE_OPENPTY)
    // 2: BSD interface
    // More preferred than the linux hacks
    char name[30];
    int master_fd, slave_fd;
    if (openpty(&master_fd, &slave_fd, name, 0L, 0L) != -1)  {
	d->ttyname = name;
	name[5]='p';
	d->ptyname = name;
        close(slave_fd); // We don't need this yet // Yes, we do.
	d->ptyfd = master_fd;
	return d->ptyfd;
    }
    d->ptyfd = -1;
    kDebug(900) << k_lineinfo << "Opening pty failed.\n";
    return -1;

#elif defined(HAVE__GETPTY)
    // 3: Irix interface
    int master_fd;
    d->ttyname = _getpty(&master_fd,O_RDWR,0600,0);
    if (d->ttyname)
	d->ptyfd = master_fd;
    else{
	d->ptyfd = -1;
	kDebug(900) << k_lineinfo << "Opening pty failed.error" << errno << '\n';
    }
    return d->ptyfd;

#else

    // 4: Open terminal device directly
    // 4.1: Try /dev/ptmx first. (Linux w/ Unix98 PTYs, Solaris)

    d->ptyfd = open("/dev/ptmx", O_RDWR);
    if (d->ptyfd >= 0) {
	d->ptyname = "/dev/ptmx";
#ifdef HAVE_PTSNAME
	d->ttyname = ::ptsname(d->ptyfd);
	return d->ptyfd;
#elif defined (TIOCGPTN)
	int ptyno;
	if (ioctl(d->ptyfd, TIOCGPTN, &ptyno) == 0) {
	    d->ttyname.sprintf("/dev/pts/%d", ptyno);
	    return d->ptyfd;
	}
#endif
	close(d->ptyfd);
    }

    // 4.2: Try /dev/pty[p-e][0-f] (Linux w/o UNIX98 PTY's)

    for (const char *c1 = "pqrstuvwxyzabcde"; *c1 != '\0'; c1++)
    {
	for (const char *c2 = "0123456789abcdef"; *c2 != '\0'; c2++)
	{
	    d->ptyname.sprintf("/dev/pty%c%c", *c1, *c2);
	    d->ttyname.sprintf("/dev/tty%c%c", *c1, *c2);
	    if (access(QFile::encodeName(d->ptyname), F_OK) < 0)
		goto linux_out;
	    d->ptyfd = open(QFile::encodeName(d->ptyname), O_RDWR);
	    if (d->ptyfd >= 0)
		return d->ptyfd;
	}
    }
linux_out:

    // 4.3: Try /dev/pty%d (SCO, Unixware)

    for (int i=0; i<256; i++)
    {
	d->ptyname.sprintf("/dev/ptyp%d", i);
	d->ttyname.sprintf("/dev/ttyp%d", i);
	if (access(QFile::encodeName(d->ptyname), F_OK) < 0)
	    break;
	d->ptyfd = open(QFile::encodeName(d->ptyname), O_RDWR);
	if (d->ptyfd >= 0)
	    return d->ptyfd;
    }


    // Other systems ??
    d->ptyfd = -1;
    kDebug(900) << k_lineinfo << "Unknown system or all methods failed.\n";
    return -1;

#endif // HAVE_GETPT && HAVE_PTSNAME

}


int PTY::grantpt()
{
    if (d->ptyfd < 0)
	return -1;

#ifdef HAVE_GRANTPT

    return ::grantpt(d->ptyfd);

#elif defined(HAVE_OPENPTY)

    // the BSD openpty() interface chowns the devices properly for us,
    // no need to do this at all
    return 0;

#else

    // konsole_grantpty only does /dev/pty??
    if (!d->ptyname.startsWith(QLatin1String("/dev/pty")))
	return 0;

    // Use konsole_grantpty:
    if (KStandardDirs::findExe("konsole_grantpty").isEmpty())
    {
	kError(900) << k_lineinfo << "konsole_grantpty not found.\n";
	return -1;
    }

    // As defined in konsole_grantpty.c
    const int pty_fileno = 3;

    pid_t pid;
    if ((pid = fork()) == -1)
    {
	kError(900) << k_lineinfo << "fork(): " << perror << "\n";
	return -1;
    }

    if (pid)
    {
	// Parent: wait for child
	int ret;
	waitpid(pid, &ret, 0);
	if (WIFEXITED(ret) && !WEXITSTATUS(ret))
	    return 0;
	kError(900) << k_lineinfo << "konsole_grantpty returned with error: "
		     << WEXITSTATUS(ret) << "\n";
	return -1;
    } else
    {
	// Child: exec konsole_grantpty
	if (d->ptyfd != pty_fileno && dup2(d->ptyfd, pty_fileno) < 0)
	    _exit(1);
	execlp("konsole_grantpty", "konsole_grantpty", "--grant", (void *)0);
	kError(900) << k_lineinfo << "exec(): " << perror << "\n";
	_exit(1);
    }

    // shut up, gcc
    return 0;

#endif // HAVE_GRANTPT
}


/**
 * Unlock the pty. This allows connections on the slave side.
 */

int PTY::unlockpt()
{
    if (d->ptyfd < 0)
	return -1;

#ifdef HAVE_UNLOCKPT

    // (Linux w/ glibc 2.1, Solaris, ...)

    return ::unlockpt(d->ptyfd);

#elif defined(TIOCSPTLCK)

    // Unlock pty (Linux w/ UNIX98 PTY's & glibc 2.0)
    int flag = 0;
    return ioctl(d->ptyfd, TIOCSPTLCK, &flag);

#else

    // Other systems (Linux w/o UNIX98 PTY's, ...)
    return 0;

#endif

}


/**
 * Return the slave side name.
 */

QByteArray PTY::ptsname()
{
    if (d->ptyfd < 0)
	return 0;

    return QFile::encodeName(d->ttyname);
}

