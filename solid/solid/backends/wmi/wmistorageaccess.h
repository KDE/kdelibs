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

#ifndef SOLID_BACKENDS_WMI_STORAGEACCESS_H
#define SOLID_BACKENDS_WMI_STORAGEACCESS_H

#include <solid/ifaces/storageaccess.h>
#include "wmideviceinterface.h"

#include <QtCore/QProcess>

namespace Solid
{
namespace Backends
{
namespace Wmi
{
class StorageAccess : public DeviceInterface, virtual public Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
    StorageAccess(WmiDevice *device);
    virtual ~StorageAccess();

    virtual bool isAccessible() const;
    virtual QString filePath() const;
    virtual bool setup();
    virtual bool teardown();

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi);
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void setupRequested(const QString &udi);
    void teardownRequested(const QString &udi);

private Q_SLOTS:
    void slotPropertyChanged(const QMap<QString,int> &changes);
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool callWmiVolumeMount();
    bool callWmiVolumeUnmount();

    bool callSystemMount();
    bool callSystemUnmount();

private:
    bool m_setupInProgress;
    bool m_teardownInProgress;
    bool m_passphraseRequested;
    QString m_lastReturnObject;
    QProcess *m_process;
};
}
}
}

#endif // SOLID_BACKENDS_WMI_STORAGEACCESS_H
