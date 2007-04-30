/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "audiooutputtest.h"
#include "../audiooutput.h"
#include <kglobal.h>
#include <QLatin1String>
#include "../phononnamespace.h"
#include <qtest_kde.h>
#include "loadfakebackend.h"

using namespace Phonon;

void AudioOutputTest::initTestCase()
{
    Phonon::loadFakeBackend();
}

void AudioOutputTest::checkName()
{
    AudioOutput ao(Phonon::NotificationCategory, this);
    QCOMPARE(ao.name(), QLatin1String("qttest"));
    QString n("lsdf");
    ao.setName(n);
    QCOMPARE(ao.name(), n);
}

void AudioOutputTest::checkVolume()
{
    AudioOutput ao(Phonon::NotificationCategory, this);
    QSignalSpy volumeSignalSpy(&ao, SIGNAL(volumeChanged(qreal)));
    qreal v = 1.0;
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(volumeSignalSpy.size(), 0);
    for (v = 0.0; v <= 10.0; v += 0.01)
    {
        ao.setVolume(v);
        QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
        QCOMPARE(volumeSignalSpy.size(), 1);
        QCOMPARE(qvariant_cast<float>(volumeSignalSpy.takeFirst().at(0)), static_cast<float>(v));
    }
}

void AudioOutputTest::checkMute()
{
    AudioOutput ao(Phonon::CommunicationCategory, this);
    QSignalSpy volumeSignalSpy(&ao, SIGNAL(volumeChanged(qreal)));
    QSignalSpy muteSignalSpy(&ao, SIGNAL(mutedChanged(bool)));
    qreal v = 1.0;
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), false);

    ao.setMuted(true);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 1);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), true);
    QCOMPARE(muteSignalSpy.takeFirst().at(0).toBool(), true);

    ao.setMuted(true);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 0);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), true);

    ao.setMuted(false);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 1);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), false);
    QCOMPARE(muteSignalSpy.takeFirst().at(0).toBool(), false);

    ao.setMuted(false);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 0);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), false);

    ao.setMuted(true);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 1);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), true);
    QCOMPARE(muteSignalSpy.takeFirst().at(0).toBool(), true);

    v = 0.25f;
    ao.setVolume(v);
    QCOMPARE(volumeSignalSpy.size(), 1);
    QCOMPARE(muteSignalSpy.size(), 0);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), true);
    QCOMPARE(qvariant_cast<float>(volumeSignalSpy.takeFirst().at(0)), static_cast<float>(v));

    ao.setMuted(true);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 0);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), true);

    ao.setMuted(false);
    QCOMPARE(volumeSignalSpy.size(), 0);
    QCOMPARE(muteSignalSpy.size(), 1);
    QCOMPARE(static_cast<float>(ao.volume()), static_cast<float>(v));
    QCOMPARE(ao.isMuted(), false);
    QCOMPARE(muteSignalSpy.takeFirst().at(0).toBool(), false);
}

void AudioOutputTest::checkCategory()
{
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        Phonon::Category cat = static_cast<Phonon::Category>(i);
        AudioOutput ao(cat, this);
        QCOMPARE(ao.category(), cat);
    }
}

void AudioOutputTest::cleanupTestCase()
{
}

QTEST_KDEMAIN_CORE(AudioOutputTest)
#include "audiooutputtest.moc"
// vim: sw=4 ts=4
