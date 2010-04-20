/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "opticaldrive.h"
#include "opticaldrive_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/opticaldrive.h>

Solid::OpticalDrive::OpticalDrive(QObject *backendObject)
    : StorageDrive(*new OpticalDrivePrivate(), backendObject)
{
    connect(backendObject, SIGNAL(ejectPressed(const QString &)),
            this, SIGNAL(ejectPressed(const QString &)));
    connect(backendObject, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
            this, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)));
    connect(backendObject, SIGNAL(ejectRequested(const QString &)),
            this, SIGNAL(ejectRequested(const QString &)));
}

Solid::OpticalDrive::~OpticalDrive()
{

}

Solid::OpticalDrive::MediumTypes Solid::OpticalDrive::supportedMedia() const
{
    Q_D(const OpticalDrive);
    return_SOLID_CALL(Ifaces::OpticalDrive *, d->backendObject(), MediumTypes(), supportedMedia());
}

int Solid::OpticalDrive::readSpeed() const
{
    Q_D(const OpticalDrive);
    return_SOLID_CALL(Ifaces::OpticalDrive *, d->backendObject(), 0, readSpeed());
}

int Solid::OpticalDrive::writeSpeed() const
{
    Q_D(const OpticalDrive);
    return_SOLID_CALL(Ifaces::OpticalDrive *, d->backendObject(), 0, writeSpeed());
}

QList<int> Solid::OpticalDrive::writeSpeeds() const
{
    Q_D(const OpticalDrive);
    return_SOLID_CALL(Ifaces::OpticalDrive *, d->backendObject(), QList<int>(), writeSpeeds());
}

bool Solid::OpticalDrive::eject()
{
    Q_D(OpticalDrive);
    return_SOLID_CALL(Ifaces::OpticalDrive *, d->backendObject(), false, eject());
}

#include "opticaldrive.moc"
