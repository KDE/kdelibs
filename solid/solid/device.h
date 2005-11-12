/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KDEHW_DEVICE_H
#define KDEHW_DEVICE_H

#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>

#include "ifaces/device.h"

namespace KDEHW
{
    class DeviceManager;

    class Device : public Ifaces::Device
    {
        Q_OBJECT
    public:
        Device( QObject *parent = 0 );
        Device( const Device &device );
        Device( Ifaces::Device *data );
        virtual ~Device();

        Device &operator=( const Device &device );

        bool isValid();

        virtual QString udi() const;

        virtual QVariant property( const QString &key ) const;
        virtual QMap<QString, QVariant> allProperties() const;
        virtual bool propertyExists( const QString &key ) const;
        virtual bool queryCapability( const QString &capability ) const;

        virtual bool lock(const QString &reason);
        virtual bool unlock();

    signals:
        void propertyChanged( const QString &key, PropertyChange change );
        void conditionRaised( const QString &condition, const QString &reason );

    private slots:
        void slotPropertyChanged( const QString &key, PropertyChange change );
        void slotConditionRaised( const QString &condition, const QString &reason );
        void slotDestroyed( QObject *object );

    private:
        class Private;
        Private *d;
    };

    typedef QList<Device> DeviceList;
}

#endif
