/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include <QtCore/QDebug>
#include <QtCore/QObject>

#include <qtest_kde.h>

#include "../libkaudiodevicelist/audiodeviceenumerator.h"

class SimpleTest : public QObject
{
    Q_OBJECT

    private Q_SLOTS:
        void sanityChecks();
        void listDevices();
        void checkCopy();
};

using namespace Phonon;

void SimpleTest::sanityChecks()
{
    QVERIFY(AudioDeviceEnumerator::self() != 0);
}

void SimpleTest::listDevices()
{
    QList<AudioDevice> deviceList = AudioDeviceEnumerator::availablePlaybackDevices();
    foreach (AudioDevice dev, deviceList) {
        qDebug() << dev.cardName() << dev.deviceIds() << dev.iconName() << dev.isAvailable();
        foreach (QString id, dev.deviceIds()) {
            QVERIFY(dev.deviceIds().count(id) == 1);
        }
    }
}

void SimpleTest::checkCopy()
{
    QList<AudioDevice> deviceList = AudioDeviceEnumerator::availablePlaybackDevices();
    AudioDevice dev = deviceList.first();
    QCOMPARE(dev, deviceList.first());
    AudioDevice dev1;
    AudioDevice dev2;
    QCOMPARE(dev1, dev2);
    QVERIFY(dev1 != dev);
    QVERIFY(dev2 != dev);
    dev1 = dev;
    QCOMPARE(dev1, dev);
    QCOMPARE(dev1, deviceList.first());
    QVERIFY(dev1 != dev2);
}

QTEST_KDEMAIN_CORE(SimpleTest)
#include "audiodevicelisttest.moc"
// vim: sw=4 ts=4 et tw=100
