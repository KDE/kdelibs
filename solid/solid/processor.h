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

#ifndef KDEHW_PROCESSOR_H
#define KDEHW_PROCESSOR_H

#include <kdehw/capability.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class Processor;
    }

    class Processor : public Capability
    {
        Q_OBJECT
    public:
        Processor( Ifaces::Processor *iface, QObject *parent = 0 );
        virtual ~Processor();

        static Type type() { return Capability::Processor; }

        int number() const;
        long maxSpeed() const;
        bool canThrottle() const;

    private:
        class Private;
        Private *d;
    };
}

#endif
