/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
 * Copyright (C) 2010 David Faure <faure@kde.org>
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

#include "dbusconnectionpool.h"
#include <QThreadStorage>

namespace {
QAtomicInt s_connectionCounter;

class DBusConnectionPoolPrivate
{
public:
    DBusConnectionPoolPrivate()
        : m_connection( QDBusConnection::connectToBus(
                            QDBusConnection::SessionBus,
                            QString::fromLatin1("NepomukQueryServiceConnection%1").arg(newNumber()) ) )
    {
    }
    ~DBusConnectionPoolPrivate() {
        QDBusConnection::disconnectFromBus( m_connection.name() );
    }

    QDBusConnection connection() const { return m_connection; }

private:
    static int newNumber() {
        return s_connectionCounter.fetchAndAddAcquire(1);
    }
    QDBusConnection m_connection;
};
}

QThreadStorage<DBusConnectionPoolPrivate *> s_perThreadConnection;

QDBusConnection DBusConnectionPool::threadConnection()
{
    if (!s_perThreadConnection.hasLocalData()) {
        s_perThreadConnection.setLocalData(new DBusConnectionPoolPrivate);
    }
    return s_perThreadConnection.localData()->connection();
}
