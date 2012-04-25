/*
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>
    Copyright 2012 Ivan Cukic <ivan.cukic@kde.org>

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

#include "fusewatcher.h"
#include <solid/backends/fuse/fusedevice.h>
#include <solid/backends/fuse/fusehandling.h>
#include <solid/backends/fuse/fuseservice.h>
#include <solid/backends/fuse/fusestorageaccess.h>
#include <QtCore/QStringList>

#include <QTimer>
#include <QDebug>

#define MTAB "/etc/mtab"

using namespace Solid::Backends::Fuse;

FuseStorageAccess::FuseStorageAccess(Solid::Backends::Fuse::FuseDevice *device) :
    QObject(device),
    m_fuseDevice(device)
{
    QStringList currentMountPoints = FuseHandling::currentMountPoints(device->device());
    if (currentMountPoints.isEmpty()) {
        m_filePath = FuseHandling::mountPoints(device->device()).first();
        m_isAccessible = false;
    } else {
        m_filePath = currentMountPoints.first();
        m_isAccessible = true;
    }

    connect(device, SIGNAL(mtabChanged(QString)), this, SLOT(onMtabChanged(QString)));
    QTimer::singleShot(0, this, SLOT(connectDBusSignals()));

    // qDebug() << "TEST:" <<
    // QMetaObject::invokeMethod(this, "accessibilityChanged", Qt::QueuedConnection,
    //         Q_ARG(bool, true), Q_ARG(QString, device->udi()));
}

FuseStorageAccess::~FuseStorageAccess()
{
}


void FuseStorageAccess::connectDBusSignals()
{
    m_fuseDevice->registerAction("setup", this,
                             SLOT(slotSetupRequested()),
                             SLOT(slotSetupDone(int,QString)));

    m_fuseDevice->registerAction("teardown", this,
                             SLOT(slotTeardownRequested()),
                             SLOT(slotTeardownDone(int,QString)));
}

const Solid::Backends::Fuse::FuseDevice *FuseStorageAccess::fuseDevice() const
{
    return m_fuseDevice;
}

bool FuseStorageAccess::isAccessible() const
{
    return m_isAccessible;
}

QString FuseStorageAccess::filePath() const
{
    return m_filePath;
}

bool FuseStorageAccess::isIgnored() const
{
    return false;
}

bool FuseStorageAccess::setup()
{
    if (filePath().isEmpty()) {
        return false;
    }
    m_fuseDevice->broadcastActionRequested("setup");
    m_process = FuseHandling::callSystemCommand("mount", filePath(),
                                                 this, SLOT(slotSetupFinished(int,QProcess::ExitStatus)));

    return m_process!=0;
}

void FuseStorageAccess::slotSetupRequested()
{
    emit setupRequested(m_fuseDevice->udi());
}

bool FuseStorageAccess::teardown()
{
    if (filePath().isEmpty()) {
        return false;
    }
    m_fuseDevice->broadcastActionRequested("teardown");
    m_process = FuseHandling::callSystemCommand("umount", filePath(),
                                                 this, SLOT(slotTeardownFinished(int,QProcess::ExitStatus)));

    return m_process!=0;
}

void FuseStorageAccess::slotTeardownRequested()
{
    emit teardownRequested(m_fuseDevice->udi());
}

void FuseStorageAccess::slotSetupFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode==0) {
        m_fuseDevice->broadcastActionDone("setup", Solid::NoError, QString());
    } else {
        m_fuseDevice->broadcastActionDone("setup", Solid::UnauthorizedOperation, m_process->readAllStandardError());
    }
    delete m_process;
}

void FuseStorageAccess::slotSetupDone(int error, const QString &errorString)
{
    emit setupDone(static_cast<Solid::ErrorType>(error), errorString, m_fuseDevice->udi());
}

void FuseStorageAccess::slotTeardownFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode==0) {
        m_fuseDevice->broadcastActionDone("teardown", Solid::NoError, QString());
    } else {
        m_fuseDevice->broadcastActionDone("teardown", Solid::UnauthorizedOperation, m_process->readAllStandardError());
    }
    delete m_process;
}

void FuseStorageAccess::slotTeardownDone(int error, const QString &errorString)
{
    emit teardownDone(static_cast<Solid::ErrorType>(error), errorString, m_fuseDevice->udi());
}

void FuseStorageAccess::onMtabChanged(const QString& device)
{
    QStringList currentMountPoints = FuseHandling::currentMountPoints(device);
    if (currentMountPoints.isEmpty()) {
        // device umounted
        m_filePath = FuseHandling::mountPoints(device).first();
        m_isAccessible = false;
        emit accessibilityChanged(false, QString(FUSE_UDI_PREFIX) + device);
    } else {
        // device added
        m_filePath = currentMountPoints.first();
        m_isAccessible = true;
        emit accessibilityChanged(true, QString(FUSE_UDI_PREFIX) + device);
    }
}
