/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_WMI_CDROM_H
#define SOLID_BACKENDS_WMI_CDROM_H

#include <solid/ifaces/opticaldrive.h>
#include "wmistorage.h"

#include <QtCore/QProcess>

namespace Solid
{
namespace Backends
{
namespace Wmi
{
class Cdrom : public Storage, virtual public Solid::Ifaces::OpticalDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDrive)

public:
    Cdrom(WmiDevice *device);
    virtual ~Cdrom();

    virtual Solid::OpticalDrive::MediumTypes supportedMedia() const;
    virtual int readSpeed() const;
    virtual int writeSpeed() const;
    virtual QList<int> writeSpeeds() const;
    virtual bool eject();

Q_SIGNALS:
    void ejectPressed(const QString &udi);
    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

private Q_SLOTS:
    void slotCondition(const QString &name, const QString &reason);
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool callWmiDriveEject();

    bool m_ejectInProgress;
    QProcess *m_process;
};
}
}
}

#endif // SOLID_BACKENDS_WMI_CDROM_H
