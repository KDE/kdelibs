/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kcookie.cpp: KDE authentication cookies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>
#include <qfile.h>

#include <kdebug.h>
#include <kprocess.h>
#include "kcookie.h"


KCookie::KCookie()
{
#ifdef Q_WS_X11
    getXCookie();
#endif
}

void KCookie::blockSigChild()
{
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sset, 0L);
}

void KCookie::unblockSigChild()
{
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sset, 0L);
}

void KCookie::getXCookie()
{
    char buf[1024];
    FILE *f;

#ifdef Q_WS_X11
    m_Display = getenv("DISPLAY");
#else
    m_Display = getenv("QWS_DISPLAY");
#endif
    if (m_Display.isEmpty())
    {
	kError(900) << k_lineinfo << "$DISPLAY is not set.\n";
	return;
    }
#ifdef Q_WS_X11 // No need to mess with X Auth stuff
    QByteArray disp = m_Display;
    if (!memcmp(disp.data(), "localhost:", 10))
       disp.remove(0, 9);

    QString cmd = "xauth list "+KProcess::quote(disp);
    blockSigChild(); // pclose uses waitpid()
    if (!(f = popen(QFile::encodeName(cmd), "r")))
    {
	kError(900) << k_lineinfo << "popen(): " << perror << "\n";
	unblockSigChild();
	return;
    }
    QByteArray output = fgets(buf, 1024, f);
    if (pclose(f) < 0)
    {
	kError(900) << k_lineinfo << "Could not run xauth.\n";
	unblockSigChild();
	return;
    }
    unblockSigChild();
    output = output.simplified();
    if (output.isEmpty())
    {
       kWarning(900) << "No X authentication info set for display " <<
       m_Display << endl; return;
    }
    QList<QByteArray> lst = output.split(' ');
    if (lst.count() != 3)
    {
	kError(900) << k_lineinfo << "parse error.\n";
	return;
    }
    m_DisplayAuth = (lst[1] + ' ' + lst[2]);
#endif
}
