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


    class MediaProfiles//TODO: cleanup
    {
    public:
        MediaProfiles(long profile,Solid::OpticalDrive::MediumTypes type) :
            profile(profile),
            type(type)
        {
            profileMap.insert(profile,type);
        }
        ulong profile;
        Solid::OpticalDrive::MediumTypes type;
        static QMap<ulong,Solid::OpticalDrive::MediumTypes> profileMap;
    } ;

signals:
    void ejectPressed(const QString &udi);

    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

private:
    Solid::OpticalDrive::MediumTypes m_supportedTypes;



};
}
}
}

#endif // WINOPTICALDRIVE_H