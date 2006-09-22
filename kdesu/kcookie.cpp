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

#include <stdlib.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>
#include <qprocess.h>

#include <kdebug.h>
#include "kcookie.h"


KCookie::KCookie()
{
#ifdef Q_WS_X11
    getXCookie();
#endif
}

void KCookie::getXCookie()
{
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
    if (disp.startsWith("localhost:"))
       disp.remove(0, 9);

    QProcess proc;
    proc.start("xauth", QStringList() << "list" << disp);
    if (!proc.waitForStarted())
    {
	kError(900) << k_lineinfo << "Could not run xauth.\n";
	return;
    }
    QByteArray output = proc.readLine().simplified();
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
