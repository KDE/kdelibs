/*
    Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QThreadPool>
#include <QtCore/qtconcurrentrun.h>

#include <QtTest/QtTest>

#include <solid/device.h>
#include <solid/predicate.h>
#include <solid/storagevolume.h>
#include <solid/storagedrive.h>
#include <solid/genericinterface.h>


class SolidMtTest : public QObject
{
    Q_OBJECT
private slots:
    void testWorkerThread();
    void testThreadedPredicate();
};

class WorkerThread : public QThread
{
    Q_OBJECT
protected:
    virtual void run()
    {
        Solid::Device dev("/org/freedesktop/Hal/devices/computer");

        QList<Solid::Device> driveList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
        foreach (const Solid::Device &solidDevice, driveList) {
            const Solid::StorageDrive* solidDrive = solidDevice.as<Solid::StorageDrive>();
        }
    }
};

static void doPredicates()
{
    Solid::Predicate p5 = Solid::Predicate::fromString("[[Processor.maxSpeed == 3201 AND Processor.canChangeFrequency == false] OR StorageVolume.mountPoint == '/media/blup']");

    Solid::Predicate p6 = Solid::Predicate::fromString("StorageVolume.usage == 'Other'");
    Solid::Predicate p7 = Solid::Predicate::fromString(QString("StorageVolume.usage == %1").arg((int)Solid::StorageVolume::Other));

    Solid::Predicate p8 = Solid::Predicate::fromString("AudioInterface.deviceType == 'AudioInput|AudioOutput'");
    Solid::Predicate p9 = Solid::Predicate::fromString("AudioInterface.deviceType == 'AudioInput'");
    Solid::Predicate p10 = Solid::Predicate::fromString("AudioInterface.deviceType  & 'AudioInput'");
    Solid::Predicate p11 = Solid::Predicate::fromString("AudioInterface.deviceType  & 'foobar'");
}

QTEST_MAIN(SolidMtTest)

void SolidMtTest::testWorkerThread()
{
    Solid::Device dev("/org/freedesktop/Hal/devices/acpi_ADP1");

    WorkerThread *wt = new WorkerThread;
    wt->start();
    wt->wait();

    const QList<Solid::Device> driveList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    foreach (const Solid::Device &solidDevice, driveList) {
        const Solid::GenericInterface* solidDrive = solidDevice.as<Solid::GenericInterface>();
    }

    delete wt;
}

void SolidMtTest::testThreadedPredicate()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QList<QFuture<void> > futures;
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    futures << QtConcurrent::run(&doPredicates);
    Q_FOREACH(QFuture<void> f, futures)
        f.waitForFinished();
    QThreadPool::globalInstance()->setMaxThreadCount(1); // delete those threads
}

#include "solidmttest.moc"

