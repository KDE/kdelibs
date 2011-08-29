/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
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
