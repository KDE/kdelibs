/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * This file contains code from TEShell.C of the KDE konsole. 
 * Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de> 
 *
 * pty.cpp: Access to PTY's on different systems a la UNIX98.
 */

#define _GNU_SOURCE   /* Needed for getpt, ptsname in glibc 2.1.x systems */
#define _XOPEN_SOURCE /* Needed for grantpt, unlockpt in glibc 2.1.x      */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <qglobal.h>
#include <qcstring.h>

#include <kdebug.h>
#include <kstddirs.h>
#include "pty.h"

// stdlib.h is meant to declare the prototypes but doesn't :(
#ifdef HAVE_GRANTPT
extern "C" int grantpt(int fd);
#endif

#ifdef HAVE_PTSNAME
extern "C" char * ptsname(int fd);
#endif

#ifdef HAVE_UNLOCKPT
extern "C" int unlockpt(int fd);
#endif

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID "PTY"
#endif

PTY::PTY()
{
    ptyfd = -1;
}

PTY::~PTY()
{
    if (ptyfd >= 0)
	close(ptyfd);
}


// Opens a pty master and returns its filedescriptor.

int PTY::getpt()
{
#if defined(HAVE_GETPT) && defined(HAVE_PTSNAME)

    ptyfd = ::getpt();
    ttyname = ::ptsname(ptyfd);
    return ptyfd;

#else

    // Try /dev/ptmx first. (Linux w/ Unix98 PTYs, Solaris)

    ptyfd = open("/dev/ptmx", O_RDWR);
    if (ptyfd >= 0) {
	ptyname = "/dev/ptmx";
#ifdef HAVE_PTSNAME
	ttyname = ::ptsname(ptyfd);
	return ptyfd;
#elif defined (TIOCGPTN)
	int ptyno;
	if (ioctl(ptyfd, TIOCGPTN, &ptyno) == 0) {
	    ttyname.sprintf("/dev/pts/%d", ptyno);
	    return ptyfd;
	}
#endif
	close(ptyfd);
    }

    // Try /dev/pty[p-e][0-f] (Linux w/o UNIX98 PTY's)

    for (const char *c1 = "pqrstuvwxyzabcde"; *c1 != '\0'; c1++) {
	for (const char *c2 = "0123456789abcdef"; *c2 != '\0'; c2++) {
	    ptyname.sprintf("/dev/pty%c%c", *c1, *c2);
	    ttyname.sprintf("/dev/tty%c%c", *c1, *c2);
	    if (access(ptyname, F_OK) < 0)
		goto linux_out;
	    ptyfd = open(ptyname, O_RDWR);
	    if (ptyfd >= 0)
		return ptyfd;
	}
    }
linux_out:
	
    // Try /dev/pty%d (SCO, Unixware)

    for (int i=0; i<256; i++) {
	ptyname.sprintf("/dev/ptyp%d", i);
	ttyname.sprintf("/dev/ttyp%d", i);
	if (access(ptyname, F_OK) < 0)
	    break;
	ptyfd = open(ptyname, O_RDWR);
	if (ptyfd >= 0)
	    return ptyfd;
    }

    
    // Other systems ??

    ptyfd = -1;
    kDebugError("%s: Unknown system or all methods failed", ID);
    return -1;

#endif // HAVE_GETPT

}


int PTY::grantpt()
{
    if (ptyfd < 0)
	return -1;

#ifdef HAVE_GRANTPT

    return ::grantpt(ptyfd);

#else

    // konsole_grantpty doesn't do this
    if (ptyname == "/dev/ptmx")
	return 0;

    // Use konsole_grantpty:
    if (KStandardDirs::findExe("konsole_grantpty").isEmpty()) {
	kDebugError("%s: konsole_grantpty not found", ID);
	return -1;
    }

    // As defined in konsole_grantpty.c
    const int pty_fileno = 3;

    pid_t pid;
    if ((pid = fork()) == -1) {
	kDebugError("%s: fork(): %m", ID);
	return -1;
    }

    if (pid) {
	// Parent: wait for child
	int ret;
	waitpid(pid, &ret, 0);
    	if (WIFEXITED(ret) && !WEXITSTATUS(ret))
	    return 0;
	kDebugError("konsole_grantpty returned with error: %d", 
		WEXITSTATUS(ret));
	return -1;
    } else {
	// Child: exec konsole_grantpty
	if (ptyfd != pty_fileno && dup2(ptyfd, pty_fileno) < 0) 
	    _exit(1);
	execlp("konsole_grantpty", "konsole_grantpty", "--grant", NULL);
	kDebugError("%s: exec(): %m", ID);
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
    if (ptyfd < 0)
	return -1;

#ifdef HAVE_UNLOCKPT

    // (Linux w/ glibc 2.1, Solaris, ...)

    return ::unlockpt(ptyfd);

#elif defined(TIOCSPTLCK)

    // Unlock pty (Linux w/ UNIX98 PTY's & glibc 2.0)
    int flag = 0;
    return ioctl(ptyfd, TIOCSPTLCK, &flag);

#else

    // Other systems (Linux w/o UNIX98 PTY's, ...)
    return 0;

#endif

}
    

/**
 * Return the slave side name.
 */

QCString PTY::ptsname()
{
    if (ptyfd < 0)
	return QCString();

    return ttyname;
}

