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

#ifndef FAKEPROCESSOR_H
#define FAKEPROCESSOR_H

#include <solid/ifaces/abstractcapability.h>
#include <solid/ifaces/processor.h>

class FakeProcessor : public Solid::Ifaces::AbstractCapability,
                      virtual public Solid::Ifaces::Processor
{
    Q_OBJECT
    Q_PROPERTY( int number READ number )
    Q_PROPERTY( qulonglong maxSpeed READ maxSpeed )
    Q_PROPERTY( bool canThrottle READ canThrottle )

public:
    FakeProcessor();
    virtual ~FakeProcessor();

    virtual QObject *qobject() { return this; }
    virtual const QObject *qobject() const { return this; }

    virtual int number() const;
    virtual qulonglong maxSpeed() const;
    virtual bool canThrottle() const;
};

#endif
