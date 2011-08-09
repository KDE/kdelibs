/* This file is part of libkdbus

   Copyright (c) 2011 David Faure <faure@kde.org>
   Copyright (c) 2011 Kevin Ottens <ervin@kde.org>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDBUSSERVICE_H
#define KDBUSSERVICE_H

#include <QObject>

#include <kdbus_export.h>

class KDBusServicePrivate;

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

