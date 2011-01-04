/*
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

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

#include "fstabstorageaccess.h"
#include "fstabwatcher.h"
#include <solid/backends/fstab/fstabdevice.h>
#include <solid/backends/fstab/fstabhandling.h>
#include <solid/backends/fstab/fstabservice.h>
#include <QtCore/QStringList>

#include <QTimer>

#define MTAB "/etc/mtab"

using namespace Solid::Backends::Fstab;

FstabStorageAccess::FstabStorageAccess(Solid::Backends::Fstab::FstabDevice *device) :
    QObject(device),
    m_fstabDevice(device)
{
    m_currentMountPoints = FstabHandling::currentMountPoints();

    connect(FstabWatcher::instance(), SIGNAL(mtabChanged()), this, SLOT(onMtabChanged()));
    QTimer::singleShot(0, this, SLOT(connectDBusSignals()));
}

FstabStorageAccess::~FstabStorageAccess()
{
}


void FstabStorageAccess::connectDBusSignals()
{
    m_fstabDevice->registerAction("setup", this,
                             SLOT(slotSetupRequested()),
                             SLOT(slotSetupDone(int, const QString&)));

    m_fstabDevice->registerAction("teardown", this,
                             SLOT(slotTeardownRequested()),
                             SLOT(slotTeardownDone(int, const QString&)));
}

const Solid::Backends::Fstab::FstabDevice *FstabStorageAccess::fstabDevice() const
{
    return m_fstabDevice;
}

bool FstabStorageAccess::isAccessible() const
{
    if (m_currentMountPoints.contains(m_fstabDevice->device())) {
        return true;
    } else {
        return false;
    }
}

QString FstabStorageAccess::filePath() const
{
    QStringList points = FstabHandling::mountPoints(m_fstabDevice->device());
    if (!points.isEmpty()) {
        return points.first();
    } else {
        return QString();
    }
}

bool FstabStorageAccess::isIgnored() const
{
    return false;
}

bool FstabStorageAccess::setup()
{
    if (filePath().isEmpty()) {
        return false;
    }
    m_fstabDevice->broadcastActionRequested("setup");
    m_process = FstabHandling::callSystemCommand("mount", filePath(),
                                                 this, SLOT(slotSetupFinished(int, QProcess::ExitStatus)));

    return m_process!=0;
}

void FstabStorageAccess::slotSetupRequested()
{
    emit setupRequested(m_fstabDevice->udi());
}

bool FstabStorageAccess::teardown()
{
    if (filePath().isEmpty()) {
        return false;
    }
    m_fstabDevice->broadcastActionRequested("teardown");
    m_process = FstabHandling::callSystemCommand("umount", filePath(),
                                                 this, SLOT(slotTeardownFinished(int, QProcess::ExitStatus)));

    return m_process!=0;
}

void FstabStorageAccess::slotTeardownRequested()
{
    emit teardownRequested(m_fstabDevice->udi());
}

void FstabStorageAccess::slotSetupFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode==0) {
        m_fstabDevice->broadcastActionDone("setup", Solid::NoError, QString());
    } else {
        m_fstabDevice->broadcastActionDone("setup", Solid::UnauthorizedOperation, m_process->readAllStandardError());
    }
    delete m_process;
}

void FstabStorageAccess::slotSetupDone(int error, const QString &errorString)
{
    emit setupDone(static_cast<Solid::ErrorType>(error), errorString, m_fstabDevice->udi());
}

void FstabStorageAccess::slotTeardownFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode==0) {
        m_fstabDevice->broadcastActionDone("teardown", Solid::NoError, QString());
    } else {
        m_fstabDevice->broadcastActionDone("teardown", Solid::UnauthorizedOperation, m_process->readAllStandardError());
    }
    delete m_process;
}

void FstabStorageAccess::slotTeardownDone(int error, const QString &errorString)
{
    emit teardownDone(static_cast<Solid::ErrorType>(error), errorString, m_fstabDevice->udi());
}

void FstabStorageAccess::onMtabChanged()
{
    QStringList currentMountPoints = FstabHandling::currentMountPoints();
    if (currentMountPoints.count() > m_currentMountPoints.count()) {
        // device mounted
        if (currentMountPoints.contains(m_fstabDevice->device()) && !m_currentMountPoints.contains(m_fstabDevice->device())) {
            emit accessibilityChanged(true, QString(FSTAB_UDI_PREFIX) + "/" + m_fstabDevice->device());
        }
    } else {
        // device umounted
        if (!currentMountPoints.contains(m_fstabDevice->device()) && m_currentMountPoints.contains(m_fstabDevice->device())) {
            emit accessibilityChanged(false, QString(FSTAB_UDI_PREFIX) + "/" + m_fstabDevice->device());
        }
    }

    m_currentMountPoints = currentMountPoints;
}
