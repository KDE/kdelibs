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

#ifndef HALMANAGER_H
#define HALMANAGER_H

#include <QObject>
#include <QVariant>
#include <QStringList>

class HalManagerPrivate;

class HalManager : public QObject
{
    Q_OBJECT

public:
    HalManager();
    virtual ~HalManager();

    QStringList allDevices();
    bool deviceExists( const QString &udi );
    QStringList findDeviceStringMatch( const QString &key, const QString &value );
    QStringList findDeviceByCapability( const QString &capability );

signals:
    void deviceAdded( const QString &udi );
    void deviceRemoved( const QString &udi );
    void newCapability( const QString &udi, const QString &capability );

private slots:
    void slotDeviceAdded( const QString &udi );
    void slotDeviceRemoved( const QString &udi );
    void slotNewCapability( const QString &udi, const QString &capability );

private:
    QList<QVariant> callHalMethod( const QString &methodName,
                                   const QList<QVariant> &parameters = QList<QVariant>() );

    HalManagerPrivate *d;
};

#endif
