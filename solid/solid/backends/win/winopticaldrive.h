/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

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
#ifndef WINOPTICALDRIVE_H
#define WINOPTICALDRIVE_H

#include <solid/ifaces/opticaldrive.h>

#include "winstoragedrive.h"

#include <ntddcdrm.h>
#include <ntddmmc.h>


namespace Solid
{
namespace Backends
{
namespace Win
{
class WinOpticalDrive : public WinStorageDrive, virtual public Solid::Ifaces::OpticalDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDrive)
public:
    WinOpticalDrive(WinDevice *device);
    virtual ~WinOpticalDrive();

    virtual Solid::OpticalDrive::MediumTypes supportedMedia() const;

    virtual int readSpeed() const;

    virtual int writeSpeed() const;

    virtual QList<int> writeSpeeds() const;

    virtual bool eject();

signals:
    void ejectPressed(const QString &udi);

    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

private:
    Solid::OpticalDrive::MediumTypes m_supportedTypes;

};

class MediaProfiles//TODO: cleanup
{
public:
    MediaProfiles();
    ulong profile;
    Solid::OpticalDrive::MediumTypes type;
    QString name;
    bool active;

    static QMap<ulong, MediaProfiles> profiles(const QString &drive);

private:

    MediaProfiles(ulong profile,Solid::OpticalDrive::MediumTypes type,QString name = "");

    MediaProfiles(FEATURE_DATA_PROFILE_LIST_EX* feature);
    bool isNull();

    static const MediaProfiles getProfile(ulong id);
} ;
}
}
}

#endif // WINOPTICALDRIVE_H
