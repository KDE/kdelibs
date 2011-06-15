/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "dbusbackend.h"

#include <QtDBus/QDBusConnection>

namespace KSecretsService {

#define SERVICE_NAME "org.freedesktop.secrets"

bool KSecretsService::DBusSession::startDaemon()
{
    // launch the daemon if it's not yet started
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1( SERVICE_NAME )))
    {
        QString error;
        // FIXME: find out why this is not working
        int ret = KToolInvocation::startServiceByDesktopPath("ksecretserviced.desktop", QStringList(), &error);
        QVERIFY2( ret == 0, qPrintable( error ) );
        
        QVERIFY2( QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1( SERVICE_NAME )),
                 "Secret Service was started but the service is not registered on the DBus");
    }

}

}

