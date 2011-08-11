/* This file is part of libkdbus

   Copyright (c) 2011 David Faure <faure@kde.org>
   Copyright (c) 2011 Kevin Ottens <ervin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDBUSSERVICE_H
#define KDBUSSERVICE_H

#include <QObject>

#include <kdbus_export.h>

class KDBusServicePrivate;

/**
 *
 *
 * Important: in order to avoid a race, the application should try to export its
 * objects to DBus before instanciating KDBusService.
 * Otherwise the application appears on the bus before its objects are accessible
 * via DBus, which could be a problem for other apps or scripts which start the
 * application in order to talk DBus to it immediately.
 */
class KDBUS_EXPORT KDBusService : public QObject
{
    Q_OBJECT
    Q_ENUMS(StartupOption)
    Q_FLAGS(StartupOptions)

public:
    enum StartupOption {
        Unique = 1,
        Multiple = 2,
        NoExitOnFailure = 4
    };

    Q_DECLARE_FLAGS(StartupOptions, StartupOption)

    explicit KDBusService(StartupOptions options = Multiple, QObject *parent = 0);
    ~KDBusService();

    bool isRegistered() const;
    QString errorMessage() const;

private:
    KDBusServicePrivate * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDBusService::StartupOptions)

#endif /* KDBUSSERVICE_H */

