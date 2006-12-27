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

#include "simpletest.h"
#include <qtest_kde.h>
#include <QtDebug>
#include "../alsadeviceenumerator.h"

using namespace Phonon;

void SimpleTest::sanityChecks()
{
    QVERIFY(AlsaDeviceEnumerator::self() != 0);
}

void SimpleTest::listDevices()
{
    QList<AlsaDevice> deviceList = AlsaDeviceEnumerator::availableDevices();
    foreach (AlsaDevice dev, deviceList) {
        qDebug() << dev.cardName() << dev.mixerName() << dev.deviceIds() << dev.iconName();
        foreach (QString id, dev.deviceIds()) {
            QVERIFY(dev.deviceIds().count(id) == 1);
        }
    }
}

void SimpleTest::checkCopy()
{
    QList<AlsaDevice> deviceList = AlsaDeviceEnumerator::availableDevices();
    AlsaDevice dev = deviceList.first();
    QCOMPARE(dev, deviceList.first());
    AlsaDevice dev1;
    AlsaDevice dev2;
    QCOMPARE(dev1, dev2);
    QVERIFY(dev1 != dev);
    QVERIFY(dev2 != dev);
    dev1 = dev;
    QCOMPARE(dev1, dev);
    QCOMPARE(dev1, deviceList.first());
    QVERIFY(dev1 != dev2);
}

QTEST_KDEMAIN( SimpleTest, NoGUI )
#include "simpletest.moc"
// vim: sw=4 ts=4 et tw=100
