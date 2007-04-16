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
#include "audiocapturedevicetest.h"
#include "../objectdescription.h"
#include "loadfakebackend.h"

using namespace Phonon;

void AudioCaptureDeviceTest::initTestCase()
{
    Phonon::loadFakeBackend();
}

void AudioCaptureDeviceTest::sensibleValues()
{
    AudioCaptureDevice a;
    QCOMPARE(a.isValid(), false);
    AudioCaptureDevice b(a);
    QCOMPARE(b.isValid(), false);
    b = a;
    QCOMPARE(b.isValid(), false);

    if (Factory::backendName() == QLatin1String("Fake")) {
        AudioCaptureDevice c = AudioCaptureDevice::fromIndex(20000);
        QCOMPARE(c.isValid(), true);
        QCOMPARE(c.index(), 20000);
        QCOMPARE(c.name(), QString("Soundcard"));
        QCOMPARE(c.description(), QString("first description"));
        b = AudioCaptureDevice::fromIndex(20001);
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.index(), 20001);
        QCOMPARE(b.name(), QString("DV"));
        QCOMPARE(b.description(), QString("second description"));
        QCOMPARE(a.isValid(), false);
        a = c;
        QCOMPARE(a, c);
        QCOMPARE(a.isValid(), true);
        QCOMPARE(a.index(), 20000);
        QCOMPARE(a.name(), QString("Soundcard"));
        QCOMPARE(a.description(), QString("first description"));
    } else {
        // TODO check for ALSA devices listed by libaudiodevicelist
    }
}

void AudioCaptureDeviceTest::cleanupTestCase()
{
}

QTEST_KDEMAIN(AudioCaptureDeviceTest, NoGUI)
#include "audiocapturedevicetest.moc"
// vim: sw=4 ts=4
