/*  This file is part of the KDE project
    Copyright (C) 2005,2006 Kevin Ottens <ervin@kde.org>

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

#ifndef HALMANAGER_H
#define HALMANAGER_H

#include <solid/ifaces/devicemanager.h>
#include <QVariant>
#include <QStringList>
#include <kdemacros.h>

#include "deviceinterface.h"

class HalManagerPrivate;

class KDE_EXPORT HalManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    HalManager(QObject *parent, const QStringList &args);
    virtual ~HalManager();

    virtual QStringList allDevices();
    virtual bool deviceExists(const QString &udi);

    virtual QStringList devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type);

    virtual QObject *createDevice(const QString &udi);


private Q_SLOTS:
    void slotDeviceAdded(const QString &udi);
    void slotDeviceRemoved(const QString &udi);

private:
    QStringList findDeviceStringMatch(const QString &key, const QString &value);
    QStringList findDeviceByDeviceInterface(const Solid::DeviceInterface::Type &type);

    HalManagerPrivate *d;
};

#endif
