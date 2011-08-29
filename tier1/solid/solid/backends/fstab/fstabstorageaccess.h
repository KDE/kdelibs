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

#ifndef SOLID_BACKENDS_FSTAB_STORAGEACCESS_H
#define SOLID_BACKENDS_FSTAB_STORAGEACCESS_H

#include <solid/ifaces/storageaccess.h>

#include <QtCore/QObject>
#include <QtCore/QProcess>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
    class FstabDevice;
    class FstabStorageAccess : public QObject, public Solid::Ifaces::StorageAccess
    {
        Q_OBJECT
        Q_INTERFACES(Solid::Ifaces::StorageAccess)

        public:
            explicit FstabStorageAccess(Solid::Backends::Fstab::FstabDevice *device);

            virtual ~FstabStorageAccess();

            virtual bool isAccessible() const;

            virtual QString filePath() const;

            virtual bool isIgnored() const;

            virtual bool setup();

            virtual bool teardown();

        public:
            const Solid::Backends::Fstab::FstabDevice* fstabDevice() const;

        Q_SIGNALS:
            void accessibilityChanged(bool accessible, const QString &udi);

            void setupDone(Solid::ErrorType error, QVariant data, const QString &udi);

            void teardownDone(Solid::ErrorType error, QVariant data, const QString &udi);

            void setupRequested(const QString &udi);

            void teardownRequested(const QString &udi);

        private Q_SLOTS:
            void slotSetupFinished(int exitCode, QProcess::ExitStatus exitStatus);
            void slotTeardownFinished(int exitCode, QProcess::ExitStatus exitStatus);
            void onMtabChanged();
            void connectDBusSignals();

            void slotSetupRequested();
            void slotSetupDone(int error, const QString &errorString);
            void slotTeardownRequested();
            void slotTeardownDone(int error, const QString &errorString);

        private:
            Solid::Backends::Fstab::FstabDevice *m_fstabDevice;
            QProcess *m_process;
            QStringList m_currentMountPoints;
    };

}
}
}

#endif // SOLID_BACKENDS_FSTAB_DEVICE_INTERFACE_H
