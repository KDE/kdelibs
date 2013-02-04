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
#ifndef WINCPU_H
#define WINCPU_H

#include <solid/ifaces/processor.h>

#include "wininterface.h"
namespace Solid
{
namespace Backends
{
namespace Win
{

class WinCpu :public WinInterface, public Solid::Ifaces::Processor
{
    friend class WinDevice;
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Processor)
public:
    WinCpu(WinDevice *device);
    ~WinCpu();

    virtual int number() const;

    virtual int maxSpeed() const;

    virtual bool canChangeFrequency() const;

    virtual Solid::Processor::InstructionSets instructionSets() const;

    static QStringList getUdis();



private:
    int m_number;

    class ProcessorInfo
    {
    public:
        int lgicalId;
        int id;
        int speed;
        QString vendor;
        QString name;
        QString produuct;

    };

    static DWORD countSetBits(ULONG_PTR bitMask);
    static const QMap<int, WinCpu::ProcessorInfo> &updateCache();
};
}
}
}

#endif // WINCPU_H
