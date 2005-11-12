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

#ifndef KDEHW_IFACES_DEVICE_H
#define KDEHW_IFACES_DEVICE_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QMap>

namespace KDEHW
{
    enum PropertyChange { PropertyModified, PropertyAdded, PropertyRemoved };

namespace Ifaces
{
    class Device : public QObject
    {
        Q_OBJECT
    public:
        Device( QObject *parent = 0 );
        virtual ~Device();

        virtual QString udi() const = 0;

        virtual bool setProperty( const QString &key, const QVariant &value );
        virtual bool setProperty( const QString &key, const QString &value );
        virtual bool setProperty( const QString &key, int value );
        virtual bool setProperty( const QString &key, bool value );
        virtual bool setProperty( const QString &key, double value );

        virtual QVariant property( const QString &key ) const = 0;
        virtual QString stringProperty( const QString &key ) const;
        virtual int intProperty( const QString &key ) const;
        virtual bool boolProperty( const QString &key ) const;
        virtual double doubleProperty( const QString &key ) const;

        virtual QMap<QString, QVariant> allProperties() const = 0;

        virtual bool removeProperty( const QString &key );
        virtual bool propertyExists( const QString &key ) const = 0;

        virtual bool addCapability( const QString &capability );
        virtual bool queryCapability( const QString &capability ) const = 0;

        virtual bool lock(const QString &reason);
        virtual bool unlock();

    signals:
        void propertyChanged( const QString &key, PropertyChange change );
        void conditionRaised( const QString &condition, const QString &reason );
    };
}
}

#endif
