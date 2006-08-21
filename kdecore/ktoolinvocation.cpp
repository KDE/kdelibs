/* This file is part of the KDE libraries
    Copyright (C) 2005 Brad Hards
    Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

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

#include "ktoolinvocation.h"
#include "klauncher_iface.h"
#include "kdebug.h"
#include "kstaticdeleter.h"

#include <QMutex>
#include <QMutexLocker>

Q_GLOBAL_STATIC_WITH_ARGS(QMutex,mutex,(QMutex::Recursive))

KToolInvocation* KToolInvocation::s_self = 0L;

KToolInvocation::KToolInvocation() : QObject(0)
{
}

KStaticDeleter<KToolInvocation> ktoolinvocation_sd;
KToolInvocation *KToolInvocation::self()
{
    QMutexLocker locker(mutex());
    if (s_self==0) {
        ktoolinvocation_sd.setObject( s_self, new KToolInvocation );
    }
    return s_self;
}

KToolInvocation::~KToolInvocation()
{
    QMutexLocker locker(mutex());
    s_self=0;
}

Q_GLOBAL_STATIC_WITH_ARGS(org::kde::KLauncher, klauncherIface,
                          ("org.kde.klauncher", "/KLauncher", QDBusConnection::sessionBus()))

org::kde::KLauncher *KToolInvocation::klauncher()
{
    if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.klauncher" ) )
    {
        kDebug() << "klauncher not running... launching kdeinit" << endl;
        KToolInvocation::startKdeinit();
    }
    return ::klauncherIface();
}

// this seems a bit silly, but equally seems to be required.
#include "ktoolinvocation.moc"
