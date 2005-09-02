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

#ifndef HALDEVICE_H
#define HALDEVICE_H

#include <QObject>
#include <QVariant>

class HalDevicePrivate;

class HalDevice : public QObject
{
    Q_OBJECT

public:
    HalDevice(const QString &udi);
    virtual ~HalDevice();

    bool setProperty( const QString &key, const QVariant &value );
    bool setProperty( const QString &key, const QString &value );
    bool setProperty( const QString &key, qint32 value );
    bool setProperty( const QString &key, bool value );
    bool setProperty( const QString &key, double value );

    QVariant property( const QString &key ) const ;
    QString stringProperty( const QString &key ) const;
    qint32 intProperty( const QString &key ) const;
    bool boolProperty( const QString &key ) const;
    double doubleProperty( const QString &key ) const;

    QMap<QString, QVariant> allProperties() const;

    bool removeProperty( const QString &key );
    bool propertyExists( const QString &key ) const;

    bool addCapability( const QString &capability );
    bool queryCapability( const QString &capability ) const;

    bool lock(const QString &reason);
    bool unlock();

signals:
    void propertyModified( const QString &key, bool added, bool removed );
    // FIXME How am I supposed to implement this???
    //void slotCondition( const QString &condition, ... );

private slots:
    void slotPropertyModified( const QString &key, bool added, bool removed );
    // FIXME How am I supposed to implement this???
    //void slotCondition( const QString &condition, ... );

private:
    HalDevicePrivate *d;
};

#endif
