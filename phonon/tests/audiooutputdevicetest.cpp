/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#include <qtest_kde.h>
#include "audiooutputdevicetest.h"
#include "../objectdescription.h"
#include "loadfakebackend.h"

using namespace Phonon;

void AudioOutputDeviceTest::initTestCase()
{
    QCoreApplication::setApplicationName("audiooutputdevicetest");
    Phonon::loadFakeBackend();
}

void AudioOutputDeviceTest::sensibleValues()
{
    AudioOutputDevice a;
    QCOMPARE(a.isValid(), false);
    AudioOutputDevice b(a);
    QCOMPARE(b.isValid(), false);
    b = a;
    QCOMPARE(b.isValid(), false);

    if (Factory::backendName() == QLatin1String("Fake")) {
        AudioOutputDevice c = AudioOutputDevice::fromIndex(10000);
        QCOMPARE(c.isValid(), true);
        QCOMPARE(c.index(), 10000);
        QCOMPARE(c.name(), QString("internal Soundcard"));
        QCOMPARE(c.description(), QString());
        b = AudioOutputDevice::fromIndex(10001);
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.index(), 10001);
        QCOMPARE(b.name(), QString("USB Soundcard"));
        QCOMPARE(b.description(), QString());
        QCOMPARE(a.isValid(), false);
        a = c;
        QCOMPARE(a, c);
        QCOMPARE(a.isValid(), true);
        QCOMPARE(a.index(), 10000);
        QCOMPARE(a.name(), QString("internal Soundcard"));
        QCOMPARE(a.description(), QString());
    } else {
        // TODO check for ALSA devices listed by libaudiodevicelist
    }
}

void AudioOutputDeviceTest::cleanupTestCase()
{
}

QTEST_MAIN(AudioOutputDeviceTest)
#include "audiooutputdevicetest.moc"
// vim: sw=4 ts=4
