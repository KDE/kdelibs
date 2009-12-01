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

#include "kcookie.h"

#include <stdlib.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QBool>
#include <QtCore/Q_PID>

#include <kdebug.h>

extern int kdesuDebugArea();

namespace KDESu {
namespace KDESuPrivate {

class KCookie::KCookiePrivate
{
public:
    QByteArray m_Display;
#ifdef Q_WS_X11
    QByteArray m_DisplayAuth;
#endif
};



KCookie::KCookie()
    : d( new KCookiePrivate )
{
#ifdef Q_WS_X11
    getXCookie();
#endif
}

KCookie::~KCookie()
{
    delete d;
}

QByteArray KCookie::display() const
{
    return d->m_Display;
}

#ifdef Q_WS_X11
QByteArray KCookie::displayAuth() const
{
    return d->m_DisplayAuth;
}
#endif

void KCookie::getXCookie()
{
#ifdef Q_WS_X11
    d->m_Display = qgetenv("DISPLAY");
#else
    d->m_Display = qgetenv("QWS_DISPLAY");
#endif
    if (d->m_Display.isEmpty())
    {
	kError(kdesuDebugArea()) << k_lineinfo << "$DISPLAY is not set.";
	return;
    }
#ifdef Q_WS_X11 // No need to mess with X Auth stuff
    QByteArray disp = d->m_Display;
    if (disp.startsWith("localhost:")) // krazy:exclude=strings
       disp.remove(0, 9);

    QProcess proc;
    proc.start("xauth", QStringList() << "list" << disp);
    if (!proc.waitForStarted())
    {
	kError(kdesuDebugArea()) << k_lineinfo << "Could not run xauth.";
	return;
    }
    proc.waitForReadyRead(100);
    QByteArray output = proc.readLine().simplified();
    if (output.isEmpty())
    {
       kWarning(kdesuDebugArea()) << "No X authentication info set for display" << d->m_Display;
       return;
    }
    QList<QByteArray> lst = output.split(' ');
    if (lst.count() != 3)
    {
	kError(kdesuDebugArea()) << k_lineinfo << "parse error.";
	return;
    }
    d->m_DisplayAuth = (lst[1] + ' ' + lst[2]);
    proc.waitForFinished(100); // give QProcess a chance to clean up gracefully
#endif
}

}}
