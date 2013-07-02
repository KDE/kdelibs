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

#ifndef WINBLOCK_H
#define WINBLOCK_H

#include <solid/ifaces/block.h>
#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{

class WinBlock : public WinInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)
public:
    WinBlock(WinDevice *device);
    ~WinBlock();

    virtual int deviceMajor() const;

    virtual int deviceMinor() const;

    virtual QString device() const;

    static QSet<QString> getUdis();

    static QString driveLetterFromUdi(const QString &udi);
    static QString udiFromDriveLetter(const QString &drive);
    static QString resolveVirtualDrive(const QString &drive);
    static QSet<QString> updateUdiFromBitMask(const DWORD unitmask);
    static QSet<QString> getFromBitMask(const DWORD unitmask);
private:
    static QMap<QString,QString> m_driveLetters;
    static QMap<QString,QSet<QString> > m_driveUDIS;
    static QMap<QString,QString> m_virtualDrives;

    int m_major;
    int m_minor;

    static QStringList drivesFromMask(const DWORD unitmask);
};

}
}
}

#endif // WINBLOCK_H
