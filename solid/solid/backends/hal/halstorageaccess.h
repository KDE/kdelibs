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

#ifndef SOLID_BACKENDS_HAL_STORAGEACCESS_H
#define SOLID_BACKENDS_HAL_STORAGEACCESS_H

#include <solid/ifaces/storageaccess.h>
#include "haldeviceinterface.h"

#include <QtCore/QProcess>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusVariant>

namespace Solid
{
namespace Backends
{
namespace Hal
{
class StorageAccess : public DeviceInterface, virtual public Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
    StorageAccess(HalDevice *device);
    virtual ~StorageAccess();

    virtual bool isAccessible() const;
    virtual QString filePath() const;
    virtual bool setup();
    virtual bool teardown();

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi);
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void setupRequested(const QString &udi);
    void teardownRequested(const QString &udi);

private Q_SLOTS:
    void connectDBusSignals();
    void slotPropertyChanged(const QMap<QString,int> &changes);
    void slotDBusReply(const QDBusMessage &reply);
    void slotDBusError(const QDBusError &error);
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotSetupRequested();
    void slotTeardownRequested();
    void slotSetupDone(int error, QDBusVariant errorData, const QString &udi);
    void slotTeardownDone(int error, QDBusVariant errorData, const QString &udi);
    void slotEjectDone(int error, QDBusVariant errorData, const QString &udi);

public Q_SLOTS:
    Q_SCRIPTABLE Q_NOREPLY void passphraseReply(const QString &passphrase);

private:
    bool callHalVolumeMount();
    bool callHalVolumeUnmount();
    bool callHalVolumeEject();

    bool callSystemMount();
    bool callSystemUnmount();

    bool requestPassphrase();
    void callCryptoSetup(const QString &passphrase);
    bool callCryptoTeardown();

private:
    bool m_setupInProgress;
    bool m_teardownInProgress;
    bool m_ejectInProgress;
    bool m_passphraseRequested;
    QString m_lastReturnObject;
    QProcess *m_process;
};
}
}
}

#endif // SOLID_BACKENDS_HAL_STORAGEACCESS_H
